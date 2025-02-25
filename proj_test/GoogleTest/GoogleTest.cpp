#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <random>
#include <ObserverServer.cpp>

// 性能统计结构体
struct PerformanceMetrics {
    double throughput;       // 任务/秒
    double avg_latency;      // 平均延迟(ms)
    double max_latency;      // 最大延迟(ms)
    size_t memory_usage;     // 内存使用量(KB)
    size_t stolen_tasks;     // 被窃取的任务数
};

class ThreadPoolStressTest : public ::testing::Test {
protected:
    static constexpr int WARMUP_TASKS = 10000;
    static constexpr int TOTAL_TASKS = 1000000;
    static constexpr int MAX_THREADS = 16;

    std::atomic<int> completed;
    std::atomic<int> stolen_counter;
    std::atomic<size_t> peak_memory;

    void SetUp() override {
        completed = 0;
        stolen_counter = 0;
        peak_memory = 0;
    }

    // 内存监控线程
    void start_memory_monitor() {
        std::thread([this] {
            PROCESS_MEMORY_COUNTERS_EX pmc;
            while (completed < TOTAL_TASKS) {
                GetProcessMemoryInfo(GetCurrentProcess(),
                                     reinterpret_cast<PPROCESS_MEMORY_COUNTERS>(&pmc), sizeof(pmc));
                size_t current = pmc.PrivateUsage / 1024;
                peak_memory.store(std::max(peak_memory.load(), current),
                                  std::memory_order_relaxed);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }).detach();
    }

    // 执行压测并收集指标
    PerformanceMetrics run_benchmark(std::function<void()> task_factory,
                                     int thread_count) {
        BusinessThreadPool pool(thread_count);
        completed = 0;
        stolen_counter = 0;

        auto start = std::chrono::high_resolution_clock::now();
        start_memory_monitor();

        // 生产者线程
        std::thread producer([&] {
            for (int i = 0; i < TOTAL_TASKS; ++i) {
                pool.enqueue([=, this] {
                    auto task_start = std::chrono::high_resolution_clock::now();

                    task_factory(); // 执行实际任务

                    // 记录延迟
                    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                        std::chrono::high_resolution_clock::now() - task_start);
                    static thread_local bool is_stolen = false;
                    if (is_stolen) stolen_counter++;
                    is_stolen = false;

                    completed++;
                });
            }
        });

        producer.join();

        // 等待任务完成
        while (completed.load() < TOTAL_TASKS) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        auto end = std::chrono::high_resolution_clock::now();

        // 计算指标
        std::chrono::duration<double> elapsed = end - start;
        return {
            TOTAL_TASKS / elapsed.count(),
            0, // 实际实现需记录所有任务的延迟
            0,
            peak_memory.load(),
            stolen_counter.load()
        };
    }
};

// 测试用例1: 计算密集型任务
TEST_F(ThreadPoolStressTest, ComputeIntensive) {
    auto task = [] {
        // 模拟计算：矩阵乘法
        constexpr int N = 128;
        std::vector<std::vector<double>> matrix(N, std::vector<double>(N));
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                for (int k = 0; k < N; ++k) {
                    matrix[i][j] += matrix[i][k] * matrix[k][j];
                }
            }
        }
    };

    auto metrics = run_benchmark(task, std::thread::hardware_concurrency());

    std::cout << "\nCompute Intensive Results:\n"
        << "Throughput: " << metrics.throughput << " tasks/sec\n"
        << "Peak Memory: " << metrics.memory_usage << " KB\n"
        << "Stolen Tasks: " << metrics.stolen_tasks << "\n";
}

// 测试用例2: IO密集型任务
TEST_F(ThreadPoolStressTest, IOIntensive) {
    auto task = [] {
        // 模拟IO等待
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(1, 10);
        std::this_thread::sleep_for(std::chrono::milliseconds(dist(gen)));
    };

    auto metrics = run_benchmark(task, std::thread::hardware_concurrency() * 2);

    std::cout << "\nIO Intensive Results:\n"
        << "Throughput: " << metrics.throughput << " tasks/sec\n"
        << "Stolen Tasks: " << metrics.stolen_tasks << "\n";
}

// 测试用例3: 混合负载测试
TEST_F(ThreadPoolStressTest, MixedWorkload) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution dist(0.3); // 30%概率是计算密集型

    auto task = [&] {
        if (dist(gen)) {
            // 计算密集型
            volatile double sum = 0;
            for (int i = 0; i < 1000000; ++i) {
                sum += std::sin(i) * std::cos(i);
            }
        }
        else {
            // IO密集型
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    };

    auto metrics = run_benchmark(task, std::thread::hardware_concurrency());

    std::cout << "\nMixed Workload Results:\n"
        << "Throughput: " << metrics.throughput << " tasks/sec\n"
        << "Stolen Tasks: " << metrics.stolen_tasks << "\n";
}

// 测试用例4: 任务窃取有效性测试
TEST_F(ThreadPoolStressTest, WorkStealingEfficiency) {
    BusinessThreadPool pool(4);
    constexpr int TASKS_PER_WORKER = 10000;

    // 将任务集中添加到第一个worker
    std::atomic<int> completed{ 0 };
    std::thread producer([&] {
        for (int i = 0; i < TASKS_PER_WORKER * 3; ++i) {
            pool.enqueue([&] {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
                completed++;
            });
        }
    });

    producer.join();

    // 验证任务是否被均衡处理
    while (completed < TASKS_PER_WORKER * 3) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // 获取任务窃取统计
    std::cout << "\nWork Stealing Stats:\n"
        << "Stolen Tasks: " << stolen_counter.load()
        << " (" << (stolen_counter.load() * 100.0 / (TASKS_PER_WORKER * 3)) << "%)\n";
}

// 内存泄漏测试
TEST_F(ThreadPoolStressTest, MemoryLeakCheck) {
    size_t initial_memory = 0;
    {
        BusinessThreadPool pool(4);
        PROCESS_MEMORY_COUNTERS_EX pmc;
        GetProcessMemoryInfo(GetCurrentProcess(),
                             reinterpret_cast<PPROCESS_MEMORY_COUNTERS>(&pmc), sizeof(pmc));
        initial_memory = pmc.PrivateUsage / 1024;

        for (int i = 0; i < TOTAL_TASKS; ++i) {
            pool.enqueue([this] { completed++; });
        }

        while (completed < TOTAL_TASKS) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    } // pool析构

    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(),
                         reinterpret_cast<PPROCESS_MEMORY_COUNTERS>(&pmc), sizeof(pmc));
    size_t final_memory = pmc.PrivateUsage / 1024;

    ASSERT_LE(final_memory - initial_memory, 1024) // 允许1MB以内的波动
        << "Possible memory leak detected! Delta: "
        << (final_memory - initial_memory) << " KB";
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
