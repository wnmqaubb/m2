#pragma once
#include <any>
#include <shared_mutex>

namespace NetUtils
{
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
            event_handler_type handler;
            {
                // 使用局部作用域，快速释放读锁
                //std::shared_lock<std::shared_mutex> lck(mtx_);
                auto it = handlers_.find(event_id);
                if (it == handlers_.end())
                    return false;
                handler = it->second;  // 复制处理器
            }

            // 在锁外执行处理器
            if (handler) {
                handler(std::forward<Args>(args)...);
            }
            return true;
        }
        std::shared_mutex mtx_;
        std::unordered_map<unsigned int, event_handler_type> handlers_;
    };
    /*
    template <typename event_handler_type>
    class EventMgr {
    private:
        // 使用原子操作和无锁容器
        std::atomic<std::unordered_map<unsigned int, event_handler_type>*> handlers_;

        // 使用原子标志防止重入
        struct alignas(std::hardware_destructive_interference_size) ThreadSafeFlag {
            std::atomic<bool> flag{ false };

            bool try_lock() {
                bool expected = false;
                return flag.compare_exchange_strong(
                    expected,
                    true,
                    std::memory_order_acquire,
                    std::memory_order_relaxed
                );
            }

            void unlock() {
                flag.store(false, std::memory_order_release);
            }
        };

        // 每个线程的重入保护
        static thread_local ThreadSafeFlag dispatch_lock;

        // 诊断信息
        struct EventDiagnostics {
            std::atomic<size_t> total_dispatches{ 0 };
            std::atomic<size_t> failed_dispatches{ 0 };
            std::atomic<size_t> locked_dispatches{ 0 };
        } diagnostics;

    public:
        // 构造函数初始化
        EventMgr() {
            handlers_.store(
                new std::unordered_map<unsigned int, event_handler_type>(),
                std::memory_order_release
            );
        }

        // 安全的内存释放
        ~EventMgr() {
            auto handlers = handlers_.load(std::memory_order_acquire);
            delete handlers;
        }

        // 清空所有处理器
        inline void clear_handler() {
            auto handlers = handlers_.load(std::memory_order_acquire);
            delete handlers;
            handlers_.store(new std::unordered_map<unsigned int, event_handler_type>(),
                std::memory_order_release);
        }

        // 移除特定事件处理器
        inline bool remove_handler(unsigned int event_id) {
            while (true) {
                auto old_handlers = handlers_.load(std::memory_order_relaxed);
                auto new_handlers = new std::unordered_map<unsigned int, event_handler_type>(*old_handlers);

                auto it = new_handlers->find(event_id);
                if (it == new_handlers->end()) {
                    delete new_handlers;
                    return false;
                }

                new_handlers->erase(it);

                if (handlers_.compare_exchange_weak(
                    old_handlers,
                    new_handlers,
                    std::memory_order_release,
                    std::memory_order_relaxed
                )) {
                    delete old_handlers;
                    return true;
                }

                delete new_handlers;
            }
        }

        // 替换事件处理器
        inline bool replace_handler(unsigned int event_id, event_handler_type handler) {
            while (true) {
                auto old_handlers = handlers_.load(std::memory_order_relaxed);
                auto new_handlers = new std::unordered_map<unsigned int, event_handler_type>(*old_handlers);

                (*new_handlers)[event_id] = std::move(handler);

                if (handlers_.compare_exchange_weak(
                    old_handlers,
                    new_handlers,
                    std::memory_order_release,
                    std::memory_order_relaxed
                )) {
                    delete old_handlers;
                    return true;
                }

                delete new_handlers;
            }
        }

        // 注册事件处理器
        inline bool register_handler(unsigned int event_id, event_handler_type handler) {
            while (true) {
                auto old_handlers = handlers_.load(std::memory_order_relaxed);
                auto new_handlers = new std::unordered_map<unsigned int, event_handler_type>(*old_handlers);

                auto it = new_handlers->find(event_id);
                if (it != new_handlers->end()) {
                    // 可选：是否覆盖或抛出异常
                    return false;
                }

                (*new_handlers)[event_id] = std::move(handler);

                if (handlers_.compare_exchange_weak(
                    old_handlers,
                    new_handlers,
                    std::memory_order_release,
                    std::memory_order_relaxed
                )) {
                    delete old_handlers;
                    return true;
                }

                delete new_handlers;
            }
        }

        // 获取事件处理器
        inline event_handler_type get_handler(unsigned int event_id) {
            auto current_handlers = handlers_.load(std::memory_order_acquire);
            auto it = current_handlers->find(event_id);
            if (it == current_handlers->end())
                return []() {};
            return it->second;
        }

        // 事件分发（核心改进）
        template <typename... Args>
        bool dispatch(unsigned int event_id, Args&&... args) {
            // 防止重入
            if (!dispatch_lock.try_lock()) {
                diagnostics.locked_dispatches++;
                return false;
            }

            // RAII风格的锁管理
            struct LockGuard {
                ThreadSafeFlag& lock;
                LockGuard(ThreadSafeFlag& l) : lock(l) {}
                ~LockGuard() { lock.unlock(); }
            } guard(dispatch_lock);

            // 获取处理器
            auto current_handlers = handlers_.load(std::memory_order_acquire);
            if (!current_handlers) {
                diagnostics.failed_dispatches++;
                return false;
            }

            // 原子获取处理器
            event_handler_type handler;
            {
                auto it = current_handlers->find(event_id);
                if (it == current_handlers->end()) {
                    diagnostics.failed_dispatches++;
                    return false;
                }
                handler = it->second;
            }

            // 性能和异常安全的执行
            try {
                auto start = std::chrono::steady_clock::now();

                // 执行处理器
                handler(std::forward<Args>(args)...);

                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::steady_clock::now() - start
                );

                diagnostics.total_dispatches++;

                return true;
            }
            catch (const std::exception& e) {
                diagnostics.failed_dispatches++;
                char buffer[256];
                sprintf_s(buffer, "Event dispatch failed for event_id %u: %s",
                    event_id, e.what());
                OutputDebugStringA(buffer);
                return false;
            }
            catch (...) {
                diagnostics.failed_dispatches++;
                char buffer[256];
                sprintf_s(buffer, "Unknown error in event dispatch for event_id %u", event_id);
                OutputDebugStringA(buffer);
                return false;
            }
        }

        // 获取诊断信息
        EventDiagnostics get_diagnostics() const {
            return diagnostics;
        }
    };

    // 静态成员初始化
    template <typename event_handler_type>
    thread_local typename EventMgr<event_handler_type>::ThreadSafeFlag
        EventMgr<event_handler_type>::dispatch_lock;
        */

    //template <typename EventHandlerType>
    //class ConcurrentEventDispatcher
    //{
    //    private:
    //        // 使用读写自旋锁
    //        mutable std::shared_mutex mtx_;
    //        std::unordered_map<unsigned int, EventHandlerType> handlers_;

    //        // 线程池用于并发执行
    //        std::unique_ptr<ThreadPool> thread_pool_;

    //    public:
    //        // 支持并发和异步分发
    //        template <typename... Args>
    //        void dispatch_async(unsigned int event_id, Args&&... args) {
    //            std::shared_lock<std::shared_mutex> lock(mtx_);
    //            auto it = handlers_.find(event_id);
    //            if (it != handlers_.end()) {
    //                // 异步执行处理器
    //                thread_pool_->enqueue([
    //                    handler = it->second,
    //                        ...args = std::forward<Args>(args)
    //                ]() mutable {
    //                        handler(std::forward<Args>(args)...);
    //                    });
    //            }
    //        }

    //        // 同步分发，但带超时
    //        template <typename... Args>
    //        bool dispatch_sync(
    //            unsigned int event_id,
    //            std::chrono::milliseconds timeout,
    //            Args&&... args
    //        ) {
    //            std::promise<bool> promise;
    //            std::future<bool> future = promise.get_future();

    //            {
    //                std::shared_lock<std::shared_mutex> lock(mtx_);
    //                auto it = handlers_.find(event_id);
    //                if (it == handlers_.end()) {
    //                    return false;
    //                }

    //                thread_pool_->enqueue([
    //                    &promise,
    //                        handler = it->second,
    //                        ...args = std::forward<Args>(args)
    //                ]() mutable {
    //                        try {
    //                            handler(std::forward<Args>(args)...);
    //                            promise.set_value(true);
    //                        }
    //                        catch (...) {
    //                            promise.set_exception(std::current_exception());
    //                        }
    //                    });
    //            }

    //            // 带超时的等待
    //            return future.wait_for(timeout) == std::future_status::ready
    //                && future.get();
    //        }
    //};
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