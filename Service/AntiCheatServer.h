#pragma once
#include "TcpServer.h"
#include "Protocol.h"
#include "NetUtils.h"

class CAntiCheatServer : public CTcpServerImpl
{
private:
    using self = CAntiCheatServer;
    using super = CTcpServerImpl;
protected:
    using package_handler_t = std::function<void(tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle&)>;
    using notify_handler_t = std::function<void()>;
    using user_notify_handler_t = std::function<void(tcp_session_shared_ptr_t& session)>;
    using log_cb_t = std::function<void(const wchar_t*, bool silence, bool gm_show, const std::string& identify)>;
public:
	CAntiCheatServer();
    ~CAntiCheatServer();
    bool start(const std::string& listen_addr, int port);
	bool check_timer(bool slience);
	virtual void on_accept(tcp_session_shared_ptr_t& session);
    virtual void on_init();
    virtual void on_post_connect(tcp_session_shared_ptr_t& session);
    virtual void on_post_disconnect(tcp_session_shared_ptr_t& session);
    virtual void on_start(error_code_t ec);
    virtual void on_stop(error_code_t ec);
    virtual void on_recv(tcp_session_shared_ptr_t& session, std::string_view sv);
    virtual void on_recv_handshake(tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const ProtocolC2SHandShake& msg);
    virtual void on_recv_heartbeat(tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const ProtocolC2SHeartBeat& msg);
private:
    virtual void _on_recv(tcp_session_shared_ptr_t& session, std::string_view sv);
    
protected:
    virtual void send(tcp_session_shared_ptr_t& session, RawProtocolImpl& package, unsigned int session_id = 0)
    {
        if (session)
        {
            if (session_id != 0)
            {
                package.head.session_id = session_id;
            }
            else
            {
                package.head.session_id = session->hash_key();
            }
            session->send(package.release());
        }
    }

    virtual void send(tcp_session_shared_ptr_t& session, msgpack::sbuffer& buffer, unsigned int session_id = 0)
    {
        RawProtocolImpl raw_package;
        raw_package.encode(buffer.data(), buffer.size());
        send(session, raw_package, session_id);
    }

    virtual void start_timer(unsigned int timer_id, std::chrono::system_clock::duration duration, std::function<void()> handler);
    virtual void stop_timer(unsigned int timer_id);
public:
    virtual void log(int type, LPCTSTR format, ...); 
    virtual void user_log(int type, bool silense, bool gm_show, const std::string& identify, LPCTSTR format, ...);
    virtual bool on_recv(unsigned int package_id, tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle&) { return false; };

	template <typename T>
	void send(tcp_session_shared_ptr_t& session, T* package, unsigned int session_id = 0)
	{
		if (!package)
			__debugbreak();
		msgpack::sbuffer buffer;
		msgpack::pack(buffer, *package);
        send(session, buffer, session_id);
	}

    template <typename T>
    void send(size_t session_id, T* package, unsigned int cur_session_id = 0)
    {
        send(sessions().find(session_id), package, cur_session_id);
    }

    void close(unsigned int session_id);
	inline void enable_proxy_tunnel(bool v) { enable_proxy_tunnel_ = v; }
	inline bool is_enable_proxy_tunnel() { return enable_proxy_tunnel_; }
    inline bool is_observer_server() { return is_observer_server_; }
    inline bool is_logic_server() { return is_logic_server_; }
public:
    inline void set_log_cb(log_cb_t cb) { log_cb_ = cb; }
    inline void set_log_level(char level) { log_level_ = level; }
    inline void set_auth_ticket(const std::string& ticket) { auth_ticket_ = ticket; }
    inline const std::string& get_auth_ticket() { return auth_ticket_; }
    inline void auth_success() { is_auth_success_ = true; }
    inline void auth_fail() { is_auth_success_ = false; }
protected:
	bool enable_proxy_tunnel_;
	std::string auth_ticket_;
	std::string auth_url_;
	std::chrono::system_clock::duration auth_check_timer_;
	std::chrono::system_clock::duration uuid_check_duration_;
	std::chrono::system_clock::duration heartbeat_check_duration_;
	std::chrono::system_clock::duration heartbeat_timeout_;
    NetUtils::EventMgr<package_handler_t> package_mgr_;
    NetUtils::EventMgr<package_handler_t> observer_package_mgr_;
    NetUtils::EventMgr<notify_handler_t>  notify_mgr_;
    NetUtils::EventMgr<user_notify_handler_t>  user_notify_mgr_;
    bool is_observer_server_;
    bool is_logic_server_;
    bool is_auth_success_;
    log_cb_t log_cb_;
    char log_level_;
};

struct AntiCheatUserData
{
#if ENABLE_PROXY_TUNNEL
    std::shared_ptr<GameProxyTunnel> game_proxy_tunnel;
#endif
    uint32_t uuid[4];
    bool has_handshake = false;
    std::chrono::system_clock::time_point last_heartbeat_time = std::chrono::system_clock::now();
    unsigned char step = 0;
    json data;
    template <typename R = std::wstring>
    inline R get_field(const std::string& key)
    {
        if (data.find(key) == data.end())
            return R();
        return data.at(key).get<R>();
    }

    template <>
    inline int get_field(const std::string& key)
    {
        if (data.find(key) == data.end())
            return 0;
        return data.at(key).get<int>();
    }
    
    template <>
    inline bool get_field(const std::string& key)
    {
        if (data.find(key) == data.end())
            return false;
        return data.at(key).get<bool>();
    }

    template <typename T>
    inline void set_field(const std::string& key, const T& val)
    {
        data[key] = val;
    }
    inline void update_heartbeat_time()
    {
        last_heartbeat_time = std::chrono::system_clock::now();
    }
    inline std::chrono::system_clock::duration get_heartbeat_duration()
    {
        return std::chrono::system_clock::now() - last_heartbeat_time;
    }
};

inline AntiCheatUserData* get_user_data(const CAntiCheatServer::tcp_session_shared_ptr_t& session)
{
    auto userdata = session->user_data<AntiCheatUserData*>();
    if (userdata == nullptr)
    {
        userdata = new AntiCheatUserData();
#if ENABLE_PROXY_TUNNEL
        userdata->game_proxy_tunnel = std::make_shared<GameProxyTunnel>(session->io().context());
#endif
        session->user_data<AntiCheatUserData*>(std::move(userdata));
    }
    return userdata;
}
