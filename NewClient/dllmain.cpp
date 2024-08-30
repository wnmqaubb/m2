// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "ClientImpl.h"
#include "WndProcHook.h"
#include "version.build"
#include "../lf_rungate_server_plug/lf_plug_sdk.h"

#define RUNGATE_API void __stdcall

lfengine::client::TAppFuncDefExt g_AppFunc;

HINSTANCE dll_base = NULL;
asio::io_service g_game_io;
asio::detail::thread_group g_thread_group;
int g_client_rev_version = REV_VERSION;
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
    WndProcHook::install_hook();
    auto ip = client_->cfg()->get_field<std::string>(ip_field_id);
	auto port = client_->cfg()->get_field<int>(port_field_id);
	client_->async_start(ip, port);
	DbgPrint("client_start_routine ip:%s, port:%d", ip.c_str(), port);
	
}
//#ifdef _DEBUG
#pragma comment(linker, "/EXPORT:client_entry=?client_entry@@YGXABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z")
//#endif
/*__declspec(dllexport)*/ RUNGATE_API client_entry(const std::string& guard_gate_ip) noexcept
{
	using namespace lfengine::client;
	VMP_VIRTUALIZATION_BEGIN();
	setlocale(LC_CTYPE, "");
	ProtocolCFGLoader cfg;
	cfg.set_field<std::string>(ip_field_id, guard_gate_ip.empty () ? "127.0.0.1" : guard_gate_ip);
	cfg.set_field<int>(port_field_id, 23268);
	client_ = std::make_shared<CClientImpl>();
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
	//AddChatText("lf客户端插件拉起成功", 0x0000ff, 0);
	lfengine::TDefaultMessage initok(0, 10000, 0, 0, 0);
	SendSocket(&initok, 0, 0);
}


RUNGATE_API HookRecv(lfengine::PTDefaultMessage defMsg, char* lpData, int dataLen)
{
	using namespace lfengine::client;
	if (defMsg->ident == 10001)
	{
		client_entry(lpData);
		AddChatText(lpData, 0x0000ff, 0);
	}

}

RUNGATE_API DoUnInit()
{
	try
	{
		DbgPrint("插件卸载开始");
		if (!g_game_io.stopped()) {
			g_game_io.stop();
			g_game_io.reset();
		}

		//client_->stop_all_timers();
		//client_->stop_all_timed_tasks();
		client_->stop();
		Sleep(1000);
		//必须要destroy,否则会导致定时器无法销毁,导致定时器的线程还在执行,小退再开始游戏时线程还在执行之前dll的地址,会导致崩溃
		client_->destroy();
		DbgPrint("插件卸载完成");
	}
	catch (...)
	{
		DbgPrint("DoUnInit 异常");
	}
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
		//DoUnInit();
        break;
    }
    return TRUE;
}

