#include "pch.h"

// 项目自有头文件
#include "ObserverServer.h"
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
    // 优化后的配置参数
    // 添加内存池配置参数
    static constexpr size_t MEMORY_POOL_LOW_WATERMARK = 4096;  // 低水位线
    static constexpr size_t GLOBAL_QUEUE_MAX = 100000;         // 新增全局队列容量限制
    static constexpr size_t MAX_LOCAL_QUEUE_SIZE = 8192;  // 增大本地队列容量
    static constexpr size_t WORKER_BATCH_SIZE = 128;      // 增加批量处理数量
    static constexpr int MAX_EMPTY_ITERATIONS = 50;       // 减少空转次数
    static constexpr size_t MEMORY_POOL_INIT_SIZE = 16384;// 预分配内存池大小
    struct TaskWrapper {
        Task task;
        explicit TaskWrapper(Task&& t) noexcept : task(std::move(t)) {}

        // 确保移动安全
        TaskWrapper(TaskWrapper&& other) noexcept : task(std::move(other.task)) {}
        TaskWrapper& operator=(TaskWrapper&& other) noexcept {
            task = std::move(other.task);
            return *this;
        }
    };

    // 线程本地队列（SPSC）
    struct alignas(64) WorkerContext {
        moodycamel::ReaderWriterQueue<TaskWrapper*> local_queue{ MAX_LOCAL_QUEUE_SIZE };
        //moodycamel::ConcurrentQueue<TaskWrapper*> local_pool;  // 本地内存池
        std::atomic<bool> active{ true };
        std::atomic<size_t> total_steals{ 0 };  // 窃取统计
    };


    // 全局队列（MPMC）
    moodycamel::ConcurrentQueue<TaskWrapper*> global_queue_;

    // 工作线程管理
    std::vector<std::thread> workers_;
    std::atomic<bool> stop_{ false };
    std::atomic<bool> monitor_stop_{ false };  // 专用停止标志
    std::atomic<int64_t> pending_tasks_{ 0 };
    std::thread monitor_thread_;  // 添加监控线程成员

    // 内存池和线程上下文
    moodycamel::ConcurrentQueue<TaskWrapper*> task_pool_;
    // 使用weak_ptr避免悬挂指针
    std::vector<std::weak_ptr<WorkerContext>> worker_contexts_;
    std::shared_mutex context_mutex_;
public:
    explicit BusinessThreadPool(size_t thread_count = std::thread::hardware_concurrency())
        : global_queue_(thread_count * 4096/*8192*/)
    {
        // 预分配内存池对象（假设 Task 可默认构造）
        const size_t INITIAL_POOL_SIZE = global_queue_.size_approx() * 4;
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
        start_monitor();
    }

    ~BusinessThreadPool() {
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
    }

    bool enqueue(Task task) {
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
private:
    // 修改监控线程实现
    void start_monitor() {
        monitor_thread_ = std::thread([this] {
            while (!monitor_stop_.load(std::memory_order_acquire)) {
                std::this_thread::sleep_for(std::chrono::seconds(10));

                // 双重检查避免竞态条件
                if (monitor_stop_.load(std::memory_order_acquire)) break;

                // 安全访问成员变量
                const size_t pool_size = task_pool_.size_approx();
                if (pool_size < MEMORY_POOL_LOW_WATERMARK) {
                    const size_t need_alloc = MEMORY_POOL_INIT_SIZE - pool_size;
                    for (size_t i = 0; i < need_alloc; ++i) {
                        // 使用安全内存分配
                        try {
                            auto wrapper = new TaskWrapper(Task{});
                            if (!task_pool_.enqueue(wrapper)) {
                                delete wrapper;
                            }
                        }
                        catch (const std::bad_alloc& e) {
                            slog->error("Memory allocation failed: {}", e.what());
                            break;
                        }
                    }
                }
            }
        });
    }
    void worker_loop(std::shared_ptr<WorkerContext>& ctx, size_t worker_id) {
        SetThreadPriority(GetCurrentThread(), THREAD_MODE_BACKGROUND_BEGIN);       

        constexpr size_t MAX_BATCH = 128;
        TaskWrapper* local_batch[MAX_BATCH];
        
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
                Sleep(1);
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

    size_t process_local(WorkerContext* ctx, std::vector<TaskWrapper*>& batch) {
        size_t processed = 0;
        TaskWrapper* task;
        while (processed < WORKER_BATCH_SIZE && ctx->local_queue.try_dequeue(task)) {
            execute_task(task);
            ++processed;
        }
        return processed;
    }

    size_t process_global(std::vector<TaskWrapper*>& batch) {
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
    
    size_t try_steal(std::vector<TaskWrapper*>& batch, size_t thief_id) {
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

        // 批量执行窃取到的任务
        if (!batch.empty()) {
            for (auto task : batch) {
                execute_task(task);
            }
            batch.clear();
        }

        pending_tasks_.fetch_sub(stolen, std::memory_order_relaxed);
        return stolen;
    }
    void execute_task(TaskWrapper* task) {
        if (!task) return;

        // 异常安全的移动捕获
        try {
            Task local_task;
            local_task = std::move(task->task);  // 正确的移动赋值

            // 执行任务
            CObserverServer::instance().process_task(std::move(local_task));
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

    void handle_idle(size_t processed, int& empty_cycles) {
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

    void drain_queue(std::shared_ptr<WorkerContext>& ctx) {
        TaskWrapper* task;
        while (ctx->local_queue.try_dequeue(task)) {
            execute_task(task);
        }
    }    
public:
    void check_memory_usage() {
        constexpr size_t MAX_POOL_SIZE = 100000;
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

    void log_pool_stats() {
        size_t global = task_pool_.size_approx();
        size_t in_flight = pending_tasks_.load();
        slog->info("Memory: pool={} tasks={} workers={}",
                   global, in_flight, worker_contexts_.size());
    }
};

// 全局线程池
static BusinessThreadPool pool(std::max<size_t>(4, std::thread::hardware_concurrency() * 2));
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
        if (!pool.enqueue(std::move(task))) {
            slog->warn("Failed to enqueue task, queue may be full");
            return false;
        }
        // 检查内存使用情况
        //pool.check_memory_usage();

        // 记录状态
        //pool.log_pool_stats();

        return true;
    }
    catch (const std::exception& e) {
        slog->error("Exception in on_recv: {}", e.what());
        return false;
    }
}

void CObserverServer::process_task(Task&& task)
{
    if (IsBadReadPtr(task.raw_msg.get(), sizeof(msgpack::object_handle))) {
        slog->critical("Invalid msgpack handle detected");
        return;
    }
    auto& [package_id, session, package, raw_msg] = task;
    std::shared_ptr<AntiCheatUserData> user_data;
    if (SPKG_ID_START < package_id && package_id < SPKG_ID_MAX_PACKAGE_ID_SIZE) {
        // 减少包处理数量,临时用,待登录器更新后移除
        if (SPKG_ID_C2S_UPDATE_USER_NAME == package_id) {
            user_data = get_user_data_(session);
            auto& req = raw_msg->get().as<ProtocolC2SUpdateUsername>();
            auto username = user_data->get_field("usrname");
            std::wstring username_str = username.has_value() ? username.value() : L"";
            //slog->warn("Update username: {}=={}", username_str, Utils::String::w2c(req.username));
            if (!req.username.empty() && username_str == req.username) {
                //slog->warn("Update username: {}===={}", username_str, Utils::String::w2c(req.username));
                return;
            }
        }
        //slog->info("ProtocolLC2LSSend package_id: {}", package_id);
        ProtocolLC2LSSend req;
        req.package = package;
        req.package.head.session_id = session->hash_key();
        logic_client_->async_send(&req, package.head.session_id);
        return;
    }

    if (!user_data)
    {
        user_data = get_user_data_(session);
    }

    // auto is_observer_client = user_data->get_field<bool>("is_observer_client");
    // if (!(is_observer_client.has_value() && is_observer_client.value()))
    // {
    //     super::log(LOG_TYPE_ERROR, TEXT("未授权Service调用"));
    //     return;
    // }

    if (OBSPKG_ID_START < package_id && package_id < OBSPKG_ID_END) {
        // 执行任务
        try {
            // 调用 dispatch
            ob_pkg_mgr_.dispatch(package_id, session, package, *raw_msg);
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
// 优化后的心跳处理(分批+无锁)
void CObserverServer::batch_heartbeat(const std::vector<tcp_session_shared_ptr_t>& sessions) {
    constexpr size_t BATCH_SIZE = 512;
    auto now = std::chrono::steady_clock::now();
    
    // 分批处理减少锁持有时间
    for (size_t i = 0; i < sessions.size(); i += BATCH_SIZE) {
        auto end = std::min(i + BATCH_SIZE, sessions.size());
        std::shared_lock<std::shared_mutex> lock(session_times_mtx_);
        
        // 使用预分配内存避免重复分配
        thread_local static phmap::flat_hash_map<std::size_t, std::chrono::steady_clock::time_point> local_cache;
        local_cache.reserve(BATCH_SIZE);
        
        for (size_t j = i; j < end; ++j) {
            auto& session = sessions[j];
            local_cache[session->hash_key()] = now;
            if (auto user_data = get_user_data_(session)) {
                user_data->update_heartbeat_time();
            }
        }
        
        // 批量更新到主存储
        {
            std::unique_lock<std::shared_mutex> unique_lock(session_times_mtx_);
            // 手动合并map内容,因为merge不支持不同类型map的合并
            for (const auto& [session_id, time] : local_cache) {
                session_last_active_times_[session_id] = time;
            }
        }
        local_cache.clear();
    }
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
                    if (user_data && user_data->uuid) {
                        memcpy(_userdata.uuid, user_data->uuid, sizeof(_userdata.uuid));
                    }
                    else {
                        memset(_userdata.uuid, 0, sizeof(_userdata.uuid));
                    }
                    _userdata.has_handshake = user_data->get_handshake();
                    _userdata.last_heartbeat_time = user_data->last_heartbeat_time;
                    _userdata.json = user_data->data;
                    logic_client_->send(&req);
                }
            }
        });
    });
    logic_client_->package_mgr().register_handler(LSPKG_ID_S2C_SEND, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        auto resp = msg.get().as<ProtocolLS2LCSend>();
        auto session_id = resp.package.head.session_id;
        auto session = find_session(session_id);
        if (!session)
        {
            slog->error("Session not found: {}", session_id);
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
        slog->info("查询所有用户数据");
        std::vector<std::pair<uint64_t, ProtocolUserData>> user_data_list;
        foreach_session([this, &user_data_list](tcp_session_shared_ptr_t& session) {
            auto user_data = get_user_data_(session);
            if (user_data->get_handshake()) {
                ProtocolUserData _userdata;
                _userdata.session_id = session->hash_key();
                if (user_data && user_data->uuid) {
                    memcpy(_userdata.uuid, user_data->uuid, sizeof(_userdata.uuid));
                }
                else {
                    memset(_userdata.uuid, 0, sizeof(_userdata.uuid));
                }
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
            if (user_data && user_data->uuid) {
                memcpy(_userdata.uuid, user_data->uuid, sizeof(_userdata.uuid));
            }
            else {
                memset(_userdata.uuid, 0, sizeof(_userdata.uuid));
            }
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
// OBPKG_ID_C2S_AUTH
void CObserverServer::obpkg_id_c2s_auth(tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg)
{
#ifdef _DEBUG        
    get_user_data_(session)->set_field("is_observer_client", true);
    super::log(LOG_TYPE_EVENT, TEXT("observer client auth success:%s:%u"), Utils::c2w(session->remote_address()).c_str(),
               session->remote_port());
    ProtocolOBS2OBCQueryVmpExpire resp;
    resp.vmp_expire = get_vmp_expire();
    slog->warn("vmp_expire: {}", Utils::w2c(resp.vmp_expire));
    send(session, &resp);
    ProtocolLC2LSAddObsSession req;
    req.session_id = session->hash_key();
    logic_client_->async_send(&req);
    ProtocolOBS2OBCAuth auth;
    auth.status = true;
    async_send(session, &auth);
    if (auth_lock_.load()) {
        auth_lock_.store(false); OutputDebugStringA("网关后台认证12");
    }
    return;
#else
    auto auth_key = raw_msg.get().as<ProtocolOBC2OBSAuth>().key;
    get_user_data_(session)->set_field("auth_key", auth_key);
    if (auth_key == get_auth_ticket())
    {
        get_user_data_(session)->set_field("is_observer_client", true);
        slog->info("授权观察者客户端成功 client_addr: {}:{}, session_id: {}",
                   session->remote_address(), session->remote_port(), session->hash_key());
        ProtocolOBS2OBCQueryVmpExpire resp;
        resp.vmp_expire = get_vmp_expire();
        send(session, &resp);
        ProtocolLC2LSAddObsSession req;
        req.session_id = session->hash_key();
        logic_client_->async_send(&req);
        ProtocolOBS2OBCAuth auth;
        auth.status = true;
        async_send(session, &auth);
        // 发送有效期日期
        session->start_timer("query_vmp_expire", 2000, [this, resp, weak_session = std::weak_ptr(session)]() {
            if (auto session = weak_session.lock()) {
                send(session, &resp);
            }
        });
        if (auth_lock_.load()) {
            auth_lock_.store(false); OutputDebugStringA("网关后台认证12");
        }
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
