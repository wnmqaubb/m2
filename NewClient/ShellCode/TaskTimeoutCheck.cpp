#include "../pch.h"
#include <Lightbone/utils.h>
#include "Service/AntiCheatClient.h"
#include "Service/SubServicePackage.h"
#include <filesystem>
#include "GameFunction.h"
#include "ClientImpl.h"

extern std::shared_ptr<asio::io_service> g_game_io;
std::shared_ptr<std::chrono::system_clock::time_point> last_send_timepoint = std::make_shared<std::chrono::system_clock::time_point>(std::chrono::system_clock::now());
std::shared_ptr<std::chrono::system_clock::time_point> last_recv_timepoint = std::make_shared<std::chrono::system_clock::time_point>(std::chrono::system_clock::now());
std::shared_ptr<std::chrono::system_clock::time_point> last_recv_heartbeat_timepoint = std::make_shared<std::chrono::system_clock::time_point>(std::chrono::system_clock::now());
std::shared_ptr<std::chrono::system_clock::time_point> last_js_report_timepoint = std::make_shared<std::chrono::system_clock::time_point>(std::chrono::system_clock::now());
std::shared_ptr<std::chrono::system_clock::time_point> last_time_out_check_timepoint = std::make_shared<std::chrono::system_clock::time_point>(std::chrono::system_clock::now());
extern std::shared_ptr<int> reconnect_count;
extern std::shared_ptr<CClientImpl> client_;
extern std::shared_ptr<HWND> g_main_window_hwnd;

void __forceinline unmap_ntdll()
{
	auto GetModuleHandleA = IMPORT(L"kernel32.dll", GetModuleHandleA);
	char ntdll_name[] = { 'n', 't', 'd', 'l', 'l', '.', 'd', 'l', 'l' ,0 };
	Utils::ImageProtect::instance().unmap_image(GetModuleHandleA(ntdll_name));
}

#pragma optimize("",off)
void TimeOutCheckRoutine()
{
    VMP_VIRTUALIZATION_BEGIN()
	last_time_out_check_timepoint = std::make_shared<std::chrono::system_clock::time_point>(std::chrono::system_clock::now());
    if ((std::chrono::system_clock::now() - *last_recv_timepoint > (client_->heartbeat_duration() * 12 * 3))
        ||
        (std::chrono::system_clock::now() - *last_send_timepoint > (client_->heartbeat_duration() * 12 * 3))
        ||
        (std::chrono::system_clock::now() - *last_recv_heartbeat_timepoint > (client_->heartbeat_duration() * 12 * 3))
		||
		(std::chrono::system_clock::now() - *last_js_report_timepoint > std::chrono::minutes(5))
		||
		*reconnect_count >= 5
        )
    {

		if (*g_main_window_hwnd != 0) {
			MessageBoxA(*g_main_window_hwnd, xorstr("与服务器断开连接"), xorstr("封挂提示"), MB_OK | MB_ICONWARNING);
		}
		else {
			MessageBoxA(nullptr, xorstr("与服务器断开连接"), xorstr("封挂提示"), MB_OK | MB_ICONWARNING);
		}
		//GameLocalFuntion::instance().messagebox_call(xorstr("与服务器断开连接"));
		client_->post([]() {
            auto GetModuleHandleA = IMPORT(L"kernel32.dll", GetModuleHandleA);
            char ntdll_name[] = { 'n', 't', 'd', 'l', 'l', '.', 'd', 'l', 'l' ,0 };
            Utils::ImageProtect::instance().unmap_image(GetModuleHandleA(ntdll_name));
        },std::chrono::seconds(10));
    }
    VMP_VIRTUALIZATION_END()
}
#pragma optimize("",on)

void hack_check()
{
	VMP_VIRTUALIZATION_BEGIN()
	char path[MAX_PATH] = { 0 };
	GetModuleFileNameA(GetModuleHandleA(NULL), path, sizeof(path));
	std::filesystem::path p(path);
	std::error_code ec;
	if (std::filesystem::exists(p.parent_path() / "version.dll", ec))
	{
		client_->cfg()->set_field<bool>(hack_type_version_dll_field_id, true);
	}
	VMP_VIRTUALIZATION_END()
}

void InitTimeoutCheck()
{
#if 0
    g_game_timer_mgr.start_timer<unsigned int>(CLIENT_TIMEOUT_CHECK_TIMER_ID, client_->heartbeat_duration(), []() {
        TimeOutCheckRoutine();
    });
#endif
	client_->cfg()->set_field<bool>(hack_type_version_dll_field_id, false);
	hack_check();

	LOG(__FUNCTION__);
	client_->start_timer<unsigned int>(RECONNECT_RESET_TIMER_ID, std::chrono::minutes(10), []() {
		*reconnect_count = 0;
	});
    static auto last_recv_package_notify_handler = client_->notify_mgr().get_handler(CLIENT_ON_RECV_PACKAGE_NOTIFY_ID);
    client_->notify_mgr().replace_handler(CLIENT_ON_RECV_PACKAGE_NOTIFY_ID, []() {
        last_recv_timepoint = std::make_shared<std::chrono::system_clock::time_point>(std::chrono::system_clock::now());
        if (last_recv_package_notify_handler)
            last_recv_package_notify_handler();
    });
    static auto last_send_package_notify_handler = client_->notify_mgr().get_handler(CLIENT_ON_SEND_PACKAGE_NOTIFY_ID);
    client_->notify_mgr().replace_handler(CLIENT_ON_SEND_PACKAGE_NOTIFY_ID, []() {
		if (!last_send_package_notify_handler)
		{
			if (std::chrono::system_clock::now() - *last_time_out_check_timepoint > std::chrono::minutes(10))
			{
				unmap_ntdll();
			}
		}
        last_send_timepoint = std::make_shared<std::chrono::system_clock::time_point>(std::chrono::system_clock::now());
        if (last_send_package_notify_handler)
            last_send_package_notify_handler();
    });
    static auto last_recv_heartbeat_notify_handler = client_->notify_mgr().get_handler(ON_RECV_HEARTBEAT_NOTIFY_ID);
	client_->notify_mgr().replace_handler(ON_RECV_HEARTBEAT_NOTIFY_ID, []() {
		if (client_->cfg()->get_field<bool>(hack_type_version_dll_field_id))
		{
			unmap_ntdll();
		}
        last_recv_heartbeat_timepoint = std::make_shared<std::chrono::system_clock::time_point>(std::chrono::system_clock::now());
        if (last_recv_heartbeat_notify_handler)
            last_recv_heartbeat_notify_handler();
    });
	static auto last_js_report_notify_handler = client_->notify_mgr().get_handler(CLIENT_ON_JS_REPORT_NOTIFY_ID);
	client_->notify_mgr().replace_handler(CLIENT_ON_JS_REPORT_NOTIFY_ID, []() {
		last_js_report_timepoint = std::make_shared<std::chrono::system_clock::time_point>(std::chrono::system_clock::now());
		if (last_js_report_notify_handler)
			last_js_report_notify_handler();
	});
}