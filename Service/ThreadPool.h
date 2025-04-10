#pragma once
#include "pch.h"
#include "Protocol.h"
#include <asio2/tcp/tcp_session.hpp>
#include <memory>
#include <readerwriterqueue/readerwriterqueue.h>
#include <concurrentqueue/concurrentqueue.h>
using tcp_session_shared_ptr_t = std::shared_ptr<asio2::tcp_session>;
extern std::shared_ptr<spdlog::logger> slog;

struct Task {
    unsigned int package_id;
    tcp_session_shared_ptr_t session;
    RawProtocolImpl package;
    std::unique_ptr<msgpack::v1::object_handle> raw_msg;  // 改为智能指针

    // 默认构造函数
    Task()
        : package_id(0),
        session(nullptr),
        package(),
        raw_msg(nullptr)
    {
    }
    Task(unsigned int id, tcp_session_shared_ptr_t s,
         const RawProtocolImpl& p, msgpack::v1::object_handle&& msg)
        : package_id(id),
        session(std::move(s)),
        package(p),
        raw_msg(std::make_unique<msgpack::v1::object_handle>(std::move(msg)))
    {
        if (!raw_msg) {
            slog->error("Failed to initialize raw_msg in Task constructor");
        }
    }

    // 修改移动构造函数和移动赋值运算符，移除冗余操作
    Task(Task&& other) noexcept
        : package_id(other.package_id),
        session(std::move(other.session)),
        package(std::move(other.package)),
        raw_msg(std::move(other.raw_msg))
    {
        other.package_id = 0; // 仅置package_id，其他由移动操作处理
    }

    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            // 成员移动自动处理旧资源
            raw_msg.reset();
            package_id = other.package_id;
            session = std::move(other.session);
            package = std::move(other.package);
            raw_msg = std::move(other.raw_msg);

            other.package_id = 0;
        }
        return *this;
    }
    // 添加资源跟踪日志
    ~Task() = default;
    // 禁止拷贝
    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;
};

static constexpr size_t MAX_LOCAL_QUEUE_SIZE = 8192 * 2;  // 增大本地队列容量

struct TaskWrapper {
    Task task;
    explicit TaskWrapper(Task&& t) noexcept : task(std::move(t)) {}

    // 确保移动安全
    TaskWrapper(TaskWrapper&& other) noexcept : task(std::move(other.task)) {}
    TaskWrapper& operator=(TaskWrapper&& other) noexcept {
        task = std::move(other.task);
        return *this;
    }
};

// 线程本地队列（SPSC）
struct alignas(64) WorkerContext {
    moodycamel::ReaderWriterQueue<TaskWrapper*> local_queue{ MAX_LOCAL_QUEUE_SIZE };
    //moodycamel::ConcurrentQueue<TaskWrapper*> local_pool;  // 本地内存池
    std::atomic<bool> active{ true };
    std::atomic<size_t> total_steals{ 0 };  // 窃取统计
};
class BusinessThreadPool {
public:
    explicit BusinessThreadPool(size_t thread_count = std::thread::hardware_concurrency());

    ~BusinessThreadPool();
private:
    // 修改监控线程实现
    void start_monitor(const size_t INITIAL_POOL_SIZE);

    void worker_loop(std::shared_ptr<WorkerContext>& ctx, size_t worker_id);

    size_t process_local(WorkerContext* ctx, std::vector<TaskWrapper*>& batch);

    size_t process_global(std::vector<TaskWrapper*>& batch);

    size_t try_steal(std::vector<TaskWrapper*>& batch, size_t thief_id);

    void execute_task(TaskWrapper* task);

    void handle_idle(size_t processed, int& empty_cycles);

    void drain_queue(std::shared_ptr<WorkerContext>& ctx);
public:
    bool enqueue(Task task);

    void check_memory_usage();

    void log_pool_stats();
    void log_thread_pool_status();
private:
    // 优化后的配置参数
    // 添加内存池配置参数
    static constexpr size_t MEMORY_POOL_LOW_WATERMARK = 2048;  // 低水位线
    static constexpr size_t GLOBAL_QUEUE_MAX = 5000;         // 新增全局队列容量限制
    static constexpr size_t WORKER_BATCH_SIZE = 64;      // 增加批量处理数量
    static constexpr int MAX_EMPTY_ITERATIONS = 50;       // 减少空转次数
    static constexpr size_t MEMORY_POOL_INIT_SIZE = 4096;// 预分配内存池大小

    // 全局队列（MPMC）
    moodycamel::ConcurrentQueue<TaskWrapper*> global_queue_;

    // 工作线程管理
    std::vector<std::thread> workers_;
    std::atomic<bool> stop_{ false };
    std::atomic<bool> monitor_stop_{ false };  // 专用停止标志
    std::atomic<int64_t> pending_tasks_{ 0 };
    std::thread monitor_thread_;  // 添加监控线程成员

    // 内存池和线程上下文
    moodycamel::ConcurrentQueue<TaskWrapper*> task_pool_;
    // 使用weak_ptr避免悬挂指针
    std::vector<std::weak_ptr<WorkerContext>> worker_contexts_;
    std::shared_mutex context_mutex_;
};

