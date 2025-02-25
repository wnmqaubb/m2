// StressTest.cpp
#include "ProcessPool.h"
#include <sstream>

// ���ɵ���������������
std::wstring build_process_command(int player_id) {
    std::wstringstream cmd;
    cmd << L"ClientProcess.exe "         // Ԥ����Ŀͻ��˳���
        << L"--player-id=" << player_id  // ���ID����
        << L" --ip=140.210.20.215"       // ��������ַ
        << L" --port=8080"               // ����˿�
        << L" --timeout=300";            // ��ʱʱ�䣨�룩
    return cmd.str();
}

int main() {
    constexpr int TOTAL_PLAYERS = 2000;
    constexpr int BATCH_SIZE = 50;      // ÿ������50������

    ProcessPool pool;

    // �����ύ�������˲����Դ�߷�
    for (int i = 0; i < TOTAL_PLAYERS; ) {
        int end = std::min(i + BATCH_SIZE, TOTAL_PLAYERS);

        // �ύ��ǰ����
        for (int j = i; j < end; ++j) {
            pool.add_task(build_process_command(j));
        }

        // �ȴ������β��ֽ�������
        std::this_thread::sleep_for(std::chrono::seconds(1));
        i = end;
    }

    // �ȴ����н������
    pool.wait_completion();
    return 0;
}
