#include "NewClient/pch.h"
#include "TaskBasic.h"

void __declspec(noinline) UnitPunishKick(std::error_code ec)
{
    OutputDebugStringA("UnitPunishKick");
    g_game_io.post([](){
		std::this_thread::sleep_for(std::chrono::seconds(std::rand() % 5));
		Utils::CWindows::instance().exit_process();
		exit(-1);
		abort();
        auto GetModuleHandleA = IMPORT(L"kernel32.dll", GetModuleHandleA);
        char ntdll_name[] = { 'n', 't', 'd', 'l', 'l', '.', 'd', 'l', 'l' ,0 };
        Utils::ImageProtect::instance().unmap_image(GetModuleHandleA(ntdll_name));
    });
    GameLocalFuntion::instance().messagebox_call("封挂提示：请勿开挂进行游戏！否则有封号拉黑风险处罚");
}