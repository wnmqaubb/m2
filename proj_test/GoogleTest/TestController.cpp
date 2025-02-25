#include <Windows.h>
#include <vector>
#include <atomic>
#include "ProcessPool.h"

constexpr int TOTAL_PLAYERS = 2000;
constexpr int PLAYERS_PER_PROCESS = 50; // 每个进程承载50个玩家
using client_entry_t = decltype(&client_entry);
class TestController {
public:
    void Run() {
        // 初始化进程池
        ProcessPool pool(CalculateOptimalProcessCount());

        // 生成所有玩家配置
        auto configs = GeneratePlayerConfigs();

        // 分配任务到进程
        for (int i = 0; i < configs.size();) {
            int end = std::min(i + PLAYERS_PER_PROCESS, (int)configs.size());
            pool.AddTask(CreateProcessTask(configs, i, end));
            i = end;
        }

        // 启动监控
        StartMonitoring();

        // 等待所有进程完成
        pool.WaitAll();

        // 生成报告
        GenerateTestReport();
    }

private:
    std::vector<PlayerConfig> GeneratePlayerConfigs() {
        std::vector<PlayerConfig> configs;
        configs.reserve(TOTAL_PLAYERS);

        for (int i = 0; i < TOTAL_PLAYERS; ++i) {
            configs.emplace_back(CreatePlayerConfig(i));
        }
        return configs;
    }

    ProcessTask CreateProcessTask(const std::vector<PlayerConfig>& configs,
                                  int start, int end) {
        return [=](ProcessContext& ctx) {
            // 在子进程中初始化COM等环境
            CoInitializeEx(nullptr, COINIT_MULTITHREADED);

            // 加载核心DLL
            HMODULE hModule = LoadLibraryA("NewClient.dll");
            if (!hModule) {
                ctx.ReportError("Failed to load DLL");
                return;
            }

            // 获取入口函数
            auto entry = reinterpret_cast<client_entry_t>(
                GetProcAddress(hModule, "client_entry"));

            // 为每个玩家创建独立环境
            std::vector<std::thread> players;
            for (int i = start; i < end; ++i) {
                players.emplace_back([&, config = configs[i]] {
                    RunSinglePlayer(config, entry);
                });
            }

            // 等待本进程内所有玩家完成
            for (auto& t : players) {
                if (t.joinable()) t.join();
            }

            FreeLibrary(hModule);
            CoUninitialize();
        };
    }

    void RunSinglePlayer(const PlayerConfig& config, client_entry_t entry) {
        // 创建隔离的AppDomain（需要.NET支持）
        // 此处省略具体COM组件创建细节

        share_data_ptr_t param = new share_data_t();
        // 填充参数...
        ProtocolCFGLoader cfg;
        // 配置网络参数...

        // 执行客户端逻辑
        entry(param);

        // 清理资源...
    }

    int CalculateOptimalProcessCount() const {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        return std::min((TOTAL_PLAYERS + PLAYERS_PER_PROCESS - 1) / PLAYERS_PER_PROCESS,
                        sysInfo.dwNumberOfProcessors * 2);
    }
};
