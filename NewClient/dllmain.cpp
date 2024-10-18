// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "ClientImpl.h"
#include "WndProcHook.h"
#include "version.build"
#include "../lf_rungate_server_plug/lf_plug_sdk.h"
#include "asio/detail/win_thread.hpp"
#include "BasicUtils.h"
#include "Lightbone/lighthook.h"
#include "anti_monitor_directory/ReadDirectoryChanges.h"

#define RUNGATE_API void __stdcall

lfengine::client::TAppFuncDefExt g_AppFunc;

HINSTANCE dll_base = NULL;
asio::io_service g_game_io;
asio::detail::thread_group g_thread_group;
int g_client_rev_version = REV_VERSION;
std::shared_ptr<CClientImpl> client_ = nullptr;
// dll 退出信号
HANDLE dll_exit_event_handle_ = NULL;
void client_start_routine()
{
	LOG("client_start_routine ");
    WndProcHook::install_hook();
    auto ip = client_->cfg()->get_field<std::string>(ip_field_id);
	auto port = client_->cfg()->get_field<int>(port_field_id);
	client_->async_start(ip, port);
	LOG("client_start_routine ip:%s, port:%d", ip.c_str(), port);
	
}
//#ifdef _DEBUG
#pragma comment(linker, "/EXPORT:client_entry=?client_entry@@YGXABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z")
//#endif
/*__declspec(dllexport)*/ RUNGATE_API client_entry(const std::string& guard_gate_ip) //noexcept
{
	using namespace lfengine::client;
	VMP_VIRTUALIZATION_BEGIN();
	setlocale(LC_CTYPE, "");
	ProtocolCFGLoader cfg;
	cfg.set_field<std::string>(ip_field_id, guard_gate_ip.empty () ? "127.0.0.1" : guard_gate_ip);
	cfg.set_field<int>(port_field_id, 23268);
	dll_exit_event_handle_ = CreateEvent(NULL, TRUE, FALSE, NULL);
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
		LOG("启用安全模式1");
		Utils::ImageProtect::instance().register_callback(&client_start_routine);
		Utils::ImageProtect::instance().install();
	}
	else
	{
		LOG("启用安全模式2");
		client_start_routine();
	}
	VMP_VIRTUALIZATION_END();
}

RUNGATE_API Init(lfengine::client::PAppFuncDef AppFunc, int AppFuncCrc)// noexcept
{
	using namespace lfengine::client;
	g_AppFunc.AddChatText = AppFunc->AddChatText;
	g_AppFunc.SendSocket = AppFunc->SendSocket;

	AddChatText = AppFunc->AddChatText;
	SendSocket = AppFunc->SendSocket;

	LOG("lf客户端插件拉起成功 dll_base:%p", dll_base);
	//AddChatText("lf客户端插件拉起成功", 0x0000ff, 0);
	lfengine::TDefaultMessage initok(0, 10000, 0, 0, 0);
	SendSocket(&initok, 0, 0);
}


RUNGATE_API HookRecv(lfengine::PTDefaultMessage defMsg, char* lpData, int dataLen)
{
	using namespace lfengine::client; 
	if (defMsg->ident == 10001)
	{
		try
		{
			client_entry(lpData);
		}
		catch (...)
		{
			LOG("client_entry 异常");
		}
		//AddChatText(lpData, 0x0000ff, 0);
		LOG("lf客户端插件HookRecv--gate_ip: %s ", lpData);
	}

}
std::unique_ptr<FileChangeNotifier> notifier;
RUNGATE_API DoUnInit() noexcept
{
	try
	{	
		LOG("插件卸载开始");
		//SetEvent(dll_exit_event_handle_);

		//if (notifier) {
		//	notifier->join_all();
		//}
		LOG("插件卸载 FileChangeNotifier -- join_all ok");
		WndProcHook::restore_hook();
		LightHook::HookMgr::instance().restore();
		LOG("插件卸载  -- restore_hook ok");
		if (!g_game_io.stopped()) {
			g_game_io.stop();
			g_game_io.reset();
		}
		
		client_->stop_all_timers();
		client_->stop_all_timed_tasks();
		client_->stop();
		/*
		* set_terminate_threads 强制退出asio的线程,在win7下, 否则会导致线程无法退出, 导致线程句柄泄露, 导致崩溃
		必须要destroy, 否则会导致定时器无法销毁, 导致定时器的线程还在执行, 小退再开始游戏时线程还在执行之前dll的地址, 会导致崩溃
		if (Utils::CWindows::instance().get_system_version() <= Utils::CWindows::WINDOWS_7) {
			//asio::detail::win_thread::set_terminate_threads(true);
		}
		*/
		client_->destroy();
		if(dll_exit_event_handle_){
			CloseHandle(dll_exit_event_handle_);
		}
		//Sleep(500);
		LOG("插件卸载完成");
	}
	catch (...)
	{
		LOG("DoUnInit 异常");
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
		dll_base = hModule; LOG("DLL_PROCESS_ATTACH"); break;
    case DLL_THREAD_ATTACH:
		LOG("DLL_THREAD_ATTACH"); break;
	case DLL_THREAD_DETACH:
		LOG("DLL_THREAD_DETACH"); break;
    case DLL_PROCESS_DETACH:
		//DoUnInit();
		LOG("DLL_PROCESS_DETACH"); break;
    }
    return TRUE;
}

