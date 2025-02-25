#include <Windows.h>
#include <vector>
#include <atomic>
#include "ProcessPool.h"

constexpr int TOTAL_PLAYERS = 2000;
constexpr int PLAYERS_PER_PROCESS = 50; // ÿ�����̳���50�����
using client_entry_t = decltype(&client_entry);
class TestController {
public:
    void Run() {
        // ��ʼ�����̳�
        ProcessPool pool(CalculateOptimalProcessCount());

        // ���������������
        auto configs = GeneratePlayerConfigs();

        // �������񵽽���
        for (int i = 0; i < configs.size();) {
            int end = std::min(i + PLAYERS_PER_PROCESS, (int)configs.size());
            pool.AddTask(CreateProcessTask(configs, i, end));
            i = end;
        }

        // �������
        StartMonitoring();

        // �ȴ����н������
        pool.WaitAll();

        // ���ɱ���
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
            // ���ӽ����г�ʼ��COM�Ȼ���
            CoInitializeEx(nullptr, COINIT_MULTITHREADED);

            // ���غ���DLL
            HMODULE hModule = LoadLibraryA("NewClient.dll");
            if (!hModule) {
                ctx.ReportError("Failed to load DLL");
                return;
            }

            // ��ȡ��ں���
            auto entry = reinterpret_cast<client_entry_t>(
                GetProcAddress(hModule, "client_entry"));

            // Ϊÿ����Ҵ�����������
            std::vector<std::thread> players;
            for (int i = start; i < end; ++i) {
                players.emplace_back([&, config = configs[i]] {
                    RunSinglePlayer(config, entry);
                });
            }

            // �ȴ�������������������
            for (auto& t : players) {
                if (t.joinable()) t.join();
            }

            FreeLibrary(hModule);
            CoUninitialize();
        };
    }

    void RunSinglePlayer(const PlayerConfig& config, client_entry_t entry) {
        // ���������AppDomain����Ҫ.NET֧�֣�
        // �˴�ʡ�Ծ���COM�������ϸ��

        share_data_ptr_t param = new share_data_t();
        // ������...
        ProtocolCFGLoader cfg;
        // �����������...

        // ִ�пͻ����߼�
        entry(param);

        // ������Դ...
    }

    int CalculateOptimalProcessCount() const {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        return std::min((TOTAL_PLAYERS + PLAYERS_PER_PROCESS - 1) / PLAYERS_PER_PROCESS,
                        sysInfo.dwNumberOfProcessors * 2);
    }
};
