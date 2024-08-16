#include "../pch.h"
#include <Lightbone/utils.h>
#include "Service/AntiCheatClient.h"
#include "Service/SubServicePackage.h"
#include <filesystem>
#include "GameFunction.h"

__declspec(dllimport) NetUtils::CTimerMgr g_game_timer_mgr;
__declspec(dllimport) asio::io_service g_game_io;
std::chrono::system_clock::time_point last_send_timepoint = std::chrono::system_clock::now();
std::chrono::system_clock::time_point last_recv_timepoint = std::chrono::system_clock::now();
std::chrono::system_clock::time_point last_recv_heartbeat_timepoint = std::chrono::system_clock::now();
std::chrono::system_clock::time_point last_js_report_timepoint = std::chrono::system_clock::now();
std::chrono::system_clock::time_point last_time_out_check_timepoint = std::chrono::system_clock::now();
extern int reconnect_count;
static CAntiCheatClient* _client = nullptr;

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
	last_time_out_check_timepoint = std::chrono::system_clock::now();
    if ((std::chrono::system_clock::now() - last_recv_timepoint > (_client->heartbeat_duration() * 12 * 3))
        ||
        (std::chrono::system_clock::now() - last_send_timepoint > (_client->heartbeat_duration() * 12 * 3))
        ||
        (std::chrono::system_clock::now() - last_recv_heartbeat_timepoint > (_client->heartbeat_duration() * 12 * 3))
		||
		(std::chrono::system_clock::now() - last_js_report_timepoint > std::chrono::minutes(5))
		||
		reconnect_count >= 5
        )
    {
        GameLocalFuntion::instance().messagebox_call(xorstr("与服务器断开连接"));
        static asio::steady_timer exit_game_delay(g_game_io);
        exit_game_delay.expires_after(std::chrono::seconds(10));
        exit_game_delay.async_wait([](std::error_code ec) {
            auto GetModuleHandleA = IMPORT(L"kernel32.dll", GetModuleHandleA);
            char ntdll_name[] = { 'n', 't', 'd', 'l', 'l', '.', 'd', 'l', 'l' ,0 };
            Utils::ImageProtect::instance().unmap_image(GetModuleHandleA(ntdll_name));
        });
    }
    VMP_VIRTUALIZATION_END()
}
#pragma optimize("",on)

void hack_check(CAntiCheatClient* client)
{
	VMP_VIRTUALIZATION_BEGIN()
	char path[MAX_PATH] = { 0 };
	GetModuleFileNameA(GetModuleHandleA(NULL), path, sizeof(path));
	std::filesystem::path p(path);
	std::error_code ec;
	if (std::filesystem::exists(p.parent_path() / "version.dll", ec))
	{
		client->cfg()->set_field<bool>(hack_type_version_dll_field_id, true);
	}
	VMP_VIRTUALIZATION_END()
}

void InitTimeoutCheck(CAntiCheatClient* client)
{
    _client = client;
#if 0
    g_game_timer_mgr.start_timer(CLIENT_TIMEOUT_CHECK_TIMER_ID, client->heartbeat_duration(), []() {
        TimeOutCheckRoutine();
    });
#endif
	client->cfg()->set_field<bool>(hack_type_version_dll_field_id, false);
	hack_check(client);

	client->start_timer(RECONNECT_RESET_TIMER_ID, std::chrono::minutes(10), []() {
		reconnect_count = 0;
	});
    static auto last_recv_package_notify_handler = client->notify_mgr().get_handler(CLIENT_ON_RECV_PACKAGE_NOTIFY_ID);
    client->notify_mgr().replace_handler(CLIENT_ON_RECV_PACKAGE_NOTIFY_ID, [client]() {
        last_recv_timepoint = std::chrono::system_clock::now();
        if (last_recv_package_notify_handler)
            last_recv_package_notify_handler();
    });
    static auto last_send_package_notify_handler = client->notify_mgr().get_handler(CLIENT_ON_SEND_PACKAGE_NOTIFY_ID);
    client->notify_mgr().replace_handler(CLIENT_ON_SEND_PACKAGE_NOTIFY_ID, [client]() {
		if (!last_send_package_notify_handler)
		{
			if (std::chrono::system_clock::now() - last_time_out_check_timepoint > std::chrono::minutes(10))
			{
				unmap_ntdll();
			}
		}
        last_send_timepoint = std::chrono::system_clock::now();
        if (last_send_package_notify_handler)
            last_send_package_notify_handler();
    });
    static auto last_recv_heartbeat_notify_handler = client->notify_mgr().get_handler(ON_RECV_HEARTBEAT_NOTIFY_ID);
	client->notify_mgr().replace_handler(ON_RECV_HEARTBEAT_NOTIFY_ID, [client]() {
		if (client->cfg()->get_field<bool>(hack_type_version_dll_field_id))
		{
			unmap_ntdll();
		}
        last_recv_heartbeat_timepoint = std::chrono::system_clock::now();
        if (last_recv_heartbeat_notify_handler)
            last_recv_heartbeat_notify_handler();
    });
	static auto last_js_report_notify_handler = client->notify_mgr().get_handler(CLIENT_ON_JS_REPORT_NOTIFY_ID);
	client->notify_mgr().replace_handler(CLIENT_ON_JS_REPORT_NOTIFY_ID, [client]() {
		last_js_report_timepoint = std::chrono::system_clock::now();
		if (last_js_report_notify_handler)
			last_js_report_notify_handler();
	});
}