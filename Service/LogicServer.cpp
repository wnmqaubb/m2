#include "pch.h"
#include "../version.build"
#include "AntiCheatServer.h"
#include "Ini_tool.h"
#include "LogicServer.h"
#include "NetUtils.h"
#include "ObServerPackage.h"
#include "Package.h"
#include "Protocol.h"
#include "ServicePackage.h"
#include "SubServicePackage.h"
#include "vmprotect/VMProtectSDK.h"
#include <chrono>
#include <clocale>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <functional>
#include <iomanip>
#include <iosfwd>
#include <map>
#include <memory>
#include <msgpack/v1/object.hpp>
#include <msgpack/v1/object_fwd.hpp>
#include <msgpack/v1/object_fwd_decl.hpp>
#include <msgpack/v1/pack.hpp>
#include <msgpack/v1/sbuffer.hpp>
#include <msgpack/v3/unpack.hpp>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog-inl.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <stdlib.h>
#include <string>
#include <system_error>
#include <thread>
#include <time.h>
#include <vector>
#include <WinBase.h>
#include <WinDef.h>
#include <WinError.h>
#include <spdlog/async_logger.h>
#include <spdlog/sinks/rotating_file_sink.h>
std::filesystem::path g_cur_dir;
using namespace std::literals;
#if 0
#define ENABLE_POLICY_TIMEOUT_CHECK
#define ENABLE_POLICY_TIMEOUT_CHECK_TIMES 3
#define ENABLE_DETAIL_USER_LOGIN_LOGOUT_LOG
#endif

#ifdef LOG_SHOW
#define LOG(x) OutputDebugStringA(x);
#else
#define LOG(x)
#endif
std::shared_ptr<spdlog::logger> slog;
std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> clog;
// 全局声明线程池和日志器
std::shared_ptr<spdlog::details::thread_pool> tp;
void init_logger();

int main(int argc, char** argv)
{
    SetUnhandledExceptionFilter(GlobalExceptionFilter);
    // 在 main 函数或初始化代码中注册 SEH 过滤器
    _se_translator_function old_seh = _set_se_translator([](unsigned code, EXCEPTION_POINTERS*) {
        throw std::runtime_error("SEH Exception: code=" + std::to_string(code));
    });
    init_logger();
    VMProtectBeginVirtualization(__FUNCTION__);
    // 创建互斥体，检查是否已存在实例
    HANDLE hMutex = CreateMutex(NULL, FALSE, TEXT("mtx_logic_server_2"));
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        // 互斥体创建失败或已存在实例，退出程序
        if (hMutex) CloseHandle(hMutex);
        return 0;
    }
    g_cur_dir = std::filesystem::path(argv[0]).parent_path();
    setlocale(LC_ALL, "");
    CLogicServer::instance().start(kDefaultLocalhost, kDefaultLogicServicePort);
    // 处理命令行参数
    if (argc >= 2)
    {
        unsigned int ppid = std::stoi(argv[1]); // 父进程ID
        if (argc >= 3)
        {
            char log_level = std::stoi(argv[2]); // 日志级别
            CLogicServer::instance().set_log_level(log_level);
        }

        // 打开父进程并等待其结束
        HANDLE phandle = OpenProcess(PROCESS_VM_OPERATION | SYNCHRONIZE, FALSE, ppid);
        if (phandle != NULL)
        {
            WaitForSingleObject(phandle, INFINITE);
            CloseHandle(phandle); // 释放句柄资源
        }

        slog->info("logic_server stop");
        CLogicServer::instance().stop();
    }
#ifndef _DEBUG
    CLogicServer::instance().set_log_level(LOG_TYPE_EVENT | LOG_TYPE_ERROR);
#endif
    while (!CLogicServer::instance().is_stopped())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
    VMProtectEnd();
}

void init_logger()
{
    // 创建异步线程池（队列大小8192）
    tp = std::make_shared<spdlog::details::thread_pool>(8192, 2);
    // 创建控制台日志器
    clog = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    // 设置控制台日志器日志级别为错误级别
    clog->set_level(spdlog::level::err);
    //auto slog = spdlog::create_async<spdlog::sinks::daily_file_sink_mt>("logger", "logs/anti_cheat.log");
    // 初始化日志系统
    auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logic_server.log", 1024 * 1024 * 5, 1, true);

    // 创建组合日志器时使用 sink
    std::vector<spdlog::sink_ptr> sinks{ clog, rotating_sink };

    // 使用 sinks 创建异步日志器
    slog = std::make_shared<spdlog::async_logger>(
        "async_logger",
        sinks.begin(),
        sinks.end(),
        tp,
        spdlog::async_overflow_policy::block
    );
    // 日志格式添加线程ID和毫秒时间戳
#ifdef _DEBUG
    slog->set_level(spdlog::level::trace);
    slog->flush_on(spdlog::level::trace);
    spdlog::flush_every(std::chrono::seconds(3));
    spdlog::set_pattern("%^[%Y-%m-%d %H:%M:%S.%e][T%t][%l]%$ %v");
#else
    slog->set_level(spdlog::level::info);
    slog->flush_on(spdlog::level::info);
    spdlog::flush_every(std::chrono::seconds(5));
    // 设置日志消息的格式模式
    spdlog::set_pattern("%^[%m-%d %H:%M:%S][%l]%$ %v");
#endif

    // 注册为全局日志器
    spdlog::register_logger(slog);
    spdlog::set_default_logger(slog);

}

#define REGISTER_TRANSPORT(PKG_ID) \
{ \
    ob_pkg_mgr_.register_handler(PKG_ID, [this](tcp_session_shared_ptr_t& session, std::size_t ob_session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) { \
        if (!obs_sessions_mgr()->exist(ob_session_id)) return; \
        ProtocolOBS2OBCSend req; \
        req.package = package; \
        this->async_send(session, ob_session_id, &req); \
    }); \
}

enum LogicTimerId {
    DEFINE_TIMER_ID(PLUGIN_RELOAD_TIMER_ID),
    DEFINE_TIMER_ID(ONLINE_CHECK_TIMER_ID),
};

void CLogicServer::send_policy(std::shared_ptr<ProtocolUserData>& user_data, tcp_session_shared_ptr_t& session, std::size_t session_id)
{
    VMProtectBeginVirtualization(__FUNCTION__);
#ifdef ENABLE_POLICY_TIMEOUT_CHECK
    if (!user_data->policy_recv_timeout_timer_)
    {
        user_data->policy_recv_timeout_timer_ = std::make_shared<asio::steady_timer>(io().context());
    }
#endif
    async_send(session, session_id, &policy_mgr_->get_policy());
    user_data->last_send_policy_time = std::chrono::system_clock::now();
    user_data->add_send_policy_count();
#ifdef ENABLE_POLICY_TIMEOUT_CHECK

    auto uuid_str = user_data->get_uuid().str();
    auto local_session_id = user_data->session_id;
    user_data->policy_recv_timeout_timer_->expires_from_now(std::chrono::minutes(get_policy_detect_interval()));
    user_data->policy_recv_timeout_timer_->async_wait([this, user_data, service_session_id = session->hash_key(), uuid_str, local_session_id](std::error_code ec) {
        if (ec != asio::error::operation_aborted)
        {
            user_log(LOG_TYPE_EVENT, true, false, uuid_str, TEXT("策略收到超时:%u"), local_session_id);
            user_data->add_policy_timeout_times();
            if (user_data->get_policy_timeout_times() >= ENABLE_POLICY_TIMEOUT_CHECK_TIMES)
            {
                if (auto service_session = find_session(service_session_id))
                    close_socket(service_session, local_session_id);
                user_log(LOG_TYPE_EVENT, true, false, uuid_str, TEXT("策略收到严重超时，请手动处罚:%u"), local_session_id);
                //std::wstring usr_name = user_data->json.find("usrname") != user_data->json.end() ? user_data->json.at("usrname").get<std::wstring>() : L"(NULL)";
                //write_txt(".\\恶性开挂人员名单.txt", trim_user_name(Utils::w2c(usr_name)));
            }
        }
        else
        {
            user_data->clear_policy_timeout_times();
        }
    });
#endif
    VMProtectEnd();
}

bool CLogicServer::on_recv(unsigned int package_id, tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, msgpack::v1::object_handle&& raw_msg) {
    if (!session || session->is_stopped()) {
        slog->warn("Enqueue task for stopped session");
        return false;
    }
    // 在创建 Task 前检查传入的 raw_msg 是否为空
    if (raw_msg.get().is_nil()) {
        slog->warn("on_recv received a nil msgpack handle for package_id: {}", package_id);
        return false;
    }
    try {
        //auto pkg = std::move(package);
        //auto raw_msg_local = std::move(raw_msg);
        // 提交任务到 BS::thread_pool
        //get_global_thread_pool().detach_task([
        //session->post([
        //    this, session = session->weak_from_this(),
        //    pkg_id = package_id, pkg = std::move(pkg),
        //    raw_msg = std::make_shared<msgpack::v1::object_handle>(std::move(raw_msg_local))]() mutable {


                // 使用移动语义传递msgpack对象
                process_task(package_id, session, std::move(package), std::move(raw_msg));

        //});

        return true;
    }
    catch (const std::exception& e) {
        slog->error("Exception in on_recv: {}", e.what());
        return false;
    }
    catch (...) {
        slog->error("Exception in on_recv");
        return false;
    }

}

void CLogicServer::process_task(unsigned int package_id, tcp_session_shared_ptr_t session, const RawProtocolImpl&& package, msgpack::v1::object_handle&& raw_msg)
{
    if (raw_msg.get().is_nil()) {
        slog->warn("Invalid msgpack handle detected");
        return;
    }

    // 执行任务
    try {
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
            super::async_send(local_session, &resp);
            auto userdata = get_user_data_(local_session);
            userdata->update_heartbeat_time();
            //user_notify_mgr_.dispatch(CLIENT_HEARTBEAT_NOTIFY_ID, session);
            //on_recv_heartbeat(session, package, msg);
            return;
        }

        // 1. 复制必要数据（避免移动后失效）
        unsigned int local_pkg_id = package_id;
        RawProtocolImpl local_pkg = std::move(package); // ✅ 明确转移所有权
        auto raw_msg_ptr = std::make_shared<msgpack::v1::object_handle>(std::move(raw_msg));
        package_mgr_.dispatch(local_pkg_id, local_session, local_pkg, std::move(*raw_msg_ptr));
    }
    catch (const std::exception& e) {
        slog->error("Package dispatch failed: {}", e.what());
    }
    catch (...) {
        slog->error("Package dispatch failed with unknown exception");
    }
}

CLogicServer::CLogicServer():policy_detect_interval_(3)
{
    VMProtectBeginVirtualization(__FUNCTION__);
    is_logic_server_ = true;
    plugin_mgr_ = std::make_shared<CServerPluginMgr>();
    policy_mgr_ = std::make_shared<CServerPolicyMgr>();
    obs_sessions_mgr_ = std::make_shared<CObsSessionMgr>();
    usr_sessions_mgr_ = std::make_shared<CSessionMgr>();
    set_log_cb(std::bind(&CLogicServer::log_cb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    // 简化定时器启动代码，并添加异常处理
    auto reload_plugins_and_policies = [this]() {
        try {
            plugin_mgr_->reload_all_plugin();
            policy_mgr_->reload_all_policy();
            policy_mgr_->on_policy_reload();
        }
        catch (const std::exception& e) {
            slog->error("重新加载插件和策略时出错：{}", e.what());
        }
        catch (...) {
            slog->error("重新加载插件和策略时发生未知错误");
        }
    }; 
    start_timer(PLUGIN_RELOAD_TIMER_ID, std::chrono::seconds(5), reload_plugins_and_policies);
    start_timer(ONLINE_CHECK_TIMER_ID, std::chrono::seconds(40), [this]() {
        try {
            OnlineCheck();
        }
        catch (...) {
            slog->error("在线检查出错");
        }
    });
    package_mgr_.register_handler(LSPKG_ID_C2S_CLOSE, [this](tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        this->stop();
        exit(0);
    });
    package_mgr_.register_handler(LSPKG_ID_C2S_ADD_OBS_SESSION, [this](tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        auto req = msg.get().as<ProtocolLC2LSAddObsSession>();
        obs_sessions_mgr()->add_session(req.session_id);
        set_field(session, req.session_id, "logic_ver", TEXT(FILE_VERSION_STR));
        //slog->debug("LSPKG_ID_C2S_ADD_OBS_SESSION {} {}", req.session_id, session->hash_key());
    });
    package_mgr_.register_handler(LSPKG_ID_C2S_REMOVE_OBS_SESSION, [this](tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        auto req = msg.get().as<ProtocolLC2LSRemoveObsSession>();
        obs_sessions_mgr()->remove_session(req.session_id);
        //slog->debug("LSPKG_ID_C2S_REMOVE_OBS_SESSION {} {}", req.session_id, session->hash_key());
    });
    package_mgr_.register_handler(LSPKG_ID_C2S_ADD_USR_SESSION, [this](tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        try {
            auto req = msg.get().as<ProtocolLC2LSAddUsrSession>();
            usr_sessions_mgr()->add_session(req.data.session_id, req.data);
            //slog->debug("LSPKG_ID_C2S_ADD_USR_SESSION {} {}", req.data.session_id, session->hash_key());
            auto user_data = usr_sessions_mgr()->get_user_data(req.data.session_id);
            if (user_data) {
                std::wstring json_dump = Utils::c2w(user_data->json.dump());
                //user_log(LOG_TYPE_EVENT, true, false, user_data->get_uuid().str(), TEXT("建立连接:%s"), json_dump.c_str());
            }

            // 白名单用户不检测心跳和策略
            if (!is_svip(package.head.session_id)) {
                detect(session, req.data.session_id);
            }
        }
        catch (const std::exception& e) {
            slog->error("添加用户会话时出错：{}", e.what());
        }
        catch (...) {
            slog->error("添加用户会话时发生未知错误。");
        }
    });
    package_mgr_.register_handler(LSPKG_ID_C2S_REMOVE_USR_SESSION, [this](tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        auto req = msg.get().as<ProtocolLC2LSRemoveUsrSession>();
        auto user_data = usr_sessions_mgr()->get_user_data(req.data.session_id);
        if (user_data) {
        #if defined(ENABLE_DETAIL_USER_LOGIN_LOGOUT_LOG)
            user_log(LOG_TYPE_EVENT, true, false, user_data->get_uuid().str(), TEXT("断开连接:%d"), user_data->session_id);
        #endif
        #if defined(ENABLE_POLICY_TIMEOUT_CHECK)
            if (user_data->policy_recv_timeout_timer_) user_data->policy_recv_timeout_timer_->cancel();
        #endif
        }
        usr_sessions_mgr()->remove_session(req.data.session_id);
        //slog->debug("LSPKG_ID_C2S_REMOVE_USR_SESSION {} {}", req.data.session_id, session->hash_key());
    });
    // service 转发玩家的包
    package_mgr_.register_handler(LSPKG_ID_C2S_SEND, [this](tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
         try {
             auto req = msg.get().as<ProtocolLC2LSSend>().package;
             auto raw_msg = msgpack::unpack((char*)req.body.buffer.data(), req.body.buffer.size());
             if (raw_msg.get().type != msgpack::type::ARRAY) throw msgpack::type_error();
             if (raw_msg.get().via.array.size < 1) throw msgpack::type_error();
             if (raw_msg.get().via.array.ptr[0].type != msgpack::type::POSITIVE_INTEGER) throw msgpack::type_error();
             const auto package_id = raw_msg.get().via.array.ptr[0].as<unsigned int>();
             if (package_id == LSPKG_ID_C2S_SEND)
             {
                 slog->info("嵌套转发");
                 return;
             }
             auto user_data = usr_sessions_mgr()->get_user_data(package.head.session_id);
             if (user_data)
             {
                 user_data->pkg_id_time_map()[package_id] = std::chrono::system_clock::now();
             }

             if (!policy_pkg_mgr_.dispatch(package_id | package.head.session_id, session, package.head.session_id, req, raw_msg))
                 ob_pkg_mgr_.dispatch(package_id, session, package.head.session_id, req, raw_msg);
         }
         catch (const std::exception& e) {
             slog->error("LSPKG_ID_C2S_SEND：{}", e.what());
         }
         catch (...) {
             slog->error("LSPKG_ID_C2S_SEND");
         }
     });

    REGISTER_TRANSPORT(SPKG_ID_C2S_CHECK_PLUGIN);
    REGISTER_TRANSPORT(SPKG_ID_C2S_QUERY_PROCESS);
    REGISTER_TRANSPORT(SPKG_ID_C2S_QUERY_DRIVERINFO);
    REGISTER_TRANSPORT(SPKG_ID_C2S_QUERY_WINDOWSINFO);
    REGISTER_TRANSPORT(SPKG_ID_C2S_QUERY_SCREENSHOT);
    ob_pkg_mgr_.register_handler(PKG_ID_C2S_HEARTBEAT, [this](tcp_session_shared_ptr_t& session, unsigned int ob_session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        try {
            auto user_data = usr_sessions_mgr()->get_user_data(package.head.session_id);
            if (user_data)
            {
                user_data->last_heartbeat_time = std::chrono::system_clock::now();
                set_policy_detect_interval(IniTool::read_ini<int>(".\\jishiyu.ini", "Gate", "Policy_Detect_Interval", 3));
                //slog->debug("set_policy_detect_interval：{}", get_policy_detect_interval());
                if (user_data->get_send_policy_duration() > std::chrono::minutes(get_policy_detect_interval()))
                {
                    // 白名单用户不检测心跳和策略
                    if (is_svip(package.head.session_id)) {
                        slog->debug("is_svip ok：{}", package.head.session_id);
                        return;
                    }
                    detect(session, package.head.session_id);
                    send_policy(user_data, session, package.head.session_id);
                    /*
                    if (user_data->has_been_check_pkg())
                    {
                        if (user_data->pkg_id_time_map().count(689060) == 0)
                        {
                            user_log(LOG_TYPE_EVENT, true, false, user_data->get_uuid().str(), TEXT("水仙超时:%d"), user_data->session_id);
                            //write_txt(".\\恶性开挂人员名单.txt", trim_user_name(Utils::w2c(usr_name)));
                        }
                        else
                        {
                            auto dura_689060 = std::chrono::system_clock::now() - user_data->pkg_id_time_map()[689060];
                            if (dura_689060 > std::chrono::minutes(get_policy_detect_interval() + 2))
                            {
                                user_log(LOG_TYPE_EVENT, true, false, user_data->get_uuid().str(), TEXT("689060超时:%d s"), std::chrono::duration_cast<std::chrono::seconds>(dura_689060).count());
                            }
                        }

                        if (user_data->pkg_id_time_map().count(689051) == 0)
                        {
                            slog->debug("机器码收到超时:{}", user_data->session_id);
                            user_log(LOG_TYPE_EVENT, true, false, user_data->get_uuid().str(), TEXT("机器码收到超时:%d"), user_data->session_id);
                            //close_socket(session, user_data->session_id);
                            //std::wstring usr_name = user_data->json.find("usrname") != user_data->json.end() ? user_data->json.at("usrname").get<std::wstring>() : L"(NULL)";
                            //write_txt(".\\恶性开挂人员名单.txt", trim_user_name(Utils::w2c(usr_name)));
                        }
                        else
                        {
                            auto dura_689051 = std::chrono::system_clock::now() - user_data->pkg_id_time_map()[689051];
                            if (dura_689051 > std::chrono::minutes(get_policy_detect_interval() + 2))
                            {
                                auto t689051 = std::chrono::duration_cast<std::chrono::seconds>(dura_689051).count();
                                slog->debug("689051超时：{}s", t689051);
                                user_log(LOG_TYPE_EVENT, true, false, user_data->get_uuid().str(), TEXT("689051超时:%d s"), t689051);
                                // std::wstring usr_name = user_data->json.find("usrname") != user_data->json.end() ? user_data->json.at("usrname").get<std::wstring>() : L"(NULL)";
                            }
                        }

                        //if (user_data->pkg_id_time_map().count(SPKG_ID_C2S_QUERY_PROCESS) == 0)
                        //{
                        //    slog->debug("查看进程收到超时：{}s", user_data->session_id);
                        //    user_log(LOG_TYPE_EVENT, true, false, user_data->get_uuid().str(), TEXT("查看进程收到超时:%d"), user_data->session_id);
                        //    //close_socket(session, user_data->session_id);
                        //    //std::wstring usr_name = user_data->json.find("usrname") != user_data->json.end() ? user_data->json.at("usrname").get<std::wstring>() : L"(NULL)";
                        //    //write_txt(".\\恶性开挂人员名单.txt", trim_user_name(Utils::w2c(usr_name)));
                        //}
                    }
                    else
                    {
                        user_data->has_been_check_pkg(true);
                        /*ProtocolS2CQueryProcess req;
                        async_send(session, package.head.session_id, &req);
                        slog->debug("下发查看进程：{}", user_data->session_id);
                        user_log(LOG_TYPE_EVENT, true, false, user_data->get_uuid().str(), TEXT("下发查看进程:%d"), user_data->session_id);
                    }*/
                }
            }
        }
        catch (const std::exception& e) {
            slog->error("PKG_ID_C2S_HEARTBEAT：{}", e.what());
        }
        catch (...) {
            slog->error("PKG_ID_C2S_HEARTBEAT");
        }
    });
    // 查询插件列表 客户端 --> service --> logicserver
    ob_pkg_mgr_.register_handler(SPKG_ID_C2S_QUERY_PLUGIN, [this](tcp_session_shared_ptr_t& session, unsigned int ob_session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        auto plugin_hash_set = plugin_mgr_->get_plugin_hash_set();
        ProtocolS2CQueryPlugin resp(plugin_hash_set);
        async_send(session, package.head.session_id, &resp);
    });

    // 请求下载插件 客户端 --> service --> logicserver
    ob_pkg_mgr_.register_handler(SPKG_ID_C2S_DOWNLOAD_PLUGIN, [this](tcp_session_shared_ptr_t& session, unsigned int ob_session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        auto& req = msg.get().as<ProtocolC2SDownloadPlugin>();
        auto resp = plugin_mgr_->get_plugin(req.plugin_hash);
        if (!resp.body.buffer.empty())
        {
            auto user_data = usr_sessions_mgr()->get_user_data(package.head.session_id);
            if (user_data && !user_data->is_loaded_plugin())
            async_send(session, package.head.session_id, plugin_mgr_->get_plugin(req.plugin_hash));
        }
    });
    ob_pkg_mgr_.register_handler(SPKG_ID_S2C_LOADED_PLUGIN, [this](tcp_session_shared_ptr_t& session, unsigned int ob_session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        auto user_data = usr_sessions_mgr()->get_user_data(package.head.session_id);
        if (user_data)
        {
            user_data->set_loaded_plugin(true);
            // 白名单用户不检测心跳和策略
            if (is_svip(package.head.session_id)) {
                return;
            }
            // 延迟发送策略,防止用户还在登录
            post([user_data, session, session_id = package.head.session_id, this]() mutable {
                send_policy(user_data, session, session_id);
            }, std::chrono::seconds(15));
            detect(session, package.head.session_id);
        }
    });
    ob_pkg_mgr_.register_handler(SPKG_ID_C2S_UPDATE_USER_NAME, [this](tcp_session_shared_ptr_t& session, unsigned int ob_session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        auto& req = msg.get().as<ProtocolC2SUpdateUsername>();
        auto user_data = usr_sessions_mgr()->get_user_data(package.head.session_id);
        auto user_name = (user_data && user_data->json.contains("usrname") ? user_data->json.at("usrname").get<std::wstring>().c_str() : L"");
        //wprintf(L"usrname %s==%s\n", user_name, req.username.c_str());
        if (user_data && !req.username.empty() && req.username != user_name)
        {
            //wprintf(L"set_field=======\n");
            set_field(session, package.head.session_id, "usrname", req.username);
            user_data->json["usrname"] = req.username;
            //user_log(LOG_TYPE_EVENT, true, false, user_data->get_uuid().str(), TEXT("用户登录:%s sid:[%d]"), req.username.c_str(), user_data->session_id);
        }
    });
    ob_pkg_mgr_.register_handler(SPKG_ID_C2S_TASKECHO, [this](tcp_session_shared_ptr_t& session, unsigned int ob_session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        try {
            auto& req = msg.get().as<ProtocolC2STaskEcho>();
            auto policy = policy_mgr_->find_policy(req.task_id);
            std::wstring reason = Utils::c2w(req.text);
            auto user_data = usr_sessions_mgr()->get_user_data(package.head.session_id);
            if (user_data)
            {
                user_data->pkg_id_time_map()[req.task_id] = std::chrono::system_clock::now();
            }
        #if 0
            wchar_t buffer[1024];
            swprintf(buffer, 1024, TEXT("===>>玩家:%s 策略ID:%d 日志:%s"), user_data->json.at("usrname").get<std::wstring>().c_str(), req.task_id, reason.c_str());
            OutputDebugString(buffer);
            log(LOG_TYPE_EVENT, TEXT("玩家:%s 策略ID:%d 日志:%s"), user_data->json.at("usrname").get<std::wstring>().c_str(), req.task_id, req.text.c_str());
        #endif
            if (req.is_cheat && policy)
            {
                punish(session, package.head.session_id, *policy, reason);
                return;
            }

            if (user_data)
            {
                std::wstring usr_name = user_data->json.find("usrname") != user_data->json.end() ? user_data->json.at("usrname").get<std::wstring>() : L"(NULL)";
                std::wstring reason = Utils::c2w(req.text);
                bool gm_show = 688000 < req.task_id && req.task_id < 689051;
                bool silence = req.task_id != 0;

                if (req.task_id == 689999 && req.is_cheat)
                {
                    //write_txt(".\\恶性开挂人员名单.txt", trim_user_name(Utils::w2c(usr_name)));
                    user_log(LOG_TYPE_EVENT, silence, false, user_data->get_uuid().str(), TEXT("玩家:%s 策略ID:%d 日志:多次脚本错误定性为外挂,已写入恶性名单!"), usr_name.c_str(), req.task_id, reason.c_str());
                }

                //user_log(LOG_TYPE_EVENT, silence, gm_show, user_data->get_uuid().str(), TEXT("玩家:%s 策略ID:%d 日志:%s"), usr_name.c_str(), req.task_id, reason.c_str());
            #if defined(ENABLE_POLICY_TIMEOUT_CHECK)
                if (user_data->policy_recv_timeout_timer_) user_data->policy_recv_timeout_timer_->cancel();
            #endif
            }
        }
        catch (const std::exception& e) {
            slog->error("SPKG_ID_C2S_TASKECHO：{}", e.what());
        }
        catch (...) {
            slog->error("SPKG_ID_C2S_TASKECHO");
        }
    });
    ob_pkg_mgr_.register_handler(SPKG_ID_C2S_POLICY, [this](tcp_session_shared_ptr_t& session, unsigned int ob_session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        try {
            auto& req = msg.get().as<ProtocolC2SPolicy>();
            auto user_data = usr_sessions_mgr()->get_user_data(package.head.session_id);
        #if defined(ENABLE_POLICY_TIMEOUT_CHECK)
            if (user_data)
            {
                if (user_data->policy_recv_timeout_timer_)
                {
                    user_data->policy_recv_timeout_timer_->cancel();
                }
            }
        #endif
            ProtocolC2SPolicy resp;
            if (req.results.size())
            {
                for (auto row : req.results)
                {
                    auto policy = policy_mgr_->find_policy(row.policy_id);
                    if (policy)
                    {
                        punish(session, package.head.session_id, *policy, row.information);
                        if (policy->punish_type == PunishType::ENM_PUNISH_TYPE_KICK) {
                            return;
                        }
                    }
                    else
                    {
                        resp.results.push_back(row);
                    }
                }
            }
            if (resp.results.size())
            {
                ProtocolOBS2OBCSend board_cast;
                msgpack::sbuffer sbuf;
                msgpack::pack(sbuf, resp);
                board_cast.package.encode(sbuf.data(), sbuf.size());

                foreach_session([this, &board_cast](tcp_session_shared_ptr_t& session) {
                    for (auto session_id : obs_sessions_mgr()->sessions())
                    {
                        async_send(session, session_id, &board_cast);
                    }
                });
            }
        }
        catch (const std::exception& e) {
            slog->error("SPKG_ID_C2S_POLICY：{}", e.what());
        }
        catch (...) {
            slog->error("SPKG_ID_C2S_POLICY");
        }
    });
    REGISTER_TRANSPORT(SPKG_ID_C2S_RMC_CREATE_CMD);
    REGISTER_TRANSPORT(SPKG_ID_C2S_RMC_DOWNLOAD_FILE);
    REGISTER_TRANSPORT(SPKG_ID_C2S_RMC_UPLOAD_FILE);
    REGISTER_TRANSPORT(SPKG_ID_C2S_RMC_ECHO);
    ob_pkg_mgr_.register_handler(LSPKG_ID_C2S_UPLOAD_CFG, [this](tcp_session_shared_ptr_t& session, unsigned int ob_session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        auto& req = msg.get().as<ProtocolOBC2LSUploadConfig>();
        policy_mgr_->create_policy_file(req.file_name, req.data);
        log(LOG_TYPE_EVENT, TEXT("更新策略:%s"), Utils::c2w(req.file_name).c_str());
    });
    ob_pkg_mgr_.register_handler(LSPKG_ID_C2S_REMOVE_CFG, [this](tcp_session_shared_ptr_t& session, unsigned int ob_session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        auto& req = msg.get().as<ProtocolOBC2LSRemoveConfig>();
        policy_mgr_->remove_policy_file(req.file_name);
        log(LOG_TYPE_EVENT, TEXT("卸载策略:%s"), Utils::c2w(req.file_name).c_str());
    });
    ob_pkg_mgr_.register_handler(LSPKG_ID_C2S_UPLOAD_PLUGIN, [this](tcp_session_shared_ptr_t& session, unsigned int ob_session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        auto& req = msg.get().as<ProtocolOBC2LSUploadPlugin>();
        plugin_mgr_->create_plugin_file(req.file_name, req.data);
        log(LOG_TYPE_EVENT, TEXT("更新插件:%s"), Utils::c2w(req.file_name).c_str());
    });
    ob_pkg_mgr_.register_handler(LSPKG_ID_C2S_REMOVE_PLUGIN, [this](tcp_session_shared_ptr_t& session, unsigned int ob_session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        auto& req = msg.get().as<ProtocolOBC2LSRemovePlugin>();
        plugin_mgr_->remove_plugin_file(req.file_name);
        log(LOG_TYPE_EVENT, TEXT("卸载插件:%s"), Utils::c2w(req.file_name).c_str());
    });
    ob_pkg_mgr_.register_handler(LSPKG_ID_C2S_ADD_LIST, [this](tcp_session_shared_ptr_t& session, unsigned int ob_session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        auto& req = msg.get().as<ProtocolOBC2LSAddList>();
        write_txt(req.file_name, req.text, true);
    });
    ob_pkg_mgr_.register_handler(LSPKG_ID_C2S_CLEAR_LIST, [this](tcp_session_shared_ptr_t& session, unsigned int ob_session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        auto& req = msg.get().as<ProtocolOBC2LSClearList>();
        clear_txt(req.file_name);
    });
    VMProtectEnd();
}

void CLogicServer::clear_txt(const std::string& file_name)
{
    std::ofstream list(file_name, std::ios::out);
    if (list.is_open())
    {
        list << "";
        list.close();
        log(LOG_TYPE_EVENT, TEXT("清空列表[%s]"), Utils::c2w(file_name).c_str());
    }
}
class counter
{
public:
    counter(std::chrono::system_clock::duration dura, int limit)
        :dura_(dura), count_(0), limit_(limit), last_tick_(std::chrono::system_clock::now())
    {
    }
    std::chrono::system_clock::duration get_duration()
    {
        return std::chrono::system_clock::now() - last_tick_;
    }
    void update_last_tick()
    {
        last_tick_ = std::chrono::system_clock::now();
    }
    bool count(std::function<void()> cb)
    {
        bool r = false;
        count_++;
        if (count_ > limit_)
        {
            if (get_duration() > dura_)
            {
                cb();
                r = true;
            }
            count_ = 0;
            update_last_tick();
        }
        return r;
    }
    std::chrono::system_clock::time_point last_tick_;
    const std::chrono::system_clock::duration dura_;
    int count_;
    const int limit_;
};

void CLogicServer::write_txt(const std::string& file_name, const std::string& str, bool is_from_add_list)
{
    static counter limit_counter(std::chrono::seconds(30), 5);
    static bool is_enable_write_txt = true;
    if (!is_from_add_list)
    {
        if (limit_counter.count([this, file_name = file_name]() {
            if (is_enable_write_txt == true)
            {
                clear_txt(file_name);
            }
            is_enable_write_txt = false;
        }))
        {
            log(LOG_TYPE_EVENT, TEXT("触发写入列表频率上限，暂时关闭写列表功能"));
            return;
        }
        if (!is_enable_write_txt)
            return;
    }

    std::ofstream list(file_name, std::ios::out | std::ios::app);
    if (list.is_open())
    {
        list << str << std::endl;
        list.close();
        log(LOG_TYPE_EVENT, TEXT("加入列表[%s]:%s"), Utils::c2w(file_name).c_str(), Utils::c2w(str).c_str());
    }
}

void CLogicServer::write_img(std::size_t session_id, std::vector<uint8_t>& data)
{
    namespace fs = std::filesystem;
    try
    {
        auto now_time = std::time(nullptr);
        std::tm tm_;
        localtime_s(&tm_, &now_time);

        std::stringstream ss_date;
        ss_date << std::put_time(&tm_, "%Y-%m-%d");
        std::wstring date_str = Utils::c2w(ss_date.str());

        std::stringstream ss_time;
        ss_time << std::put_time(&tm_, "%H:%M:%S");
        std::wstring time_str = Utils::c2w(ss_time.str());

        fs::path save_dir = g_cur_dir / L"log" / date_str;
        std::error_code ec;
        if (fs::is_directory(save_dir, ec) == false)
        {
            fs::create_directories(save_dir, ec);
        }

        std::wstring user_name = L"未命名";
        auto user_data = usr_sessions_mgr()->get_user_data(session_id);
        if (user_data && user_data->json.find("usrname") != user_data->json.end()) {
            user_name = user_data->json.at("usrname").get<std::wstring>();
        }

        std::wstring file_name = user_name + L"_" + time_str + L".jpg";
        std::ofstream output(save_dir / file_name, std::ios::out | std::ios::binary);
        if (output.is_open())
        {
            output.write((char*)data.data(), data.size());
            output.close();
        }
    }
    catch (const std::exception& e) {
        slog->error("存储图片失败：{}", e.what());
    }
    catch (...) {
        slog->error("存储图片失败");
    }
}

void CLogicServer::log_cb(const wchar_t* msg, bool silence, bool gm_show, const std::string& identify, bool punish_flag)
{
    ProtocolLSLCLogPrint log;
    log.text = msg;
    log.identify = identify;
    log.silence = silence;
    log.gm_show = gm_show;
    log.punish_flag = punish_flag;
    foreach_session([this, &log](tcp_session_shared_ptr_t& session) {
        for (auto session_id : obs_sessions_mgr()->sessions())
        {
            async_send(session, session_id, &log);
        }
    });
}

void CLogicServer::punish(tcp_session_shared_ptr_t& session, std::size_t session_id, ProtocolPolicy& policy, const std::wstring& comment, const std::wstring& comment_2)
{
    static std::map<PunishType, wchar_t*> punish_type_str = {
       {ENM_PUNISH_TYPE_KICK,TEXT("退出游戏")},
       {ENM_PUNISH_TYPE_NO_OPEARATION,TEXT("不处理")},
       {ENM_PUNISH_TYPE_SUPER_WHITE_LIST,TEXT("白名单")},
       {ENM_PUNISH_TYPE_BAN_MACHINE,TEXT("封机器")},
    };
    static std::map<PolicyType, wchar_t*> policy_type_str = {
        {ENM_POLICY_TYPE_MODULE_NAME,TEXT("模块名检测")},
        {ENM_POLICY_TYPE_PROCESS_NAME,TEXT("进程名检测")},
        {ENM_POLICY_TYPE_FILE_NAME,TEXT("文件路径")},
        {ENM_POLICY_TYPE_WINDOW_NAME,TEXT("窗口名")},
        {ENM_POLICY_TYPE_MACHINE,TEXT("机器码")},
        {ENM_POLICY_TYPE_MULTICLIENT,TEXT("多开限制")},
        {ENM_POLICY_TYPE_SHELLCODE,TEXT("云代码")},
        {ENM_POLICY_TYPE_THREAD_START,TEXT("线程特征")}
    };
    try {
        auto user_data = usr_sessions_mgr()->get_user_data(session_id);
        bool gm_show = true;// 688000 < policy.policy_id && policy.policy_id < 689051;
        if (user_data)
        {
            VMProtectBeginVirtualization(__FUNCTION__);
            std::string ip = user_data->json.contains("ip") ? user_data->json.at("ip").get<std::string>() : "(NULL)";
            std::wstring usr_name = user_data->json.contains("usrname") ? user_data->json.at("usrname").get<std::wstring>() : L"(NULL)";

            // 未登录用户不处罚
            if (usr_name.empty() || usr_name == L"未登录用户" || usr_name == L"(NULL)") {
                return;
            }

            ProtocolPolicy while_list_policy;
            // 白名单用户不检测心跳和策略
            if (is_svip(session_id)) {
                user_log(LOG_TYPE_EVENT, true, gm_show, user_data->get_uuid().str(), TEXT("超级白名单玩家:%s 策略类型:%s 策略id:%d 处罚类型:%s 处罚原因:%s|%s"),
                         usr_name.c_str(),
                         policy_type_str[(PolicyType)policy.policy_type],
                         policy.policy_id,
                         punish_type_str[ENM_PUNISH_TYPE_NO_OPEARATION],
                         comment.c_str(),
                         comment_2.c_str()
                );
                return;
            }

            // 处罚玩家通知到gate和admin用来标红在线玩家列表
            ProtocolOBS2OBCPunishUserUUID resp;
            resp.uuid = Utils::c2w(user_data->get_uuid().str());
            foreach_session([this, &resp](tcp_session_shared_ptr_t& session) {
                for (auto session_id : obs_sessions_mgr()->sessions())
                {
                    async_send(session, session_id, &resp);
                }
            });

            // 处罚玩家写到GM的处罚玩家日志.log
            punish_log(TEXT("处罚玩家:%s 策略类型:%s 策略id:%d 处罚类型:%s 处罚原因:%s|%s"),
                    usr_name.c_str(),
                    policy_type_str[(PolicyType)policy.policy_type],
                    policy.policy_id,
                    punish_type_str[(PunishType)policy.punish_type],
                    comment.c_str(),
                    comment_2.c_str()
            );

            user_log(LOG_TYPE_EVENT, false, gm_show, user_data->get_uuid().str(), TEXT("处罚玩家:%s 策略类型:%s 策略id:%d 处罚类型:%s 处罚原因:%s|%s"),
                     usr_name.c_str(),
                     policy_type_str[(PolicyType)policy.policy_type],
                     policy.policy_id,
                     punish_type_str[(PunishType)policy.punish_type],
                     comment.c_str(),
                     comment_2.c_str()
            );

            if (policy.punish_type != ENM_PUNISH_TYPE_NO_OPEARATION)
            {
                if (user_data->get_punish_times() == 0)
                {
                    user_data->add_punish_times();
                }
                else
                {
                    if (user_data->get_last_punish_duration() > std::chrono::minutes(2))
                    {
                        user_data->add_punish_times();
                        user_data->update_last_punish_time();
                    }
                }

                if (user_data->get_punish_times() >= 3)
                {
                    user_log(LOG_TYPE_EVENT, true, false, user_data->get_uuid().str(), TEXT("处罚失效，请手动处罚:%d"), user_data->session_id);
                    std::wstring usr_name = user_data->json.find("usrname") != user_data->json.end() ? user_data->json.at("usrname").get<std::wstring>() : L"(NULL)";
                    //write_txt(".\\恶性开挂人员名单.txt", trim_user_name(Utils::w2c(usr_name)));
                }
            }
            VMProtectEnd();
            switch (policy.punish_type)
            {
                case ENM_PUNISH_TYPE_KICK:
                case ENM_PUNISH_TYPE_BAN_MACHINE:
                {
                    ProtocolS2CPunish resp;
                    resp.type = PunishType::ENM_PUNISH_TYPE_KICK;
                    async_send(session, session_id, &resp);
                    // 踢人后关闭socket连接，防止下次心跳检测到后又重新上线
                    // 在这里不能调用close_socket,会导致service死锁
                    //close_socket(session, user_data->session_id);
                    break;
                }
                default:
                    break;
            }
        }
        else
        {
            slog->error("处罚玩家失败");
        }
    }
    catch (const std::exception& e) {
        slog->error("punish：{}", e.what());
    }
    catch (...) {
        slog->error("error punish");
    }
}

bool CLogicServer::is_svip(std::size_t session_id) {
    VMProtectBeginVirtualization(__FUNCTION__);
    try {
        auto user_data = usr_sessions_mgr()->get_user_data(session_id);
        if (user_data)
        {
            auto mac = user_data->mac;
            std::string ip = user_data->json["ip"];
            std::wstring username = user_data->json.find("usrname") == user_data->json.end() ? TEXT("") : user_data->json["usrname"];
            if (!username.empty())
            {
                size_t pos = username.find(L" - ");
                if (pos != std::wstring::npos) {
                    username = username.substr(pos + 3);
                }
            }
            return policy_mgr_->is_svip(Utils::w2c(mac), ip, Utils::w2c(username));
        }
        return false;
    }
    catch (const std::exception& e) {
        slog->error("is_svip：{}", e.what());
    }
    catch (...) {
        slog->error("error is_svip");
    }
    return false;
    VMProtectEnd();
}

void CLogicServer::detect(tcp_session_shared_ptr_t& session, std::size_t session_id)
{
    VMProtectBeginVirtualization(__FUNCTION__);
    try {
        auto user_data = usr_sessions_mgr()->get_user_data(session_id);
        if (user_data)
        {
            auto mac = user_data->mac;
            std::string ip = user_data->json["ip"];
            std::wstring wstr_ip = Utils::c2w(ip);
            std::wstring username = user_data->json.find("usrname") == user_data->json.end() ? TEXT("") : user_data->json["usrname"];
            if (!username.empty())
            {
                size_t pos = username.find(L" - ");
                if (pos != std::wstring::npos) {
                    username = username.substr(pos + 3);
                }
            }
            // 检测多开
            auto cur_usr_machine_count = usr_sessions_mgr()->get_machine_count(mac);
            if (cur_usr_machine_count > policy_mgr_->get_multi_client_limit_count())
            {
                punish(session, session_id, policy_mgr_->get_multi_client_policy(), L"超出运行客户端限定");
                return;
            }
            ProtocolPolicy policy;
            if (policy_mgr_->is_ban(Utils::w2c(mac), ip, Utils::w2c(username)))
            {
                policy.policy_id = 689888;
                policy.policy_type = PolicyType::ENM_POLICY_TYPE_MACHINE;
                policy.punish_type = PunishType::ENM_PUNISH_TYPE_BAN_MACHINE;
                policy.config = mac + L"|" + wstr_ip + L"|" + username;
                punish(session, session_id, policy, L"封机器码|IP|角色名", policy.config);
            }
        }
    }
    catch (const std::exception& e) {
        slog->error("detect：{}", e.what());
    }
    catch (...) {
        slog->error("error detect");
    }
    VMProtectEnd();
}

void CLogicServer::close_socket(tcp_session_shared_ptr_t& session, std::size_t session_id)
{
    VMProtectBeginVirtualization(__FUNCTION__);
    ProtocolLS2LCKick req;
    req.session_id = session_id;
    super::async_send(session, &req);
    VMProtectEnd();
}
std::string CLogicServer::trim_user_name(const std::string& username_)
{
    std::string username = username_;
    if (username.empty() || username == "(NULL)")
    {
        return "";
    }
    size_t pos = username.find(" - ");
    if (pos != std::string::npos)
    {
        username.replace(pos, 3, "-");
    }
    return username;
}

void CLogicServer::OnlineCheck()
{
    VMProtectBeginVirtualization(__FUNCTION__);
    try
    {
        std::filesystem::path online_path = g_cur_dir;
        online_path /= "网关在线玩家.txt";
        std::ofstream online(online_path, std::ios::out | std::ios::binary | std::ios::trunc);

        std::string gamer, username;
        usr_sessions_mgr()->foreach_session([&username, &online](std::shared_ptr<ProtocolUserData>& user_data) {
            if (user_data->json.find("usrname") == user_data->json.end()) return;
            username = Utils::w2c(user_data->json.at("usrname").get<std::wstring>());
            if (username.empty() || username == "(NULL)")
            {
                return;
            }
            size_t pos = username.find(" - ");
            if (pos != std::string::npos)
            {
                username.replace(pos, 3, "-");
            }
            online << username << "\r\n";
        });

        online.flush();
        online.close();
    }
    catch (...)
    {
        slog->error("---网关在线玩家写入出错,请检测文件是否存在!");
    }
    VMProtectEnd();
}