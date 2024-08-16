// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "ClientImpl.h"
#include "WndProcHook.h"
#include "version.build"
#include "lf_plug_sdk.h"

#define RUNGATE_API void __stdcall

lfengine::client::TAppFuncDefExt g_AppFunc;

HINSTANCE dll_base = NULL;

asio::io_service g_io;
asio::io_service g_game_io;
asio::detail::thread_group g_thread_group;
int g_client_rev_version = REV_VERSION;
NetUtils::CTimerMgr g_game_timer_mgr(g_game_io);
std::shared_ptr<CClientImpl> client_ = nullptr;
VOID DbgPrint(const char* fmt, ...)
{
	char    buf[1024];
	va_list args;
	va_start(args, fmt);
	vsprintf_s(buf, fmt, args);
	va_end(args);
	OutputDebugStringA(buf);
}

void client_start_routine()
{
    //WndProcHook::install_hook();
    auto ip = client_->cfg()->get_field<std::string>(ip_field_id);
    //std::string ip = "43.139.236.115";
	auto port = client_->cfg()->get_field<int>(port_field_id);
	client_->start(ip, port);
	DbgPrint("client_start_routine ip:%s, port:%d", ip.c_str(), port);
	g_thread_group.create_thread([]() {
		auto work_guard = asio::make_work_guard(g_io);
		g_io.run();
	});
}
#ifdef _DEBUG
#pragma comment(linker, "/EXPORT:client_entry=?client_entry@@YGXXZ")
#endif
RUNGATE_API client_entry() noexcept
{
	VMP_VIRTUALIZATION_BEGIN();
	setlocale(LC_CTYPE, "");
	ProtocolCFGLoader cfg;
	cfg.set_field<std::string>(ip_field_id, "127.0.0.1");
	cfg.set_field<int>(port_field_id, 23268);
	client_ = std::make_shared<CClientImpl>(g_io);
	client_->cfg() = std::make_unique<ProtocolCFGLoader>(cfg);
	client_->cfg()->set_field<bool>(test_mode_field_id, false);
	client_->cfg()->set_field<bool>(sec_no_change_field_id, false);
#if defined(_DEBUG)
	client_->cfg()->set_field<bool>(test_mode_field_id, true);
	client_->cfg()->set_field<bool>(sec_no_change_field_id, false);
#endif
	if (client_->cfg()->get_field<bool>(sec_no_change_field_id))
	{
		DbgPrint("启用安全模式1");
		Utils::ImageProtect::instance().register_callback(&client_start_routine);
		Utils::ImageProtect::instance().install();
	}
	else
	{
		DbgPrint("启用安全模式2");
		client_start_routine();
	}
	VMP_VIRTUALIZATION_END();
}

RUNGATE_API Init(lfengine::client::PAppFuncDef AppFunc, int AppFuncCrc)
{
	using namespace lfengine::client;
	g_AppFunc.AddChatText = AppFunc->AddChatText;
	g_AppFunc.SendSocket = AppFunc->SendSocket;

	AddChatText = AppFunc->AddChatText;
	SendSocket = AppFunc->SendSocket;
	AddChatText("lf客户端插件拉起成功", 0x0000ff, 0);
	TDefaultMessage initok(0, 10000, 0, 0, 0);
	SendSocket(&initok, 0, 0);

	client_entry();
}


RUNGATE_API HookRecv(lfengine::client::PTDefaultMessage defMsg, char* lpData, int dataLen)
{
	using namespace lfengine::client;
	if (defMsg->ident == 10000)
	{
		AddChatText("这个没什么用，只是测试下网关插件返回数据", 0x0000ff, 0);
	}

}

RUNGATE_API UnInit()
{
	//DbgPrint("插件管理器卸载开始");
	client_->stop();
	//g_io.stop();
	//g_game_io.stop();
	//g_game_timer_mgr.stop_all_timer();
	//client_ = nullptr;
	//DbgPrint("插件管理器卸载成功");
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        dll_base = hModule;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

