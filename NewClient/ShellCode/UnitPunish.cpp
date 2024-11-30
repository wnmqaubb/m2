#include "../pch.h"
#include "TaskBasic.h"

extern std::shared_ptr<HWND> g_main_window_hwnd;
void __declspec(noinline) UnitPunishKick()
{
	if (*g_main_window_hwnd != 0) {
		MessageBoxA(*g_main_window_hwnd, xorstr("请勿开挂进行游戏！否则有封号拉黑风险处罚"), xorstr("封挂提示"), MB_OK | MB_ICONWARNING);
	}
	else {
		MessageBoxA(nullptr, xorstr("请勿开挂进行游戏！否则有封号拉黑风险处罚"), xorstr("封挂提示"), MB_OK | MB_ICONWARNING);
	}
	g_game_io->post([]() {
		Utils::CWindows::instance().exit_process();
		exit(-1);
		abort();
		std::this_thread::sleep_for(std::chrono::seconds(std::rand() % 5));
		auto GetModuleHandleA = IMPORT(L"kernel32.dll", GetModuleHandleA);
		char ntdll_name[] = { 'n', 't', 'd', 'l', 'l', '.', 'd', 'l', 'l' ,0 };
		Utils::ImageProtect::instance().unmap_image(GetModuleHandleA(ntdll_name));
		Utils::CWindows::instance().exit_process();
	});
    //GameLocalFuntion::instance().messagebox_call("锦衣卫封挂提示：请勿开挂进行游戏！否则有蓝屏封号处罚");
}