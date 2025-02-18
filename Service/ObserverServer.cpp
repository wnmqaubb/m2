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
    static constexpr size_t MAX_LOCKFREE_QUEUE_SIZE = 100000;
    static constexpr size_t WORKER_BATCH_SIZE = 64;
    static constexpr int MAX_EMPTY_ITERATIONS = 100;
    static constexpr int MAX_STEAL_ATTEMPTS = 3;

    // 任务封装
    struct TaskWrapper {
        Task task;
        std::atomic<bool> completed{ false };
        explicit TaskWrapper(Task&& t) : task(std::move(t)) {}
    };
    using TaskPtr = std::shared_ptr<TaskWrapper>;

    // 线程本地队列（SPSC）
    struct WorkerQueue {
        moodycamel::ReaderWriterQueue<TaskPtr> queue{ MAX_LOCKFREE_QUEUE_SIZE };
        // 精确计算填充（假设 64 字节缓存行）
        char padding[64 - (sizeof(moodycamel::ReaderWriterQueue<TaskPtr>) % 64)];
    };
    static_assert(sizeof(WorkerQueue) % 64 == 0, "Cache line alignment failed");


    // 全局队列（MPMC）
    moodycamel::ConcurrentQueue<TaskPtr> global_queue{ MAX_LOCKFREE_QUEUE_SIZE };

    // 工作线程管理
    std::vector<std::thread> workers;
    std::atomic<bool> stop{ false };
    phmap::parallel_node_hash_map<size_t, WorkerQueue*> worker_queues;

    // 统计指标
    std::atomic<int> pending_tasks{ 0 };
    std::atomic<int64_t> total_tasks{ 0 };
    std::mutex map_mutex;
public:
    explicit BusinessThreadPool(size_t thread_count = std::thread::hardware_concurrency() * 2)
        : worker_queues(thread_count * 2) {

        std::lock_guard<std::mutex> lock(map_mutex);
        // 创建所有工作队列
        for (size_t i = 0; i < thread_count; ++i) {
            worker_queues.emplace(i, new WorkerQueue);
        }

        // 启动工作线程
        workers.reserve(thread_count);
        for (size_t i = 0; i < thread_count; ++i) {
            workers.emplace_back([this, i] { worker_loop(i); });
        }
    }

    ~BusinessThreadPool() {
        stop.store(true, std::memory_order_release);

        // 等待任务完成
        while (pending_tasks.load(std::memory_order_acquire) > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        // 回收资源
        for (auto& w : workers) {
            if (w.joinable()) w.join();
        }
        //worker_queues.~AtomicUnorderedInsertMap();
    }

    bool enqueue(Task task) {
        if (stop.load(std::memory_order_acquire)) return false;

        auto task_ptr = std::make_shared<TaskWrapper>(std::move(task));
        if (global_queue.try_enqueue(task_ptr)) { // 非阻塞尝试写入
            pending_tasks.fetch_add(1, std::memory_order_relaxed);
            total_tasks.fetch_add(1, std::memory_order_relaxed);
            return true;
        }
        return false;
    }

private:
    void worker_loop(size_t worker_id) {
        WorkerQueue* local_queue = nullptr;
        {
            std::lock_guard<std::mutex> lock(map_mutex);
            auto it = worker_queues.find(worker_id);
            if (it == worker_queues.end()) {
                slog->critical("Fatal: Worker queue missing for {}", worker_id);
                return;
            }
            local_queue = it->second;
        }


        std::vector<TaskPtr> local_batch;
        local_batch.reserve(WORKER_BATCH_SIZE);

        int empty_iterations = 0;

        while (!stop.load(std::memory_order_acquire)) {
            // 处理本地队列
            size_t processed = 0;
            TaskPtr task;
            while (processed < WORKER_BATCH_SIZE && local_queue->queue.try_dequeue(task)) {
                execute_task(task);
                ++processed;
            }

            // 处理全局队列
            process_global_queue(local_batch);

            // 工作窃取
            int steal_attempts = try_work_stealing(worker_id);

            // 空闲管理
            handle_idle_state(processed, empty_iterations);
            
            // 状态检查
            check_system_status();
        }
    }

    size_t process_local_queue(WorkerQueue* local_queue) {
        size_t processed = 0;
        TaskPtr task;
        while (processed < WORKER_BATCH_SIZE) {
            if (local_queue->queue.try_dequeue(task)) {
                execute_task(task);
                ++processed;
            }
            else {
                break;
            }
        }
        return processed;
    }

    void process_global_queue(std::vector<TaskPtr>& local_batch) {
        TaskPtr task;

        // 批量从全局队列获取任务
        size_t count = global_queue.try_dequeue_bulk(
            std::back_inserter(local_batch),
            WORKER_BATCH_SIZE - local_batch.size()
        );

        for (auto& t : local_batch) {
            execute_task(t);
        }
        local_batch.clear();
    }

    int try_work_stealing(size_t worker_id) {
        constexpr int MAX_STEAL_BATCH = 16;
        std::array<TaskPtr, MAX_STEAL_BATCH> stolen_tasks;

        // 严格仅从全局队列窃取
        size_t count = global_queue.try_dequeue_bulk(stolen_tasks.begin(), MAX_STEAL_BATCH);
        for (size_t i = 0; i < count; ++i) {
            execute_task(stolen_tasks[i]);
        }
        return (count > 0) ? 0 : MAX_STEAL_ATTEMPTS;
    }

    // 空闲时休眠，避免CPU空转
    void handle_idle_state(size_t processed, int& empty_iterations) {
        if (processed == 0) {
            if (++empty_iterations > MAX_EMPTY_ITERATIONS) {
                std::this_thread::sleep_for(
                    std::chrono::microseconds(10 * empty_iterations)
                );
                empty_iterations = std::min(empty_iterations, 100);
            }
        }
        else {
            empty_iterations = 0;
        }
    }

    void execute_task(TaskPtr task) {
        if (!task) return;

        try {
            auto start = std::chrono::steady_clock::now();

            // 调用实际业务处理逻辑
            CObserverServer::instance().process_task(
                std::move(task->task)
            );

            // 性能监控
            auto end = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            if (duration.count() > 300) {
                slog->warn("Slow task: {}ms, package_id={}", duration.count(), task->task.package_id);
            }

            task->completed.store(true, std::memory_order_release);
            pending_tasks.fetch_sub(1, std::memory_order_relaxed);
        }
        catch (const std::exception& e) {
            slog->error("Task failed (package_id={}): {}", task->task.package_id, e.what());
        }
    }

    void check_system_status() {
        static std::atomic<int> check_counter{ 0 };
        if (check_counter++ % 1000 == 0) {
            check_memory_usage();
            log_status();
        }
    }

    void check_memory_usage() {
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
            double usage_mb = pmc.WorkingSetSize / 1024.0 / 1024.0;
            handle_memory_warning(usage_mb);
        }
    }

private:
    void handle_memory_warning(double usage_mb) {
        static auto last_gc = std::chrono::steady_clock::now();

        if (usage_mb > 2048) {
            slog->critical("Memory critical: {:.2f}MB", usage_mb);
            if (std::chrono::steady_clock::now() - last_gc > std::chrono::minutes(5)) {
                perform_memory_gc();
                last_gc = std::chrono::steady_clock::now();
            }
        }
        else if (usage_mb > 1024) {
            slog->warn("Memory high: {:.2f}MB", usage_mb);
        }
    }

    void perform_memory_gc() {
        // 示例内存回收逻辑
        size_t cleaned = 0;
        // 这里添加实际的内存回收逻辑
        slog->info("Performed memory GC, cleaned {} items", cleaned);
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
        slog->info("ProtocolLC2LSSend package_id: {}", package_id);
        ProtocolLC2LSSend req;
        req.package = package;
        req.package.head.session_id = session->hash_key();
        logic_client_->async_send(&req, package.head.session_id);
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
    package_mgr_.register_handler(OBPKG_ID_C2S_AUTH, [this](tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
    #ifndef _DEBUG
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
        slog->info("查询所有用户数据");
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