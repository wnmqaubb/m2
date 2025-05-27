#include "pch.h"
#include "ClientImpl.h"
#include "WndProcHook.h"
#include "version.build"
#include "../lf_rungate_server_plug/lf_plug_sdk.h"
#include "Lightbone/lighthook.h"
//#include <asio/experimental/channel.hpp>
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
HWND g_main_window_hwnd;
const UINT WM_UNLOAD_COMPLETE = RegisterWindowMessageW(L"WM_UNLOAD_COMPLETE_7A3F1B");  // [!code ++]
// ����ȫ�ֱ�����ר��JS�̳߳غ��ź���
//std::shared_ptr<asio::thread_pool> g_js_thread_pool = std::make_shared<asio::thread_pool>(4); // 4�̳߳�
//std::shared_ptr<asio::experimental::channel<void(asio::error_code)>> g_js_semaphore;
// dll �˳��ź�
//std::shared_ptr<HANDLE> dll_exit_event_handle_;
RUNGATE_API HookRecv(lfengine::PTDefaultMessage defMsg, char* lpData, int dataLen);
extern "C" __declspec(dllexport) void DoUnInit() noexcept;
void client_start_routine();
RUNGATE_API client_entry(std::string guard_gate_ip);
#include <Psapi.h>
#include <dbghelp.h>

#pragma comment(lib, "Dbghelp.lib")

// ȫ�ֻ�������StackWalk64���̰߳�ȫ��
std::mutex g_stackwalk_mutex;

bool IsThreadFromDllModule(DWORD tid) {
    std::lock_guard<std::mutex> lock(g_stackwalk_mutex);

    // 1. ��ȡ��ǰDLLģ����Ϣ
    MODULEINFO modInfo = { 0 };
    if (!GetModuleInformation(
        GetCurrentProcess(),
        *dll_base,
        &modInfo,
        sizeof(MODULEINFO))
        ) {
        return false;
    }

    // 2. ��Ŀ���߳�
    HANDLE hThread = OpenThread(
        THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION,
        FALSE,
        tid
    );
    if (!hThread) return false;

    // 3. ��ȡ�߳�������
    CONTEXT ctx = { 0 };
    ctx.ContextFlags = CONTEXT_ALL;
    if (!GetThreadContext(hThread, &ctx)) {
        CloseHandle(hThread);
        return false;
    }

    // 4. ��ʼ����ջ֡��32λ�ܹ���
    STACKFRAME64 stack = { 0 };
    stack.AddrPC.Offset = ctx.Eip;
    stack.AddrPC.Mode = AddrModeFlat;
    stack.AddrStack.Offset = ctx.Esp;
    stack.AddrStack.Mode = AddrModeFlat;
    stack.AddrFrame.Offset = ctx.Ebp;
    stack.AddrFrame.Mode = AddrModeFlat;

    // 5. ��������ջ
    bool belongs = false;
    for (int i = 0; i < 32; i++) { // ���32�����ջ
        if (!StackWalk64(
            IMAGE_FILE_MACHINE_I386, // 32λ����
            GetCurrentProcess(),
            hThread,
            &stack,
            &ctx,
            NULL,
            SymFunctionTableAccess64,
            SymGetModuleBase64,
            NULL)
            ) {
            break;
        }

        // ����ַ�Ƿ���DLLģ�鷶Χ��
        if (stack.AddrPC.Offset >= (DWORD64)modInfo.lpBaseOfDll &&
            stack.AddrPC.Offset < (DWORD64)modInfo.lpBaseOfDll + modInfo.SizeOfImage
            ) {
            belongs = true;
            break;
        }
    }

    CloseHandle(hThread);
    return belongs;
}

// ȫ�ֻ�����
std::mutex socket_mutex;

// �����첽ж�ؿ�����
class UnloadController {
public:
    static void RequestUnload() {
        // ����ר��ж���߳�
        HANDLE hThread = CreateThread(NULL, 0, UnloadThreadProc, NULL, 0, NULL);
        SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);
    }

private:
    static DWORD WINAPI UnloadThreadProc(LPVOID) {
        __try {
            // ִ��ʵ��ж�ز���
            CoreUnloadProcess();

            // ������Ϣǰ����ڴ�����
            MemoryBarrier();
            // ��ǿ����Ϣ�����߼�
            if (g_main_window_hwnd && IsWindow(g_main_window_hwnd))
            {
                // �������η���
                for (int i = 0; i < 3; i++)
                {
                    if (PostMessage(g_main_window_hwnd, WM_UNLOAD_COMPLETE, 0, 0))
                    {
                        LOG("Unload complete message sent");
                        break;
                    }
                    Sleep(10);
                }
            }
            else
            {
                LOG("Invalid window handle");
            }
        }
        __except (FilterException(GetExceptionCode())) {
            LOG("ж���̱߳���: 0x%X", GetExceptionCode());
        }
        return 0;
    }

    // �Զ����쳣������
    static LONG FilterException(DWORD code) {
        if (code == EXCEPTION_ACCESS_VIOLATION) {
            LOG("����AV�쳣����ȫ����");
            return EXCEPTION_EXECUTE_HANDLER;
        }
        return EXCEPTION_CONTINUE_SEARCH;
    }
    static void SafeCloseSocket(asio::ip::tcp::socket& socket) {
        std::lock_guard<std::mutex> lock(socket_mutex);
        asio::error_code ec;
        try {
            // ֱ�ӵ���ԭ�� API �ر� Socket������ ASIO ��������⣩
            if (socket.is_open()) {
                // 1. ASIO �ر�
                socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
                if (ec) {
                    if (ec == asio::error::not_connected) {
                        LOG("ASIO shutdown: �׽���δ���ӣ��޺���");
                    }
                    else {
                        LOG("ASIO shutdown ����: %s", ec.message().c_str());
                    }
                }

                socket.close(ec);
                if (ec) {
                    LOG("ASIO close ����: %s", ec.message().c_str());
                }

                // 2. ԭ�� API ����
                SOCKET native_handle = socket.native_handle();
                if (native_handle != INVALID_SOCKET) {
                    if (::shutdown(native_handle, SD_BOTH) == SOCKET_ERROR) {
                        DWORD error = WSAGetLastError();
                        if (error != WSAENOTCONN && error != WSAENOTSOCK) {
                            LOG("ԭ�� shutdown ����: %d", error);
                        }
                    }

                    if (::closesocket(native_handle) == SOCKET_ERROR) {
                        DWORD error = WSAGetLastError();
                        if (error != WSAENOTSOCK) {
                            LOG("ԭ�� closesocket ����: %d", error);
                        }
                    }
                }

                // 3. ǿ�Ʊ��Ϊ�ر�
                socket.close(ec); // �ٴε���ȷ��״̬ͬ��
            }
        }
        catch (const std::exception& e) {
            LOG("C++ �쳣: SafeCloseSocket %s", e.what());
        }
        catch (...) {
            LOG("δ֪�쳣 SafeCloseSocket");
        }
    }

    static void CoreUnloadProcess() {
        __try {
            LOG("ģ��ж�ؿ�ʼ");
            auto start = std::chrono::steady_clock::now(); // ��¼��ʼʱ��
            if (client_) {
                // �����Զ�����
                LOG("���ж����� line:%d", __LINE__);
                client_->auto_reconnect(false);

                LOG("���ж����� line:%d", __LINE__);
                // ֹͣio_context
                client_->stop();

                LOG("���ж����� line:%d", __LINE__);
                while (!client_->is_stopped()) { // �ȴ�������ȫֹͣ
                    LOG("���ж����� line:%d", __LINE__);
                    std::this_thread::sleep_for(std::chrono::milliseconds(2));
                }

                // �ͷ��׽���
                LOG("���ж����� line:%d", __LINE__);
                client_->socket().close();


                LOG("���ж����� line:%d", __LINE__);
                client_->destroy();
                LOG("���ж����� line:%d", __LINE__);
                // ����clientָ��
                //client_->();
                // 1. ����ֹͣ���ж�ʱ��
                if (g_timer) {
                    LOG("���ж����� line:%d", __LINE__);
                    g_timer->stop();
                    LOG("���ж����� line:%d", __LINE__);
                    //g_timer->destroy();
                    LOG("���ж����� line:%d", __LINE__);
                }
                // 3. ֹͣI/O������ն���
                if (g_game_io) {
                    LOG("���ж����� line:%d", __LINE__);
                    g_game_io->stop();
                    LOG("���ж����� line:%d", __LINE__);
                    //g_game_io->reset();
                    LOG("���ж����� line:%d", __LINE__);
                }
                LOG("���ж����� line:%d", __LINE__);
                //client_->socket().cancel();  // ����ȡ�������첽����
                //LOG("���ж����� line:%d", __LINE__);
                //client_->socket().shutdown(asio::socket_base::shutdown_both);
                //LOG("���ж����� line:%d", __LINE__);
                ////client_->socket().close();
                //LOG("���ж����� line:%d", __LINE__);
                //client_->stop();
                //LOG("���ж����� line:%d", __LINE__);
                //client_->destroy();
                //LOG("���ж����� line:%d", __LINE__);
                //__try {
                //    client_->iopool().destroy();
                //}
                //__except (EXCEPTION_EXECUTE_HANDLER) {
                //    LOG("�ͷ� client_ �쳣: 0x%X", GetExceptionCode());
                //}
            }
            //// 1. ����ֹͣ���ж�ʱ��
            //if (g_timer) {
            //    LOG("���ж����� line:%d", __LINE__);
            //    g_timer->stop();
            //    LOG("���ж����� line:%d", __LINE__);
            //    g_timer->destroy();
            //    LOG("���ж����� line:%d", __LINE__);
            //}
            //if (Utils::CWindows::instance().get_system_version() > WINDOWS_7) {
            //    // 2. �ر��������ӣ�����ģʽ��
            //    if (client_) {
            //        LOG("���ж����� line:%d", __LINE__);
            //        client_->socket().cancel();  // ����ȡ�������첽����
            //        LOG("���ж����� line:%d", __LINE__);
            //        client_->socket().shutdown(asio::socket_base::shutdown_both);
            //        LOG("���ж����� line:%d", __LINE__);
            //        //client_->socket().close();
            //        LOG("���ж����� line:%d", __LINE__);
            //        client_->stop();
            //        LOG("���ж����� line:%d", __LINE__);
            //        client_->destroy();
            //        LOG("���ж����� line:%d", __LINE__);
            //        __try {
            //            client_->iopool().destroy();
            //        }
            //        __except (EXCEPTION_EXECUTE_HANDLER) {
            //            LOG("�ͷ� client_ �쳣: 0x%X", GetExceptionCode());
            //        }
            //    }

            //    // 3. ֹͣI/O������ն���
            //    if (g_game_io) {
            //        LOG("���ж����� line:%d", __LINE__);
            //        g_game_io->stop();
            //        LOG("���ж����� line:%d", __LINE__);
            //        g_game_io->reset();
            //        LOG("���ж����� line:%d", __LINE__);
            //    }
            //}
            //else if (Utils::CWindows::instance().get_system_version() == WINDOWS_7) {
            //    // 2. �ر��������ӣ�����ģʽ��
            //    if (client_) {
            //        LOG("���ж����� line:%d", __LINE__);
            //        client_->socket().close();
            //        LOG("���ж����� line:%d", __LINE__);
            //        client_->super::super::stop();
            //        LOG("���ж����� line:%d", __LINE__);
            //        client_->destroy();
            //        LOG("���ж����� line:%d", __LINE__);
            //        __try {
            //            client_.reset();
            //        }
            //        __except (EXCEPTION_EXECUTE_HANDLER) {
            //            LOG("�ͷ� client_ �쳣: 0x%X", GetExceptionCode());
            //        }
            //    }
            //}
            //else {
            //    // 1. ֹͣ�����첽����
            //    if (client_) {
            //        client_->stop_all_timers();
            //        client_->stop_all_timed_tasks();
            //        client_->stop_all_timed_events();
            //    }

            //    // 1. ���ȹر� Socket��ʹ��ԭ�� API ���ף�
            //    if (client_ && client_->socket().is_open()) {
            //        SafeCloseSocket(client_->socket());
            //    }

            //    // 3. ֹͣ I/O ���񣨴����߳��˳���
            //    if (g_game_io) {
            //        g_game_io->stop();
            //        g_game_io->reset();
            //    }

            //    // 4. �ӳ��ͷ� client_ ����ȷ���첽������ɣ�
            //    std::this_thread::sleep_for(std::chrono::milliseconds(50));

            //    __try {
            //        // XPϵͳ��Ҫ����ǿ�ƹر�
            //        //if (Utils::CWindows::instance().get_system_version() <= WINDOWS_7) {
            //        //    LOG("���ж����� line:%d", __LINE__);
            //        //    asio::detail::win_thread::set_terminate_threads(true); // ǿ����ֹASIO�߳�
            //        //}
            //        //LOG("���ж����� line:%d", __LINE__);
            //        //client_->stop();
            //        //LOG("���ж����� line:%d", __LINE__);
            //        //client_->destroy();
            //        //LOG("���ж����� line:%d", __LINE__);
            //        //client_.reset();
            //        LOG("���ж����� line:%d", __LINE__);
            //    }
            //    __except (EXCEPTION_EXECUTE_HANDLER) {
            //        LOG("�ͷ� client_ �쳣: 0x%X", GetExceptionCode());
            //    }
            //}


            // 4. �����̣߳����ٵȴ���
            if (g_thread_group) {
                LOG("���ж����� line:%d", __LINE__);
                g_thread_group->~thread_group(); // ֱ������
                LOG("���ж����� line:%d", __LINE__);
                g_thread_group.reset();
                LOG("���ж����� line:%d", __LINE__);
            }

            // 5. ���ٻָ�ϵͳ����
            LOG("���ж����� line:%d", __LINE__);
            LightHook::HookMgr::instance().restore();
            LOG("���ж����� line:%d", __LINE__);

            auto end = std::chrono::steady_clock::now(); // ��¼����ʱ��
            auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            LOG("���ж��stop 2 %d ms", elapsed);
            LOG("ģ��ж�����");
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            
            LOG("ж�ع����в����쳣: 0x%X", GetExceptionCode());
        }
    }
};

extern "C" __declspec(dllexport) void Init(PAppFuncDef AppFunc)
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

	LOG("lf�ͻ��˲������ɹ� dll_base:%p", *dll_base);
	//AddChatText("lf�ͻ��˲������ɹ�", 0x0000ff, 0);
}

inline LONG WINAPI GlobalExceptionFilter(_EXCEPTION_POINTERS* pExp)
{
    LOG("GlobalExceptionFilter�����쳣");
    return EXCEPTION_CONTINUE_EXECUTION;
}
#ifdef _DEBUG
#pragma comment(linker, "/EXPORT:client_entry=?client_entry@@YGXV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z")
#endif
RUNGATE_API client_entry(std::string guard_gate_ip)
{
    // �� main �������ʼ��������ע�� SEH ������
    SetUnhandledExceptionFilter(GlobalExceptionFilter);
    _se_translator_function old_seh = _set_se_translator([](unsigned code, EXCEPTION_POINTERS*) {
        throw std::runtime_error("SEH Exception: code=" + std::to_string(code));
    });
#if 1
	g_client_rev_version = std::make_shared<int>(REV_VERSION);
	g_AppFunc = std::make_shared<TAppFuncDefExt>();
	g_game_io = std::make_shared<asio::io_service>();
	g_thread_group = std::make_shared<asio::detail::thread_group>();
	g_timer = std::make_shared<asio2::timer>();
    //dll_base = std::make_shared<HINSTANCE>(nullptr);
    //*dll_base = GetModuleHandle(nullptr);
	//dll_exit_event_handle_ = std::make_shared<HANDLE>(CreateEvent(NULL, TRUE, FALSE, NULL));
#endif
	VMP_VIRTUALIZATION_BEGIN();
	try
	{
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
		client_start_routine();

        /*printf("-----------------------------------------------------------------------------------------------------------------\n");
        std::thread t([]() {
            try {
                for (int i = 0; i < 10; i++) {
                    Sleep(5000);
                    Utils::CWindows::ProcessInfo pi;
                    Utils::CWindows::instance().get_process(Utils::CWindows::instance().get_current_process_id(), pi);
                    int n = 0;
                    for (auto& i : pi.threads) {
                        if (!IsThreadFromDllModule(i.second.tid)) continue;
                        printf("%d \t%d \t%d \t%p\n", ++n, i.first, i.second.tid, i.second.start_address);
                    }
                    printf("-----------------------------------------------------------------------------------------------------------------\n");
                }
                
            }
            catch (const std::exception& e) {
                LOG("IsThreadFromModule: %s", e.what());
            }
        });
        t.detach();*/
	}
	catch (std::exception& e)
	{
		LOG("client_start_routine �쳣: %s ",e.what());
	}
	VMP_VIRTUALIZATION_END();
}

void client_start_routine()
{
    LOG("client_start_routine ");
    //WndProcHook::install_hook();
    // �� client_start_routine ������ ASIO ������
    g_game_io->notify_fork(asio::io_service::fork_prepare);
    g_game_io->notify_fork(asio::io_service::fork_child);
    asio::detail::socket_ops::clear_last_error(); // ���Ǳ�ڴ�����
    // ȷ�� ASIO �߳����� client_ ǰ��ʼ��
    //g_thread_group = std::make_shared<asio::detail::thread_group>();
    // �޸�client_start_routine�е��̴߳���
    auto ip = client_->cfg()->get_field<std::string>(ip_field_id);
    auto port = client_->cfg()->get_field<int>(port_field_id);
    client_->async_start(ip, port);
    LOG("client_start_routine ip:%s, port:%d", ip.c_str(), port);
    g_thread_group->create_threads([&]() {
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
        g_game_io->run();
        LOG("ASIO�̰߳�ȫ�˳�"); // ȷ���߳��˳�ʱ��ģ�����
    }, 2);
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
// �޸�DoUnInit����
extern "C" __declspec(dllexport) void DoUnInit() noexcept
{
    UnloadController::RequestUnload(); // ����������
    return; // ��������
}

RUNGATE_API DoUnInit1() noexcept
{
    //try
    __try
    {
        LOG("���ж�ؿ�ʼ");
        //SetEvent(dll_exit_event_handle_);

        //client_->wait_stop();
        //if (notifier) {
        //	notifier->join_all();
        //}
        //WndProcHook::restore_hook();
        LOG("���ж��LightHook::HookMgr::instance().restore()1");
        LightHook::HookMgr::instance().restore();
        LOG("���ж��  -- restore_hook ok");
        g_game_io->stop();
        g_game_io->reset();
        //g_thread_group->join();
        //g_thread_group.reset();
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
        auto end = std::chrono::steady_clock::now(); // ��¼����ʱ��
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        LOG("���ж��stop 2 %d ms", elapsed);
        /*
        * set_terminate_threads ǿ���˳�asio���߳�,����ᵼ���߳��޷��˳�, �����߳̾��й¶, ���±���
        ����Ҫdestroy, ����ᵼ�¶�ʱ���޷�����, ���¶�ʱ�����̻߳���ִ��, С���ٿ�ʼ��Ϸʱ�̻߳���ִ��֮ǰdll�ĵ�ַ, �ᵼ�±���
        */
        LOG("���ж�����1");
        client_->destroy();
        LOG("���ж�����2");
        client_.reset();
        LOG("���ж�����3");
        /*if(dll_exit_event_handle_){
            CloseHandle(dll_exit_event_handle_);
        }*/
        LOG("���ж�����");
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    //catch (...)
    {
        LOG("DoUnInit �쳣");
    }
}

//RUNGATE_API DoUnInit() noexcept 
//{
//    __try {
//    //try {
//        LOG("���ж�ؿ�ʼ");
//
//        // 1. ֹͣJS�̳߳�
//        if (g_js_thread_pool) {
//            g_js_thread_pool->stop(); // ֹͣ����������
//            g_js_thread_pool->join();  // �ȴ������������
//            g_js_thread_pool.reset();
//            g_js_semaphore.reset();
//            LOG("JS�̳߳����ͷ�");
//        }
//
//        // 2. �ָ�����
//        //WndProcHook::restore_hook();
//        //LightHook::HookMgr::instance().restore();
//
//        // 3. ǿ�ƹر�Socket�Ϳͻ���
//        if (client_ && client_->is_started()) {
//            client_->socket().close();
//            client_->stop_all_timers();
//            asio::detail::win_thread::set_terminate_threads(true);
//            client_->stop();
//            client_->destroy();
//        }
//        client_.reset();
//
//        // 4. ����Asio��Դ
//        if (g_game_io) {
//            g_game_io->stop();
//            g_game_io.reset();
//        }
//        g_thread_group.reset();
//
//        LOG("���ж�����");
//    }
//    __except (EXCEPTION_EXECUTE_HANDLER) {
//        LOG("DoUnInit �쳣");
//    }
//    /*catch (...) {
//        LOG("DoUnInit �쳣");
//    }*/
//}

BOOL APIENTRY DllMain(HMODULE hModule,	DWORD  ul_reason_for_call,	LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			dll_base = std::make_shared<HINSTANCE>(hModule); LOG("DLL_PROCESS_ATTACH1 hModule:%p", hModule); break;
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