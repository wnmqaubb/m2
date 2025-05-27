#include "pch.h"
#include "AntiCheatServer.h"
#include "NetUtils.h"

#define ENABLE_PROXY_TUNNEL 0
#if ENABLE_PROXY_TUNNEL
#include "ProxyTunnel.h"
#endif
#include "Protocol.h"
#include <WinBase.h>
#include <stdio.h>
#include <time.h>
#include <chrono>
#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <exception>
#include <functional>
#include <iomanip>
#include <iosfwd>
#include <iostream>
#include <mutex>
#include <string>
#include <type_traits>
#include <vadefs.h>
#include <asio2/base/error.hpp>
#include <asio2/base/impl/user_timer_cp.hpp>
#include <asio2/tcp/tcp_server.hpp>
#include <msgpack/v1/object_fwd.hpp>
#include "Package.h"
#ifdef G_SERVICE 
#include <ObServerPackage.h>
#include <ObServerServer.h>
#include <parallel_hashmap/phmap.h>
#endif
#include "ServicePackage.h"

#ifdef G_SERVICE 
static phmap::parallel_flat_hash_set<std::string> ddos_black_list;    // 并行哈希表黑名单
static phmap::parallel_flat_hash_map<std::string, uint32_t> ddos_black_map; // 并行哈希计数器
static std::shared_mutex ddos_black_list_mutex;  // 改用标准库的共享锁
static std::shared_mutex ddos_black_map_mutex;
#endif

inline BS::light_thread_pool& get_global_thread_pool() {
    static BS::light_thread_pool pool(std::max(4U, std::thread::hardware_concurrency())); // 非模板类，无需参数
    return pool;
}

CAntiCheatServer::CAntiCheatServer() : super(asio2::detail::tcp_frame_size, asio2::detail::max_buffer_size, std::thread::hardware_concurrency())
{
    bind_init(&CAntiCheatServer::on_init, this);
    bind_start(&CAntiCheatServer::on_start, this);
    bind_accept(&CAntiCheatServer::on_accept, this);
    bind_connect(&CAntiCheatServer::on_post_connect, this);
    bind_recv(&CAntiCheatServer::on_recv_package, this);
    bind_disconnect(&CAntiCheatServer::on_post_disconnect, this);
    bind_stop(&CAntiCheatServer::on_stop, this);

	//验证相关，serverless 云函数+数据库
	auth_check_timer_ = std::chrono::minutes(24 * 60);
    auth_url_ = "";//"http://service-4v2g0j35-1305025354.sh.apigw.tencentcs.com/release/check";
	auth_ticket_ = "1234";

	uuid_check_duration_ = std::chrono::seconds(90);
	heartbeat_check_duration_ = std::chrono::seconds(150);
	heartbeat_timeout_ = std::chrono::seconds(200);
	
    is_observer_server_ = false;
    is_logic_server_ = false;
    is_auth_success_ = false;
    set_log_level(LOG_TYPE_DEBUG | LOG_TYPE_EVENT | LOG_TYPE_ERROR);


}

CAntiCheatServer::~CAntiCheatServer()
{
    is_shutdown.store(true, std::memory_order_release);
    package_mgr_.clear_handler();
}

bool CAntiCheatServer::start(const std::string& listen_addr, int port)
{
    enable_proxy_tunnel(false);
    if (super::start(listen_addr, port, RawProtocolImpl()))
    {
        return true;
    }
    return false;
}

bool CAntiCheatServer::check_timer(bool slience)
{

    return false;
}

void CAntiCheatServer::on_init()
{
    log(LOG_TYPE_EVENT, TEXT("套接字初始化成功"));
#ifdef _DEBUG
    start_timer(MONITOR_TASKS_TIMER_ID,std::chrono::seconds(30), [this]() {
        slog->info("{} tasks total, {} tasks running, {} tasks queued.", 
                   get_global_thread_pool().get_tasks_total(),
                   get_global_thread_pool().get_tasks_running(),
                   get_global_thread_pool().get_tasks_queued());        
    });
#endif
}

void CAntiCheatServer::on_start()
{
    log(LOG_TYPE_EVENT, TEXT("开始监听:%s:%u "), Utils::c2w(listen_address()).c_str(), listen_port());
    notify_mgr_.dispatch(SERVER_START_NOTIFY_ID);
}

void CAntiCheatServer::on_accept(tcp_session_shared_ptr_t& session)
{
    try {
        std::string remote_address = session->get_remote_address();
        if (remote_address.empty()) return;
        unsigned short remote_port = session->get_remote_port();
#ifdef G_SERVICE 
        // 并行哈希表的读操作自动加共享锁
        if (ddos_black_list.contains(remote_address)) {
            session->socket().close(asio2::get_last_error());
            if (auto it = ddos_black_map.find(remote_address); it != ddos_black_map.end() && it->second > 0) {
                it->second -= 5;
                log(LOG_TYPE_ERROR, "accept 拦截ddos攻击黑名单IP:%s", remote_address.c_str());
                slog->warn("accept 拦截ddos攻击黑名单IP: {}", remote_address);
            }
        }
#endif
    }
    catch (const std::exception& e) {
        log(LOG_TYPE_ERROR, "on_accept: {}", e.what());
    }
    catch (...) {
        log(LOG_TYPE_ERROR, "on_accept 异常");
    }
}

void CAntiCheatServer::on_post_connect(tcp_session_shared_ptr_t& session)
{
    session->no_delay(true);
    session->start_timer((unsigned int)UUID_CHECK_TIMER_ID, uuid_check_duration_, [this, session]() {
        auto userdata = get_user_data_(session);
        if (userdata->get_handshake() == false)
        {
            session->socket().close(asio2::get_last_error());
            return;
        }
    });
}

void CAntiCheatServer::on_recv_package(tcp_session_shared_ptr_t& session, std::string_view sv)
{
    try
    {
        std::string remote_address;
        unsigned short remote_port;
        CHECK_SESSION(session);
        remote_address = session->get_remote_address();
        remote_port = session->get_remote_port();
        if (remote_address.empty()) return;
        get_global_thread_pool().detach_task(
            [this, session1 = session->weak_from_this(),sv = std::move(sv), remote_address, remote_port]() {
            auto session = session1.lock();
            if (!session) return;
        #ifdef G_SERVICE 
            // 认证锁,优先处理网关后台认证
            if (auth_lock_.load()) [[unlikely]] {
                std::shared_lock lock(mutex_);
                if (session->is_stopped()) return; // 检查会话状态
                if (remote_address == kDefaultLocalhost) {
                    _on_recv(session, std::move(sv), remote_address, remote_port);
                    return;
                }
            }
            else [[likely]] {
                if (ddos_black_list.contains(remote_address)) {
                    session->socket().close(asio2::get_last_error());
                    log(LOG_TYPE_ERROR, "on_recv 拦截ddos攻击黑名单IP:%s", remote_address.c_str());
                    slog->warn("on_recv 拦截ddos攻击黑名单IP: {}", remote_address);
                    return;
                }

                if (sv.empty()/* && remote_address != kDefaultLocalhost*/) {
                    // 使用try_emplace保证线程安全的插入/访问
                    auto [it, inserted] = ddos_black_map.try_emplace(remote_address, 0);

                    // 递增计数器
                    uint32_t count = ++(it->second);
                    // 当达到阈值时转移至黑名单
                    if (count >= 10) {
                        // 写操作需要独占锁保护
                        ddos_black_list.emplace(remote_address);

                        // 从map中移除（erase会自动加锁）
                        //ddos_black_map.erase(remote_address); 
                        log(LOG_TYPE_ERROR, "IP %s 触发阈值加入黑名单", remote_address.c_str());
                        return;
                    }
                    return;
                }
                // 关键点5：验证服务器实例有效性
                if (!is_shutdown.load(std::memory_order_acquire)) {
                    // 关键点6：使用拷贝后的数据
                    _on_recv(session, std::move(sv), remote_address, remote_port);
                }
            }
       #else
            // 关键点5：验证服务器实例有效性
            if (!is_shutdown.load(std::memory_order_acquire)) {
                // 关键点6：使用拷贝后的数据
                _on_recv(session, std::move(sv), remote_address, remote_port);
            }
        #endif
        });
    }
    catch (...)
    {
        //slog->info("收包解析异常: {}{} 长度:{}", session->get_remote_address(), session->remote_port(), sv.size());
        //log(LOG_TYPE_DEBUG, TEXT("收包解析异常:%s:%d 长度:%d"), Utils::c2w(session->get_remote_address()).c_str(), session->remote_port(), sv.size());
    }
}

void CAntiCheatServer::_on_recv(tcp_session_shared_ptr_t session, std::string_view sv,const std::string remote_address, unsigned short remote_port)
{
    try {
        CHECK_SESSION(session);
        //auto session = std::move(session1);
        // 提交任务到线程池
        RawProtocolImpl package;
        msgpack::v1::object_handle raw_msg;     // 反序列化结果
        std::error_code ec;            // 错误码

        // 合并步骤：协议解析+反序列化
        if (!package.decode(sv, raw_msg, ec)) {
            log(LOG_TYPE_DEBUG, "协议解析失败: %s", ec.message().c_str());
            return;
        }
        // 2. 使用 shared_ptr 包装不可复制的 raw_msg
        auto raw_msg_ptr = std::make_shared<msgpack::v1::object_handle>(std::move(raw_msg));
        // 直接使用反序列化结果
        const auto& root = raw_msg_ptr->get();
        const auto package_id = root.via.array.ptr[0].as<unsigned int>();
        if (!session || session->is_stopped()) {
            return;
        }
        auto user_data = get_user_data_(session);
        if (user_data->get_handshake() == false)
        {
            if (package_id == PackageId::PKG_ID_C2S_HANDSHAKE)
            {
                if (!on_recv(package_id, session, package, std::move(*raw_msg_ptr)))
                {
                    log(LOG_TYPE_ERROR, TEXT("[%s:%d] 未知包id %d"), Utils::c2w(remote_address).c_str(), remote_port, package_id);
                }
                return;
            }
            else
            {
                //log(LOG_TYPE_ERROR, TEXT("[%s:%d] 未握手用户"), Utils::c2w(remote_address).c_str(), remote_port);
                return;
            }
            }

            if (package_id == PackageId::PKG_ID_C2S_HEARTBEAT)
            {
                if (!on_recv(package_id, session, package, std::move(*raw_msg_ptr)))
                {
                    log(LOG_TYPE_ERROR, TEXT("[%s:%d] 未知包id %d"), Utils::c2w(remote_address).c_str(), remote_port, package_id);
                }
                return;
            }
#ifdef G_SERVICE    
        // service只注册了OBPKG_ID_C2S_AUTH一个事件,所以避免不需要的调用dispatch,避免锁竞争
        if (OBPKG_ID_C2S_AUTH == package_id)
        {
            CObserverServer::instance().obpkg_id_c2s_auth(session, package, std::move(*raw_msg_ptr));
            return;
        }
#endif
        if (!on_recv(package_id, session, package, std::move(*raw_msg_ptr)))
        {
            log(LOG_TYPE_ERROR, TEXT("[%s:%d] 未知包id %d"), Utils::c2w(remote_address).c_str(), remote_port, package_id);
        }
        return;
    }
    catch (const std::exception& e) {
        log(LOG_TYPE_ERROR, "_on_recv异常: %s", e.what());
    }
    catch (...) {
        log(LOG_TYPE_ERROR, "_on_recv异常");
    }
}

void CAntiCheatServer::on_post_disconnect(tcp_session_shared_ptr_t& session)
{
    if (session->is_timer_exists((unsigned int)UUID_CHECK_TIMER_ID)) {
        session->stop_timer((unsigned int)UUID_CHECK_TIMER_ID);
    }
}

void CAntiCheatServer::on_stop()
{
    stop_all_timers();
    stop_all_timed_tasks();
    stop_all_timed_events();
    auto ec = asio2::get_last_error();
    if (ec) {
        log(LOG_TYPE_EVENT, TEXT("停止监听:%s:%d"), Utils::c2w(listen_address()).c_str(), listen_port());
    }
    else {
        log(LOG_TYPE_EVENT, TEXT("停止监听:%s:%d %s"), Utils::c2w(listen_address()).c_str(), listen_port(), ec.message().c_str());
    }
}

void CAntiCheatServer::close_client(std::size_t session_id)
{
    if (auto session = find_session(session_id)) {
        session->socket().close(asio2::get_last_error());
    }
}

void CAntiCheatServer::start_timer(unsigned int timer_id, std::chrono::steady_clock::duration duration, std::function<void()> handler)
{
    super::start_timer<int>((int)timer_id, duration, [this, handler = std::move(handler)]() {
        handler();
    });
}

void CAntiCheatServer::stop_timer(unsigned int timer_id)
{
    super::stop_timer<int>((int)timer_id);
}

void CAntiCheatServer::log(int type, LPCTSTR format, ...)
{
    static std::mutex mtx;
    std::lock_guard<std::mutex> lck(mtx);
#ifndef _DEBUG
    if ((log_level_ & type) == 0)
        return;
#endif // _DEBUG
    auto now_time = std::time(nullptr);
    std::tm tm_;
    localtime_s(&tm_, &now_time);
    std::stringstream ss;
    ss << std::put_time(&tm_, "%H:%M:%S");
    std::wstring time_str = Utils::c2w(ss.str());
    std::wcout << time_str.c_str() << ":";
    std::wstring buffer;
    buffer.resize(1024);
    va_list ap;
    va_start(ap, format);
    int result = _vsnwprintf_s(buffer.data(), buffer.size(), buffer.size(), format, ap);
    va_end(ap);
    if (result < 0)
    {
        // 处理缓冲区溢出
        OutputDebugStringW(L"[Error] Log message too long\n");
        return;
    }

    switch (type)
    {
        case LOG_TYPE_DEBUG:
            std::wcout << L"[Debug]";
            break;
        case LOG_TYPE_EVENT:
            std::wcout << L"[Event]";
            break;
        case LOG_TYPE_ERROR:
            std::wcout << L"[Error]";
            break;
        default:
            break;
    }
    wprintf(TEXT("%s\n"), buffer.c_str());

    if (log_cb_)
        log_cb_(buffer.c_str(), false, true, "", false);
}

void CAntiCheatServer::log(int type, LPCSTR format, ...)
{
    static std::mutex mtx;
    std::lock_guard<std::mutex> lck(mtx);
#ifndef _DEBUG
    if ((log_level_ & type) == 0)
        return;
#endif // _DEBUG
    auto now_time = std::time(nullptr);
    std::tm tm_;
    localtime_s(&tm_, &now_time);
    std::stringstream ss;
    ss << std::put_time(&tm_, "%H:%M:%S");
    std::string time_str = ss.str();
    std::cout << time_str.c_str() << ":";
    std::string buffer;
    buffer.resize(1024);
    va_list ap;
    va_start(ap, format);
    int result = _vsnprintf_s(buffer.data(), buffer.size(), buffer.size(), format, ap);
    va_end(ap);
    if (result < 0)
    {
        // 处理缓冲区溢出
        OutputDebugStringA("[Error] Log message too long");
        return;
    }

    switch (type)
    {
        case LOG_TYPE_DEBUG:
            std::cout << "[Debug]";
            break;
        case LOG_TYPE_EVENT:
            std::cout << "[Event]";
            break;
        case LOG_TYPE_ERROR:
            std::cout << "[Error]";
            break;
        default:
            break;
    }
    printf("%s\n", buffer.c_str());

    if (log_cb_)
        log_cb_(Utils::c2w(buffer).c_str(), false, true, "", false);
}

void CAntiCheatServer::user_log(int type, bool silense, bool gm_show, const std::string& identify, LPCTSTR format, ...)
{
    static std::mutex mtx;
    std::lock_guard<std::mutex> lck(mtx);
    if ((log_level_ & type) == 0)
        return;
    auto now_time = std::time(nullptr);
    std::tm tm_;
    localtime_s(&tm_, &now_time);
    std::stringstream ss;
    ss << std::put_time(&tm_, "%H:%M:%S");
    std::wstring time_str = Utils::c2w(ss.str());
    //std::wcout << time_str.c_str() << ":";
    std::wstring buffer;
    buffer.resize(1024);
    va_list ap;

    va_start(ap, format);
    int result = _vsnwprintf_s(buffer.data(), buffer.size(), buffer.size(), format, ap);
    va_end(ap);
    if (result < 0)
    {
        // 处理缓冲区溢出
        OutputDebugStringW(L"[Error] Log message too long\n");
        return;
    }

    //switch (type)
    //{
    //    case LOG_TYPE_DEBUG:
    //        std::wcout << L"[Debug]";
    //        break;
    //    case LOG_TYPE_EVENT:
    //        std::wcout << L"[Event]";
    //        break;
    //    case LOG_TYPE_ERROR:
    //        std::wcout << L"[Error]";
    //        break;
    //    default:
    //        break;
    //}
    //wprintf(TEXT("%s\n"), buffer.c_str());

    if (log_cb_)
        log_cb_(buffer.c_str(), silense, gm_show, identify, false);
}

void CAntiCheatServer::punish_log(LPCTSTR format, ...)
{
    static std::mutex mtx;
    std::lock_guard<std::mutex> lck(mtx);
    auto now_time = std::time(nullptr);
    std::tm tm_;
    localtime_s(&tm_, &now_time);
    std::stringstream ss;
    ss << std::put_time(&tm_, "%H:%M:%S");
    std::wstring time_str = Utils::c2w(ss.str());
    std::wcout << time_str.c_str() << ":";
    std::wstring buffer;
    buffer.resize(1024);
    va_list ap;
    va_start(ap, format);
    int result = _vsnwprintf_s(buffer.data(), buffer.size(), buffer.size(), format, ap);
    va_end(ap);
    if (result < 0)
    {
        // 处理缓冲区溢出
        OutputDebugStringW(L"[Error] Log message too long\n");
        return;
    }
    //std::wcout << L"[Event]";
    //wprintf(TEXT("%s\n"), buffer.c_str());

    if (log_cb_)
        log_cb_(buffer.c_str(), true, true, "", true);
}

void CAntiCheatServer::on_recv_heartbeat(tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const ProtocolC2SHeartBeat& msg)
{
}

void CAntiCheatServer::on_recv_handshake(tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const ProtocolC2SHandShake& msg)
{
    try
    {        
        if (!session || session->is_stopped()) {
            return;
        }
        // 使用原子操作和更安全的方法
        auto userdata = get_user_data_(session);

        // 安全复制UUID
        std::memcpy(userdata->uuid, msg.uuid, sizeof(userdata->uuid));

        // 原子地标记握手状态
        userdata->set_handshake(true);   
        session->stop_timer((unsigned int)UUID_CHECK_TIMER_ID);

        userdata->set_field("sysver", msg.system_version);
        userdata->set_field("64bits", msg.is_64bit_system);
        userdata->set_field("cpuid", msg.cpuid);
        userdata->set_field("mac", msg.mac);
        userdata->set_field("vol", msg.volume_serial_number);
        userdata->set_field("rev_ver", msg.rev_version);
        userdata->set_field("commit_ver", msg.commited_hash);    
        std::string remote_address;    
        if (session && !session->is_stopped()) {
            remote_address = session->get_remote_address();
        }
        userdata->set_field("ip", remote_address);
        userdata->set_field("logintime", std::time(nullptr));
        userdata->set_field("pid", msg.pid);
        userdata->set_field("is_client", msg.is_client);

        ProtocolS2CHandShake resp;
        memcpy(resp.uuid, msg.uuid, sizeof(msg.uuid));
        async_send(session, &resp);

        // 通知和响应
        user_notify_mgr_.dispatch(CLIENT_HANDSHAKE_NOTIFY_ID, session);
    }
    catch (const std::exception& e)
    {
        log(LOG_TYPE_ERROR, TEXT("握手异常:%s"), Utils::c2w(e.what()).c_str());
        session->socket().close(asio2::get_last_error());
    }
    catch (...)
    {
        log(LOG_TYPE_ERROR, TEXT("握手异常"));
    }
}
