#include "pch.h"
#include "ThreadPool.h"
#ifdef G_SERVICE
#include <ObserverServer.h>
#else
#include <LogicServer.h>
#endif
#include <Psapi.h>

BusinessThreadPool::BusinessThreadPool(size_t thread_count)
    : global_queue_(GLOBAL_QUEUE_MAX), // 初始化全局队列
    configured_thread_count_(thread_count), // 保存配置的线程数
    last_log_time_(std::chrono::steady_clock::now()) // 初始化上次日志时间
{
    // 预分配内存池对象（假设 Task 可默认构造）
    const size_t INITIAL_POOL_SIZE = std::min(MAX_LOCAL_QUEUE_SIZE * 4, MEMORY_POOL_INIT_SIZE);
    slog->info("Initializing task pool with {} wrappers.", INITIAL_POOL_SIZE);
    for (size_t i = 0; i < INITIAL_POOL_SIZE; ++i) {
        //TaskWrapper* wrapper = new TaskWrapper(Task{});  // 需 Task 支持默认构造
        auto* wrapper = new TaskWrapper(
            Task(0, nullptr, RawProtocolImpl{}, msgpack::v1::object_handle{})
        );
        if (wrapper) {
            task_pool_.enqueue(wrapper);
        }
        else {
            slog->error("Failed to allocate TaskWrapper during initialization.");
        }
    }
    workers_.reserve(thread_count);
    slog->info("Starting {} worker threads.", thread_count);
    for (size_t i = 0; i < thread_count; ++i) {
        workers_.emplace_back([this, i] {
            if constexpr (ENABLE_CPU_AFFINITY) {
                DWORD_PTR mask = (1ull << (i % 64)) | IDEAL_CORE_MASK;
                SetThreadAffinityMask(GetCurrentThread(), mask);
            }

            auto ctx = std::make_shared<WorkerContext>();
            {
                std::unique_lock lock(context_mutex_);
                worker_contexts_.emplace_back(ctx);
            }

            slog->debug("Worker thread {} started.", i);
            worker_loop(ctx, i);
            slog->debug("Worker thread {} finished.", i);

            // 清理逻辑
            {
                std::unique_lock lock(context_mutex_);
                auto it = std::find_if(worker_contexts_.begin(), worker_contexts_.end(),
                                       [&](const std::weak_ptr<WorkerContext>& weak_ctx) {
                    auto shared_ctx = weak_ctx.lock();
                    return shared_ctx && (shared_ctx == ctx);  // 比较shared_ptr对象
                });
            }

            ctx->active.store(false, std::memory_order_seq_cst);
            drain_queue(ctx);
        });

    }
    // 在构造函数中启动监控线程
    start_monitor(INITIAL_POOL_SIZE);
    slog->info("BusinessThreadPool initialized successfully.");
}

BusinessThreadPool::~BusinessThreadPool() {
    stop_.store(true, std::memory_order_release);
    // 先停止监控线程
    monitor_stop_.store(true, std::memory_order_release);
    if (monitor_thread_.joinable()) {
        monitor_thread_.join();
    }

    // 等待所有任务完成
    while (pending_tasks_.load(std::memory_order_acquire) > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // 停止工作线程
    for (auto& w : workers_) {
        if (w.joinable()) w.join();
    }
    // 清理内存池
    TaskWrapper* task;
    while (task_pool_.try_dequeue(task)) {
        delete task;
    }
    // 清空队列防止残留任务
    while (global_queue_.try_dequeue(task)) {
        delete task;
    }

    log_thread_pool_status();
}

bool BusinessThreadPool::enqueue(Task task) {
    if (stop_.load(std::memory_order_acquire)) {
        slog->warn("Enqueue rejected: Thread pool is stopping");
        rejected_tasks_count_.fetch_add(1, std::memory_order_relaxed); // 拒绝计数
        return false;
    }

    // 获取或创建任务包装器
    TaskWrapper* wrapper = nullptr;
    if (!task_pool_.try_dequeue(wrapper)) {
        constexpr int BATCH_SIZE = 64;
        for (int i = 0; i < BATCH_SIZE; ++i) {
            auto* prealloc = new TaskWrapper(Task{});
            if (!task_pool_.enqueue(prealloc)) {
                delete prealloc;
            }
        }
        if (!task_pool_.try_dequeue(wrapper)) {
            wrapper = new (std::nothrow) TaskWrapper(Task{}); 
            if (!wrapper) {
                slog->error("Failed to allocate TaskWrapper for enqueue.");
                rejected_tasks_count_.fetch_add(1, std::memory_order_relaxed); // 拒绝计数
                return false;
            }
        }
    }

    // 安全移动任务
    try {
        wrapper->task = std::move(task);
    }
    catch (const std::exception& e) {
        slog->error("Task move failed: {}", e.what());
        task_pool_.enqueue(wrapper);
        rejected_tasks_count_.fetch_add(1, std::memory_order_relaxed); // 拒绝计数
        return false;
    }
    catch (...) {
        slog->error("Task move failed during enqueue due to unknown exception.");
        task_pool_.enqueue(wrapper); // 归还包装器
        rejected_tasks_count_.fetch_add(1, std::memory_order_relaxed); // 拒绝计数
        return false;
    }

    // 选择目标工作线程
    std::shared_ptr<WorkerContext> target_ctx;
    {
        std::shared_lock<std::shared_mutex> lock(context_mutex_);
        if (!worker_contexts_.empty()) {
            static thread_local std::mt19937_64 rng(std::random_device{}());
            std::uniform_int_distribution<size_t> dist(0, worker_contexts_.size() - 1);

            for (int attempt = 0; attempt < 3; ++attempt) {
                size_t index = dist(rng);
                if (index >= worker_contexts_.size()) continue;

                if (auto ctx = worker_contexts_[index].lock()) {  // 正确转换weak_ptr
                    if (ctx->active.load(std::memory_order_acquire)) {
                        target_ctx = ctx;
                        break;
                    }
                }
            }
        }
    }

    // 尝试本地队列入队
    if (target_ctx && target_ctx->local_queue.try_enqueue(wrapper)) {
        pending_tasks_.fetch_add(1, std::memory_order_relaxed);
        return true;
    }
    slog->debug("降级到全局队列");
    // 降级到全局队列
    if (global_queue_.enqueue(wrapper)) {
        pending_tasks_.fetch_add(1, std::memory_order_relaxed);
        return true;
    }

    // --- 全局队列也满了，入队失败 ---
    slog->error("Enqueue failed: Global queue is full (size approx {}).", global_queue_.size_approx());
    rejected_tasks_count_.fetch_add(1, std::memory_order_relaxed); // 拒绝计数

    // 最终回收
    if (!task_pool_.enqueue(wrapper)) {
        slog->warn("Memory pool full, direct delete");
        delete wrapper;
    }
    return false;
}


// 修改监控线程实现
void BusinessThreadPool::start_monitor(const size_t INITIAL_POOL_SIZE) {
    monitor_thread_ = std::thread([this, INITIAL_POOL_SIZE] {
        size_t prev_pending = 0;
        while (!monitor_stop_.load(std::memory_order_acquire)) {
            // 动态调整休眠时间(1-10秒)
            const auto sleep_duration = std::chrono::seconds(prev_pending > 1000 ? 5 : 30);
            std::this_thread::sleep_for(sleep_duration);
            log_thread_pool_status();
            // 弹性内存补充策略
            const size_t current_pool = task_pool_.size_approx();
            const size_t current_pending = pending_tasks_.load(std::memory_order_relaxed);

            // 计算目标池大小（基于当前负载）
            const size_t target_pool = std::clamp(
                current_pending /** 2*/,
                MEMORY_POOL_LOW_WATERMARK,
                MEMORY_POOL_INIT_SIZE
            );

            // 渐进式补充
            if (current_pool < target_pool) {
                const size_t need = target_pool - current_pool;
                for (size_t i = 0; i < need; ++i) {
                    auto wrapper = new (std::nothrow) TaskWrapper(Task{});
                    if (wrapper && !task_pool_.enqueue(wrapper)) {
                        delete wrapper;
                        break;
                    }
                }
            }

            // 主动释放策略（调整释放阈值）
            if (current_pool > target_pool * 1.2) {
                TaskWrapper* buffer[128];
                while (auto count = task_pool_.try_dequeue_bulk(buffer, 128)) {
                    for (size_t i = 0; i < count; ++i) {
                        delete buffer[i];
                    }
                }
            }
            prev_pending = current_pending;
        
        }
    });
}
void BusinessThreadPool::worker_loop(std::shared_ptr<WorkerContext>& ctx, size_t worker_id) {
    SetThreadPriority(GetCurrentThread(), THREAD_MODE_BACKGROUND_BEGIN);

    constexpr size_t MAX_BATCH = 128;
    std::vector<TaskWrapper*> batch;
    batch.reserve(MAX_BATCH);

    while (!stop_.load(std::memory_order_relaxed)) {
        // 每次循环前检查活性
        if (!ctx->active.load(std::memory_order_acquire)) {
            break;
        }
        size_t processed = 0;

        // 优先处理本地队列
        TaskWrapper* task;
        while (processed < MAX_BATCH && ctx->local_queue.try_dequeue(task)) {
            execute_task(task);
            processed++;
        }

        // 处理全局队列（保持原有逻辑）
        processed += process_global(batch);

        // 工作窃取优化（使用已有方法）
        if (processed == 0) {
            processed += try_steal(batch, worker_id);
        }

        // 修改为基于全局队列状态的动态休眠
        if (processed == 0) {
            const size_t global_pending = pending_tasks_.load(std::memory_order_relaxed);
            if (global_pending == 0) {
                // 使用 Windows 高效休眠 API
                ::SleepEx(10, TRUE);  // 1ms 可中断休眠
            }
            else {
                // 少量任务时使用 yield
                std::this_thread::yield();
            }
        }
    }
    // 退出时自动从上下文中移除
    {
        std::unique_lock lock(context_mutex_);
        auto it = std::find_if(worker_contexts_.begin(), worker_contexts_.end(),
                               [&](auto& weak_ctx) { return weak_ctx.lock() == ctx; });
        if (it != worker_contexts_.end()) {
            worker_contexts_.erase(it);
        }
    }
    ctx->active.store(false, std::memory_order_seq_cst); // 标记为非活动
    slog->debug("Worker {} loop finished.", worker_id);
}

size_t BusinessThreadPool::process_local(WorkerContext* ctx, std::vector<TaskWrapper*>& batch) {
    size_t processed = 0;
    TaskWrapper* task;
    while (processed < WORKER_BATCH_SIZE && ctx->local_queue.try_dequeue(task)) {
        execute_task(task);
        ++processed;
    }
    return processed;
}

size_t BusinessThreadPool::process_global(std::vector<TaskWrapper*>& batch) {
    // 清空 batch 以便重用
    batch.clear();
    size_t count = global_queue_.try_dequeue_bulk(
        std::back_inserter(batch),
        WORKER_BATCH_SIZE
    );
    for (auto task : batch) {
        execute_task(task);
    }
    return count;
}

size_t BusinessThreadPool::try_steal(std::vector<TaskWrapper*>& batch, size_t thief_id) {
    batch.clear(); // 清空 batch 准备接收窃取的任务
    std::shared_lock<std::shared_mutex> lock(context_mutex_);
    size_t stolen = 0;
    const size_t total_workers = worker_contexts_.size();

    // 优化参数配置
    constexpr size_t MAX_STEAL_PER_WORKER = 16;    // 单线程最大窃取量
    // 根据负载动态调整参数
    size_t dynamic_steal_limit = std::min(
        MAX_STEAL_PER_WORKER,
        static_cast<size_t>(std::max<int64_t>(pending_tasks_.load(), 0)) /
        (worker_contexts_.size() + 1)
    );
    // 随机选择起始窃取位置
    static thread_local std::mt19937_64 rng(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, total_workers - 1);
    size_t start_index = dist(rng);

    for (size_t i = 0; i < total_workers && stolen < dynamic_steal_limit; ++i) {
        // 环状遍历避免偏向
        size_t target_index = (start_index + i) % total_workers;
        if (target_index == thief_id) continue;

        // 安全获取目标上下文
        auto weak_ctx = worker_contexts_[target_index];
        auto ctx = weak_ctx.lock();
        if (!ctx || !ctx->active.load(std::memory_order_acquire)) {
            continue;
        }

        // 单任务窃取模式
        TaskWrapper* stolen_task = nullptr;
        size_t local_stolen = 0;
        while (local_stolen < dynamic_steal_limit) {
            // 尝试窃取单个任务
            if (!ctx->local_queue.try_dequeue(stolen_task)) {
                break;
            }

            // 安全处理窃取的任务
            if (stolen_task) {
                batch.push_back(stolen_task);
                ++local_stolen;
                ++stolen;
            }

            // 达到单线程窃取上限
            if (local_stolen >= dynamic_steal_limit) {
                break;
            }
        }

        // 更新目标线程的窃取统计
        if (local_stolen > 0) {
            ctx->total_steals.fetch_add(local_stolen, std::memory_order_relaxed);
        }

        // 提前退出条件
        if (stolen >= WORKER_BATCH_SIZE) {
            break;
        }
    }

    // 动态计算休眠时间（基于全局队列深度）
    const size_t global_pending = pending_tasks_.load();
    const auto sleep_time = global_pending > 1000 ?
        2 :   // 高负载时短休眠
        10;   // 低负载时长休眠

    if (stolen == 0) {
        Sleep(sleep_time);  // 动态休眠
    }
    // 批量执行窃取到的任务
    if (!batch.empty()) {
        slog->debug("已窃取{}个任务", batch.size());
        for (auto task : batch) {
            execute_task(task);
        }
    }
    return stolen;
}

//size_t BusinessThreadPool::try_steal(std::vector<TaskWrapper*>& batch, size_t thief_id) {
//    std::shared_lock<std::shared_mutex> lock(context_mutex_);
//    size_t stolen = 0;
//    const size_t total_workers = worker_contexts_.size();
//
//    // 新增：根据全局队列深度动态限制窃取频率
//    const size_t global_pending = pending_tasks_.load(std::memory_order_relaxed);
//    if (global_pending > 0 && global_pending < WORKER_BATCH_SIZE * 2) {
//        // 当全局队列有足够任务时，减少窃取尝试
//        return 0;
//    }
//
//    // 优化后的批量窃取参数
//    constexpr size_t BULK_STEAL_COUNT = 32;  // 单次批量窃取数量
//    constexpr size_t MAX_STEAL_PER_WORKER = BULK_STEAL_COUNT * 2;
//
//    // 随机选择起始位置（避免线程饥饿）
//    static thread_local std::mt19937_64 rng(std::random_device{}());
//    std::uniform_int_distribution<size_t> dist(0, total_workers - 1);
//    size_t start_index = dist(rng);
//
//    for (size_t i = 0; i < total_workers && stolen < MAX_STEAL_PER_WORKER; ++i) {
//        size_t target_index = (start_index + i) % total_workers;
//        if (target_index == thief_id) continue;
//
//        auto weak_ctx = worker_contexts_[target_index];
//        auto ctx = weak_ctx.lock();
//        if (!ctx || !ctx->active.load(std::memory_order_acquire)) {
//            continue;
//        }
//
//        // 批量窃取模式（减少锁竞争）
//        TaskWrapper* stolen_tasks[BULK_STEAL_COUNT];
//        size_t steal_count = 0;
//
//        // 一次性窃取BULK_STEAL_COUNT个任务
//        std::vector<TaskWrapper*> batch;
//        steal_count = process_local(ctx.get(), batch);
//        //steal_count = ctx->local_queue.try_dequeue_bulk(stolen_tasks, BULK_STEAL_COUNT);
//        if (steal_count == 0) continue;
//
//        // 将窃取的任务加入批量列表
//        for (size_t j = 0; j < steal_count; ++j) {
//            if (stolen_tasks[j]) {
//                batch.push_back(stolen_tasks[j]);
//                stolen++;
//            }
//        }
//
//        // 更新统计信息
//        ctx->total_steals.fetch_add(steal_count, std::memory_order_relaxed);
//
//        // 检查是否达到窃取上限
//        if (stolen >= MAX_STEAL_PER_WORKER) break;
//    }
//
//    // 执行窃取到的任务
//    if (!batch.empty()) {
//        for (auto task : batch) execute_task(task);
//        batch.clear();
//    }
//
//    return stolen;
//}

void BusinessThreadPool::execute_task(TaskWrapper* task) {
    if (!task) return;

    // 异常安全的移动捕获
    try {
        Task local_task;
        local_task = std::move(task->task);  // 正确的移动赋值
        // 执行任务
    #ifdef G_SERVICE    
        CObserverServer::instance().process_task(std::move(local_task));
    #else
        CLogicServer::instance().process_task(std::move(local_task));
    #endif
        // 3. 任务成功完成，增加完成计数
        completed_tasks_count_.fetch_add(1, std::memory_order_relaxed);
    }
    catch (...) {
        slog->error("Task execution failed");
    }
    // 重置任务包装器
    task->task = Task{};  // 显式重置为默认状态

    // 回收内存
    if (!task_pool_.enqueue(task)) {
        slog->warn("Failed to recycle task wrapper");
        delete task;
    }
    if(pending_tasks_.load() > 0)
        pending_tasks_.fetch_sub(1, std::memory_order_relaxed);
}

void BusinessThreadPool::handle_idle(size_t processed, int& empty_cycles) {
    if (processed == 0) {
        if (++empty_cycles > MAX_EMPTY_ITERATIONS) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(1)
            );
        }
    }
    else {
        empty_cycles = 0;
    }
}

void BusinessThreadPool::drain_queue(std::shared_ptr<WorkerContext>& ctx) {
    TaskWrapper* task;
    size_t drained_count = 0;
    // 持续从本地队列取出并执行，直到队列为空
    while (ctx->local_queue.try_dequeue(task)) {
        execute_task(task);
        drained_count++;
    }
    if (drained_count > 0) {
        slog->debug("Drained {} tasks from local queue during shutdown.", drained_count);
    }
}

void BusinessThreadPool::check_memory_usage() {
    constexpr size_t MAX_POOL_SIZE = 10000;
    size_t current = task_pool_.size_approx();

    // 安全释放多余对象
    if (current > MAX_POOL_SIZE) {
        TaskWrapper* wrappers[256];
        size_t count = task_pool_.try_dequeue_bulk(wrappers, 256);
        for (size_t i = 0; i < count; ++i) {
            delete wrappers[i];  // 正确释放内存
        }
    }
}

void BusinessThreadPool::log_pool_stats() {
    size_t global = task_pool_.size_approx();
    size_t in_flight = pending_tasks_.load();
    slog->info("Memory: pool={} tasks={} workers={}",
               global, in_flight, worker_contexts_.size());
}
// BusinessThreadPool.cpp 新增方法
//void BusinessThreadPool::log_thread_pool_status() {
//    const size_t pending = pending_tasks_.load(std::memory_order_relaxed);
//    const size_t workers = worker_contexts_.size();
//    const size_t global_queue_size = global_queue_.size_approx();
//    size_t total_local_queue_size = 0;
//
//    // 遍历所有worker的本地队列
//    {
//        std::shared_lock lock(context_mutex_);
//        for (const auto& weak_ctx : worker_contexts_) {
//            if (auto ctx = weak_ctx.lock()) {
//                total_local_queue_size += ctx->local_queue.size_approx();
//            }
//        }
//    }
//
//    slog->info("[ThreadPool Status] "
//               "Workers: {} | "
//               "Pending Tasks: {} | "
//               "Global Queue: {} | "
//               "Total Local Queue: {} | "
//               "Memory Pool: {}",
//               workers,
//               pending,
//               global_queue_size,
//               total_local_queue_size,
//               task_pool_.size_approx());
//
//    // 触发警告的条件（示例：当队列深度超过2500时）
//    constexpr size_t WARNING_THRESHOLD = 2500;
//    if (pending > WARNING_THRESHOLD) {
//        slog->warn("[ThreadPool Warning] High pending tasks: {}", pending);
//    }
//
//    PROCESS_MEMORY_COUNTERS pmc;
//    GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
//    slog->info("[Resource] 内存使用: {:.2f} MB", pmc.WorkingSetSize / 1024.0 / 1024.0);
//
//    // 监控句柄数（Windows API）
//    DWORD handleCount;
//    GetProcessHandleCount(GetCurrentProcess(), &handleCount);
//    slog->info("[Resource] 当前句柄数: {}", handleCount);
//}
// --- 记录线程池状态 (增强版) ---
void BusinessThreadPool::log_thread_pool_status() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed_duration = now - last_log_time_;
    double elapsed_seconds = std::chrono::duration<double>(elapsed_duration).count();

    size_t current_completed = completed_tasks_count_.load(std::memory_order_relaxed);
    size_t completed_in_interval = current_completed - last_completed_tasks_count_;

    double tasks_per_second = 0.0;
    if (elapsed_seconds > 0.01) { // 避免除零或间隔过短导致数值爆炸
        tasks_per_second = static_cast<double>(completed_in_interval) / elapsed_seconds;
    }

    // 更新记录，为下次计算准备
    last_log_time_ = now;
    last_completed_tasks_count_ = current_completed;

    // 获取其他现有指标
    const size_t pending = pending_tasks_.load(std::memory_order_relaxed);
    const size_t workers_active = worker_contexts_.size(); // 活跃worker数
    const size_t global_queue_size = global_queue_.size_approx();
    const size_t pool_size = task_pool_.size_approx();
    const size_t rejected_total = rejected_tasks_count_.load(std::memory_order_relaxed);

    size_t total_local_queue_size = 0;
    {
        std::shared_lock lock(context_mutex_);
        for (const auto& weak_ctx : worker_contexts_) {
            if (auto ctx = weak_ctx.lock()) {
                total_local_queue_size += ctx->local_queue.size_approx();
            }
        }
    }

    // --- 格式化日志输出 ---
    slog->info("[ThreadPool Status] "
               "ConfiguredThreads: {} | " // 新增：配置线程数
               "ActiveWorkers: {} | "     // 活跃工作线程数
               "PendingTasks: {} | "      // 待处理任务 (队列中+执行中)
               "GlobalQueue: {} | "       // 全局队列大小
               "LocalQueuesTotal: {} | "  // 所有本地队列总大小
               "CompletedTotal: {} | "    // 新增：累计完成任务
               "RejectedTotal: {} | "     // 新增：累计拒绝任务
               "Throughput: {:.2f} Tasks/Sec | " // 新增：吞吐量
               "MemoryPool: {}",          // 内存池大小
               configured_thread_count_, // 使用保存的配置值
               workers_active,
               pending,
               global_queue_size,
               total_local_queue_size,
               current_completed,        // 使用当前累计完成值
               rejected_total,           // 使用当前累计拒绝值
               tasks_per_second,         // 计算出的吞吐量
               pool_size);

    // --- 资源使用情况 (保留原有逻辑) ---
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        slog->info("[Resource Usage] WorkingSet: {:.2f} MB | PeakWorkingSet: {:.2f} MB",
                   pmc.WorkingSetSize / 1024.0 / 1024.0,
                   pmc.PeakWorkingSetSize / 1024.0 / 1024.0);
    }
    else {
        slog->warn("[Resource Usage] Failed to get process memory info.");
    }

    DWORD handleCount = 0;
    if (GetProcessHandleCount(GetCurrentProcess(), &handleCount)) {
        slog->info("[Resource Usage] Handle Count: {}", handleCount);
    }
    else {
        slog->warn("[Resource Usage] Failed to get process handle count.");
    }

    // --- 增加一些告警判断 ---
    constexpr size_t PENDING_WARN_THRESHOLD = 5000; // 待处理任务警告阈值
    constexpr size_t REJECTED_RATE_WARN_THRESHOLD = 10; // 每秒拒绝任务警告阈值

    if (pending > PENDING_WARN_THRESHOLD) {
        slog->warn("[ThreadPool Alert] High pending tasks: {} exceeds threshold {}", pending, PENDING_WARN_THRESHOLD);
    }
    // 可以增加对拒绝速率的监控，但这需要记录上一次的拒绝总数
    // double rejected_per_second = ...;
    // if (rejected_per_second > REJECTED_RATE_WARN_THRESHOLD) {
    //     slog->warn("[ThreadPool Alert] High rejection rate: {:.2f} Tasks/Sec", rejected_per_second);
    // }
    if (global_queue_size >= GLOBAL_QUEUE_MAX * 0.9) { // 全局队列接近满
        slog->warn("[ThreadPool Alert] Global queue near capacity: {} / {}", global_queue_size, GLOBAL_QUEUE_MAX);
    }

}