#pragma once
#include "Protocol.h"
#include "NetUtils.h"

extern std::shared_ptr<spdlog::logger> slog;
class CAntiCheatServer : public asio2::tcp_server
{
public:
    using self = CAntiCheatServer;
    using tcp_session_t = session_type;
    using error_code_t = asio2::error_code;
    using super = asio2::tcp_server;
    using tcp_session_shared_ptr_t = std::shared_ptr<session_type>;
protected:
    using package_handler_t = std::function<void(tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle&)>;
    using notify_handler_t = std::function<void()>;
    using user_notify_handler_t = std::function<void(tcp_session_shared_ptr_t& session)>;
    /**
     * msg: 日志信息
     * silence: 是否显示到界面日志窗口
     * gm_show: 是否显示到gm
     * identify: 玩家uuid标识符
     * punish_flag: 是否是惩罚log
     */
    using log_cb_t = std::function<void(const wchar_t*, bool silence, bool gm_show, const std::string& identify, bool punish_flag)>;
public:
    CAntiCheatServer();
    ~CAntiCheatServer();
    bool start(const std::string& listen_addr, int port);
    bool check_timer(bool slience);
    virtual void on_accept(tcp_session_shared_ptr_t& session);
    virtual void on_init();
    virtual void on_post_connect(tcp_session_shared_ptr_t& session);
    virtual void on_post_disconnect(tcp_session_shared_ptr_t& session);
    virtual void on_start();
    virtual void on_stop();
    virtual void on_recv_package(tcp_session_shared_ptr_t& session, std::string_view sv);
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

    virtual void async_send(tcp_session_shared_ptr_t& session, RawProtocolImpl& package, unsigned int session_id = 0)
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
            session->async_send(package.release());
        }
    }

    virtual void async_send(tcp_session_shared_ptr_t& session, msgpack::sbuffer& buffer, unsigned int session_id = 0)
    {
        RawProtocolImpl raw_package;
        raw_package.encode(buffer.data(), buffer.size());
        async_send(session, raw_package, session_id);
    }

    virtual void start_timer(unsigned int timer_id, std::chrono::system_clock::duration duration, std::function<void()> handler);
    virtual void stop_timer(unsigned int timer_id);
public:
    virtual void log(int type, LPCTSTR format, ...);
    virtual void user_log(int type, bool silense, bool gm_show, const std::string& identify, LPCTSTR format, ...);
    void punish_log(LPCTSTR format, ...);
    virtual bool on_recv(unsigned int package_id, tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, msgpack::v1::object_handle&& raw_msg) { return false; };

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

    template <typename T>
    void async_send(tcp_session_shared_ptr_t& session, T* package, unsigned int session_id = 0)
    {
        if (!package)
            __debugbreak();
        msgpack::sbuffer buffer;
        msgpack::pack(buffer, *package);
        async_send(session, buffer, session_id);
    }

    template <typename T>
    void async_send(size_t session_id, T* package, unsigned int cur_session_id = 0)
    {
        async_send(sessions().find(session_id), package, cur_session_id);
    }

    void close_client(unsigned int session_id);
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

class HeartbeatTracker {
private:
    std::atomic<std::chrono::steady_clock::time_point> last_heartbeat_time{
        std::chrono::steady_clock::now()
    };

public:
    void update_heartbeat() {
        last_heartbeat_time.store(
            std::chrono::steady_clock::now(), 
            std::memory_order_release  // 使用 release 语义
        );
    }

    std::chrono::steady_clock::duration get_heartbeat_duration() const {
        auto current_time = std::chrono::steady_clock::now();
        auto last_time = last_heartbeat_time.load(std::memory_order_acquire);
        
        // 防止时间计算溢出
        if (current_time < last_time) {
            return std::chrono::steady_clock::duration::zero();
        }
        
        return current_time - last_time;
    }

    bool is_timeout(std::chrono::seconds timeout) const {
        return get_heartbeat_duration() > timeout;
    }
};

struct AntiCheatUserData
{
#if ENABLE_PROXY_TUNNEL
    std::shared_ptr<GameProxyTunnel> game_proxy_tunnel;
#endif
    uint32_t uuid[4];
    std::atomic<bool> has_handshake{ false };
    mutable std::shared_mutex mutex_;
    std::chrono::system_clock::time_point last_heartbeat_time = std::chrono::system_clock::now();
    unsigned char step = 0;
    json data;
    HeartbeatTracker heartbeat_tracker;
    template <typename R = std::wstring>
    inline std::optional<R> get_field(const std::string& key) {
        try {
            std::shared_lock<std::shared_mutex> lock(mutex_);   
            if (!data.contains(key)) {
                return std::nullopt;
            }
            return data.at(key).get<R>();
        } catch (const std::exception& e) {
            slog->error("AntiCheatUserData: Field {} type conversion error: {}", key, e.what());
            return std::nullopt;
        }
    }

    template <typename T>
    inline void set_field(const std::string& key, const T& val)
    {
        try {
            std::unique_lock<std::shared_mutex> lock(mutex_);
            data[key] = val;
        }
        catch (const std::exception& e) {
            slog->error("AntiCheatUserData: Field {} set error: {}", key, e.what());
        }
    }
    inline void update_heartbeat_time()
    {
        heartbeat_tracker.update_heartbeat();
    }
    inline std::chrono::steady_clock::duration get_heartbeat_duration()
    {
        return heartbeat_tracker.get_heartbeat_duration();
    }
    inline bool is_timeout(std::chrono::seconds timeout) const {
        return heartbeat_tracker.is_timeout(timeout);
    }
    // 提供线程安全的方法
    void set_handshake(bool value) {
        has_handshake.store(value, std::memory_order_release);
    }

    bool get_handshake() const {
        return has_handshake.load(std::memory_order_acquire);
    }
};

inline std::shared_ptr<AntiCheatUserData> get_user_data_(const CAntiCheatServer::tcp_session_shared_ptr_t& session) {
    auto userdata = session->get_user_data<std::shared_ptr<AntiCheatUserData>>();
    if (!userdata) {
        static std::mutex init_mutex;
        std::lock_guard<std::mutex> lock(init_mutex);
        userdata = session->get_user_data<std::shared_ptr<AntiCheatUserData>>();
        if (!userdata) {
            try {
                userdata = std::make_shared<AntiCheatUserData>();
                session->set_user_data(userdata);
            }
            catch (const std::exception& e) {
                std::cerr << "Error setting user data: " << e.what() << std::endl;
                throw;  // 重新抛出异常
            }
        }
    }
    return userdata;
}