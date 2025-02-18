#pragma once
#include "AntiCheatClient.h"
#include "AntiCheatServer.h"
#include "LogicClient.h"
#include "Protocol.h"
#include <asio2/tcp/tcp_server.hpp>
#include <asio2/tcp/tcp_session.hpp>
#include <chrono>
#include <memory>
#include <msgpack/v1/object.hpp>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <type_traits>
#include <unordered_map>
//#include <folly/ProducerConsumerQueue.h>
#include <readerwriterqueue/readerwriterqueue.h>
using tcp_session_shared_ptr_t = std::shared_ptr<asio2::tcp_session>;

struct Task {
    unsigned int package_id;
    tcp_session_shared_ptr_t session;
    RawProtocolImpl package;
    msgpack::v1::object_handle raw_msg;

    Task() = default;
    Task(unsigned int id, tcp_session_shared_ptr_t s, const RawProtocolImpl& p, msgpack::v1::object_handle&& msg)
        : package_id(id), session(std::move(s)), package(p), raw_msg(std::move(msg)) {
    }

    // 禁止拷贝
    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    // 允许移动
    Task(Task&&) noexcept = default;
    Task& operator=(Task&&) noexcept = default;
};

class CObserverServer : public CAntiCheatServer {
    using super = CAntiCheatServer;

public:
    CObserverServer();
    void on_recv_client_heartbeat(tcp_session_shared_ptr_t& session);
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
    std::unordered_map<tcp_session_shared_ptr_t, std::chrono::steady_clock::time_point> session_last_active_times_;
    std::shared_timed_mutex task_mtx_;
    std::timed_mutex session_mtx_;  // 用于保护session相关操作
public:
    std::mutex session_times_mtx_;
};


class HeartbeatThreadPool {
private:
    static constexpr size_t BUFFER_SIZE = 65536;  // 2^16
    static constexpr size_t BATCH_SIZE = 256;     // 256个任务/批次
    static constexpr DWORD IDLE_SLEEP_MS = 1;

    struct alignas(64) HeartbeatTask {  // 缓存行对齐
        std::size_t session_id;
        time_t timestamp;
    };

    //folly::ProducerConsumerQueue<HeartbeatTask> queue_{ BUFFER_SIZE };
    moodycamel::ReaderWriterQueue<HeartbeatTask> queue_{ BUFFER_SIZE };
    std::atomic<bool> running_{ true };
    std::vector<std::thread> workers_;
    std::atomic<size_t> dropped_{ 0 };

    // 高性能计数器
    LARGE_INTEGER frequency_;
    std::atomic<uint64_t> total_processed_{ 0 };

public:
    explicit HeartbeatThreadPool(size_t threads = std::thread::hardware_concurrency()) {
        QueryPerformanceFrequency(&frequency_);
        workers_.reserve(threads);
        for (size_t i = 0; i < threads; ++i) {
            workers_.emplace_back([this] {
                SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
                SetThreadAffinityMask(GetCurrentThread(), 0x0F);  // 绑定到前4个核心

                HeartbeatTask tasks[BATCH_SIZE];
                while (running_.load(std::memory_order_relaxed)) {
                    size_t count = try_read_batch(tasks, BATCH_SIZE);
                    if (count > 0) {
                        LARGE_INTEGER start, end;
                        QueryPerformanceCounter(&start);

                        process_batch(tasks, count);

                        QueryPerformanceCounter(&end);
                        total_processed_.fetch_add(count);
                    }
                    else {
                        Sleep(IDLE_SLEEP_MS);
                    }
                }
            });
        }
    }

    ~HeartbeatThreadPool() {
        running_ = false;
        for (auto& t : workers_) if (t.joinable()) t.join();

        slog->info("Heartbeat stats: Processed={} Dropped={}", total_processed_.load(), dropped_.load());
    }

    bool push(std::size_t session_id, time_t timestamp) noexcept {
        HeartbeatTask task{ session_id, timestamp };
        if (queue_.try_enqueue(task)) return true;
        dropped_.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

private:
    size_t try_read_batch(HeartbeatTask* tasks, size_t max) {
        size_t count = 0;
        while (count < max && queue_.try_dequeue(tasks[count])) {
            ++count;
        }
        return count;
    }

    void process_batch(HeartbeatTask* tasks, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            if (auto session = CObserverServer::instance().find_session(tasks[i].session_id)) {
                CObserverServer::instance().on_recv_client_heartbeat(session);
            }
        }
    }
};

static HeartbeatThreadPool g_heartbeat_pool;