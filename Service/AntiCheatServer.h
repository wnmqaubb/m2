#pragma once
#include "Protocol.h"
#include "NetUtils.h"
#include "concurrentqueue/concurrentqueue.h"
#include "parallel_hashmap/phmap.h"
extern std::shared_ptr<spdlog::logger> slog;
class CAntiCheatServer : public asio2::tcp_server
{
public:
    // 分层时间轮结构（15s/8min/4h三级检测）
    struct alignas(64) HierarchicalTimeWheel {
        static constexpr size_t L1_SLOTS = 512;   // 15秒级（30ms/槽）
        static constexpr size_t L2_SLOTS = 512;   // 8分钟级（~1s/槽）
        static constexpr size_t L3_SLOTS = 512;   // 4小时级（~30s/槽）
        static constexpr size_t LEVEL_1_INTERVAL_MS = 15000;  // 15秒间隔
        static constexpr size_t MAX_EXPIRE_BATCH_SIZE = 1000; // 最大批量清理数量
        
        // 使用原子标记位数组（按CPU缓存行对齐）
        struct alignas(64) SlotBucket {
            std::atomic<uint64_t> active_mask{0};
            std::array<std::chrono::steady_clock::time_point, 64> timestamps;
        };
        
        std::array<SlotBucket, (L1_SLOTS + L2_SLOTS + L3_SLOTS) / 64> buckets;
        std::atomic<uint32_t> current_epoch{0};

        // 带层级参数的无锁更新时间戳
        void update_timestamp(size_t level, size_t slot, size_t timestamp, size_t interval_ms) noexcept {
            const auto now = std::chrono::steady_clock::time_point(std::chrono::milliseconds(timestamp));
            const size_t bucket_idx = (level == 0) ? slot / 64 : 
                                     (level == 1) ? (L1_SLOTS / 64) + slot / 64 : 
                                     (L1_SLOTS + L2_SLOTS) / 64 + slot / 64;
            auto& bucket = buckets[bucket_idx];
            const size_t bit_pos = slot % 64;
            
            // 使用CAS循环确保原子更新
            uint64_t old_mask = bucket.active_mask.load(std::memory_order_relaxed);
            while (!bucket.active_mask.compare_exchange_weak(old_mask, old_mask | (1ULL << bit_pos),
                std::memory_order_release, std::memory_order_relaxed)) {}
            
            bucket.timestamps[bit_pos] = now;
        }
        
        // 带参数的批量检测过期槽位
        template<typename F>
        void detect_expired_slots(size_t cutoff_time, F&& callback, size_t interval_ms, size_t batch_size) {
            const auto threshold = std::chrono::milliseconds(cutoff_time);
            const uint32_t epoch = current_epoch.fetch_add(1, std::memory_order_relaxed);
            for (auto& bucket : buckets) {
                const uint64_t mask = bucket.active_mask.exchange(0, std::memory_order_acquire);
                if (mask == 0) continue;
                
                const auto now = std::chrono::steady_clock::now();
                for (size_t i = 0; i < 64; ++i) {
                    if (mask & (1ULL << i)) {
                        if (now - bucket.timestamps[i] > threshold) {
                            callback(i); // 传递slot索引而不是时间戳
                        }
                    }
                }
            }
        }
    };

private:    
    // 声明分层时间轮成员变量
    std::array<HierarchicalTimeWheel, 3> time_wheels_; // 三级时间轮
    std::atomic<uint32_t> current_wheel_index_{0};

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
            session->send(package.release());
            /*auto data_size = session->send(package.release());
            if (data_size < 1){
                if (auto error = asio2::get_last_error()) {
                    slog->error("AntiCheatServer::send data size < 1, error: {}", error.message());
                }
            }*/
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
            session->async_send(package.release());
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
    // 分片数=CPU核心数×2的动态分片
    phmap::parallel_flat_hash_map<
        std::size_t, // Key
        std::chrono::steady_clock::time_point, // Value
        phmap::Hash<std::size_t>, // Hash
        phmap::EqualTo<std::size_t>, // Eq
        std::allocator<std::pair<const std::size_t, std::chrono::steady_clock::time_point>>, // Alloc
        32, // 分片数
        phmap::NullMutex // 互斥类型
    > session_last_active_times_;
    
    // 分层时间轮成员变量
    // 更新后的分片访问包装器（使用phmap线程安全遍历）
    class SubmapAccessor {
    public:
        explicit SubmapAccessor(decltype(session_last_active_times_)& map) : map_(map) {}
        
        template <typename F>
        void for_each(F&& f) {
            map_.with_submaps([&](auto& submap) {
                for (auto& kv : submap) {
                    f(kv);
                }
                return true;
            });
        }
    private:
        decltype(session_last_active_times_)& map_;
    };

protected:
    // 批量发送队列（每会话独立）
    struct alignas(64) SessionSendQueue {
        moodycamel::ConcurrentQueue<std::string> packets; // 还原为原始队列类型
        std::atomic<uint32_t> batch_counter{0};
        std::atomic_flag sending = ATOMIC_FLAG_INIT;
        static constexpr uint32_t BATCH_SIZE = 32;
        static constexpr uint32_t MAX_PACKET_SIZE = 4096;
    };
    phmap::parallel_node_hash_map<std::size_t,
                                std::unique_ptr<SessionSendQueue>,
                                phmap::Hash<std::size_t>,
                                phmap::EqualTo<std::size_t>,
                                phmap::priv::Allocator<std::pair<const std::size_t, std::unique_ptr<SessionSendQueue>>>> batch_send_queue_;

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
    std::chrono::steady_clock::time_point last_heartbeat_time{}; // 统一使用steady_clock
    unsigned char step = 0;
    json data;
    HeartbeatTracker heartbeat_tracker;
    char padding[64 - sizeof(std::shared_mutex)%64]; // 缓存行填充

    // 保持原有方法不变...
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

inline auto get_user_data_(const CAntiCheatServer::tcp_session_shared_ptr_t& session)
-> std::shared_ptr<AntiCheatUserData>
{
    try {
        // 双检锁+内存屏障优化
        auto userdata = session->get_user_data<std::shared_ptr<AntiCheatUserData>>();
        if (!userdata) { // 第一次无锁检查
            static std::mutex init_mutex;
            std::unique_lock<std::mutex> lock(init_mutex);
            userdata = session->get_user_data<std::shared_ptr<AntiCheatUserData>>(); // 第二次加锁检查
            if (!userdata) {
                userdata = std::make_shared<AntiCheatUserData>();
                session->set_user_data(userdata);
            }
        }
        return userdata;
    }
    catch (const std::bad_any_cast& e) {
        slog->error("Bad cast: {}", e.what());
        auto smart_ptr = std::make_shared<AntiCheatUserData>();
        session->set_user_data(smart_ptr);
        return smart_ptr;
    }
    catch (const std::exception& e) {
        slog->error("Get user data failed: {}", e.what());
        return std::make_shared<AntiCheatUserData>();
    }
}
