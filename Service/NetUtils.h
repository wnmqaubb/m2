#pragma once
#include <any>
#include <shared_mutex>

namespace NetUtils
{

    class CTimerMgr
    {
    public:
        CTimerMgr(asio::io_service& io) :io_(io) {};
        ~CTimerMgr() { stop_all_timer(); }
        struct TimerInfo
        {
            unsigned int timer_id;
            asio::steady_timer timer;
            bool exited = false;
            std::function<void()> cb;
            TimerInfo(unsigned int id, asio::io_service& service, std::function<void()> t)
                : timer_id(id), timer(service), cb(std::move(t)) {}
        };
    public:
        virtual void start_timer(unsigned int timer_id, std::chrono::system_clock::duration duration, std::function<void()> cb)
        {
            asio::post(io_, [this, timer_id, duration, cb = std::move(cb)]() {
                std::unique_lock<std::shared_mutex> lck(mtx_);
                if (timers_.find(timer_id) != timers_.end())
                {
                    return;
                }
                auto timer_info = std::make_shared<TimerInfo>(timer_id, io_, std::move(cb));
                timers_[timer_id] = timer_info;
                _post_timer(std::move(timer_info), duration);
            });
        }
        virtual void stop_timer(unsigned int timer_id)
        {
            asio::post(io_, [this, timer_id]() {
                std::unique_lock<std::shared_mutex> lck(mtx_);
                if (timers_.find(timer_id) == timers_.end())
                    return;
                timers_[timer_id]->timer.cancel();
                timers_.erase(timer_id);
            });
        }

        virtual void stop_all_timer()
        {
            asio::post(io_, [this]() {
                std::unique_lock<std::shared_mutex> lck(mtx_);
                for (auto& timer_info : timers_)
                {
                    timer_info.second->timer.cancel();
                }
                timers_.clear();
            });
        }

    protected:
        void _post_timer(const std::shared_ptr<TimerInfo>& timer_info, std::chrono::system_clock::duration duration)
        {
            timer_info->timer.expires_after(duration);
            timer_info->timer.async_wait([this, duration, timer_info = std::move(timer_info)](std::error_code ec) {
                std::shared_lock<std::shared_mutex> lck(mtx_);
                if (ec == asio::error::operation_aborted)
                {
                    timer_info->exited = true;
                    return;
                }
                timer_info->cb();
                _post_timer(timer_info, duration);
            });
        }
        std::shared_mutex mtx_;
        std::unordered_map<unsigned int, std::shared_ptr<TimerInfo>> timers_;
        asio::io_service& io_;
    };


    template <typename event_handler_type>
    class EventMgr
    {
    public:
        ~EventMgr()
        {
            clear_handler();
        }
        inline void clear_handler()
        {
            std::unique_lock<std::shared_mutex> lck(mtx_);
            handlers_.clear();
        }
        inline bool remove_handler(unsigned int event_id)
        {
            std::unique_lock<std::shared_mutex> lck(mtx_);
            if (handlers_.find(event_id) == handlers_.end())
                return false;
            handlers_.erase(event_id);
            return true;
        }
        inline bool replace_handler(unsigned int event_id, event_handler_type handler)
        {
            std::unique_lock <std::shared_mutex> lck(mtx_);
            handlers_[event_id] = std::move(handler);
            return true;
        }
        inline bool register_handler(unsigned int event_id, event_handler_type handler)
        {
            std::unique_lock <std::shared_mutex> lck(mtx_);
            if (handlers_.find(event_id) != handlers_.end())
                return false;
            handlers_[event_id] = std::move(handler);
            return true;
        }
        inline event_handler_type get_handler(unsigned int event_id)
        {
            std::shared_lock<std::shared_mutex> lck(mtx_);
            if (handlers_.find(event_id) == handlers_.end())
                return []() {};
            return handlers_[event_id];
        }
        template <typename... Args>
        inline bool dispatch(unsigned int event_id, Args&&... args)
        {
            std::shared_lock<std::shared_mutex> lck(mtx_);
            if (handlers_.find(event_id) == handlers_.end())
                return false;
            handlers_[event_id](std::forward<Args>(args)...);
            return true;
        }
        std::shared_mutex mtx_;
        std::unordered_map<unsigned int, event_handler_type> handlers_;
    };
    class UsersData
    {
    public:
        ~UsersData()
        {
            clear_field();
        }
        virtual void clear_field()
        {
            std::unique_lock<std::shared_mutex> lck(mtx_);
            data_.clear();
        }
        virtual void set_field(uint32_t field_id, std::any value) 
        {
            std::unique_lock<std::shared_mutex> lck(mtx_);
            data_[field_id] = value;
        }
        virtual std::any get_field(uint32_t field_id)
        {
            std::shared_lock<std::shared_mutex> lck(mtx_);
            if (data_.find(field_id) == data_.end())
                return std::any();
            return data_[field_id];
        }
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
};
enum TimerId
{
    DEFINE_TIMER_ID(CLIENT_HEARTBEAT_TIMER_ID),
    DEFINE_TIMER_ID(CLIENT_RECONNECT_TIMER_ID),
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
const unsigned short kDefaultServicePort = 22228;
const unsigned short kDefaultLogicServicePort = 22229;