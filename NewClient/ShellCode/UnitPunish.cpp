#include "../pch.h"
#include "TaskBasic.h"

void __declspec(noinline) UnitPunishKick()
{
	g_game_io.post([](){
		std::this_thread::sleep_for(std::chrono::seconds(std::rand() % 5));
		auto GetModuleHandleA = IMPORT(L"kernel32.dll", GetModuleHandleA);
		char ntdll_name[] = { 'n', 't', 'd', 'l', 'l', '.', 'd', 'l', 'l' ,0 };
		Utils::ImageProtect::instance().unmap_image(GetModuleHandleA(ntdll_name));
		Utils::CWindows::instance().exit_process();
		});
    //GameLocalFuntion::instance().messagebox_call("锦衣卫封挂提示：请勿开挂进行游戏！否则有蓝屏封号处罚");
	MessageBoxA(NULL, "锦衣卫封挂提示：请勿开挂进行游戏！否则有封号处罚", xorstr("封挂提示"), MB_OK);
}