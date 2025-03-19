#pragma once
#include <msgpack.hpp>
#if !defined(MSGPACK_DEFAULT_API_VERSION)
#define MSGPACK_DEFAULT_API_VERSION 1
#endif
#include <vector>
#include <atomic>
#include <mutex>
#include <thread>

class ThreadLocalPool {
    static constexpr size_t MAX_ZONE_SIZE = 2 * 1024 * 1024; // 2MB per thread
    static constexpr size_t CLEANUP_INTERVAL = 10;
    
    struct ZoneWrapper {
        msgpack::zone zone;
        std::atomic<bool> in_use{true};
    };

#if defined(_MSC_VER)
    __declspec(thread) static ZoneWrapper* tls_zone_;
#else
    thread_local static ZoneWrapper* tls_zone_;
#endif

    static std::mutex zones_mutex_;
    static std::vector<ZoneWrapper*> global_zones_;
    static std::atomic<size_t> global_index_;

public:
    static msgpack::zone& get_zone() {
        if (!tls_zone_ || 
#if MSGPACK_DEFAULT_API_VERSION >= 2
            tls_zone_->zone.size()
#else
            tls_zone_->zone.total_allocated()
#endif
            > MAX_ZONE_SIZE) {
            std::lock_guard<std::mutex> lock(zones_mutex_);
            
            // 尝试重用已存在的zone
            for (auto& zw : global_zones_) {
                bool expected = false;
                if (zw && zw->in_use.compare_exchange_strong(expected, true)) {
                    tls_zone_ = zw;
                    zw->zone.~zone();
                    new (&zw->zone) msgpack::zone();
                    return zw->zone;
                }
            }

            // 创建新zone
            tls_zone_ = new ZoneWrapper();
            global_zones_.push_back(tls_zone_);
            
            // 定期清理
            if (++global_index_ % CLEANUP_INTERVAL == 0) {
                auto it = std::remove_if(global_zones_.begin(), global_zones_.end(),
                    [](ZoneWrapper* zw) {
                        if (!zw->in_use.load()) {
                            delete zw;
                            return true;
                        }
                        return false;
                    });
                global_zones_.erase(it, global_zones_.end());
            }
        }
        return tls_zone_->zone;
    }

    static void release_zone() {
        if (tls_zone_) {
            tls_zone_->in_use.store(false);
            tls_zone_ = nullptr;
        }
    }
};

// 静态成员初始化
#if defined(_MSC_VER)
__declspec(thread) ThreadLocalPool::ZoneWrapper* ThreadLocalPool::tls_zone_ = nullptr;
#else
thread_local ThreadLocalPool::ZoneWrapper* ThreadLocalPool::tls_zone_ = nullptr;
#endif

std::mutex ThreadLocalPool::zones_mutex_;
std::vector<ThreadLocalPool::ZoneWrapper*> ThreadLocalPool::global_zones_;
std::atomic<size_t> ThreadLocalPool::global_index_{0};
