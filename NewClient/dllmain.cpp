#include "pch.h"
#include "ClientImpl.h"
#include "WndProcHook.h"
#include "version.build"
#include "../lf_rungate_server_plug/lf_plug_sdk.h"
#include "Lightbone/lighthook.h"
//#include "anti_monitor_directory/ReadDirectoryChanges.h"

#define RUNGATE_API void __stdcall
using namespace lfengine::client;

std::shared_ptr<TAppFuncDefExt> g_AppFunc;
std::shared_ptr<HINSTANCE> dll_base;
std::shared_ptr<asio::io_service> g_game_io;
std::shared_ptr<asio::detail::thread_group> g_thread_group;
std::shared_ptr<int> g_client_rev_version;
std::shared_ptr<CClientImpl> client_;
std::shared_ptr<asio2::timer> g_timer;
// dll 退出信号
//std::shared_ptr<HANDLE> dll_exit_event_handle_;
RUNGATE_API HookRecv(lfengine::PTDefaultMessage defMsg, char* lpData, int dataLen);
RUNGATE_API DoUnInit() noexcept;
void client_start_routine();
RUNGATE_API client_entry(std::string guard_gate_ip);

RUNGATE_API Init(PAppFuncDef AppFunc)
{
	if (AppFunc->AddChatText && AppFunc->SendSocket) {
		LOG("===== Plugin Init OK =====");

		lfengine::TDefaultMessage initok(0, 10000, 0, 0, 0);
		AppFunc->SendSocket(&initok, 0, 0);
	}
	else {
		LOG("Init 异常");
		return;
	}
	g_client_rev_version = std::make_shared<int>(REV_VERSION);
	g_AppFunc = std::make_shared<TAppFuncDefExt>();
	g_game_io = std::make_shared<asio::io_service>();
	g_thread_group = std::make_shared<asio::detail::thread_group>();
	g_timer = std::make_shared<asio2::timer>();
	//dll_exit_event_handle_ = std::make_shared<HANDLE>(CreateEvent(NULL, TRUE, FALSE, NULL));
	g_AppFunc->AddChatText = AppFunc->AddChatText;
	g_AppFunc->SendSocket = AppFunc->SendSocket;

	AddChatText = AppFunc->AddChatText;
	SendSocket = AppFunc->SendSocket;

	LOG("lf客户端插件拉起成功 dll_base:%p", dll_base);
	//AddChatText("lf客户端插件拉起成功", 0x0000ff, 0);
}

#ifdef _DEBUG
#pragma comment(linker, "/EXPORT:client_entry=?client_entry@@YGXV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z")
#endif
RUNGATE_API client_entry(std::string guard_gate_ip)
{
#if 0
	g_client_rev_version = std::make_shared<int>(REV_VERSION);
	g_AppFunc = std::make_shared<TAppFuncDefExt>();
	g_game_io = std::make_shared<asio::io_service>();
	g_thread_group = std::make_shared<asio::detail::thread_group>();
	g_timer = std::make_shared<asio2::timer>();
	dll_exit_event_handle_ = std::make_shared<HANDLE>(CreateEvent(NULL, TRUE, FALSE, NULL));
#endif
	VMP_VIRTUALIZATION_BEGIN();
	setlocale(LC_CTYPE, "");
	ProtocolCFGLoader cfg;
	cfg.set_field<std::string>(ip_field_id, guard_gate_ip.empty() ? "127.0.0.1" : guard_gate_ip);
	cfg.set_field<int>(port_field_id, 23268);
	client_ = std::make_shared<CClientImpl>();
	client_->cfg() = std::make_unique<ProtocolCFGLoader>(cfg);
	client_->cfg()->set_field<bool>(test_mode_field_id, false);
	client_->cfg()->set_field<bool>(sec_no_change_field_id, false);
#ifdef _DEBUG
	client_->cfg()->set_field<bool>(test_mode_field_id, true);
	client_->cfg()->set_field<bool>(sec_no_change_field_id, false);
#endif
	try
	{
		client_start_routine();
	}
	catch (...)
	{
		LOG("client_start_routine 异常");
	}
	VMP_VIRTUALIZATION_END();
}

void client_start_routine()
{
	LOG("client_start_routine ");
	WndProcHook::install_hook();
	auto ip = client_->cfg()->get_field<std::string>(ip_field_id);
	auto port = client_->cfg()->get_field<int>(port_field_id);
	client_->start(ip, port);
	LOG("client_start_routine ip:%s, port:%d", ip.c_str(), port);
}

RUNGATE_API HookRecv(lfengine::PTDefaultMessage defMsg, char* lpData, int dataLen)
{
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

	if (defMsg->ident == 10002 || defMsg->ident == 10003)
	{
		AddChatText(lpData, 0x0000ff, 0);
	}

}

//std::unique_ptr<FileChangeNotifier> notifier;
RUNGATE_API DoUnInit() noexcept
{
	try
	{
		LOG("插件卸载开始");
		//SetEvent(dll_exit_event_handle_);

		//client_->wait_stop();
		//if (notifier) {
		//	notifier->join_all();
		//}
		LOG("插件卸载 FileChangeNotifier -- join_all ok");
		g_timer->stop();
		g_timer->destroy();
		WndProcHook::restore_hook();
		LightHook::HookMgr::instance().restore();
		LOG("插件卸载  -- restore_hook ok");
		g_game_io->stop();
		g_game_io->reset();
		/*作者建议：
		if(!game_io->stopped()) 把这个判断删了，直接stop即可
		你在这个uninit里加个日志，检测一下是在哪个步骤阻塞的，好判断问题范围
		client.post([&client](){client.socket().close();})  先把socket直接关了就行 这样后续的包就不会发送了，很快就结束 了
		client.stop();
		*/
		LOG("插件卸载stop 1");
		auto start = std::chrono::steady_clock::now(); // 记录开始时间
		/*client_->post([]() {LOG("插件卸载socket close 1");
			client_->socket().close(); LOG("插件卸载socket close 2");
		});*/
		if (client_->is_started())
		{
			client_->stop_all_timers();
			client_->stop_all_timed_tasks();
			client_->stop_all_timed_events();
			if (Utils::CWindows::instance().get_system_version() > WINDOWS_7) {				
				LOG("win_version > WINDOWS_7");
			}
			else {
				LOG("win_version <= WINDOWS_7");
				asio::detail::win_thread::set_terminate_threads(true);
				// 判断是否是win7系统，如果是，则设置线程终止标志
			}
			client_->socket().close();
			client_->stop();
		}
		auto end = std::chrono::steady_clock::now(); // 记录结束时间
		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
		LOG("插件卸载stop 2 %d ms", elapsed);
		/*
		* set_terminate_threads 强制退出asio的线程,否则会导致线程无法退出, 导致线程句柄泄露, 导致崩溃
		必须要destroy, 否则会导致定时器无法销毁, 导致定时器的线程还在执行, 小退再开始游戏时线程还在执行之前dll的地址, 会导致崩溃
		*/
		client_->destroy();
		client_.reset();
		/*if(dll_exit_event_handle_){
			CloseHandle(dll_exit_event_handle_);
		}*/
		LOG("插件卸载完成");
	}
	catch (...)
	{
		LOG("DoUnInit 异常");
	}
}

BOOL APIENTRY DllMain(HMODULE hModule,	DWORD  ul_reason_for_call,	LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			dll_base = std::make_shared<HINSTANCE>(hModule); LOG("DLL_PROCESS_ATTACH"); break;
		case DLL_THREAD_ATTACH:
			LOG("DLL_THREAD_ATTACH"); break;
		case DLL_THREAD_DETACH:
			LOG("DLL_THREAD_DETACH"); break;
		case DLL_PROCESS_DETACH:
			//DoUnInit();不能在这里调用DoUnInit-->>不能在Windows Dll的DLL_PROCESS_DETACH块中调用asio2的server或client对象的stop函数，会导致stop函数永远阻塞无法返回。
			//原因是由于在DLL_PROCESS_DETACH时，通过PostQueuedCompletionStatus投递的IOCP事件永远得不到执行。
			LOG("DLL_PROCESS_DETACH"); break;
	}
	return TRUE;
}