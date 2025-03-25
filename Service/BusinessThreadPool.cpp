#include "pch.h"
#include "ThreadPool.h"
#ifdef G_SERVICE
#include <ObserverServer.h>
#else
#include <LogicServer.h>
#endif


BusinessThreadPool::BusinessThreadPool(size_t thread_count)
    : global_queue_(thread_count * 4096/*8192*/)
{
    // Ԥ�����ڴ�ض��󣨼��� Task ��Ĭ�Ϲ��죩
    const size_t INITIAL_POOL_SIZE = global_queue_.size_approx() * 4;
    for (size_t i = 0; i < INITIAL_POOL_SIZE; ++i) {
        //TaskWrapper* wrapper = new TaskWrapper(Task{});  // �� Task ֧��Ĭ�Ϲ���
        auto* wrapper = new TaskWrapper(
            Task(0, nullptr, RawProtocolImpl{}, msgpack::v1::object_handle{})
        );
        task_pool_.enqueue(wrapper);
    }
    workers_.reserve(thread_count);
    for (size_t i = 0; i < thread_count; ++i) {
        workers_.emplace_back([this, i] {
            auto ctx = std::make_shared<WorkerContext>();
            {
                std::unique_lock lock(context_mutex_);
                worker_contexts_.emplace_back(ctx);
            }

            worker_loop(ctx, i);

            // �����߼�
            {
                std::unique_lock lock(context_mutex_);
                auto it = std::find_if(worker_contexts_.begin(), worker_contexts_.end(),
                                       [&](const std::weak_ptr<WorkerContext>& weak_ctx) {
                    auto shared_ctx = weak_ctx.lock();
                    return shared_ctx && (shared_ctx == ctx);  // �Ƚ�shared_ptr����
                });
            }

            ctx->active.store(false, std::memory_order_release);
            drain_queue(ctx);
        });

    }
    // �ڹ��캯������������߳�
    start_monitor();
}

BusinessThreadPool::~BusinessThreadPool() {
    stop_.store(true, std::memory_order_release);
    // ��ֹͣ����߳�
    monitor_stop_.store(true, std::memory_order_release);
    if (monitor_thread_.joinable()) {
        monitor_thread_.join();
    }

    // �ȴ������������
    while (pending_tasks_.load(std::memory_order_acquire) > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // ֹͣ�����߳�
    for (auto& w : workers_) {
        if (w.joinable()) w.join();
    }
    // �����ڴ��
    TaskWrapper* task;
    while (task_pool_.try_dequeue(task)) {
        delete task;
    }
}

bool BusinessThreadPool::enqueue(Task task) {
    if (stop_.load(std::memory_order_acquire)) {
        slog->warn("Enqueue rejected: Thread pool is stopping");
        return false;
    }

    // ��ȡ�򴴽������װ��
    TaskWrapper* wrapper = nullptr;
    if (!task_pool_.try_dequeue(wrapper)) {
        constexpr int BATCH_SIZE = 64;
        for (int i = 0; i < BATCH_SIZE; ++i) {
            auto* prealloc = new TaskWrapper(Task{});
            if (!task_pool_.enqueue(prealloc)) {
                delete prealloc;
            }
        }
        if (!task_pool_.try_dequeue(wrapper)) {
            wrapper = new TaskWrapper(Task{});
        }
    }

    // ��ȫ�ƶ�����
    try {
        wrapper->task = std::move(task);
    }
    catch (const std::exception& e) {
        slog->error("Task move failed: {}", e.what());
        task_pool_.enqueue(wrapper);
        return false;
    }

    // ѡ��Ŀ�깤���߳�
    std::shared_ptr<WorkerContext> target_ctx;
    {
        std::shared_lock<std::shared_mutex> lock(context_mutex_);
        if (!worker_contexts_.empty()) {
            static thread_local std::mt19937_64 rng(std::random_device{}());
            std::uniform_int_distribution<size_t> dist(0, worker_contexts_.size() - 1);

            for (int attempt = 0; attempt < 3; ++attempt) {
                size_t index = dist(rng);
                if (index >= worker_contexts_.size()) continue;

                if (auto ctx = worker_contexts_[index].lock()) {  // ��ȷת��weak_ptr
                    if (ctx->active.load(std::memory_order_acquire)) {
                        target_ctx = ctx;
                        break;
                    }
                }
            }
        }
    }

    // ���Ա��ض������
    if (target_ctx && target_ctx->local_queue.try_enqueue(wrapper)) {
        pending_tasks_.fetch_add(1, std::memory_order_relaxed);
        return true;
    }

    // ������ȫ�ֶ���
    if (global_queue_.size_approx() < GLOBAL_QUEUE_MAX) {  // ʹ����ȷ�������
        if (global_queue_.enqueue(wrapper)) {
            pending_tasks_.fetch_add(1, std::memory_order_relaxed);
            return true;
        }
    }

    // ���ջ���
    if (!task_pool_.enqueue(wrapper)) {
        slog->warn("Memory pool full, direct delete");
        delete wrapper;
    }
    return false;
}

// �޸ļ���߳�ʵ��
void BusinessThreadPool::start_monitor() {
    monitor_thread_ = std::thread([this] {
        while (!monitor_stop_.load(std::memory_order_acquire)) {
            std::this_thread::sleep_for(std::chrono::seconds(10));

            // ˫�ؼ����⾺̬����
            if (monitor_stop_.load(std::memory_order_acquire)) break;
            log_thread_pool_status();
            // ��ȫ���ʳ�Ա����
            const size_t pool_size = task_pool_.size_approx();
            if (pool_size < MEMORY_POOL_LOW_WATERMARK) {
                const size_t need_alloc = MEMORY_POOL_INIT_SIZE - pool_size;
                for (size_t i = 0; i < need_alloc; ++i) {
                    // ʹ�ð�ȫ�ڴ����
                    try {
                        auto wrapper = new TaskWrapper(Task{});
                        if (!task_pool_.enqueue(wrapper)) {
                            delete wrapper;
                        }
                    }
                    catch (const std::bad_alloc& e) {
                        slog->error("Memory allocation failed: {}", e.what());
                        break;
                    }
                }
            }
        }
    });
}
void BusinessThreadPool::worker_loop(std::shared_ptr<WorkerContext>& ctx, size_t worker_id) {
    SetThreadPriority(GetCurrentThread(), THREAD_MODE_BACKGROUND_BEGIN);

    constexpr size_t MAX_BATCH = 128;
    TaskWrapper* local_batch[MAX_BATCH];

    while (!stop_.load(std::memory_order_relaxed)) {
        // ÿ��ѭ��ǰ������
        if (!ctx->active.load(std::memory_order_acquire)) {
            break;
        }
        size_t processed = 0;

        // ���ȴ����ض���
        TaskWrapper* task;
        while (ctx->local_queue.try_dequeue(task)) {
            execute_task(task);
            processed++;
        }

        // ����ȫ�ֶ��У�����ԭ���߼���
        std::vector<TaskWrapper*> batch;
        int count = process_global(batch);
        processed += count;

        // ������ȡ�Ż���ʹ�����з�����
        if (processed == 0) {
            processed += try_steal(batch, worker_id);  // ����������
        }

        // ����Ӧ����
        if (processed == 0) {
            Sleep(1);
        }
    }
    // �˳�ʱ�Զ������������Ƴ�
    {
        std::unique_lock lock(context_mutex_);
        auto it = std::find_if(worker_contexts_.begin(), worker_contexts_.end(),
                               [&](auto& weak_ctx) { return weak_ctx.lock() == ctx; });
        if (it != worker_contexts_.end()) {
            worker_contexts_.erase(it);
        }
    }
}

size_t BusinessThreadPool::process_local(WorkerContext* ctx, std::vector<TaskWrapper*>& batch) {
    size_t processed = 0;
    TaskWrapper* task;
    while (processed < WORKER_BATCH_SIZE && ctx->local_queue.try_dequeue(task)) {
        execute_task(task);
        ++processed;
    }
    return processed;
}

size_t BusinessThreadPool::process_global(std::vector<TaskWrapper*>& batch) {
    size_t count = global_queue_.try_dequeue_bulk(
        std::back_inserter(batch),
        WORKER_BATCH_SIZE
    );

    for (auto task : batch) {
        execute_task(task);
    }
    batch.clear();
    return count;
}

size_t BusinessThreadPool::try_steal(std::vector<TaskWrapper*>& batch, size_t thief_id) {
    std::shared_lock<std::shared_mutex> lock(context_mutex_);
    size_t stolen = 0;
    const size_t total_workers = worker_contexts_.size();

    // �Ż���������
    constexpr size_t MAX_STEAL_PER_WORKER = 32;    // ���߳������ȡ��
    // ���ݸ��ض�̬��������
    size_t dynamic_steal_limit = std::min(
        MAX_STEAL_PER_WORKER,
        static_cast<size_t>(std::max<int64_t>(pending_tasks_.load(), 0)) /
        (worker_contexts_.size() + 1)
    );
    // ���ѡ����ʼ��ȡλ��
    static thread_local std::mt19937_64 rng(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, total_workers - 1);
    size_t start_index = dist(rng);

    for (size_t i = 0; i < total_workers && stolen < dynamic_steal_limit; ++i) {
        // ��״��������ƫ��
        size_t target_index = (start_index + i) % total_workers;
        if (target_index == thief_id) continue;

        // ��ȫ��ȡĿ��������
        auto weak_ctx = worker_contexts_[target_index];
        auto ctx = weak_ctx.lock();
        if (!ctx || !ctx->active.load(std::memory_order_acquire)) {
            continue;
        }

        // ��������ȡģʽ
        TaskWrapper* stolen_task = nullptr;
        size_t local_stolen = 0;
        while (local_stolen < dynamic_steal_limit) {
            // ������ȡ��������
            if (!ctx->local_queue.try_dequeue(stolen_task)) {
                break;
            }

            // ��ȫ������ȡ������
            if (stolen_task) {
                batch.push_back(stolen_task);
                ++local_stolen;
                ++stolen;
            }

            // �ﵽ���߳���ȡ����
            if (local_stolen >= dynamic_steal_limit) {
                break;
            }
        }

        // ����Ŀ���̵߳���ȡͳ��
        if (local_stolen > 0) {
            ctx->total_steals.fetch_add(local_stolen, std::memory_order_relaxed);
        }

        // ��ǰ�˳�����
        if (stolen >= WORKER_BATCH_SIZE) {
            break;
        }
    }

    // ����ִ����ȡ��������
    if (!batch.empty()) {
        for (auto task : batch) {
            execute_task(task);
        }
        batch.clear();
    }

    pending_tasks_.fetch_sub(stolen, std::memory_order_relaxed);
    return stolen;
}
void BusinessThreadPool::execute_task(TaskWrapper* task) {
    if (!task) return;

    // �쳣��ȫ���ƶ�����
    try {
        Task local_task;
        local_task = std::move(task->task);  // ��ȷ���ƶ���ֵ
        // ִ������
    #ifdef G_SERVICE    
        CObserverServer::instance().process_task(std::move(local_task));
    #else
        CLogicServer::instance().process_task(std::move(local_task));
    #endif
    }
    catch (...) {
        slog->error("Task execution failed");
    }

    // ���������װ��
    task->task = Task{};  // ��ʽ����ΪĬ��״̬

    // �����ڴ�
    if (!task_pool_.enqueue(task)) {
        slog->warn("Failed to recycle task wrapper");
        delete task;
    }
    pending_tasks_.fetch_sub(1, std::memory_order_relaxed);
}

void BusinessThreadPool::handle_idle(size_t processed, int& empty_cycles) {
    if (processed == 0) {
        if (++empty_cycles > MAX_EMPTY_ITERATIONS) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(1)
            );
        }
    }
    else {
        empty_cycles = 0;
    }
}

void BusinessThreadPool::drain_queue(std::shared_ptr<WorkerContext>& ctx) {
    TaskWrapper* task;
    while (ctx->local_queue.try_dequeue(task)) {
        execute_task(task);
    }
}

void BusinessThreadPool::check_memory_usage() {
    constexpr size_t MAX_POOL_SIZE = 100000;
    size_t current = task_pool_.size_approx();

    // ��ȫ�ͷŶ������
    if (current > MAX_POOL_SIZE) {
        TaskWrapper* wrappers[256];
        size_t count = task_pool_.try_dequeue_bulk(wrappers, 256);
        for (size_t i = 0; i < count; ++i) {
            delete wrappers[i];  // ��ȷ�ͷ��ڴ�
        }
    }
}

void BusinessThreadPool::log_pool_stats() {
    size_t global = task_pool_.size_approx();
    size_t in_flight = pending_tasks_.load();
    slog->info("Memory: pool={} tasks={} workers={}",
               global, in_flight, worker_contexts_.size());
}
// BusinessThreadPool.cpp ��������
void BusinessThreadPool::log_thread_pool_status() {
    const size_t pending = pending_tasks_.load(std::memory_order_relaxed);
    const size_t workers = worker_contexts_.size();
    const size_t global_queue_size = global_queue_.size_approx();
    size_t total_local_queue_size = 0;

    // ��������worker�ı��ض���
    {
        std::shared_lock lock(context_mutex_);
        for (const auto& weak_ctx : worker_contexts_) {
            if (auto ctx = weak_ctx.lock()) {
                total_local_queue_size += ctx->local_queue.size_approx();
            }
        }
    }

    slog->info("[ThreadPool Status] "
               "Workers: {} | "
               "Pending Tasks: {} | "
               "Global Queue: {} | "
               "Total Local Queue: {} | "
               "Memory Pool: {}",
               workers,
               pending,
               global_queue_size,
               total_local_queue_size,
               task_pool_.size_approx());

    // ���������������ʾ������������ȳ���2500ʱ��
    constexpr size_t WARNING_THRESHOLD = 2500;
    if (pending > WARNING_THRESHOLD) {
        slog->warn("[ThreadPool Warning] High pending tasks: {}", pending);
    }
}