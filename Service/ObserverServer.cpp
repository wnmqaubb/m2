#include "pch.h"

// 项目自有头文件
#include "ObserverServer.h"
#include "LogicClient.h"
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
//#include <concurrentqueue/concurrentqueue.h>
//#include <readerwriterqueue/readerwriterqueue.h>
//#include <parallel_hashmap/phmap.h>

// Windows头文件（置于最后以避免宏冲突）
#include <Psapi.h>
#include <WinBase.h>
#include "ThreadPool.h"
//slog->info(" {}:{}:{}", __FUNCTION__, __FILE__, __LINE__);
// 转发到logic_server

bool CObserverServer::on_recv(unsigned int package_id, tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, msgpack::v1::object_handle&& raw_msg) {
    if (!session || session->is_stopped()) {
        slog->warn("Enqueue task for stopped session");
        return true;
    }
    // 在创建 Task 前检查传入的 raw_msg 是否为空
    if (raw_msg.get().is_nil()) {
        slog->warn("on_recv received a nil msgpack handle for package_id: {}", package_id);
        return true;
    }
    try {
        // 使用 weak_ptr 代替 shared_ptr
        std::weak_ptr<asio2::tcp_session> weak_session = session;
        if (weak_session.expired()) {
            slog->warn("Enqueue task for expired session");
            return true;
        }
        // weak_ptr 仍然有效，可以继续使用 session
        auto local_session = weak_session.lock();
        // 1. 使用 shared_ptr 包装所有不可复制对象
        //auto pkg = std::move(package);
        //auto raw_msg_local = std::move(raw_msg);
        // 提交任务到 BS::thread_pool
        //get_global_thread_pool().detach_task([
        //local_session->post([
            //this,session = session->weak_from_this(),
            //pkg_id = package_id, pkg = std::move(pkg),
            //raw_msg = std::make_shared<msgpack::v1::object_handle>(std::move(raw_msg_local))]() mutable {

            if (local_session) {
                // 使用移动语义传递msgpack对象
                process_task(package_id, local_session, std::move(package), std::move(raw_msg));
            }
        //});

        return true;
    }
    catch (const std::exception& e) {
        slog->error("Exception in on_recv: {}", e.what());
        return true;
    }
}

void CObserverServer::process_task(unsigned int package_id, tcp_session_shared_ptr_t session, const RawProtocolImpl&& package, msgpack::v1::object_handle&& raw_msg)
{
    if (raw_msg.get().is_nil()) {
        slog->warn("Invalid msgpack handle detected");
        return;
    }
    // ========== 关键修改 1：统一会话有效性检查 ==========
    tcp_session_shared_ptr_t local_session;

    // 使用 weak_ptr 检测会话有效性
    std::weak_ptr<asio2::tcp_session> weak_session = session;
    local_session = weak_session.lock();
    if (!local_session) {
        slog->warn("Session expired in task processing");
        return;
    }

    if (package_id == PackageId::PKG_ID_C2S_HANDSHAKE)
    {
        auto msg = raw_msg.get().as<ProtocolC2SHandShake>();
        on_recv_handshake(local_session, package, msg);
        return;
    }

    if (package_id == PackageId::PKG_ID_C2S_HEARTBEAT)
    {
        auto msg = raw_msg.get().as<ProtocolC2SHeartBeat>();

        ProtocolS2CHeartBeat resp;
        resp.tick = msg.tick;
        async_send(local_session, &resp);

        // ========== 关键修改 2：安全访问 userdata ==========
        AntiCheatUserData* userdata = nullptr;

        // 方法 1：双重有效性检查
        if (local_session && !local_session->is_stopped()) {
            userdata = get_user_data_(local_session);
        }

        if (userdata) {
            userdata->update_heartbeat_time();
        }
        else {
            slog->warn("Failed to get userdata for session");
            return;
        }

        on_recv_client_heartbeat(local_session); // 使用 local_session
        return;
    }
    AntiCheatUserData* user_data = nullptr;
    if (SPKG_ID_START < package_id && package_id < SPKG_ID_MAX_PACKAGE_ID_SIZE) {
        // 减少包处理数量,临时用,待登录器更新后移除
        if (SPKG_ID_C2S_UPDATE_USER_NAME == package_id) {
            user_data = get_user_data_(local_session);
            auto& req = raw_msg.get().as<ProtocolC2SUpdateUsername>();
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
        req.package.head.session_id = local_session->hash_key();
        logic_client_->async_send(&req, package.head.session_id);
        return;
    }

    if (!user_data)
    {
        user_data = get_user_data_(local_session);
    }

    // auto is_observer_client = user_data->get_field<bool>("is_observer_client");
    // if (!(is_observer_client.has_value() && is_observer_client.value()))
    // {
    //     super::log(LOG_TYPE_ERROR, TEXT("未授权Service调用"));
    //     return;
    // }

    if (OBSPKG_ID_START < package_id && package_id < OBSPKG_ID_END) {
        // 执行任务
        //try {
            // 调用 dispatch
            // 1. 复制必要数据（避免移动后失效）
            unsigned int local_pkg_id = package_id;
            RawProtocolImpl local_pkg = std::move(package); // ✅ 明确转移所有权
            ob_pkg_mgr_.dispatch(local_pkg_id, local_session, local_pkg, std::move(raw_msg));
        /*}
        catch (const std::exception& e) {
            slog->error("Package dispatch failed: {}", e.what());
        }
        catch (...) {
            slog->error("Package dispatch failed with unknown exception");
        }*/
    }
    else if (LSPKG_ID_START < package_id && package_id < LSPKG_ID_END) {
        ProtocolLC2LSSend req;
        req.package = package;
        req.package.head.session_id = local_session->hash_key();
        logic_client_->async_send(&req, package.head.session_id);
        return;
    }
    return;
}
// 优化后的心跳处理(分批+无锁)
void CObserverServer::batch_heartbeat(const std::vector<tcp_session_shared_ptr_t>& sessions) {

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
                    logic_client_->async_send(&req);
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
                    logic_client_->async_send(&req);
                }
            }
        });
    });
    logic_client_->package_mgr().register_handler(LSPKG_ID_S2C_SEND, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        auto resp = msg.get().as<ProtocolLS2LCSend>();
        auto session = find_session(resp.package.head.session_id);
        if (!session)
        {
            slog->error("LSPKG_ID_S2C_SEND Session not found");
            return;
        }
        async_send(session, resp.package);
    });
    logic_client_->package_mgr().register_handler(LSPKG_ID_S2C_SET_FIELD, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        auto param = msg.get().as<ProtocolLS2LCSetField>();
        auto session = find_session(param.session_id);
        if (!session)
        {
            slog->error("LSPKG_ID_S2C_SET_FIELD Session not found");
            return;
        }
        get_user_data_(session)->set_field(param.key, param.val);
        // 发给管理员网关
        if (get_user_data_(session)->get_field<bool>("is_observer_client"))
        {
            ProtocolOBS2OBCSetField req;
            req.session_id = param.session_id;
            req.key = param.key;
            req.val = param.val;
            async_send(session, &req);
        }
    });
    // 踢人 CLogicServer::close_socket
    logic_client_->package_mgr().register_handler(LSPKG_ID_S2C_KICK, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        auto param = msg.get().as<ProtocolLS2LCKick>();
        auto session = find_session(param.session_id);
        if (!session)
        {
            slog->error("LSPKG_ID_S2C_KICK Session not found");
            return;
        }
        session->stop();
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
        auto session1 = find_session(req.package.head.session_id);
        if (!session1)
        {
            slog->error("OBPKG_ID_C2S_SEND Session not found");
            return;
        }
        async_send(session1, req.package, session->hash_key());
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
            if (user_data && user_data->get_handshake()) {
                ProtocolUserData _userdata;
                _userdata.session_id = session->hash_key();
                if (user_data->uuid) {
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
        async_send(session, &resp);
    });
    ob_pkg_mgr_.register_handler(OBPKG_ID_C2S_UPDATE_LOGIC, [this](tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
        auto buf = raw_msg.get().as<ProtocolOBC2OBSUpdateLogic>().data;
        ProtocolLC2LSClose req;
        logic_client_->async_send(&req);
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
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            super::log(LOG_TYPE_EVENT, TEXT("更新LogicServer成功"));
        }
    });
    user_notify_mgr_.register_handler(CLIENT_HANDSHAKE_NOTIFY_ID, [this](tcp_session_shared_ptr_t& session) {
        ProtocolLC2LSAddUsrSession req;
        auto user_data = get_user_data_(session);
        ProtocolUserData& _userdata = req.data;
        if (auto handshake =user_data->get_handshake())
        {
            _userdata.session_id = session->hash_key();
            if (user_data && user_data->uuid) {
                memcpy(_userdata.uuid, user_data->uuid, sizeof(_userdata.uuid));
            }
            else {
                memset(_userdata.uuid, 0, sizeof(_userdata.uuid));
            }
            _userdata.has_handshake = handshake;
            _userdata.last_heartbeat_time = user_data->last_heartbeat_time;
            _userdata.json = user_data->data;
            logic_client_->async_send(&req);
        }

    });
}

// OBPKG_ID_C2S_AUTH
void CObserverServer::obpkg_id_c2s_auth(tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg)
{
    if (!session || session->is_stopped()) {
        return;
    }
#ifdef _DEBUG        
    get_user_data_(session)->set_field("is_observer_client", true);
    super::log(LOG_TYPE_EVENT, TEXT("observer client auth success:%s:%u"), Utils::c2w(session->remote_address()).c_str(),
               session->remote_port());
    ProtocolOBS2OBCQueryVmpExpire resp;
    resp.vmp_expire = get_vmp_expire();
    slog->warn("vmp_expire: {}", Utils::w2c(resp.vmp_expire));
    async_send(session, &resp);
    ProtocolLC2LSAddObsSession req;
    req.session_id = session->hash_key();
    logic_client_->async_send(&req);
    ProtocolOBS2OBCAuth auth;
    auth.status = true;
    async_send(session, &auth);
    if (auth_lock_.load()) {
        auth_lock_.store(false);
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
        slog->warn("vmp_expire: {}", Utils::w2c(resp.vmp_expire));
        async_send(session, &resp);
        ProtocolLC2LSAddObsSession req;
        req.session_id = session->hash_key();
        logic_client_->async_send(&req);
        ProtocolOBS2OBCAuth auth;
        auth.status = true;
        async_send(session, &auth);
        // 发送有效期日期
        session->start_timer("query_vmp_expire", 2000, [this, resp, weak_session = std::weak_ptr(session)]() {
            if (auto session = weak_session.lock()) {
                async_send(session, &resp);
            }
        });
        if (auth_lock_.load()) {
            auth_lock_.store(false);
        }
    }
    else
    {
        get_user_data_(session)->set_field("is_observer_client", false);
        slog->info("授权观察者客户端失败 client_addr: {}:{}, session_id: {}",
                   session->remote_address(), session->remote_port(), session->hash_key());
        ProtocolOBS2OBCAuth auth;
        auth.status = false;
        async_send(session, &auth);
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
    heartbeat.tick = time(nullptr);
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
        logic_client_->async_send(&req);
    }

    //slog->info("移除用户session session_id: {}", session->hash_key());
    ProtocolLC2LSRemoveUsrSession req;
    req.data.session_id = session->hash_key();
    logic_client_->async_send(&req);
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
    log.text = msg;
    log.silence = silence;
    log.gm_show = gm_show;
    log.punish_flag = punish_flag;
    foreach_session([this, &log](tcp_session_shared_ptr_t& session) {
        if (get_user_data_(session)->get_field<bool>("is_observer_client").value_or(false))//admin网关
        {
            async_send(session, &log);
        }
    });
}
