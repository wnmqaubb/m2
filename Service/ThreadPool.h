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
    std::unique_ptr<msgpack::v1::object_handle> raw_msg;  // ��Ϊ����ָ��

    // Ĭ�Ϲ��캯��
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

    // �޸��ƶ����캯�����ƶ���ֵ��������Ƴ��������
    Task(Task&& other) noexcept
        : package_id(other.package_id),
        session(std::move(other.session)),
        package(std::move(other.package)),
        raw_msg(std::move(other.raw_msg))
    {
        other.package_id = 0; // ����package_id���������ƶ���������
    }

    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            // ��Ա�ƶ��Զ��������Դ
            raw_msg.reset();
            package_id = other.package_id;
            session = std::move(other.session);
            package = std::move(other.package);
            raw_msg = std::move(other.raw_msg);

            other.package_id = 0;
        }
        return *this;
    }
    // �����Դ������־
    ~Task() = default;
    // ��ֹ����
    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;
};

static constexpr size_t MAX_LOCAL_QUEUE_SIZE = 8192*4;  // ���󱾵ض�������

struct TaskWrapper {
    Task task;
    explicit TaskWrapper(Task&& t) noexcept : task(std::move(t)) {}

    // ȷ���ƶ���ȫ
    TaskWrapper(TaskWrapper&& other) noexcept : task(std::move(other.task)) {}
    TaskWrapper& operator=(TaskWrapper&& other) noexcept {
        task = std::move(other.task);
        return *this;
    }
};

// �̱߳��ض��У�SPSC��
struct alignas(64) WorkerContext {
    moodycamel::ReaderWriterQueue<TaskWrapper*> local_queue{ MAX_LOCAL_QUEUE_SIZE };
    //moodycamel::ConcurrentQueue<TaskWrapper*> local_pool;  // �����ڴ��
    std::atomic<bool> active{ true };
    std::atomic<size_t> total_steals{ 0 };  // ��ȡͳ��
};
class BusinessThreadPool {
public:
    explicit BusinessThreadPool(size_t thread_count = std::thread::hardware_concurrency());

    ~BusinessThreadPool();
private:
    // �޸ļ���߳�ʵ��
    void start_monitor();

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
    // �Ż�������ò���
    // ����ڴ�����ò���
    static constexpr size_t MEMORY_POOL_LOW_WATERMARK = 4096;  // ��ˮλ��
    static constexpr size_t GLOBAL_QUEUE_MAX = 100000;         // ����ȫ�ֶ�����������
    static constexpr size_t WORKER_BATCH_SIZE = 128;      // ����������������
    static constexpr int MAX_EMPTY_ITERATIONS = 50;       // ���ٿ�ת����
    static constexpr size_t MEMORY_POOL_INIT_SIZE = 16384;// Ԥ�����ڴ�ش�С

    // ȫ�ֶ��У�MPMC��
    moodycamel::ConcurrentQueue<TaskWrapper*> global_queue_;

    // �����̹߳���
    std::vector<std::thread> workers_;
    std::atomic<bool> stop_{ false };
    std::atomic<bool> monitor_stop_{ false };  // ר��ֹͣ��־
    std::atomic<int64_t> pending_tasks_{ 0 };
    std::thread monitor_thread_;  // ��Ӽ���̳߳�Ա

    // �ڴ�غ��߳�������
    moodycamel::ConcurrentQueue<TaskWrapper*> task_pool_;
    // ʹ��weak_ptr��������ָ��
    std::vector<std::weak_ptr<WorkerContext>> worker_contexts_;
    std::shared_mutex context_mutex_;
};

