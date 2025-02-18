#pragma once
#include <any>
#include <shared_mutex>

namespace NetUtils
{
    template <typename event_handler_type>
    class EventMgr {
    private:
        // 分片数量（可根据硬件线程数调整）
        static constexpr size_t kNumShards = 64;

        // 每个分片包含独立的锁和处理器映射
        struct Shard {
            mutable std::shared_mutex mtx;
            std::unordered_map<unsigned int, std::shared_ptr<event_handler_type>> handlers;
        };

        std::array<Shard, kNumShards> shards_;

        // 根据事件 ID 选择分片（哈希取模）
        Shard& get_shard(unsigned int event_id) {
            return shards_[event_id % kNumShards];
        }

        const Shard& get_shard(unsigned int event_id) const {
            return shards_[event_id % kNumShards];
        }

    public:
        ~EventMgr() {
            clear_handler();
        }

        // 清空所有事件处理器
        void clear_handler() {
            for (auto& shard : shards_) {
                std::unique_lock lock(shard.mtx);
                shard.handlers.clear();
            }
        }

        // 移除指定事件 ID 的处理器
        bool remove_handler(unsigned int event_id) {
            auto& shard = get_shard(event_id);
            std::unique_lock lock(shard.mtx);
            return shard.handlers.erase(event_id) > 0;
        }

        // 替换指定事件 ID 的处理器
        bool replace_handler(unsigned int event_id, event_handler_type handler) {
            auto& shard = get_shard(event_id);
            std::unique_lock lock(shard.mtx);
            shard.handlers[event_id] = std::make_shared<event_handler_type>(std::move(handler));
            return true;
        }

        // 注册指定事件 ID 的处理器（若已存在则失败）
        bool register_handler(unsigned int event_id, event_handler_type handler) {
            auto& shard = get_shard(event_id);
            std::unique_lock lock(shard.mtx);
            auto [it, inserted] = shard.handlers.emplace(
                event_id,
                std::make_shared<event_handler_type>(std::move(handler))
            );
            return inserted;
        }

        // 获取指定事件 ID 的处理器（返回共享指针）
        std::shared_ptr<event_handler_type> get_handler(unsigned int event_id) const {
            const auto& shard = get_shard(event_id);
            std::shared_lock lock(shard.mtx);
            auto it = shard.handlers.find(event_id);
            return (it != shard.handlers.end()) ? it->second : nullptr;
        }

        // 高并发分发事件（无锁读 + 共享指针安全访问）
        template <typename... Args>
        bool dispatch(unsigned int event_id, Args&&... args) const {
            // 1. 获取对应的分片
            const auto& shard = get_shard(event_id);

            // 2. 无锁快速读取处理器指针（共享锁保护）
            std::shared_ptr<event_handler_type> handler;
            {
                std::shared_lock lock(shard.mtx);
                auto it = shard.handlers.find(event_id);
                if (it == shard.handlers.end()) {
                    return false; // 无处理器直接返回
                }
                handler = it->second; // 复制共享指针，增加引用计数
            }

            // 3. 执行处理器（无需持有锁）
            try {
                (*handler)(std::forward<Args>(args)...);
                return true;
            }
            catch (const std::exception& e) {
                // 可根据需要记录日志
                std::cerr << "Handler error: " << e.what() << std::endl;
            }
            catch (...) {
                std::cerr << "Unknown handler error" << std::endl;
            }
            return false;
        }

        // 异步分发通知，避免阻塞
        template<typename... Args>
        void async_dispatch(unsigned int event_id, Args&&... args) {
            // 使用线程池异步执行
            std::thread([this, event_id, args = std::tuple<std::decay_t<Args>...>(std::forward<Args>(args)...)]() mutable {
                try {
                    // 使用 std::apply 展开参数包
                    std::apply([this, event_id](auto&&... unpacked_args) {
                        dispatch(event_id, std::forward<decltype(unpacked_args)>(unpacked_args)...);
                    }, std::move(args));
                }
                catch (const std::exception& e) {
                    // 记录异步分发的异常
                    slog->error("Async dispatch failed for event_id={}: {}",
                                event_id, e.what());
                }
                catch (...) {
                    slog->error("Unknown async dispatch error for event_id={}", event_id);
                }
            }).detach();
        }
    };

    class UsersData {
    public:
        ~UsersData() {
            clear_field();
        }

        void clear_field() {
            std::unique_lock<std::shared_mutex> lck(mtx_);
            data_.clear();
        }

        void set_field(uint32_t field_id, std::any value) {
            std::unique_lock<std::shared_mutex> lck(mtx_);
            data_[field_id] = value;
        }

        std::any get_field(uint32_t field_id) {
            std::shared_lock<std::shared_mutex> lck(mtx_);
            if (data_.find(field_id) == data_.end())
                return std::any();
            return data_[field_id];
        }

        template <typename T>
        T get_field_as(uint32_t field_id) {
            std::shared_lock<std::shared_mutex> lck(mtx_);
            if (data_.find(field_id) == data_.end())
                throw std::runtime_error("Field not found");

            try {
                return std::any_cast<T>(data_[field_id]);
            }
            catch (const std::bad_any_cast& e) {
                throw std::runtime_error("Type cast failed: " + std::string(e.what()));
            }
        }

    private:
        std::shared_mutex mtx_;
        std::unordered_map<uint32_t, std::any> data_;
    };

    template<typename T>
    constexpr T __rol(T val, size_t count)
    {
        size_t bitcount = sizeof(T) * 8;

        count %= bitcount;
        return (val << count) | (val >> (bitcount - count));
    }

    template<typename T>
    constexpr T __ror(T val, size_t count)
    {
        size_t bitcount = sizeof(T) * 8;

        count %= bitcount;
        return (val >> count) | (val << (bitcount - count));
    }


    constexpr uint32_t hash(const wchar_t* buffer)
    {
        uint32_t _hash = 0;
        for (uint32_t n = 0; buffer[n]; n++)
        {
            _hash = __ror(_hash, 3);
            _hash += (buffer[n] >= 'a') ? _toupper(buffer[n]) : buffer[n];
        }
        return _hash;
    }

    constexpr uint32_t hash(const char* buffer)
    {
        uint32_t _hash = 0;
        for (uint32_t n = 0; buffer[n]; n++)
        {
            _hash = __ror(_hash, 3);
            _hash += (buffer[n] >= 'a') ? _toupper(buffer[n]) : buffer[n];
        }
        return _hash;
    }

    constexpr uint32_t hash(const wchar_t* buffer, size_t size)
    {
        uint32_t _hash = 0;
        for (uint32_t n = 0; n < size; n++)
        {
            _hash = __ror(_hash, 3);
            _hash += (buffer[n] >= 'a') ? _toupper(buffer[n]) : buffer[n];
        }
        return _hash;
    }

    constexpr uint32_t hash(const char* buffer, size_t size)
    {
        uint32_t _hash = 0;
        for (uint32_t n = 0; n < size; n++)
        {
            _hash = __ror(_hash, 3);
            _hash += (buffer[n] >= 'a') ? _toupper(buffer[n]) : buffer[n];
        }
        return _hash;
    }

    template <uint32_t n>
    struct GetValue
    {
        static const uint32_t value = n;
    };
	constexpr unsigned int aphash(const unsigned char* str, uint32_t len)
	{
		unsigned int hash = 0;
		for (uint32_t i = 0; i < len; i++)
		{
			if ((i & 1) == 0)
			{
				hash ^= ((hash << 7) ^ (str[i]) ^ (hash >> 3));
			}
			else
			{
				hash ^= (~((hash << 11) ^ (str[i]) ^ (hash >> 5)));
			}
		}
		return (hash & 0x7FFFFFFF);
	}
    constexpr unsigned int aphash(const char *str)
    {
        unsigned int hash = 0;
        for (int i = 0; *str; i++)
        {
            if ((i & 1) == 0)
            {
                hash ^= ((hash << 7) ^ (*str++) ^ (hash >> 3));
            }
            else
            {
                hash ^= (~((hash << 11) ^ (*str++) ^ (hash >> 5)));
            }
        }
        return (hash & 0x7FFFFFFF);
    }
}

#define DEFINE_TIMER_ID(x) x = NetUtils::GetValue<NetUtils::hash(#x)>::value
#define DEFINE_NOTIFY_ID(x) x = NetUtils::GetValue<NetUtils::hash(#x)>::value
#define DEFINE_FIELD_ID(x) x = NetUtils::GetValue<NetUtils::hash(#x)>::value

enum FieldId
{
    DEFINE_FIELD_ID(sysver_field_id),
    DEFINE_FIELD_ID(is_64bits_field_id),
    DEFINE_FIELD_ID(cpuid_field_id),
    DEFINE_FIELD_ID(mac_field_id),
    DEFINE_FIELD_ID(vol_field_id),
    DEFINE_FIELD_ID(rev_version_field_id),
    DEFINE_FIELD_ID(commited_hash_field_id),
    DEFINE_FIELD_ID(ip_field_id),
    DEFINE_FIELD_ID(port_field_id),
    DEFINE_FIELD_ID(test_mode_field_id),
    DEFINE_FIELD_ID(sec_no_change_field_id),
    DEFINE_FIELD_ID(usrname_field_id),
	DEFINE_FIELD_ID(hack_type_version_dll_field_id),
	DEFINE_FIELD_ID(is_client_field_id),
};
enum TimerId
{
    DEFINE_TIMER_ID(CLIENT_HEARTBEAT_TIMER_ID),
    //DEFINE_TIMER_ID(CLIENT_RECONNECT_TIMER_ID),
    DEFINE_TIMER_ID(AUTH_CHECK_TIMER_ID),
    DEFINE_TIMER_ID(UUID_CHECK_TIMER_ID),
    DEFINE_TIMER_ID(HEARTBEAT_CHECK_TIMER_ID),
    DEFINE_TIMER_ID(UPDATE_USERNAME_TIMER_ID),
    DEFINE_TIMER_ID(QUERY_PLUGIN_LIST_TIMER_ID),
    DEFINE_TIMER_ID(CLIENT_TIMEOUT_CHECK_TIMER_ID),
    DEFINE_TIMER_ID(DETECT_TCP_IP_TIMER_ID),
	DEFINE_TIMER_ID(RECONNECT_RESET_TIMER_ID),
};
enum NotifyId
{
    DEFINE_NOTIFY_ID(SERVER_START_NOTIFY_ID),
    DEFINE_NOTIFY_ID(CLIENT_START_NOTIFY_ID),
    DEFINE_NOTIFY_ID(CLIENT_DISCONNECT_NOTIFY_ID),
    DEFINE_NOTIFY_ID(CLIENT_CONNECT_SUCCESS_NOTIFY_ID),
    DEFINE_NOTIFY_ID(CLIENT_CONNECT_FAILED_NOTIFY_ID),
    DEFINE_NOTIFY_ID(CLIENT_RECONNECT_SUCCESS_NOTIFY_ID),
    DEFINE_NOTIFY_ID(CLIENT_HEARTBEAT_NOTIFY_ID),
    DEFINE_NOTIFY_ID(CLIENT_HANDSHAKE_NOTIFY_ID),
    DEFINE_NOTIFY_ID(ON_RECV_HANDSHAKE_NOTIFY_ID),
    DEFINE_NOTIFY_ID(ON_RECV_HEARTBEAT_NOTIFY_ID),
    DEFINE_NOTIFY_ID(PACKAGE_DECODE_ERROR_NOTIFY_ID),
    DEFINE_NOTIFY_ID(SEND_PACKAGE_ERROR_NOTIFY_ID),
    DEFINE_NOTIFY_ID(CLIENT_AUTH_SUCCESS_NOTIFY_ID),
    DEFINE_NOTIFY_ID(CLIENT_AUTH_FAILED_NOTIFY_ID),
    DEFINE_NOTIFY_ID(CLIENT_ON_SEND_PACKAGE_NOTIFY_ID),
    DEFINE_NOTIFY_ID(CLIENT_ON_RECV_PACKAGE_NOTIFY_ID),
	DEFINE_NOTIFY_ID(CLIENT_ON_JS_REPORT_NOTIFY_ID),
};
enum LogType
{
    LOG_TYPE_DEBUG = 1,
    LOG_TYPE_EVENT = 2,
    LOG_TYPE_ERROR = 4
};
const std::string kDefaultLocalhost = "127.0.0.1";
const unsigned short kDefaultServicePort = 23168;
const unsigned short kDefaultLogicServicePort = 23169;