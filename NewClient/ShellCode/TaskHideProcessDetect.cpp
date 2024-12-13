#include "../pch.h"
#include <Lightbone/utils.h>
#include "Service/AntiCheatClient.h"
#include "ClientImpl.h"

extern std::shared_ptr<CClientImpl> client_;
int cheat_time_hp = 0;
void HideProcess()
{
    ProtocolC2STaskEcho resp;
    resp.task_id = TASK_PKG_ID_HIDE_PROCESS_DETECT;
    resp.is_cheat = false;
    auto windows = Utils::CWindows::instance().enum_windows_ex();
    auto Sleep = IMPORT(L"kernel32.dll", Sleep);
    for (auto& window : windows)
    {
        if (window.class_name == L"ConsoleWindowClass")
            continue;
        if (window.is_hide_process)
        {
            resp.is_cheat = true;
            resp.text = xorstr("检测到隐藏进程窗口");
            cheat_time_hp++;
            break;
        }
        Sleep(1);
    }

    if (resp.is_cheat && cheat_time_hp >= 4)
    {
        client_->send(&resp);
        cheat_time_hp = 0;
    }

    if (resp.is_cheat == false)
    {
        cheat_time_hp = 0;
    }
};

const unsigned int DEFINE_TIMER_ID(kHideProcessTimerId);
void InitHideProcessDetect()
{
	LOG(__FUNCTION__);
	client_->start_timer(kHideProcessTimerId, std::chrono::seconds(15), []() {
        HideProcess();
    });
}