#include "pch.h"
#include "AntiCheatServer.h"
#include "NetUtils.h"

#define ENABLE_PROXY_TUNNEL 0
#if ENABLE_PROXY_TUNNEL
#include "ProxyTunnel.h"
#endif

#if ENABLE_PROXY_TUNNEL
//todo 析构释放
class GameProxyTunnel : public ProxyTunnel
{
public:
	using super = ProxyTunnel;
    using super::super;
    using recv_callback_t = std::function<void(const std::vector<char>& buf, const std::error_code& ec, std::size_t length)>;
	const std::string kProxyServerAddr = "192.168.59.133";
	const int kProxyServerPort = 7000;
    inline void set_recv_callback(recv_callback_t cb) { recv_callback_ = cb; }
	void start()
	{
		super::start(kProxyServerAddr, kProxyServerPort);
	}
    void on_connect(const std::error_code& ec)
    {
        log(LOG_TYPE_DEBUG, TEXT("游戏通道建立连接 %s:%d"),
            Utils::c2w(get_address()).c_str(),
            get_port());
    }
    void on_disconnect(const std::error_code& ec)
    {
        log(LOG_TYPE_DEBUG, TEXT("游戏通道失去连接 %s:%d"),
            Utils::c2w(get_address()).c_str(),
            get_port());
    }
    void on_recv(const std::vector<char>& buf, const std::error_code& ec, std::size_t length)
    {
        if (recv_callback_)
            recv_callback_(buf, ec, length);
    }
    void on_send(const std::error_code& ec, std::size_t length)
    {
        log(LOG_TYPE_DEBUG, TEXT("游戏通道发送消息 %s:%d 长度:%d %s"),
            Utils::c2w(get_address()).c_str(),
            get_port(),
            length,
			Utils::c2w(ec.message()).c_str()
			);
    }
private:
    recv_callback_t recv_callback_;
};
#endif


#if ENABLE_GAME_MATCH_ROLE
//游戏解包逻辑，暂时不用
class match_role
{
public:
    explicit match_role(char start_tag, char end_tag) : 
        start_tag_(start_tag),
        end_tag_(end_tag)
    {}

    template <typename Iterator>
    std::pair<Iterator, bool> operator()(Iterator begin, Iterator end) const
    {
        Iterator i = begin;
        while (i != end)
        {
            if (*i != start_tag_)
                return std::pair(begin, true); // head character is not #, return and kill the client
            i++;
            if (i == end) break;
			while (i != end)
			{
				if(*i == end_tag_)
					return std::pair(i, true);
				i++;
			}
            break;
        }
        return std::pair(begin, false);
    }

private:
    char start_tag_;
    char end_tag_;
};
#else
#include "Protocol.h"
#endif
#include <WinBase.h>
#include <WinNT.h>
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
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <string_view>
#include <type_traits>
#include <vadefs.h>
#include <asio2/base/error.hpp>
#include <asio2/base/impl/user_timer_cp.hpp>
#include <asio2/tcp/tcp_server.hpp>
#include <msgpack/v1/object_fwd.hpp>
#include <msgpack/v1/object_fwd_decl.hpp>
#include <msgpack/v3/unpack.hpp>
#include "Package.h"

static std::set<std::string> ddos_black_List;
static std::map<std::string, uint32_t> ddos_black_map; 
CAntiCheatServer::CAntiCheatServer(): super()
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

	uuid_check_duration_ = std::chrono::seconds(10);
	heartbeat_check_duration_ = std::chrono::seconds(60);
	heartbeat_timeout_ = std::chrono::seconds(120);
	
    is_observer_server_ = false;
    is_logic_server_ = false;
    is_auth_success_ = false;
    set_log_level(LOG_TYPE_DEBUG | LOG_TYPE_EVENT | LOG_TYPE_ERROR);
}

CAntiCheatServer::~CAntiCheatServer()
{
    package_mgr_.clear_handler();
}

bool CAntiCheatServer::start(const std::string& listen_addr, int port)
{
#if ENABLE_PROXY_TUNNEL
	enable_proxy_tunnel(true);
#else
	enable_proxy_tunnel(false);
#endif
	if (super::start(listen_addr, port, RawProtocolImpl()))
	{
		return true;
	}
	return false;
}

bool CAntiCheatServer::check_timer(bool slience)
{
#if 0
    std::string body;
    std::string msg;
    try {
        std::error_code ec;
        asio2::http_client http_client;
        auto res = http_client.execute(auth_url_ + "?action=login&ticket=" + auth_ticket_, ec);
        body = res.body();
        auto json_data = json::parse(body);
        unsigned int status = json_data["status"];
        msg = json_data["msg"];
        msg = Utils::from_utf8(msg);
        if (status)
        {
            is_auth_success_ = true;
            if (!slience) log(Event, TEXT("%s"), Utils::c2w(msg).c_str());
            return true;
        }
        else
        {
            is_auth_success_ = false;
            log(Event, TEXT("%s"), Utils::c2w(msg).c_str());
            return false;
        }
    }
    catch (...)
    {
        log(Event, TEXT("认证服务器错误 %s"), Utils::c2w(Utils::from_utf8(body)).c_str());
    }
#endif
	return false;
}

void CAntiCheatServer::on_init()
{
    log(LOG_TYPE_EVENT, TEXT("套接字初始化成功"));
}

void CAntiCheatServer::on_start()
{
    log(LOG_TYPE_EVENT, TEXT("开始监听:%s:%u "), Utils::c2w(listen_address()).c_str(), listen_port());
    notify_mgr_.dispatch(SERVER_START_NOTIFY_ID);
}

void CAntiCheatServer::on_accept(tcp_session_shared_ptr_t& session)
{
    auto remote_address = session->remote_address();
	// 黑名单
	if (ddos_black_List.find(remote_address) != ddos_black_List.end() && remote_address != kDefaultLocalhost) {
		session->stop();
		if (ddos_black_map[remote_address] > 0) {
			ddos_black_map[remote_address] -= 1;
			log(LOG_TYPE_ERROR, TEXT("拦截ddos攻击黑名单IP:%s"), Utils::c2w(remote_address).c_str());
            slog->error("拦截ddos攻击黑名单IP: {}", remote_address);
		}
	}
	get_user_data_(session)->set_field("is_local_client", remote_address == kDefaultLocalhost);
	//log(LOG_TYPE_DEBUG, TEXT("接受 %s:%d"), Utils::c2w(remote_address).c_str(), session->remote_port());
    //slog->debug("接受: {}{}", remote_address, session->remote_port());
}

void CAntiCheatServer::on_post_connect(tcp_session_shared_ptr_t& session)
{
    //slog->debug("建立连接: {}{}", session->remote_address(), session->remote_port());
    //log(LOG_TYPE_DEBUG, TEXT("建立连接 %s:%d"), Utils::c2w(session->remote_address()).c_str(), session->remote_port());
    //握手超时检查
    session->start_timer((unsigned int)UUID_CHECK_TIMER_ID, uuid_check_duration_, [this, session]() {
        auto userdata = get_user_data_(session);
        if (userdata->get_handshake() == false)
        {
            session->stop();
            return;
        }
    });
}

void CAntiCheatServer::on_recv_package(tcp_session_shared_ptr_t& session, std::string_view sv)
{
    //slog->debug("=============on_recv_package收包: {}{} 长度:{}", session->remote_address(), session->remote_port(), sv.size());
#ifdef _DEBUG
    _on_recv(session, sv);
#else
    try
    {
        _on_recv(session, sv);
    }
    catch (...)
    {
        slog->debug("收包解析异常: {}{} 长度:{}", session->remote_address(), session->remote_port(), sv.size());
        log(LOG_TYPE_DEBUG, TEXT("收包解析异常:%s:%d 长度:%d"), Utils::c2w(session->remote_address()).c_str(), session->remote_port(), sv.size());
    }
#endif
}

void CAntiCheatServer::_on_recv(tcp_session_shared_ptr_t& session, std::string_view sv)
{
    auto remote_address = session->remote_address();
    // 黑名单
    if (ddos_black_List.find(session->remote_address()) != ddos_black_List.end()) {
        return;
    }
    if (sv.size() == 0 && remote_address != kDefaultLocalhost)
    {
        auto it = ddos_black_map.find(remote_address);
        if (it != ddos_black_map.end()) {
            if (++it->second >= 10) {
                ddos_black_List.emplace(remote_address);
            }
        }
        else {
            ddos_black_map[remote_address] = 1;
        }

        log(LOG_TYPE_ERROR, TEXT("协议底层错误:%s:%d ==> %s:%d 长度:%d"),
            Utils::c2w(remote_address).c_str(),
            session->remote_port(),
            Utils::c2w(session->local_address()).c_str(),
            session->local_port(),
            sv.size());
        session->stop();
        return;
    }
    if (is_enable_proxy_tunnel())
    {
#if ENABLE_PROXY_TUNNEL
        get_user_data(session).game_proxy_tunnel->send(sv);
#endif
    }
    RawProtocolImpl package;
#ifdef _DEBUG
    if (is_logic_server())
    {
        if (!package.decode(sv))
        {
            log(LOG_TYPE_DEBUG, TEXT("解包校验失败:%s:%d 长度:%d"),
                Utils::c2w(session->remote_address()).c_str(),
                session->remote_port(),
                sv.size());
            return;
        }
    }
    else
    {
        if (!package.decode(sv))
        {
            log(LOG_TYPE_DEBUG, TEXT("解包校验失败:%s:%d 长度:%d"),
                Utils::c2w(session->remote_address()).c_str(),
                session->remote_port(),
                sv.size());
            return;
        }
    }
#else
    if (!package.decode(sv))
    {
        log(LOG_TYPE_DEBUG, TEXT("解包校验失败:%s:%d 长度:%d"),
            Utils::c2w(session->remote_address()).c_str(),
            session->remote_port(),
            sv.size());
        return;
    }
#endif  
    auto raw_msg = msgpack::unpack((char*)package.body.buffer.data(), package.body.buffer.size());
    if (raw_msg.get().type != msgpack::type::ARRAY) throw msgpack::type_error();
    if (raw_msg.get().via.array.size < 1) throw msgpack::type_error();
    if (raw_msg.get().via.array.ptr[0].type != msgpack::type::POSITIVE_INTEGER) throw msgpack::type_error();
    const auto package_id = raw_msg.get().via.array.ptr[0].as<unsigned int>();
    auto user_data = get_user_data_(session);

    //if (package.head.step != user_data->step + 1)
    //{
    //    user_data->set_field("miss_count", user_data->get_field<int>("miss_count").value_or(0) + 1);
    //    log(LOG_TYPE_DEBUG, TEXT("[%s:%d] 发现丢包或重放攻击"),
    //        Utils::c2w(session->remote_address()).c_str(),
    //        session->remote_port());
    //}
    user_data->step = package.head.step;

    if (user_data->get_handshake() == false && package_id == PackageId::PKG_ID_C2S_HANDSHAKE)
    {
        auto msg = raw_msg.get().as<ProtocolC2SHandShake>();
        on_recv_handshake(session, package, msg);
        return;
    }

    if (user_data->get_handshake() == false)
    {
		log(LOG_TYPE_ERROR, TEXT("[%s:%d] 未握手用户"), Utils::c2w(session->remote_address()).c_str(), session->remote_port());
        return;
    }

    if (package_id == PackageId::PKG_ID_C2S_HEARTBEAT)
    {
        auto msg = raw_msg.get().as<ProtocolC2SHeartBeat>();
        on_recv_heartbeat(session, package, msg);
        return;
    }
#if 0
    log(Debug, TEXT("收到数据包:%s:%d 长度:%d"),
        Utils::c2w(session->remote_address()).c_str(),
        session->remote_port(),
        sv.size());
#endif
    if (package_mgr_.dispatch(package_id, session, package, raw_msg))
    {
        return;
}
    if (!on_recv(package_id, session, package, std::move(raw_msg)))
    {
        log(LOG_TYPE_ERROR, TEXT("[%s:%d] 未知包id %d"),
            Utils::c2w(session->remote_address()).c_str(),
            session->remote_port(),
            package_id);
    }
}


void CAntiCheatServer::on_post_disconnect(tcp_session_shared_ptr_t& session)
{
    //log(LOG_TYPE_DEBUG, TEXT("断开连接 %s:%d"), Utils::c2w(session->remote_address()).c_str(), session->remote_port());
    if (is_enable_proxy_tunnel())
    {
#if ENABLE_PROXY_TUNNEL
        get_user_data(session).game_proxy_tunnel->stop();
#endif
    }
}

void CAntiCheatServer::on_stop()
{
    auto ec = asio2::get_last_error();
    if (ec) {
		log(LOG_TYPE_EVENT, TEXT("停止监听:%s:%d"), Utils::c2w(listen_address()).c_str(), listen_port());
	}
	else {
		log(LOG_TYPE_EVENT, TEXT("停止监听:%s:%d %s"), Utils::c2w(listen_address()).c_str(), listen_port(), ec.message().c_str());
    }
}

void CAntiCheatServer::close_client(unsigned int session_id)
{
    sessions().find(session_id)->stop();
}

void CAntiCheatServer::start_timer(unsigned int timer_id, std::chrono::system_clock::duration duration, std::function<void()> handler)
{
    super::start_timer<int>((int)timer_id, duration, [this, handler = std::move(handler)](){
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
	std::wcout << L"[Event]";
	wprintf(TEXT("%s\n"), buffer.c_str());

	if (log_cb_)
		log_cb_(buffer.c_str(), true, true, "", true);
}

void CAntiCheatServer::on_recv_heartbeat(tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const ProtocolC2SHeartBeat& msg)
{
    auto userdata = get_user_data_(session);
    userdata->update_heartbeat_time();
   // slog->debug("收到心跳包并更新时间 session_id: {}", session->hash_key());
    
    ProtocolS2CHeartBeat resp;
    resp.tick = msg.tick;
    async_send(session, &resp);
    //slog->debug("发送心跳响应包 session_id: {}", session->hash_key());
    
    user_notify_mgr_.dispatch(CLIENT_HEARTBEAT_NOTIFY_ID, session);
}

void CAntiCheatServer::on_recv_handshake(tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const ProtocolC2SHandShake& msg)
{
    try
    {
        // 使用原子操作和更安全的方法
        auto userdata = get_user_data_(session);
        
        // 安全复制UUID
        memcpy(userdata->uuid, msg.uuid, sizeof(msg.uuid));
        
        // 原子地标记握手状态
        userdata->set_handshake(true);
        session->stop_timer((unsigned int)UUID_CHECK_TIMER_ID);
        // 关闭玩家连接,不关闭logic_server
        bool is_local_client = userdata->get_field<bool>("is_local_client").value_or(false);
        slog->debug("关闭玩家连接 is_local_client:{} logic_server session_id: {}", is_local_client, session->hash_key());
        if (!is_local_client || msg.is_client)
        {
            // 配置心跳检查定时器，使用lambda捕获减少作用域
            auto heartbeat_check = [this, weak_session = std::weak_ptr(session)]() {
                if (auto session = weak_session.lock()) {
                    auto userdata = get_user_data_(session);
                    auto duration = userdata->get_heartbeat_duration();
                
                    if (userdata->is_timeout(std::chrono::duration_cast<std::chrono::seconds>(heartbeat_timeout_)))  {
                        log(LOG_TYPE_ERROR, TEXT("%s:%d 心跳超时%d秒"),
                            Utils::c2w(session->remote_address()).c_str(),
                            session->remote_port(),
                            std::chrono::duration_cast<std::chrono::seconds>(duration).count());

                            session->clear_user_data();
                            session->stop();
                    }
                }
            };

            session->start_timer(
                static_cast<unsigned int>(HEARTBEAT_CHECK_TIMER_ID), 
                heartbeat_check_duration_, 
                heartbeat_check
            );

        }

        userdata->set_field("sysver", msg.system_version);
        userdata->set_field("64bits", msg.is_64bit_system);
        userdata->set_field("cpuid", msg.cpuid);
        userdata->set_field("mac", msg.mac);
        userdata->set_field("vol", msg.volume_serial_number);
        userdata->set_field("rev_ver", msg.rev_version);
        userdata->set_field("commit_ver", msg.commited_hash);
        userdata->set_field("ip", session->remote_address());
        userdata->set_field("logintime", std::time(nullptr));
        userdata->set_field("pid", msg.pid);
        userdata->set_field("is_client", msg.is_client);

        // 通知和响应
        user_notify_mgr_.dispatch(CLIENT_HANDSHAKE_NOTIFY_ID, session);
        
        ProtocolS2CHandShake resp;
        memcpy(resp.uuid, msg.uuid, sizeof(msg.uuid));
        async_send(session, &resp);

        slog->debug("握手成功: session_id={}", session->hash_key());
    }
    catch (const std::exception& e)
    {
        log(LOG_TYPE_ERROR, TEXT("握手异常:%s"), Utils::c2w(e.what()).c_str());
        session->stop();
    }
}
