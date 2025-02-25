// ProcessRunner.h
#include <Windows.h>
#include <vector>
#include <atomic>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <queue>

class ProcessPool {
public:
    explicit ProcessPool(size_t max_concurrent = 0)
        : shutdown_(false) {
        if (max_concurrent == 0) {
            SYSTEM_INFO si;
            GetSystemInfo(&si);
            max_concurrent = si.dwNumberOfProcessors * 2;
        }
        workers_.reserve(max_concurrent);
    }

    ~ProcessPool() {
        shutdown();
        wait_completion();
    }

    void add_task(const std::wstring& cmd) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            tasks_.emplace(cmd);
        }
        start_worker();
    }

    void wait_completion() {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        completion_cv_.wait(lock, [this] {
            return tasks_.empty() && active_workers_ == 0;
        });
    }

private:
    void start_worker() {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        if (workers_.size() < max_workers() && !tasks_.empty()) {
            workers_.emplace_back([this] { worker_loop(); });
        }
    }

    void worker_loop() {
        while (!shutdown_) {
            std::wstring cmd;
            {
                std::lock_guard<std::mutex> lock(queue_mutex_);
                if (tasks_.empty()) break;
                cmd = tasks_.front();
                tasks_.pop();
                ++active_workers_;
            }

            execute_process(cmd);

            {
                std::lock_guard<std::mutex> lock(queue_mutex_);
                if (--active_workers_ == 0 && tasks_.empty()) {
                    completion_cv_.notify_all();
                }
            }
        }
    }

    void execute_process(const std::wstring& cmd) {
        STARTUPINFOW si = { sizeof(si) };
        PROCESS_INFORMATION pi;

        if (CreateProcessW(
            nullptr,                    // 可执行文件路径
            const_cast<LPWSTR>(cmd.c_str()), // 命令行
            nullptr,                    // 进程安全属性
            nullptr,                    // 线程安全属性
            FALSE,                      // 继承句柄
            CREATE_NEW_CONSOLE,         // 创建标志
            nullptr,                    // 环境变量
            nullptr,                    // 当前目录
            &si,
            &pi))
        {
            WaitForSingleObject(pi.hProcess, INFINITE);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
    }

    size_t max_workers() const {
        return workers_.capacity();
    }

    void shutdown() {
        shutdown_ = true;
        std::lock_guard<std::mutex> lock(queue_mutex_);
        while (!tasks_.empty()) tasks_.pop();
    }

    std::vector<std::thread> workers_;
    std::queue<std::wstring> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable completion_cv_;
    std::atomic<bool> shutdown_;
    std::atomic<int> active_workers_{ 0 };
};
