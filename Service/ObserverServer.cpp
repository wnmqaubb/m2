#include "pch.h"

// 项目自有头文件
#include "LogicClient.h"
#include "ObserverServer.h"
#include "VmpSerialValidate.h"

// 标准库和第三方库
#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <thread>
#include <vector>

// Folly库
//#include <folly/MPMCQueue.h>
//#include <folly/AtomicUnorderedMap.h>
//#include <folly/ProducerConsumerQueue.h>
#include <concurrentqueue/concurrentqueue.h>
#include <readerwriterqueue/readerwriterqueue.h>
#include <parallel_hashmap/phmap.h>

// Windows头文件（置于最后以避免宏冲突）
#include <Psapi.h>
#include <WinBase.h>
//slog->info(" {}:{}:{}", __FUNCTION__, __FILE__, __LINE__);
extern std::shared_ptr<spdlog::logger> slog;

// 业务线程池保持原有结构
class BusinessThreadPool {
private:
    // 配置参数
    static constexpr size_t INITIAL_QUEUE_CAPACITY = 65536;  // 提升到64k
    static constexpr size_t MAX_LOCAL_QUEUE_SIZE = 8192;     // 本地队列容量翻倍
    static constexpr size_t WORKER_BATCH_SIZE = 256;         // 加大批量处理
    static constexpr size_t MEMORY_POOL_GROWTH_FACTOR = 2;   // 内存池动态增长因子
    static constexpr int MAX_EMPTY_ITERATIONS = 100;
    static constexpr size_t MAX_MEMORY_POOL = 1024 * 1024; // 1M objects max
    static constexpr size_t OVERFLOW_CHECK_INTERVAL = 50; // ms
    // 新增调节参数
    static constexpr int MAX_BACKOFF_US = 1000;  // 最大退避时间1ms
    static constexpr int MIN_BACKOFF_US = 10;    // 最小退避时间10μs
    static constexpr double THROTTLE_FACTOR = 1.2; // 动态调节因子
    // 新增成员变量
    std::atomic<int> current_backoff_{ MIN_BACKOFF_US };
    std::atomic<size_t> consecutive_low_{ 0 };


    struct TaskWrapper {
        Task task;
        std::chrono::steady_clock::time_point enqueue_time;

        explicit TaskWrapper(Task&& t)
            : task(std::move(t)),
            enqueue_time(std::chrono::steady_clock::now()) {
        }
    };

    // 双全局队列系统
    moodycamel::ConcurrentQueue<TaskWrapper*> main_global_queue_;
    moodycamel::ConcurrentQueue<TaskWrapper*> overflow_global_queue_;
    std::atomic<size_t> main_queue_approx_size_{ 0 };
    std::atomic<size_t> overflow_queue_approx_size_{ 0 };

    // 内存管理
    moodycamel::ConcurrentQueue<TaskWrapper*> memory_pool_;
    std::atomic<size_t> memory_pool_size_{ 0 };
    std::thread memory_recycler_;
    std::atomic<bool> memory_recycler_stop_{ false };

    // 工作线程管理
    struct WorkerContext {
        moodycamel::ReaderWriterQueue<TaskWrapper*> queue{ MAX_LOCAL_QUEUE_SIZE };
        std::atomic<size_t> approx_size{ 0 };
        std::atomic<bool> active{ true };
    };

    std::vector<std::thread> workers_;
    std::vector<WorkerContext*> worker_contexts_;
    std::atomic<bool> stop_{ false };
    std::atomic<int> pending_tasks_{ 0 };
    std::mutex context_mutex_;

    // 监控统计
    std::atomic<size_t> enqueue_failures_{ 0 };
    std::atomic<size_t> total_steals_{ 0 };
    std::atomic<size_t> memory_allocated_{ 0 };

public:
    explicit BusinessThreadPool(size_t thread_count = std::thread::hardware_concurrency())
        : main_global_queue_(INITIAL_QUEUE_CAPACITY),
        overflow_global_queue_(INITIAL_QUEUE_CAPACITY)
    {
        // 初始化内存池
        init_memory_pool(thread_count * 512);

        // 启动内存回收线程
        memory_recycler_ = std::thread([this] {
            while (!memory_recycler_stop_.load(std::memory_order_relaxed)) {
                recycle_memory();
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        });

        // 创建工作线程
        workers_.reserve(thread_count);
        for (size_t i = 0; i < thread_count; ++i) {
            workers_.emplace_back([this, i] {
                // Windows线程亲和性设置
                SetThreadAffinityMask(GetCurrentThread(), (1 << (i % 32)));
                auto ctx = new WorkerContext();
                {
                    std::lock_guard<std::mutex> lock(context_mutex_);
                    worker_contexts_.push_back(ctx);
                }

                worker_loop(ctx, i);

                ctx->active.store(false, std::memory_order_release);
                drain_queue(ctx);
                delete ctx;
            });
        }
    }

    ~BusinessThreadPool() {
        stop_.store(true);
        memory_recycler_stop_.store(true);

        // 先停止内存回收线程
        if (memory_recycler_.joinable()) {
            memory_recycler_.join();
        }

        // 再停止工作线程
        for (auto& w : workers_) {
            if (w.joinable()) w.join();
        }

        // 安全清理内存池
        TaskWrapper* task;
        while (memory_pool_.try_dequeue(task)) {
            if (task) {
                // 正确析构顺序
                task->task.~Task();
                delete task;
            }
        }
    }

    bool enqueue(Task task) {
        if (stop_.load(std::memory_order_acquire)) return false;

        auto* wrapper = allocate_task_wrapper(std::move(task));
        if (!wrapper) return false;

        return enqueue_impl(wrapper);
    }

    struct Statistics {
        size_t pending_tasks;
        size_t main_queue_size;
        size_t overflow_queue_size;
        size_t memory_pool_size;
        size_t enqueue_failures;
        size_t total_steals;
        size_t memory_allocated;
    };

    Statistics get_stats() const {
        return {
            (size_t)pending_tasks_.load(std::memory_order_relaxed),
            main_queue_approx_size_.load(std::memory_order_relaxed),
            overflow_queue_approx_size_.load(std::memory_order_relaxed),
            memory_pool_size_.load(std::memory_order_relaxed),
            enqueue_failures_.load(std::memory_order_relaxed),
            total_steals_.load(std::memory_order_relaxed),
            memory_allocated_.load(std::memory_order_relaxed)
        };
    }

private:
    void init_memory_pool(size_t initial_size) {
        for (size_t i = 0; i < initial_size; ++i) {
            memory_pool_.enqueue(new TaskWrapper(Task{}));
            memory_allocated_.fetch_add(1, std::memory_order_relaxed);
        }
        memory_pool_size_.store(initial_size, std::memory_order_relaxed);
    }

    //TaskWrapper* allocate_task_wrapper(Task&& task) {
    //    TaskWrapper* wrapper = nullptr;
    //    if (memory_pool_.try_dequeue(wrapper)) {
    //        memory_pool_size_.fetch_sub(1, std::memory_order_relaxed);
    //        new (wrapper) TaskWrapper(std::move(task));
    //        return wrapper;
    //    }

    //    // 动态扩容（带内存上限保护）
    //    if (memory_allocated_.load(std::memory_order_relaxed) < MAX_MEMORY_POOL) {
    //        wrapper = new TaskWrapper(std::move(task));
    //        memory_allocated_.fetch_add(1, std::memory_order_relaxed);
    //        return wrapper;
    //    }

    //    // 内存达到上限，尝试再次获取
    //    if (memory_pool_.try_dequeue(wrapper)) {
    //        memory_pool_size_.fetch_sub(1, std::memory_order_relaxed);
    //        new (wrapper) TaskWrapper(std::move(task));
    //        return wrapper;
    //    }

    //    enqueue_failures_.fetch_add(1, std::memory_order_relaxed);
    //    return nullptr;
    //}
    // 改进任务分配策略
    TaskWrapper* allocate_task_wrapper(Task&& task) {
        TaskWrapper* wrapper = nullptr;

        // 快速路径：尝试直接获取
        if (memory_pool_.try_dequeue(wrapper)) {
            memory_pool_size_.fetch_sub(1);
            new (wrapper) TaskWrapper(std::move(task));
            return wrapper;
        }

        // 紧急扩容路径
        const size_t allocated = memory_allocated_.load();
        if (allocated < MAX_MEMORY_POOL) {
            const size_t grow_size = std::min(
                MAX_MEMORY_POOL - allocated,
                allocated * MEMORY_POOL_GROWTH_FACTOR
            );

            for (size_t i = 0; i < grow_size; ++i) {
                memory_pool_.enqueue(new TaskWrapper(Task{}));
                memory_allocated_.fetch_add(1);
            }
            memory_pool_size_.fetch_add(grow_size);

            if (memory_pool_.try_dequeue(wrapper)) {
                memory_pool_size_.fetch_sub(1);
                new (wrapper) TaskWrapper(std::move(task));
                return wrapper;
            }
        }

        // 最终尝试
        enqueue_failures_.fetch_add(1);
        return nullptr;
    }
    //bool enqueue_impl(TaskWrapper* wrapper) {
    //    constexpr int MAX_ATTEMPTS = 6;
    //    int attempts = 0;

    //    // 优先尝试主队列
    //    while (attempts < MAX_ATTEMPTS) {
    //        if (main_global_queue_.try_enqueue(wrapper)) {
    //            main_queue_approx_size_.fetch_add(1, std::memory_order_relaxed);
    //            pending_tasks_.fetch_add(1, std::memory_order_relaxed);
    //            return true;
    //        }

    //        // 次优选择：尝试本地队列随机注入
    //        if (try_random_local_injection(wrapper)) {
    //            pending_tasks_.fetch_add(1, std::memory_order_relaxed);
    //            return true;
    //        }

    //        std::this_thread::sleep_for(
    //            std::chrono::microseconds(10 * (1 << attempts))
    //        );
    //        ++attempts;
    //    }

    //    // 最终尝试溢出队列
    //    if (overflow_global_queue_.try_enqueue(wrapper)) {
    //        overflow_queue_approx_size_.fetch_add(1, std::memory_order_relaxed);
    //        pending_tasks_.fetch_add(1, std::memory_order_relaxed);
    //        return true;
    //    }

    //    // 所有队列均失败
    //    enqueue_failures_.fetch_add(1, std::memory_order_relaxed);
    //    recycle_task(wrapper);
    //    return false;
    //}
    // 增强入队策略
    bool enqueue_impl(TaskWrapper* wrapper) {
        // 阶段1：快速尝试主队列
        for (int i = 0; i < 3; ++i) {
            if (main_global_queue_.try_enqueue(wrapper)) {
                main_queue_approx_size_.fetch_add(1);
                pending_tasks_.fetch_add(1);
                return true;
            }
        }

        // 阶段2：批量尝试本地注入
        if (try_batch_local_injection(wrapper)) {
            pending_tasks_.fetch_add(1);
            return true;
        }

        // 阶段3：保证式入队（可能阻塞但确保成功）
        if (main_queue_approx_size_.load() < INITIAL_QUEUE_CAPACITY) {
            main_global_queue_.enqueue(wrapper);
            main_queue_approx_size_.fetch_add(1);
            pending_tasks_.fetch_add(1);
            return true;
        }

        // 阶段4：溢出队列保证式入队
        overflow_global_queue_.enqueue(wrapper);
        overflow_queue_approx_size_.fetch_add(1);
        pending_tasks_.fetch_add(1);
        return true;
    }
    
    // 新增批量本地注入方法
    bool try_batch_local_injection(TaskWrapper* wrapper) {
        std::lock_guard<std::mutex> lock(context_mutex_);
        if (worker_contexts_.empty()) return false;

        static thread_local std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<size_t> dist(0, worker_contexts_.size() - 1);

        // 尝试所有worker的本地队列
        for (size_t i = 0; i < worker_contexts_.size(); ++i) {
            auto* ctx = worker_contexts_[(dist(gen) + i) % worker_contexts_.size()];
            if (ctx->queue.try_enqueue(wrapper)) {
                ctx->approx_size.fetch_add(1);
                return true;
            }
        }
        return false;
    }

    bool try_random_local_injection(TaskWrapper* wrapper) {
        std::lock_guard<std::mutex> lock(context_mutex_);
        if (worker_contexts_.empty()) return false;

        static thread_local std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<size_t> dist(0, worker_contexts_.size() - 1);

        for (int i = 0; i < 3; ++i) { // 尝试3个随机worker
            auto* ctx = worker_contexts_[dist(gen)];
            if (ctx->queue.try_enqueue(wrapper)) {
                ctx->approx_size.fetch_add(1, std::memory_order_relaxed);
                return true;
            }
        }
        return false;
    }

    //void worker_loop(WorkerContext* ctx, size_t worker_id) {
    //    std::vector<TaskWrapper*> batch;
    //    batch.reserve(WORKER_BATCH_SIZE * 4);

    //    int empty_cycles = 0;
    //    auto last_overflow_check = std::chrono::steady_clock::now();

    //    while (!stop_) {
    //        size_t processed = process_local(ctx, batch);

    //        if (processed == 0) {
    //            processed += try_steal(batch, worker_id);
    //        }

    //        if (processed == 0) {
    //            processed += process_main_global(batch);
    //        }

    //        // 定期处理溢出队列
    //        auto now = std::chrono::steady_clock::now();
    //        if (now - last_overflow_check > std::chrono::milliseconds(OVERFLOW_CHECK_INTERVAL)) {
    //            processed += process_overflow_global(batch);
    //            last_overflow_check = now;
    //        }

    //        handle_idle(processed, empty_cycles);
    //    }
    //}
    // 增强的worker处理循环
    void worker_loop(WorkerContext* ctx, size_t worker_id) {
        std::vector<TaskWrapper*> batch;
        batch.reserve(WORKER_BATCH_SIZE * 4);  // 扩大批量处理容量

        while (!stop_) {
            // 优先级1：处理本地队列（批量模式）
            size_t processed = process_local(ctx, batch);

            // 优先级2：窃取其他worker的任务
            if (processed == 0) {
                processed += try_steal(batch, worker_id);
            }

            // 优先级3：处理主全局队列
            if (processed == 0) {
                processed += process_main_global(batch);
            }

            // 优先级4：处理溢出队列（实时检查）
            if (processed == 0) {
                processed += process_overflow_global(batch);
            }

            // 动态调整工作节奏
            adaptive_throttling(processed);
        }
    }

    size_t process_local(WorkerContext* ctx, std::vector<TaskWrapper*>& batch) {
        size_t processed = 0;
        TaskWrapper* task;

        while (processed < WORKER_BATCH_SIZE && ctx->queue.try_dequeue(task)) {
            execute_task(task);
            ctx->approx_size.fetch_sub(1);
            ++processed;
        }

        return processed;
    }

    size_t process_main_global(std::vector<TaskWrapper*>& batch) {
        size_t count = main_global_queue_.try_dequeue_bulk(
            std::back_inserter(batch), WORKER_BATCH_SIZE);

        for (auto task : batch) {
            execute_task(task);
        }
        batch.clear();
        main_queue_approx_size_.fetch_sub(count, std::memory_order_relaxed);
        return count;
    }

    size_t process_overflow_global(std::vector<TaskWrapper*>& batch) {
        size_t count = overflow_global_queue_.try_dequeue_bulk(
            std::back_inserter(batch), WORKER_BATCH_SIZE * 2);

        for (auto task : batch) {
            execute_task(task);
        }
        batch.clear();
        overflow_queue_approx_size_.fetch_sub(count, std::memory_order_relaxed);
        return count;
    }

    size_t try_steal(std::vector<TaskWrapper*>& batch, size_t thief_id) {
        std::lock_guard<std::mutex> lock(context_mutex_);
        if (worker_contexts_.size() < 2) return 0;

        static thread_local std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<size_t> dist(0, worker_contexts_.size() - 1);
        size_t start = dist(gen);

        for (size_t i = 0; i < worker_contexts_.size(); ++i) {
            size_t target_idx = (start + i) % worker_contexts_.size();
            if (target_idx == thief_id) continue;

            auto* target = worker_contexts_[target_idx];
            if (!target->active.load()) continue;

            TaskWrapper* task;
            size_t stolen = 0;
            while (stolen < WORKER_BATCH_SIZE &&
                   target->queue.try_dequeue(task)) {
                batch.push_back(task);
                ++stolen;
                target->approx_size.fetch_sub(1);
            }

            if (stolen > 0) {
                for (auto t : batch) execute_task(t);
                batch.clear();
                total_steals_.fetch_add(stolen);
                return stolen;
            }
        }
        return 0;
    }
    
    void execute_task(TaskWrapper* task) {
        try {
            CObserverServer::instance().process_task(std::move(task->task));
        }
        catch (const std::exception& e) {
            slog->error("Task failed: {}", e.what());
        }
        recycle_task(task);
    }

    void recycle_task(TaskWrapper* task) {
        try {
            // 安全析构Task对象
            if (task) {
                // 添加析构保护
                task->task.~Task();

                // 重置内存状态（重要！）
                new (&task->task) Task(); // 构造空对象防止野指针

                // 安全回收
                if (memory_pool_size_.load() < MAX_MEMORY_POOL) {
                    memory_pool_.enqueue(task);
                    memory_pool_size_.fetch_add(1);
                }
                else {
                    delete task;
                    memory_allocated_.fetch_sub(1);
                }
            }
        }
        catch (...) {
            // 异常保护
            delete task;
            memory_allocated_.fetch_sub(1);
        }
        pending_tasks_.fetch_sub(1);
    }

    void handle_idle(size_t processed, int& empty_cycles) {
        if (processed == 0) {
            if (++empty_cycles > MAX_EMPTY_ITERATIONS) {
                std::this_thread::sleep_for(
                    std::chrono::microseconds(empty_cycles * 10)
                );
            }
        }
        else {
            empty_cycles = std::max(0, empty_cycles - 2);
        }
    }
    // 实现自适应调节
    void adaptive_throttling(size_t processed) {
        constexpr size_t LOW_THRESHOLD = WORKER_BATCH_SIZE / 4;
        constexpr size_t HIGH_THRESHOLD = WORKER_BATCH_SIZE * 3 / 4;

        if (processed <= LOW_THRESHOLD) {
            const size_t old = consecutive_low_.fetch_add(1);

            // 动态增加退避时间
            if (old > 5) {
                current_backoff_.store(std::min(
                    static_cast<int>(current_backoff_.load() * THROTTLE_FACTOR),
                    MAX_BACKOFF_US
                ), std::memory_order_relaxed);

                // 应用退避
                std::this_thread::sleep_for(
                    std::chrono::microseconds(current_backoff_.load())
                );
            }
        }
        else if (processed >= HIGH_THRESHOLD) {
            // 重置调节状态
            consecutive_low_.store(0, std::memory_order_relaxed);
            current_backoff_.store(MIN_BACKOFF_US, std::memory_order_relaxed);
        }

        // 中间状态保持当前退避时间不变
    }
    
    //void recycle_memory() {
    //    constexpr size_t BATCH_SIZE = 512;
    //    TaskWrapper* batch[BATCH_SIZE];

    //    // 安全获取可回收对象（最多保留最近使用的25%）
    //    size_t count = memory_pool_.try_dequeue_bulk(batch, BATCH_SIZE);
    //    size_t keep = count / 4;  // 保留最近25%的对象

    //    // 确保安全删除（添加有效性校验）
    //    for (size_t i = keep; i < count; ++i) {
    //        if (batch[i]) {
    //            // 添加内存屏障确保对象状态一致
    //            std::atomic_thread_fence(std::memory_order_acquire);
    //            delete batch[i];
    //            memory_allocated_.fetch_sub(1, std::memory_order_relaxed);
    //        }
    //    }

    //    // 重新放回保留对象
    //    if (keep > 0) {
    //        memory_pool_.enqueue_bulk(batch, keep);
    //    }
    //}
    void recycle_memory() {
        constexpr size_t BATCH_SIZE = 1024;
        TaskWrapper* batch[BATCH_SIZE];

        // 获取当前内存压力
        const size_t current_pool = memory_pool_size_.load();
        const size_t target_retain = memory_allocated_.load() / 4;

        if (current_pool > target_retain) {
            size_t count = memory_pool_.try_dequeue_bulk(batch, BATCH_SIZE);
            size_t keep = std::min(count, target_retain);

            // 安全删除多余对象
            for (size_t i = keep; i < count; ++i) {
                if (batch[i]) {
                    delete batch[i];
                    memory_allocated_.fetch_sub(1);
                }
            }

            // 放回保留对象
            if (keep > 0) {
                memory_pool_.enqueue_bulk(batch, keep);
                memory_pool_size_.store(keep, std::memory_order_relaxed);
            }
        }
    }

    void clean_up_memory() {
        TaskWrapper* task;
        while (memory_pool_.try_dequeue(task)) {
            delete task;
            memory_allocated_.fetch_sub(1, std::memory_order_relaxed);
        }
    }

    void drain_queue(WorkerContext* ctx) {
        TaskWrapper* task;
        while (ctx->queue.try_dequeue(task)) {
            execute_task(task);
        }
    }
};

// 全局线程池实例
static BusinessThreadPool g_thread_pool;

// 转发到logic_server
bool CObserverServer::on_recv(unsigned int package_id, tcp_session_shared_ptr_t& session,
                              const RawProtocolImpl& package, msgpack::v1::object_handle&& raw_msg) {
    // 连接数统计
    static std::atomic<int> active_connections{ 0 };
    if (active_connections.load() >= 10000) {
        slog->warn("Too many active connections: {}", active_connections.load());
        return false;
    }

    try {
        // 将任务放入队列
        Task task(package_id, session, package, std::move(raw_msg));
        if (!g_thread_pool.enqueue(std::move(task))) {
            slog->warn("Failed to enqueue task, queue may be full");
            return false;
        }
        // 检查内存使用情况
        //pool.check_memory_usage();

        // 记录状态
        //pool.log_status();

        return true;
    }
    catch (const std::exception& e) {
        slog->error("Exception in on_recv: {}", e.what());
        return false;
    }
}

void CObserverServer::process_task(Task&& task)
{
    auto& [package_id, session, package, raw_msg] = task;
    std::shared_ptr<AntiCheatUserData> user_data;
    if (SPKG_ID_START < package_id && package_id < SPKG_ID_MAX_PACKAGE_ID_SIZE) {
        // 减少包处理数量,临时用,待登录器更新后移除
        if (SPKG_ID_C2S_UPDATE_USER_NAME == package_id) {
            user_data = get_user_data_(session);        
            auto& req = raw_msg.get().as<ProtocolC2SUpdateUsername>();
            auto username = user_data->get_field("usrname");
            std::wstring username_str = username.has_value() ? username.value() : L"";
            //slog->warn("Update username: {}=={}", username_str, Utils::String::w2c(req.username));
            if (!req.username.empty() && username_str == req.username) {
                //slog->warn("Update username: {}===={}", username_str, Utils::String::w2c(req.username));
                return;
            }
        }
        ProtocolLC2LSSend req;
        req.package = package;
        req.package.head.session_id = session->hash_key();
        logic_client_->/*async_*/send(&req, package.head.session_id);
        return;
    }

    if(!user_data)
    {
        user_data = get_user_data_(session);
    }

    auto is_observer_client = user_data->get_field<bool>("is_observer_client");
    if (!(is_observer_client.has_value() && is_observer_client.value()))
    {
        super::log(LOG_TYPE_ERROR, TEXT("未授权Service调用"));
        return;
    }

    if (OBSPKG_ID_START < package_id && package_id < OBSPKG_ID_END) {
        // 执行任务
        try {
            // 调用 dispatch
            ob_pkg_mgr_.dispatch(package_id, session, package, raw_msg);
        }
        catch (const std::exception& e) {
            slog->error("Package dispatch failed: {}", e.what());
        }
        catch (...) {
            slog->error("Package dispatch failed with unknown exception");
        }
    }
    else if (LSPKG_ID_START < package_id && package_id < LSPKG_ID_END) {
        ProtocolLC2LSSend req;
        req.package = package;
        req.package.head.session_id = session->hash_key();
        logic_client_->send(&req, package.head.session_id);
        return;
    }
    return;
}

CObserverServer::CObserverServer()
{
    static VmpSerialValidator vmp;
    logic_client_ = std::make_shared<CLogicClient>();
    is_observer_server_ = true;
    set_log_cb(std::bind(&CObserverServer::log_cb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    logic_client_->sub_notify_mgr_.register_handler(CLIENT_CONNECT_SUCCESS_NOTIFY_ID, [this]() {
        //slog->info("LogicClient重连后同步session");

        foreach_session([this](tcp_session_shared_ptr_t& session) {
            auto user_data = get_user_data_(session);
            if (user_data->get_handshake())
            {
                if (user_data->get_field<bool>("is_observer_client").value_or(false))
                {
                    ProtocolLC2LSAddObsSession req;
                    req.session_id = session->hash_key();
                    logic_client_->send(&req);
                }
                else
                {
                    //slog->info("添加用户session session_id: {}", session->hash_key());
                    ProtocolLC2LSAddUsrSession req;
                    auto& _userdata = req.data;
                    _userdata.session_id = session->hash_key();
                    memcpy(_userdata.uuid, user_data->uuid, sizeof(_userdata.uuid));
                    _userdata.has_handshake = user_data->get_handshake();
                    _userdata.last_heartbeat_time = user_data->last_heartbeat_time;
                    _userdata.json = user_data->data;
                    logic_client_->send(&req);
                }
            }
        });
    });
    // 转发logic的包到玩家
    logic_client_->package_mgr().register_handler(LSPKG_ID_S2C_SEND, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        auto resp = msg.get().as<ProtocolLS2LCSend>();
        auto session_id = resp.package.head.session_id;
        auto session = find_session(session_id); 
        if (!session)
        {
            return;
        }
        send(session, resp.package);
    });
    logic_client_->package_mgr().register_handler(LSPKG_ID_S2C_SET_FIELD, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        auto param = msg.get().as<ProtocolLS2LCSetField>();
        auto session = find_session(param.session_id);
        if (session)
        {
            get_user_data_(session)->set_field(param.key, param.val);
            // 发给管理员网关
            if (get_user_data_(session)->get_field<bool>("is_observer_client"))
            {
                ProtocolOBS2OBCSetField req;
                req.session_id = param.session_id;
                req.key = param.key;
                req.val = param.val;
                send(session, &req);
            }
        }
    });
    // 踢人 CLogicServer::close_socket
    logic_client_->package_mgr().register_handler(LSPKG_ID_S2C_KICK, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        auto param = msg.get().as<ProtocolLS2LCKick>();
        auto session = find_session(param.session_id);
        if (session)
        {
            session->stop();
        }
    });
    notify_mgr_.register_handler(SERVER_START_NOTIFY_ID, [this]() {
        for (int i = 0; i < 10; i++) {
            if (vmp.validate_timer(false))
            {
                start_timer(AUTH_CHECK_TIMER_ID, auth_check_timer_, std::bind(&VmpSerialValidator::validate_timer, vmp, true));
                connect_to_logic_server(kDefaultLocalhost, kDefaultLogicServicePort);
                break;
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });
    package_mgr_.register_handler(OBPKG_ID_C2S_AUTH, [this](tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
    #ifdef _DEBUG
        get_user_data_(session)->set_field("is_observer_client", true);
        super::log(LOG_TYPE_EVENT, TEXT("observer client auth success:%s:%u"), Utils::c2w(session->remote_address()).c_str(),
                   session->remote_port());
        ProtocolLC2LSAddObsSession req;
        req.session_id = session->hash_key();
        logic_client_->send(&req);
        ProtocolOBS2OBCAuth auth;
        auth.status = true;
        send(session, &auth);
        ProtocolOBS2OBCQueryVmpExpire resp;
        resp.vmp_expire = get_vmp_expire();
        send(session, &resp);
        return;
    #else
        auto auth_key = raw_msg.get().as<ProtocolOBC2OBSAuth>().key;
        get_user_data_(session)->set_field("auth_key", auth_key);
        if (auth_key == get_auth_ticket())
        {
            get_user_data_(session)->set_field("is_observer_client", true);
            slog->info("授权观察者客户端成功 client_addr: {}:{}, session_id: {}",
                       session->remote_address(), session->remote_port(), session->hash_key());
            ProtocolLC2LSAddObsSession req;
            req.session_id = session->hash_key();
            logic_client_->send(&req);
            ProtocolOBS2OBCAuth auth;
            auth.status = true;
            send(session, &auth);
            ProtocolOBS2OBCQueryVmpExpire resp;
            resp.vmp_expire = get_vmp_expire();
            send(session, &resp);
        }
        else
        {
            get_user_data_(session)->set_field("is_observer_client", false);
            slog->info("授权观察者客户端失败 client_addr: {}:{}, session_id: {}",
                       session->remote_address(), session->remote_port(), session->hash_key());
            ProtocolOBS2OBCAuth auth;
            auth.status = false;
            send(session, &auth);
        }
        std::wstring username = TEXT("(NULL)");
        get_user_data_(session)->set_field("usrname", username);
    #endif
    });

    // Gate to Service
    ob_pkg_mgr_.register_handler(OBPKG_ID_C2S_SEND, [this](tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
        auto req = raw_msg.get().as<ProtocolOBC2OBSSend>();
        /*async_*/send(find_session(req.package.head.session_id), req.package, session->hash_key());
    });
    // 踢人 
    ob_pkg_mgr_.register_handler(OBPKG_ID_C2S_KICK, [this](tcp_session_shared_ptr_t&, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
        auto req = raw_msg.get().as<ProtocolOBC2OBSKick>();
        auto session = find_session(req.session_id);
        if (session)
            session->stop();
    });
    ob_pkg_mgr_.register_handler(OBPKG_ID_C2S_QUERY_USERS, [this](tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
        ProtocolOBS2OBCQueryUsers resp;
        // 收集所有用户数据
        //slog->info("查询所有用户数据");
        std::vector<std::pair<uint64_t, ProtocolUserData>> user_data_list;
        foreach_session([this, &user_data_list](tcp_session_shared_ptr_t& session) {
            auto user_data = get_user_data_(session);
            if (user_data->get_handshake()) {
                ProtocolUserData _userdata;
                _userdata.session_id = session->hash_key();
                memcpy(_userdata.uuid, user_data->uuid, sizeof(_userdata.uuid));
                _userdata.has_handshake = user_data->get_handshake();
                _userdata.last_heartbeat_time = user_data->last_heartbeat_time;
                _userdata.json = user_data->data;
                user_data_list.emplace_back(_userdata.session_id, _userdata);
            }
        });

        // 在锁外构建响应
        for (const auto& [session_id, _userdata] : user_data_list) {
            resp.data.emplace(std::pair(_userdata.session_id, _userdata));
        }

        // 发送响应
        send(session, &resp);
    });
    ob_pkg_mgr_.register_handler(OBPKG_ID_C2S_UPDATE_LOGIC, [this](tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
        auto buf = raw_msg.get().as<ProtocolOBC2OBSUpdateLogic>().data;
        ProtocolLC2LSClose req;
        logic_client_->send(&req);
        while (logic_client_->is_started())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        char full_path[MAX_PATH] = { 0 };
        GetModuleFileNameA(NULL, full_path, sizeof(full_path));
        std::filesystem::path path = full_path;
        path = path.parent_path();
        auto exe_path = path / "g_LogicServer.exe";
        std::ofstream output(exe_path, std::ios::out | std::ios::binary);
        output.write((char*)buf.data(), buf.size());
        output.close();
        STARTUPINFOA si = {};
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        PROCESS_INFORMATION pi = {};
        BOOL res = CreateProcessA(NULL,
                                  (char*)(exe_path.string() + " " + std::to_string(GetCurrentProcessId()) + " 6").c_str(),
                                  0,
                                  0,
                                  FALSE,
                                  NORMAL_PRIORITY_CLASS,
                                  NULL,
                                  path.string().c_str(),
                                  &si,
                                  &pi);
        if (res == FALSE)
        {
            super::log(LOG_TYPE_EVENT, TEXT("更新LogicServer失败"));
        }
        else
        {
            super::log(LOG_TYPE_EVENT, TEXT("更新LogicServer成功"));
        }
    });
    user_notify_mgr_.register_handler(CLIENT_HANDSHAKE_NOTIFY_ID, [this](tcp_session_shared_ptr_t& session) {
        ProtocolLC2LSAddUsrSession req;
        auto user_data = get_user_data_(session);
        ProtocolUserData& _userdata = req.data;
        if (user_data->get_handshake())
        {
            _userdata.session_id = session->hash_key();
            memcpy(_userdata.uuid, user_data->uuid, sizeof(_userdata.uuid));
            _userdata.has_handshake = user_data->get_handshake();
            _userdata.last_heartbeat_time = user_data->last_heartbeat_time;
            _userdata.json = user_data->data;
            logic_client_->send(&req);
        }

    });
    user_notify_mgr_.register_handler(CLIENT_HEARTBEAT_NOTIFY_ID, [this](tcp_session_shared_ptr_t& session) {
        if (get_user_data_(session)->get_field<bool>("is_observer_client").value_or(false))
            return;
        ProtocolLC2LSSend req;
        ProtocolC2SHeartBeat heartbeat;
        heartbeat.tick = time(0);
        msgpack::sbuffer sbuf;
        msgpack::pack(sbuf, heartbeat);
        RawProtocolImplBase package;
        package.encode(sbuf.data(), sbuf.size());
        req.package.head = package.head;
        req.package.body = package.body;
        req.package.head.session_id = session->hash_key();
        logic_client_->send(&req, package.head.session_id);
    });
}

void CObserverServer::on_recv_client_heartbeat(tcp_session_shared_ptr_t& session) 
{
    auto user_data = get_user_data_(session);
    user_data->update_heartbeat_time();
    if (user_data->get_field<bool>("is_observer_client").value_or(false))
        return;
    ProtocolLC2LSSend req;
    ProtocolC2SHeartBeat heartbeat;
    heartbeat.tick = time(0);
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, heartbeat);
    RawProtocolImplBase package;
    package.encode(sbuf.data(), sbuf.size());
    req.package.head = package.head;
    req.package.body = package.body;
    req.package.head.session_id = session->hash_key();
    logic_client_->async_send(&req, package.head.session_id);
}

void CObserverServer::on_post_disconnect(tcp_session_shared_ptr_t& session)
{
    super::on_post_disconnect(session);

    //slog->info("检测到连接断开 client_addr: {}:{}, session_id: {}", session->remote_address(), session->remote_port(), session->hash_key());

    if (get_user_data_(session)->get_field<bool>("is_observer_client").value_or(false))
    {
        //slog->info("移除观察者session session_id: {}", session->hash_key());
        ProtocolLC2LSRemoveObsSession req;
        req.session_id = session->hash_key();
        logic_client_->send(&req);
    }

    //slog->info("移除用户session session_id: {}", session->hash_key());
    ProtocolLC2LSRemoveUsrSession req;
    req.data.session_id = session->hash_key();
    logic_client_->send(&req);
}

void CObserverServer::connect_to_logic_server(const std::string& ip, unsigned short port)
{
    try
    {
        if (logic_client_->start(ip, port))
        {
            printf("连接g_LogicServer成功\n");
        }
    }
    catch (...)
    {
        printf("连接g_LogicServer失败:错误号: %d, 错误信息: %s\n", asio2::get_last_error().value(), asio2::get_last_error_msg().c_str());
    }
}

void CObserverServer::log_cb(const wchar_t* msg, bool silence, bool gm_show, const std::string& identify, bool punish_flag)
{
    ProtocolOBS2OBCLogPrint log;
    log.silence = silence;
    log.gm_show = gm_show;
    log.punish_flag = punish_flag;
    foreach_session([this, &log](tcp_session_shared_ptr_t& session) {
        if (get_user_data_(session)->get_field<bool>("is_observer_client").value_or(false))//admin网关
        {
            send(session, &log);
        }
    });
}