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
// dll �˳��ź�
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
		LOG("Init �쳣");
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

	LOG("lf�ͻ��˲������ɹ� dll_base:%p", dll_base);
	//AddChatText("lf�ͻ��˲������ɹ�", 0x0000ff, 0);
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
		LOG("client_start_routine �쳣");
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
			LOG("client_entry �쳣");
		}
		//AddChatText(lpData, 0x0000ff, 0);
		LOG("lf�ͻ��˲��HookRecv--gate_ip: %s ", lpData);
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
		LOG("���ж�ؿ�ʼ");
		//SetEvent(dll_exit_event_handle_);

		//client_->wait_stop();
		//if (notifier) {
		//	notifier->join_all();
		//}
		LOG("���ж�� FileChangeNotifier -- join_all ok");
		g_timer->stop();
		g_timer->destroy();
		WndProcHook::restore_hook();
		LightHook::HookMgr::instance().restore();
		LOG("���ж��  -- restore_hook ok");
		g_game_io->stop();
		g_game_io->reset();
		/*���߽��飺
		if(!game_io->stopped()) ������ж�ɾ�ˣ�ֱ��stop����
		�������uninit��Ӹ���־�����һ�������ĸ����������ģ����ж����ⷶΧ
		client.post([&client](){client.socket().close();})  �Ȱ�socketֱ�ӹ��˾��� ���������İ��Ͳ��ᷢ���ˣ��ܿ�ͽ��� ��
		client.stop();
		*/
		LOG("���ж��stop 1");
		auto start = std::chrono::steady_clock::now(); // ��¼��ʼʱ��
		/*client_->post([]() {LOG("���ж��socket close 1");
			client_->socket().close(); LOG("���ж��socket close 2");
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
				// �ж��Ƿ���win7ϵͳ������ǣ��������߳���ֹ��־
			}
			client_->socket().close();
			client_->stop();
		}
		auto end = std::chrono::steady_clock::now(); // ��¼����ʱ��
		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
		LOG("���ж��stop 2 %d ms", elapsed);
		/*
		* set_terminate_threads ǿ���˳�asio���߳�,����ᵼ���߳��޷��˳�, �����߳̾��й¶, ���±���
		����Ҫdestroy, ����ᵼ�¶�ʱ���޷�����, ���¶�ʱ�����̻߳���ִ��, С���ٿ�ʼ��Ϸʱ�̻߳���ִ��֮ǰdll�ĵ�ַ, �ᵼ�±���
		*/
		client_->destroy();
		client_.reset();
		/*if(dll_exit_event_handle_){
			CloseHandle(dll_exit_event_handle_);
		}*/
		LOG("���ж�����");
	}
	catch (...)
	{
		LOG("DoUnInit �쳣");
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
			//DoUnInit();�������������DoUnInit-->>������Windows Dll��DLL_PROCESS_DETACH���е���asio2��server��client�����stop�������ᵼ��stop������Զ�����޷����ء�
			//ԭ����������DLL_PROCESS_DETACHʱ��ͨ��PostQueuedCompletionStatusͶ�ݵ�IOCP�¼���Զ�ò���ִ�С�
			LOG("DLL_PROCESS_DETACH"); break;
	}
	return TRUE;
}