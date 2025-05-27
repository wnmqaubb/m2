#pragma once
#include "Protocol.h"
#include "NetUtils.h"
//#include "concurrentqueue/concurrentqueue.h"
#include <ThreadPool.h>
#include <BS_thread_pool.hpp>
#define CHECK_SESSION(s) \
    if(!s || s->is_stopped()) { \
        log(LOG_TYPE_DEBUG, "会话已失效 %p", s.get()); \
        return; \
    }
extern std::shared_ptr<spdlog::logger> slog;
inline LONG WINAPI GlobalExceptionFilter(_EXCEPTION_POINTERS* pExp)
{
    slog->error("GlobalExceptionFilter捕获异常: 0x%X", pExp->ExceptionRecord->ExceptionCode);
    return EXCEPTION_CONTINUE_EXECUTION;
}
// 封装线程入口函数
template<typename F>
void SafeThread(F&& f)
{
    _set_se_translator([](unsigned code, EXCEPTION_POINTERS*) {
        throw std::runtime_error("SEH:0x" + std::to_string(code));
    });
    try { f(); }
    catch (...) {
        /* 记录日志 */ 
        slog->error("SafeThread: thread exception");
    }
}
inline BS::thread_pool<BS::tp::none>& get_global_thread_pool();
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
    virtual void _on_recv(tcp_session_shared_ptr_t session, std::string_view sv, const std::string remote_address, unsigned short remote_port);

protected:
    virtual void send(tcp_session_shared_ptr_t& session, RawProtocolImpl& package, std::size_t session_id = 0)
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
            session->send(package.release()); // 在IO线程安全发送数据

        }
    }

    virtual void send(tcp_session_shared_ptr_t& session, msgpack::sbuffer& buffer, std::size_t session_id = 0)
    {
        RawProtocolImpl raw_package;
        raw_package.encode(buffer.data(), buffer.size());
        send(session, raw_package, session_id);
    }

    virtual void async_send(tcp_session_shared_ptr_t& session, RawProtocolImpl& package, std::size_t session_id = 0)
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
            session->async_send(package.release()); // 在IO线程安全发送数据

        }
    }

    virtual void async_send(tcp_session_shared_ptr_t& session, msgpack::sbuffer& buffer, std::size_t session_id = 0)
    {
        RawProtocolImpl raw_package;
        raw_package.encode(buffer.data(), buffer.size());
        async_send(session, raw_package, session_id);
    }

    virtual void start_timer(unsigned int timer_id, std::chrono::steady_clock::duration duration, std::function<void()> handler);
    virtual void stop_timer(unsigned int timer_id);
public:
    virtual void log(int type, LPCTSTR format, ...);
    virtual void log(int type, LPCSTR format, ...);
    virtual void user_log(int type, bool silense, bool gm_show, const std::string& identify, LPCTSTR format, ...);
    void punish_log(LPCTSTR format, ...);
    virtual bool on_recv(unsigned int package_id, tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, msgpack::v1::object_handle&& raw_msg) { return false; };

    template <typename T>
    void send(tcp_session_shared_ptr_t& session, T* package, std::size_t session_id = 0)
    {
        if (!package)
            __debugbreak();
        msgpack::sbuffer buffer;
        msgpack::pack(buffer, *package);
        send(session, buffer, session_id);
    }

    template <typename T>
    void send(size_t session_id, T* package, std::size_t cur_session_id = 0)
    {
        send(find_session(session_id), package, cur_session_id);
    }

    template <typename T>
    void async_send(tcp_session_shared_ptr_t& session, T* package, std::size_t session_id = 0)
    {
        if (!package)
            __debugbreak();
        msgpack::sbuffer buffer;
        msgpack::pack(buffer, *package);
        async_send(session, buffer, session_id);
    }

    template <typename T>
    void async_send(size_t session_id, T* package, std::size_t cur_session_id = 0)
    {
        async_send(find_session(session_id), package, cur_session_id);
    }

    void close_client(std::size_t session_id);
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
    // 统一使用steady_clock类型
    std::chrono::steady_clock::duration auth_check_timer_ = std::chrono::minutes(5);
    std::chrono::steady_clock::duration uuid_check_duration_ = std::chrono::seconds(30);
    std::chrono::steady_clock::duration heartbeat_check_duration_ = std::chrono::seconds(15);
    std::chrono::steady_clock::duration heartbeat_timeout_ = std::chrono::seconds(30);
    NetUtils::EventMgr<package_handler_t> package_mgr_;
    NetUtils::EventMgr<package_handler_t> observer_package_mgr_;
    NetUtils::EventMgr<notify_handler_t>  notify_mgr_;
    NetUtils::EventMgr<user_notify_handler_t>  user_notify_mgr_;
    bool is_observer_server_;
    bool is_logic_server_;
    bool is_auth_success_;
    log_cb_t log_cb_;
    char log_level_;
    bool enable_proxy_tunnel_;
    std::string auth_ticket_;
    std::string auth_url_;
    std::atomic<bool> auth_lock_{true}; // 启动时锁定
    //BusinessThreadPool pool;
    std::atomic<bool> is_shutdown{ false };
    std::shared_mutex mutex_;

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
    alignas(64) mutable std::shared_mutex mutex_;  // 缓存行对齐
    std::chrono::system_clock::time_point last_heartbeat_time{}; // 统一使用steady_clock
    unsigned char step = 0;
    json data;
    HeartbeatTracker heartbeat_tracker;
    char padding[64 - sizeof(std::shared_mutex)%64]; // 缓存行填充

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

inline AntiCheatUserData* get_user_data_(const CAntiCheatServer::tcp_session_shared_ptr_t& session)
{
    AntiCheatUserData* userdata = nullptr;
    try {
        userdata = session->get_user_data<AntiCheatUserData*>();
        if (userdata == nullptr)
        {
            userdata = new AntiCheatUserData();
            session->set_user_data<AntiCheatUserData*>(std::move(userdata));
        }
        return userdata;
    }
    catch (const std::bad_any_cast& e) {
        slog->error("Bad cast: {}", e.what());
        userdata = new AntiCheatUserData();
        session->set_user_data<AntiCheatUserData*>(std::move(userdata));
        return userdata;
    }
    catch (const std::exception& e) {
        slog->error("Get user data failed: {}", e.what());
        return new AntiCheatUserData();
    }
}