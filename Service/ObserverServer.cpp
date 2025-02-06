#include "pch.h"
#include "LogicClient.h"
#include "ObserverServer.h"
#include "VmpSerialValidate.h"
#include <memory>
#include <Psapi.h>
#include <thread>
#include <vector>
#include <folly/AtomicUnorderedMap.h>
#include <folly/ProducerConsumerQueue.h>
//slog->info(" {}:{}:{}", __FUNCTION__, __FILE__, __LINE__);
extern std::shared_ptr<spdlog::logger> slog;
// 独立心跳处理线程池
//class HeartbeatThreadPool {
//private:
//    boost::lockfree::spsc_queue<Task> hb_queue{ MAX_HB_QUEUE_SIZE };
//    std::vector<std::thread> hb_workers;
//    std::atomic<bool> hb_stop{ false };
//
//public:
//    HeartbeatThreadPool(size_t threads = 4) {
//        for (size_t i = 0; i < threads; ++i) {
//            hb_workers.emplace_back([this] {
//                while (!hb_stop.load(std::memory_order_acquire)) {
//                    Task task;
//                    if (hb_queue.pop(task)) {
//                        process_heartbeat(task); // 专用处理函数
//                    }
//                    else {
//                        std::this_thread::yield();
//                    }
//                }
//            });
//        }
//    }
//
//    void enqueue_heartbeat(Task&& task) {
//        while (!hb_queue.push(std::move(task))) {
//            slog->warn("Heartbeat queue full, retrying...");
//            std::this_thread::sleep_for(1ms);
//        }
//    }
//
//    void process_heartbeat(Task& task) {
//        auto start = std::chrono::steady_clock::now();
//        CObserverServer::instance().update_last_active(task.client_id);
//        auto duration = std::chrono::duration_cast<ms>(std::chrono::steady_clock::now() - start);
//        if (duration > 50ms) {
//            slog->warn("Slow heartbeat processing: {}ms", duration.count());
//        }
//    }
//};

// 业务线程池保持原有结构
class BusinessThreadPool {
private:
    // 无锁队列配置
    static constexpr size_t MAX_LOCKFREE_QUEUE_SIZE = 10000;  // 比用户量略大
    static constexpr size_t WORKER_BATCH_SIZE = 32;  // 批量处理

    // 任务结构体
    struct TaskWrapper {
        Task task;
        std::atomic<bool> completed{ false };
        explicit TaskWrapper(Task&& t) : task(std::move(t)) {}
    };
    using TaskPtr = std::shared_ptr<TaskWrapper>;

    // 每个线程的本地队列（SPSC无锁队列）
    //struct WorkerQueue {
    //    boost::lockfree::spsc_queue<TaskPtr> queue{ MAX_LOCKFREE_QUEUE_SIZE };
    //};

    struct WorkerQueue {
        folly::ProducerConsumerQueue<TaskPtr> queue{ MAX_LOCKFREE_QUEUE_SIZE };
    };

    // 全局任务队列（MPMC无锁队列）
    //boost::lockfree::queue<TaskPtr> global_queue{ MAX_LOCKFREE_QUEUE_SIZE };
    folly::ProducerConsumerQueue<TaskPtr> global_queue{ MAX_LOCKFREE_QUEUE_SIZE };

    // 工作线程数组
    std::vector<std::thread> workers;
    std::atomic<bool> stop{ false };

    // 工作窃取支持
    folly::AtomicUnorderedInsertMap<size_t, WorkerQueue*> worker_queues{ 128 };  

public:
    explicit BusinessThreadPool(size_t thread_count = std::thread::hardware_concurrency() * 2) {
        workers.reserve(2);
        for (size_t i = 0; i < thread_count; ++i) {
            workers.emplace_back([this, i] { worker_loop(i); });
            worker_queues.emplace(i, new WorkerQueue);
        }
    }

    ~BusinessThreadPool() {
        stop.store(true, std::memory_order_release);
        for (auto& w : workers) {
            if (w.joinable()) w.join();
        }
    }

    // 提交任务（无锁入队）
    bool enqueue(Task task) {
        TaskPtr task_ptr = std::make_shared<TaskWrapper>(std::move(task));

        // 优先尝试本地队列（减少竞争）
        WorkerQueue* local_queue = worker_queues.find(std::hash<std::thread::id>{}(std::this_thread::get_id()))->second;
        if (local_queue && local_queue->queue.write(task_ptr)) {
            return true;
        }

        // 回退到全局队列
        return global_queue.write(task_ptr);
    }
    
private:
    // 工作线程主循环
    void worker_loop(size_t worker_id) {
        // 确保 worker_queues 中确实存在 worker_id 对应的队列
        auto it = worker_queues.find(worker_id);
        if (it == worker_queues.end()) {
            slog->error("No worker queue found for worker_id: {}", worker_id);
            return;  // 提前退出，防止空指针访问
        }

        WorkerQueue& local_queue = *it->second;  // 安全解引用
        std::vector<TaskPtr> local_batch;
        local_batch.reserve(WORKER_BATCH_SIZE);

        // 连续空转计数器
        int empty_iterations = 0;
        const int MAX_EMPTY_ITERATIONS = 100;

        // 工作窃取计数器
        int steal_attempts = 0;
        const int MAX_STEAL_ATTEMPTS = 3;

        while (!stop.load(std::memory_order_acquire)) {
            size_t processed = 0;

            // 处理本地队列
            while (processed < WORKER_BATCH_SIZE) {
                TaskPtr task;
                if (local_queue.queue.read(task)) {
                    // 添加空指针检查
                    if (task) {
                        execute_task(task);
                        processed++;
                        steal_attempts = 0;
                    }
                    else {
                        break;  // 跳过空任务
                    }
                }
                else {
                    break;
                }
            }

            // 处理全局队列
            TaskPtr task;
            while (local_batch.size() < WORKER_BATCH_SIZE && global_queue.read(task)) {
                // 添加空指针检查
                if (task) {
                    local_batch.push_back(task);
                }
            }

            for (auto& t : local_batch) {
                if (t) execute_task(t);
            }
            local_batch.clear();

            // 有限的工作窃取
            if (local_batch.empty() && steal_attempts < MAX_STEAL_ATTEMPTS) {
                bool stolen = false;
                for (auto& [i, worker_queue] : worker_queues) {
                    if (i == worker_id) continue;

                    // 安全检查
                    if (!worker_queue) continue;

                    TaskPtr steal_task;
                    if (worker_queue->queue.read(steal_task)) {
                        if (steal_task) {
                            execute_task(steal_task);
                            stolen = true;
                            break;
                        }
                    }
                }

                if (!stolen) {
                    steal_attempts++;
                }
            }

            // 空闲时休眠，避免CPU空转
            if (processed == 0) {
                empty_iterations++;
                if (empty_iterations > MAX_EMPTY_ITERATIONS) {
                    std::this_thread::sleep_for(std::chrono::microseconds(10 * empty_iterations));

                    if (empty_iterations > 100) {
                        empty_iterations = 100;
                    }
                }
            }
        }
    }

    // 执行任务
    void execute_task(TaskPtr task) {
        try {
            auto start = std::chrono::steady_clock::now();

            // 异步执行并设置超时
            std::future<void> future = std::async(
                std::launch::async,
                [task] { CObserverServer::instance().process_task(std::move(task->task)); }
            );

            if (future.wait_for(std::chrono::seconds(2)) == std::future_status::timeout) {
                slog->warn("Task timeout, package_id={}", task->task.package_id);
                return;
            }

            // 性能监控
            auto end = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            if (duration.count() > 300) {
                slog->warn("Slow task: {}ms", duration.count());
            }

            task->completed.store(true, std::memory_order_release);
        }
        catch (const std::exception& e) {
            slog->error("Task failed: {}", e.what());
        }
        catch (...) {
            slog->error("Unknown task failure");
        }
    }
public:
    void check_memory_usage() {
        static auto last_memory_check = std::chrono::steady_clock::now();
        static auto last_warning_time = std::chrono::steady_clock::now();
        static auto last_gc_time = std::chrono::steady_clock::now();

        auto now = std::chrono::steady_clock::now();
        if (now - last_memory_check > std::chrono::seconds(300)) {
            PROCESS_MEMORY_COUNTERS pmc;
            if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
                double memory_usage_mb = pmc.WorkingSetSize / 1024.0 / 1024.0;

                // 分级内存警告
                if (memory_usage_mb > 2048) {
                    slog->error("Critical memory usage: {:.2f}MB", memory_usage_mb);
                    // 触发内存回收
                    if (now - last_gc_time > std::chrono::minutes(5)) {
                        last_gc_time = now;
                        perform_memory_gc();
                    }
                }
                else if (memory_usage_mb > 1024 && now - last_warning_time > std::chrono::seconds(60)) {
                    slog->warn("High memory usage: {:.2f}MB", memory_usage_mb);
                    last_warning_time = now;
                }

                // 记录内存趋势
                static double last_usage = 0;
                if (memory_usage_mb > last_usage * 1.2) {
                    slog->info("Memory usage increased: {:.2f}MB -> {:.2f}MB", last_usage, memory_usage_mb);
                }
                last_usage = memory_usage_mb;
            }
            last_memory_check = now;
        }
    }

    void perform_memory_gc() {
        // 清理过期session
        size_t cleaned = 0;
        auto now = std::chrono::steady_clock::now();
        {
            std::unique_lock<std::mutex> lock(CObserverServer::instance().session_times_mtx_);
            CObserverServer::instance().foreach_session([&](tcp_session_shared_ptr_t& session) {
                auto last_active = CObserverServer::instance().get_last_active_time(session);
                if (now - last_active > std::chrono::minutes(10)) {
                    session->stop();
                    cleaned++;
                }
            });
        }
        if (cleaned > 0) {
            slog->info("Memory GC: cleaned {} inactive sessions", cleaned);
        }
    }

    void log_status() {
        static std::atomic<int> log_counter{ 0 };
        static constexpr int LOG_SAMPLE_RATE = 100;
        if (log_counter++ % LOG_SAMPLE_RATE == 0) {
            if (pending_tasks.load() > 0) {
                slog->info("Task queue status: pending={}, total={}",
                           pending_tasks.load(), total_tasks.load());
            }
        }
    }

private:
    std::atomic<int> pending_tasks;
    std::atomic<int64_t> total_tasks;
    // 内存管理（略，与原设计类似）
};
/*
class BusinessThreadPool {
public:
    BusinessThreadPool(size_t threads = std::max<size_t>(4, std::thread::hardware_concurrency()*2))
        : stop(false), pending_tasks(0), total_tasks(0) {
        for (size_t i = 0; i < threads; ++i)
            workers.emplace_back([this] {
            for (;;) {
                Task task;
                int package_id = 0;
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                    if (this->stop && this->tasks.empty())
                        return;

                    // 先弹出队列顶部元素
                    task = std::move(tasks.front());
                    package_id = task.package_id;
                    this->tasks.pop();
                }
                // 处理任务
                try {
                    auto start = std::chrono::steady_clock::now();

                    std::future<void> future = std::async(std::launch::async, [&task]() {
                        CObserverServer::instance().process_task(std::move(task));
                    });

                    // 等待任务完成，最多等待500ms
                    if (future.wait_for(std::chrono::milliseconds(500)) == std::future_status::timeout) {
                        slog->warn("Task processing timeout,package_id={}", package_id);
                        return; // 放弃超时任务
                    }

                    auto end = std::chrono::steady_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                    if (duration.count() > 300) {
                        slog->warn("Task processing took too long: {}ms", duration.count());
                    }
                }
                catch (const std::exception& e) {
                    slog->error("Task failed: {}", e.what());
                }
                catch (...) {
                    slog->error("Task failed with unknown exception");
                }

                // 定期检查线程健康状态
                static auto last_health_check = std::chrono::steady_clock::now();
                auto now = std::chrono::steady_clock::now();
                if (now - last_health_check > std::chrono::seconds(300)) {
                    PROCESS_MEMORY_COUNTERS pmc;
                    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
                        double memory_usage_mb = pmc.WorkingSetSize / 1024.0 / 1024.0;
                        slog->info("Thread health check - Memory: {:.2f}MB, Thread ID: {}",
                                   memory_usage_mb, GetCurrentThreadId());
                    }
                    last_health_check = now;
                }
            }
        });
    }

    void enqueue(Task task) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (stop)
                throw std::runtime_error("enqueue on stopped BusinessThreadPool");

            tasks.push(std::move(task));
        }
        condition.notify_one();
        return;
    }
    // 修改BusinessThreadPool的try_enqueue方法
    bool try_enqueue(Task&& task) {
        std::unique_lock<std::timed_mutex> lock(timed_mutex, std::defer_lock);
        if (!lock.try_lock_for(std::chrono::milliseconds(10))) {
            slog->warn("Failed to acquire lock in try_enqueue");
            return false;
        }

        if (stop) {
            return false;
        }

        // 使用原子操作更新计数
        static std::atomic<int> total_enqueued{ 0 };
        total_tasks++;
        total_enqueued++;

        try {
            // 限制队列大小
            static constexpr size_t MAX_QUEUE_SIZE = 10000;
            if (tasks.size() >= MAX_QUEUE_SIZE) {
                static std::atomic<int> queue_full_count{ 0 };
                if (++queue_full_count % 100 == 0) {
                    slog->warn("Task queue full, dropping task. Queue size: {}, Total enqueued: {}",
                               tasks.size(), total_enqueued.load());
                }
                return false;
            }

            tasks.push(std::move(task));
            condition.notify_one();

            static std::atomic<int> log_counter{ 0 };
            if (log_counter++ % 100 == 0) {
                slog->info("Task queue status - Size: {}, Total: {}, Enqueued: {}",
                            tasks.size(), total_tasks.load(), total_enqueued.load());
            }
            return true;
        }
        catch (const std::exception& e) {
            total_tasks--;
            total_enqueued--;
            slog->error("Failed to enqueue task: {}", e.what());
            return false;
        }
    }

    ~BusinessThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& worker : workers)
            worker.join();
    }

    void check_memory_usage() {
        static auto last_memory_check = std::chrono::steady_clock::now();
        static auto last_warning_time = std::chrono::steady_clock::now();
        static auto last_gc_time = std::chrono::steady_clock::now();

        auto now = std::chrono::steady_clock::now();
        if (now - last_memory_check > std::chrono::seconds(300)) {
            PROCESS_MEMORY_COUNTERS pmc;
            if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
                double memory_usage_mb = pmc.WorkingSetSize / 1024.0 / 1024.0;

                // 分级内存警告
                if (memory_usage_mb > 2048) {
                    slog->error("Critical memory usage: {:.2f}MB", memory_usage_mb);
                    // 触发内存回收
                    if (now - last_gc_time > std::chrono::minutes(5)) {
                        last_gc_time = now;
                        perform_memory_gc();
                    }
                }
                else if (memory_usage_mb > 1024 && now - last_warning_time > std::chrono::seconds(60)) {
                    slog->warn("High memory usage: {:.2f}MB", memory_usage_mb);
                    last_warning_time = now;
                }

                // 记录内存趋势
                static double last_usage = 0;
                if (memory_usage_mb > last_usage * 1.2) {
                    slog->info("Memory usage increased: {:.2f}MB -> {:.2f}MB", last_usage, memory_usage_mb);
                }
                last_usage = memory_usage_mb;
            }
            last_memory_check = now;
        }
    }

    void perform_memory_gc() {
        // 清理过期session
        size_t cleaned = 0;
        auto now = std::chrono::steady_clock::now();
        {
            std::unique_lock<std::mutex> lock(CObserverServer::instance().session_times_mtx_);
            CObserverServer::instance().foreach_session([&](tcp_session_shared_ptr_t& session) {
                auto last_active = CObserverServer::instance().get_last_active_time(session);
                if (now - last_active > std::chrono::minutes(10)) {
                    session->stop();
                    cleaned++;
                }
            });
        }
        if (cleaned > 0) {
            slog->info("Memory GC: cleaned {} inactive sessions", cleaned);
        }
    }

    void log_status() {
        static std::atomic<int> log_counter{ 0 };
        static constexpr int LOG_SAMPLE_RATE = 100;
        if (log_counter++ % LOG_SAMPLE_RATE == 0) {
            if (pending_tasks.load() > 0) {
                slog->info("Task queue status: pending={}, total={}",
                           pending_tasks.load(), total_tasks.load());
            }
        }
    }

private:
    std::vector<std::thread> workers;
    std::queue<Task> tasks;
    std::mutex queue_mutex;
    std::timed_mutex timed_mutex;
    std::condition_variable condition;
    std::atomic<int> pending_tasks;
    std::atomic<int64_t> total_tasks;
    bool stop;
};
*/
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
        pool.check_memory_usage();

        // 记录状态
        pool.log_status();

        return true;
    }
    catch (const std::exception& e) {
        slog->error("Exception in on_recv: {}", e.what());
        return false;
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
                    memcpy(_userdata.uuid, user_data->uuid, sizeof(_userdata.uuid));
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
        auto session = sessions().find(session_id);
        send(session, resp.package);
    });
    logic_client_->package_mgr().register_handler(LSPKG_ID_S2C_SET_FIELD, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        auto param = msg.get().as<ProtocolLS2LCSetField>();
        auto session = sessions().find(param.session_id);
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
        auto session = sessions().find(param.session_id);
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
        // 发送有效期日期
        session->start_timer("query_vmp_expire", 1000, 20, [this, resp, weak_session = std::weak_ptr(session)]() {
            if (auto session = weak_session.lock()) {
                send(session, &resp);
            }
        });
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
            // 发送有效期日期
            session->start_timer("query_vmp_expire", 1000, 10, [this, resp, weak_session = std::weak_ptr(session)]() {
                if (auto session = weak_session.lock()) {
                    send(session, &resp);
                }
            });
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
        send(sessions().find(req.package.head.session_id), req.package, session->hash_key());
    });
    // 踢人 
    ob_pkg_mgr_.register_handler(OBPKG_ID_C2S_KICK, [this](tcp_session_shared_ptr_t&, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
        auto req = raw_msg.get().as<ProtocolOBC2OBSKick>();
        auto session = sessions().find(req.session_id);
        if (session)
            session->stop();
    });
    // gate to service for freshuserlist
    /*ob_pkg_mgr_.register_handler(OBPKG_ID_C2S_QUERY_USERS, [this](tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
        ProtocolOBS2OBCQueryUsers resp;
        foreach_session([&resp](tcp_session_shared_ptr_t& session) {
            ProtocolUserData _userdata;
            auto user_data = get_user_data_(session);
            if (user_data->get_handshake())
            {
                _userdata.session_id = session->hash_key();
                memcpy(_userdata.uuid, user_data->uuid, sizeof(_userdata.uuid));
                _userdata.has_handshake = user_data->get_handshake();
                _userdata.last_heartbeat_time = user_data->last_heartbeat_time;
                _userdata.json = user_data->data;
                resp.data.emplace(std::pair(_userdata.session_id, _userdata));
            }
        });
        send(session, &resp);
    });*/
    ob_pkg_mgr_.register_handler(OBPKG_ID_C2S_QUERY_USERS, [this](tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
        ProtocolOBS2OBCQueryUsers resp;
        // 收集所有用户数据
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

void CObserverServer::process_task(Task&& task)
{
    auto& [package_id, session, package, raw_msg] = task;

    if (SPKG_ID_START < package_id && package_id < SPKG_ID_MAX_PACKAGE_ID_SIZE) {
        ProtocolLC2LSSend req;
        req.package = package;
        req.package.head.session_id = session->hash_key();
        logic_client_->send(&req, package.head.session_id);
        return;
    }

    auto user_data = get_user_data_(session);
    auto is_observer_client = user_data->get_field<bool>("is_observer_client");
    //slog->info(" package_id {} {}:{}:{}", package_id, __FUNCTION__, __FILE__, __LINE__);
    if (!(is_observer_client.has_value() && is_observer_client.value()))
    {
        super::log(LOG_TYPE_ERROR, TEXT("未授权Service调用"));
        return;
    }

    // 注册锁到死锁检测器
    //deadlock_detector_.register_lock(task_mtx_, "task_mutex");


    // 使用更细粒度和容错的锁策略
    std::unique_lock<std::shared_timed_mutex> lck(task_mtx_, std::defer_lock);
    auto lock_start = std::chrono::steady_clock::now();

    // 增加重试和超时机制
    const int MAX_LOCK_ATTEMPTS = 3;
    const auto LOCK_TIMEOUT = std::chrono::milliseconds(1000);
    const auto RETRY_INTERVAL = std::chrono::milliseconds(50);

    int lock_attempts = 0;
    bool lock_acquired = false;

    while (lock_attempts < MAX_LOCK_ATTEMPTS && !lock_acquired) {
        lock_acquired = lck.try_lock_for(LOCK_TIMEOUT);

        if (!lock_acquired) {
            lock_attempts++;
            slog->warn("Lock attempt failed for package_id={}, attempt={}",
                       package_id, lock_attempts);

            // 短暂等待后重试
            std::this_thread::sleep_for(RETRY_INTERVAL);
        }
    }

    if (!lock_acquired) {
        slog->error("Persistent lock failure for package_id={}, skipping task", package_id);
        return;
    }

    // 使用带超时的共享锁
    //std::shared_lock<std::shared_timed_mutex> lck(task_mtx_, std::defer_lock);
    //if (!lck.try_lock_for(std::chrono::milliseconds(300))) {
    //    slog->warn("Failed to acquire lock in process_task, wait_time=300ms");
    //    return;
    //}

    // 添加资源监控
    static std::atomic<int64_t> total_processed_packages{ 0 };
    static auto last_resource_check = std::chrono::steady_clock::now();

    auto now = std::chrono::steady_clock::now();
    if (now - last_resource_check > std::chrono::seconds(60)) {
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
            double memory_usage_mb = pmc.WorkingSetSize / 1024.0 / 1024.0;
            slog->info("Resource usage - Memory: {:.2f}MB, Processed packages: {}",
                       memory_usage_mb, total_processed_packages.load());
        }
        last_resource_check = now;
    }

    if (OBSPKG_ID_START < package_id && package_id < OBSPKG_ID_END) {
        // 执行任务
        try {
            // 执行任务
            auto start = std::chrono::steady_clock::now();
            // 调用 dispatch
            bool dispatch_result = ob_pkg_mgr_.dispatch(package_id, session, package, raw_msg);

            auto end = std::chrono::steady_clock::now();
            if (dispatch_result) {
                slog->info("Dispatch completed for package_id: {}", package_id);
            }
            else {
                slog->error("Dispatch failed for package_id: {}", package_id);
            }

            // 记录处理时间
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            if (duration.count() > 300) {
                slog->warn("Package processing took too long: package_id:{} {}ms", package_id, duration.count());
            }

            total_processed_packages++;
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