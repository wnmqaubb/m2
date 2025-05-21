#pragma once
#include "AntiCheatClient.h"
#include "AntiCheatServer.h"
#include "LogicClient.h"
#include "Protocol.h"
#include <asio2/tcp/tcp_server.hpp>
#include <memory>
#include <msgpack/v1/object.hpp>
#include <mutex>
#include <shared_mutex>
#include <string>
//#include <ConcurrentQueue/BlockingConcurrentQueue.h>
//#include <folly/ProducerConsumerQueue.h>
//#include <readerwriterqueue/readerwriterqueue.h>
//#define _DEBUG
class CAntiCheatServer;
class CObserverServer : public CAntiCheatServer {
    using super = CAntiCheatServer;

public:
    void batch_heartbeat(const std::vector<tcp_session_shared_ptr_t>& sessions);
    CObserverServer();
    void on_recv_client_heartbeat(tcp_session_shared_ptr_t& session);
    void obpkg_id_c2s_auth(tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg);
    static CObserverServer& instance() {
        static CObserverServer instance;
        return instance;
    }

    void connect_to_logic_server(const std::string& ip, unsigned short port);
    void logic_client_stop() { logic_client_->stop(); }
    void stop() { logic_client_->stop(); super::stop(); }

    std::wstring& get_vmp_expire() { return vmp_expire_; }
    void process_task(Task&& task);

    void log_cb(const wchar_t* msg, bool silence, bool gm_show, const std::string& identify, bool punish_flag);

protected:
    bool on_recv(unsigned int package_id, tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, msgpack::v1::object_handle&& raw_msg) override;
    void on_post_disconnect(tcp_session_shared_ptr_t& session) override;
    std::wstring vmp_expire_;
    std::shared_ptr<CLogicClient> logic_client_;
    NetUtils::EventMgr<package_handler_t> ob_pkg_mgr_;
    std::shared_mutex shared_mtx_;
    std::shared_timed_mutex task_mtx_;
    std::timed_mutex session_mtx_; 
public:
    std::shared_mutex session_times_mtx_;
};


// class HeartbeatThreadPool {
// private:
//     static constexpr size_t BUFFER_SIZE = 65536;  // 2^16
//     static constexpr size_t BATCH_SIZE = 1024;     // 256个任务/批次
//     static constexpr DWORD IDLE_SLEEP_MS = 1;

//     struct alignas(64) HeartbeatTask {  // 缓存行对齐
//         std::size_t session_id;
//         time_t timestamp;
//     };

//     //folly::ProducerConsumerQueue<HeartbeatTask> queue_{ BUFFER_SIZE };
//     //moodycamel::ReaderWriterQueue<HeartbeatTask> queue_{ BUFFER_SIZE };
//     // 使用更高性能的队列
//     moodycamel::BlockingConcurrentQueue<HeartbeatTask> queue_{ BUFFER_SIZE * 4 };
//     std::atomic<bool> running_{ true };
//     std::vector<std::thread> workers_;
//     std::atomic<size_t> dropped_{ 0 };

//     // 高性能计数器
//     LARGE_INTEGER frequency_;
//     std::atomic<uint64_t> total_processed_{ 0 };

// public:
//     explicit HeartbeatThreadPool(size_t threads = std::thread::hardware_concurrency()) {
//         QueryPerformanceFrequency(&frequency_);
//         workers_.reserve(threads);
//         for (size_t i = 0; i < threads; ++i) {
//             workers_.emplace_back([this] {
//                 SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
//                 SetThreadAffinityMask(GetCurrentThread(), 0x0F);  // 绑定到前4个核心

//                 HeartbeatTask tasks[BATCH_SIZE];
//                 while (running_.load(std::memory_order_relaxed)) {
//                     size_t count = try_read_batch(tasks, BATCH_SIZE);
//                     if (count > 0) {
//                         process_batch(tasks, count);
//                         total_processed_.fetch_add(count);
//                     }
//                     else {
//                         Sleep(IDLE_SLEEP_MS);
//                     }
//                 }
//             });
//         }
//     }

//     ~HeartbeatThreadPool() {
//         running_ = false;
//         for (auto& t : workers_) if (t.joinable()) t.join();

//         slog->info("Heartbeat stats: Processed={} Dropped={}", total_processed_.load(), dropped_.load());
//     }

//     bool push(std::size_t session_id, time_t timestamp) noexcept {  
//         HeartbeatTask task{ session_id, timestamp };      
//         // 优先非阻塞写入
//         if (queue_.try_enqueue(task)) return true;
        
//         // 使用正确的阻塞写入方法
//         if (queue_.enqueue(task)) return true;  // 移除非法的timed参数
        
//         dropped_.fetch_add(1, std::memory_order_relaxed);
//         return false;
//     }

// private:
//     size_t try_read_batch(HeartbeatTask* tasks, size_t max) {
//         size_t count = 0;
//         while (count < max && queue_.try_dequeue(tasks[count])) {
//             ++count;
//         }
//         return count;
//     }

//     void process_batch(HeartbeatTask* tasks, size_t count) {
//         std::vector<tcp_session_shared_ptr_t> sessions;
//         sessions.reserve(count);
//         // 批量获取session
//         for (size_t i = 0; i < count; ++i) {
//             if (auto session = CObserverServer::instance().find_session(tasks[i].session_id)) {
//                 sessions.emplace_back(std::move(session));
//             }
//         }
        
//         // 批量处理心跳
//         if (!sessions.empty()) {
//             CObserverServer::instance().batch_heartbeat(sessions);
//         }
//     }
// };

//static HeartbeatThreadPool g_heartbeat_pool;