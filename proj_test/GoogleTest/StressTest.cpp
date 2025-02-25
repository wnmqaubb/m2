// StressTest.cpp
#include "ProcessPool.h"
#include <sstream>

// 生成单个进程启动命令
std::wstring build_process_command(int player_id) {
    std::wstringstream cmd;
    cmd << L"ClientProcess.exe "         // 预编译的客户端程序
        << L"--player-id=" << player_id  // 玩家ID参数
        << L" --ip=140.210.20.215"       // 服务器地址
        << L" --port=8080"               // 服务端口
        << L" --timeout=300";            // 超时时间（秒）
    return cmd.str();
}

int main() {
    constexpr int TOTAL_PLAYERS = 2000;
    constexpr int BATCH_SIZE = 50;      // 每批启动50个进程

    ProcessPool pool;

    // 分批提交任务避免瞬间资源高峰
    for (int i = 0; i < TOTAL_PLAYERS; ) {
        int end = std::min(i + BATCH_SIZE, TOTAL_PLAYERS);

        // 提交当前批次
        for (int j = i; j < end; ++j) {
            pool.add_task(build_process_command(j));
        }

        // 等待本批次部分进程启动
        std::this_thread::sleep_for(std::chrono::seconds(1));
        i = end;
    }

    // 等待所有进程完成
    pool.wait_completion();
    return 0;
}
