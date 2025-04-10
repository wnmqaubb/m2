#include "pch.h"
#include "ThreadPool.h"
#ifdef G_SERVICE
#include <ObserverServer.h>
#else
#include <LogicServer.h>
#endif
#include <Psapi.h>

BusinessThreadPool::BusinessThreadPool(size_t thread_count)
    : global_queue_(thread_count * 4096)
{
    // 预分配内存池对象（假设 Task 可默认构造）
    const size_t INITIAL_POOL_SIZE = global_queue_.size_approx() * 2;
    for (size_t i = 0; i < INITIAL_POOL_SIZE; ++i) {
        //TaskWrapper* wrapper = new TaskWrapper(Task{});  // 需 Task 支持默认构造
        auto* wrapper = new TaskWrapper(
            Task(0, nullptr, RawProtocolImpl{}, msgpack::v1::object_handle{})
        );
        task_pool_.enqueue(wrapper);
    }
    workers_.reserve(thread_count);
    for (size_t i = 0; i < thread_count; ++i) {
        workers_.emplace_back([this, i] {
            auto ctx = std::make_shared<WorkerContext>();
            {
                std::unique_lock lock(context_mutex_);
                worker_contexts_.emplace_back(ctx);
            }

            worker_loop(ctx, i);

            // 清理逻辑
            {
                std::unique_lock lock(context_mutex_);
                auto it = std::find_if(worker_contexts_.begin(), worker_contexts_.end(),
                                       [&](const std::weak_ptr<WorkerContext>& weak_ctx) {
                    auto shared_ctx = weak_ctx.lock();
                    return shared_ctx && (shared_ctx == ctx);  // 比较shared_ptr对象
                });
            }

            ctx->active.store(false, std::memory_order_release);
            drain_queue(ctx);
        });

    }
    // 在构造函数中启动监控线程
    start_monitor(INITIAL_POOL_SIZE);
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
            wrapper = new TaskWrapper(Task{});
        }
    }

    // 安全移动任务
    try {
        wrapper->task = std::move(task);
    }
    catch (const std::exception& e) {
        slog->error("Task move failed: {}", e.what());
        task_pool_.enqueue(wrapper);
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

    // 降级到全局队列
    if (global_queue_.size_approx() < GLOBAL_QUEUE_MAX) {  // 使用正确容量检查
        if (global_queue_.enqueue(wrapper)) {
            pending_tasks_.fetch_add(1, std::memory_order_relaxed);
            return true;
        }
    }

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
            const auto sleep_duration = std::chrono::seconds(prev_pending > 1000 ? 1 : 10);
            std::this_thread::sleep_for(sleep_duration);

            // 弹性内存补充策略
            const size_t current_pool = task_pool_.size_approx();
            const size_t current_pending = pending_tasks_.load();

            // 计算目标池大小（基于当前负载）
            const size_t target_pool = std::clamp(
                current_pending * 2,
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
    TaskWrapper* local_batch[MAX_BATCH];
    int idle_retries = 0;

    while (!stop_.load(std::memory_order_relaxed)) {
        // 每次循环前检查活性
        if (!ctx->active.load(std::memory_order_acquire)) {
            break;
        }
        size_t processed = 0;

        // 优先处理本地队列
        TaskWrapper* task;
        while (ctx->local_queue.try_dequeue(task)) {
            execute_task(task);
            processed++;
        }

        // 处理全局队列（保持原有逻辑）
        std::vector<TaskWrapper*> batch;
        int count = process_global(batch);
        processed += count;

        // 工作窃取优化（使用已有方法）
        if (processed == 0) {
            processed += try_steal(batch, worker_id);  // 修正方法名
        }

        // 自适应休眠
        if (processed == 0) {
            if (idle_retries < 10) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1 << idle_retries));
                idle_retries++;
            }
            else {
                std::this_thread::yield();
            }
        }
        else {
            idle_retries = 0;
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
    size_t count = global_queue_.try_dequeue_bulk(
        std::back_inserter(batch),
        WORKER_BATCH_SIZE
    );

    for (auto task : batch) {
        execute_task(task);
    }
    batch.clear();
    return count;
}

size_t BusinessThreadPool::try_steal(std::vector<TaskWrapper*>& batch, size_t thief_id) {
    std::shared_lock<std::shared_mutex> lock(context_mutex_);
    size_t stolen = 0;
    const size_t total_workers = worker_contexts_.size();

    // 优化参数配置
    constexpr size_t MAX_STEAL_PER_WORKER = 32;    // 单线程最大窃取量
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
        std::chrono::microseconds(50) :   // 高负载时短休眠
        std::chrono::microseconds(200);   // 低负载时长休眠

    if (stolen == 0) {
        std::this_thread::sleep_for(sleep_time);  // 动态休眠
    }
    // 批量执行窃取到的任务
    if (!batch.empty()) {
        for (auto task : batch) {
            execute_task(task);
        }
        batch.clear();
    }

    //pending_tasks_.fetch_sub(stolen, std::memory_order_relaxed);
    return stolen;
}

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
    while (ctx->local_queue.try_dequeue(task)) {
        execute_task(task);
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
void BusinessThreadPool::log_thread_pool_status() {
    const size_t pending = pending_tasks_.load(std::memory_order_relaxed);
    const size_t workers = worker_contexts_.size();
    const size_t global_queue_size = global_queue_.size_approx();
    size_t total_local_queue_size = 0;

    // 遍历所有worker的本地队列
    {
        std::shared_lock lock(context_mutex_);
        for (const auto& weak_ctx : worker_contexts_) {
            if (auto ctx = weak_ctx.lock()) {
                total_local_queue_size += ctx->local_queue.size_approx();
            }
        }
    }

    slog->info("[ThreadPool Status] "
               "Workers: {} | "
               "Pending Tasks: {} | "
               "Global Queue: {} | "
               "Total Local Queue: {} | "
               "Memory Pool: {}",
               workers,
               pending,
               global_queue_size,
               total_local_queue_size,
               task_pool_.size_approx());

    // 触发警告的条件（示例：当队列深度超过2500时）
    constexpr size_t WARNING_THRESHOLD = 2500;
    if (pending > WARNING_THRESHOLD) {
        slog->warn("[ThreadPool Warning] High pending tasks: {}", pending);
    }

    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
    slog->info("[Resource] 内存使用: {:.2f} MB", pmc.WorkingSetSize / 1024.0 / 1024.0);

    // 监控句柄数（Windows API）
    DWORD handleCount;
    GetProcessHandleCount(GetCurrentProcess(), &handleCount);
    slog->info("[Resource] 当前句柄数: {}", handleCount);
}