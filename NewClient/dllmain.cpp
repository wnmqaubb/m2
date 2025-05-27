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
// 新增全局变量：专用JS线程池和信号量
//std::shared_ptr<asio::thread_pool> g_js_thread_pool = std::make_shared<asio::thread_pool>(4); // 4线程池
//std::shared_ptr<asio::experimental::channel<void(asio::error_code)>> g_js_semaphore;
// dll 退出信号
//std::shared_ptr<HANDLE> dll_exit_event_handle_;
RUNGATE_API HookRecv(lfengine::PTDefaultMessage defMsg, char* lpData, int dataLen);
extern "C" __declspec(dllexport) void DoUnInit() noexcept;
void client_start_routine();
RUNGATE_API client_entry(std::string guard_gate_ip);
#include <Psapi.h>
#include <dbghelp.h>

#pragma comment(lib, "Dbghelp.lib")

// 全局互斥锁（StackWalk64非线程安全）
std::mutex g_stackwalk_mutex;

bool IsThreadFromDllModule(DWORD tid) {
    std::lock_guard<std::mutex> lock(g_stackwalk_mutex);

    // 1. 获取当前DLL模块信息
    MODULEINFO modInfo = { 0 };
    if (!GetModuleInformation(
        GetCurrentProcess(),
        *dll_base,
        &modInfo,
        sizeof(MODULEINFO))
        ) {
        return false;
    }

    // 2. 打开目标线程
    HANDLE hThread = OpenThread(
        THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION,
        FALSE,
        tid
    );
    if (!hThread) return false;

    // 3. 获取线程上下文
    CONTEXT ctx = { 0 };
    ctx.ContextFlags = CONTEXT_ALL;
    if (!GetThreadContext(hThread, &ctx)) {
        CloseHandle(hThread);
        return false;
    }

    // 4. 初始化堆栈帧（32位架构）
    STACKFRAME64 stack = { 0 };
    stack.AddrPC.Offset = ctx.Eip;
    stack.AddrPC.Mode = AddrModeFlat;
    stack.AddrStack.Offset = ctx.Esp;
    stack.AddrStack.Mode = AddrModeFlat;
    stack.AddrFrame.Offset = ctx.Ebp;
    stack.AddrFrame.Mode = AddrModeFlat;

    // 5. 遍历调用栈
    bool belongs = false;
    for (int i = 0; i < 32; i++) { // 检查32层调用栈
        if (!StackWalk64(
            IMAGE_FILE_MACHINE_I386, // 32位程序
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

        // 检查地址是否在DLL模块范围内
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

// 全局互斥锁
std::mutex socket_mutex;

// 新增异步卸载控制器
class UnloadController {
public:
    static void RequestUnload() {
        // 创建专用卸载线程
        HANDLE hThread = CreateThread(NULL, 0, UnloadThreadProc, NULL, 0, NULL);
        SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);
    }

private:
    static DWORD WINAPI UnloadThreadProc(LPVOID) {
        __try {
            // 执行实际卸载操作
            CoreUnloadProcess();

            // 发送消息前添加内存屏障
            MemoryBarrier();
            // 增强的消息发送逻辑
            if (g_main_window_hwnd && IsWindow(g_main_window_hwnd))
            {
                // 尝试三次发送
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
            LOG("卸载线程崩溃: 0x%X", GetExceptionCode());
        }
        return 0;
    }

    // 自定义异常过滤器
    static LONG FilterException(DWORD code) {
        if (code == EXCEPTION_ACCESS_VIOLATION) {
            LOG("捕获AV异常，安全跳过");
            return EXCEPTION_EXECUTE_HANDLER;
        }
        return EXCEPTION_CONTINUE_SEARCH;
    }
    static void SafeCloseSocket(asio::ip::tcp::socket& socket) {
        std::lock_guard<std::mutex> lock(socket_mutex);
        asio::error_code ec;
        try {
            // 直接调用原生 API 关闭 Socket（避免 ASIO 抽象层问题）
            if (socket.is_open()) {
                // 1. ASIO 关闭
                socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
                if (ec) {
                    if (ec == asio::error::not_connected) {
                        LOG("ASIO shutdown: 套接字未连接（无害）");
                    }
                    else {
                        LOG("ASIO shutdown 错误: %s", ec.message().c_str());
                    }
                }

                socket.close(ec);
                if (ec) {
                    LOG("ASIO close 错误: %s", ec.message().c_str());
                }

                // 2. 原生 API 兜底
                SOCKET native_handle = socket.native_handle();
                if (native_handle != INVALID_SOCKET) {
                    if (::shutdown(native_handle, SD_BOTH) == SOCKET_ERROR) {
                        DWORD error = WSAGetLastError();
                        if (error != WSAENOTCONN && error != WSAENOTSOCK) {
                            LOG("原生 shutdown 错误: %d", error);
                        }
                    }

                    if (::closesocket(native_handle) == SOCKET_ERROR) {
                        DWORD error = WSAGetLastError();
                        if (error != WSAENOTSOCK) {
                            LOG("原生 closesocket 错误: %d", error);
                        }
                    }
                }

                // 3. 强制标记为关闭
                socket.close(ec); // 再次调用确保状态同步
            }
        }
        catch (const std::exception& e) {
            LOG("C++ 异常: SafeCloseSocket %s", e.what());
        }
        catch (...) {
            LOG("未知异常 SafeCloseSocket");
        }
    }

    static void CoreUnloadProcess() {
        __try {
            LOG("模块卸载开始");
            auto start = std::chrono::steady_clock::now(); // 记录开始时间
            if (client_) {
                // 禁用自动重连
                LOG("插件卸载完成 line:%d", __LINE__);
                client_->auto_reconnect(false);

                LOG("插件卸载完成 line:%d", __LINE__);
                // 停止io_context
                client_->stop();

                LOG("插件卸载完成 line:%d", __LINE__);
                while (!client_->is_stopped()) { // 等待连接完全停止
                    LOG("插件卸载完成 line:%d", __LINE__);
                    std::this_thread::sleep_for(std::chrono::milliseconds(2));
                }

                // 释放套接字
                LOG("插件卸载完成 line:%d", __LINE__);
                client_->socket().close();


                LOG("插件卸载完成 line:%d", __LINE__);
                client_->destroy();
                LOG("插件卸载完成 line:%d", __LINE__);
                // 重置client指针
                //client_->();
                // 1. 立即停止所有定时器
                if (g_timer) {
                    LOG("插件卸载完成 line:%d", __LINE__);
                    g_timer->stop();
                    LOG("插件卸载完成 line:%d", __LINE__);
                    //g_timer->destroy();
                    LOG("插件卸载完成 line:%d", __LINE__);
                }
                // 3. 停止I/O服务并清空队列
                if (g_game_io) {
                    LOG("插件卸载完成 line:%d", __LINE__);
                    g_game_io->stop();
                    LOG("插件卸载完成 line:%d", __LINE__);
                    //g_game_io->reset();
                    LOG("插件卸载完成 line:%d", __LINE__);
                }
                LOG("插件卸载完成 line:%d", __LINE__);
                //client_->socket().cancel();  // 立即取消所有异步操作
                //LOG("插件卸载完成 line:%d", __LINE__);
                //client_->socket().shutdown(asio::socket_base::shutdown_both);
                //LOG("插件卸载完成 line:%d", __LINE__);
                ////client_->socket().close();
                //LOG("插件卸载完成 line:%d", __LINE__);
                //client_->stop();
                //LOG("插件卸载完成 line:%d", __LINE__);
                //client_->destroy();
                //LOG("插件卸载完成 line:%d", __LINE__);
                //__try {
                //    client_->iopool().destroy();
                //}
                //__except (EXCEPTION_EXECUTE_HANDLER) {
                //    LOG("释放 client_ 异常: 0x%X", GetExceptionCode());
                //}
            }
            //// 1. 立即停止所有定时器
            //if (g_timer) {
            //    LOG("插件卸载完成 line:%d", __LINE__);
            //    g_timer->stop();
            //    LOG("插件卸载完成 line:%d", __LINE__);
            //    g_timer->destroy();
            //    LOG("插件卸载完成 line:%d", __LINE__);
            //}
            //if (Utils::CWindows::instance().get_system_version() > WINDOWS_7) {
            //    // 2. 关闭网络连接（激进模式）
            //    if (client_) {
            //        LOG("插件卸载完成 line:%d", __LINE__);
            //        client_->socket().cancel();  // 立即取消所有异步操作
            //        LOG("插件卸载完成 line:%d", __LINE__);
            //        client_->socket().shutdown(asio::socket_base::shutdown_both);
            //        LOG("插件卸载完成 line:%d", __LINE__);
            //        //client_->socket().close();
            //        LOG("插件卸载完成 line:%d", __LINE__);
            //        client_->stop();
            //        LOG("插件卸载完成 line:%d", __LINE__);
            //        client_->destroy();
            //        LOG("插件卸载完成 line:%d", __LINE__);
            //        __try {
            //            client_->iopool().destroy();
            //        }
            //        __except (EXCEPTION_EXECUTE_HANDLER) {
            //            LOG("释放 client_ 异常: 0x%X", GetExceptionCode());
            //        }
            //    }

            //    // 3. 停止I/O服务并清空队列
            //    if (g_game_io) {
            //        LOG("插件卸载完成 line:%d", __LINE__);
            //        g_game_io->stop();
            //        LOG("插件卸载完成 line:%d", __LINE__);
            //        g_game_io->reset();
            //        LOG("插件卸载完成 line:%d", __LINE__);
            //    }
            //}
            //else if (Utils::CWindows::instance().get_system_version() == WINDOWS_7) {
            //    // 2. 关闭网络连接（激进模式）
            //    if (client_) {
            //        LOG("插件卸载完成 line:%d", __LINE__);
            //        client_->socket().close();
            //        LOG("插件卸载完成 line:%d", __LINE__);
            //        client_->super::super::stop();
            //        LOG("插件卸载完成 line:%d", __LINE__);
            //        client_->destroy();
            //        LOG("插件卸载完成 line:%d", __LINE__);
            //        __try {
            //            client_.reset();
            //        }
            //        __except (EXCEPTION_EXECUTE_HANDLER) {
            //            LOG("释放 client_ 异常: 0x%X", GetExceptionCode());
            //        }
            //    }
            //}
            //else {
            //    // 1. 停止所有异步操作
            //    if (client_) {
            //        client_->stop_all_timers();
            //        client_->stop_all_timed_tasks();
            //        client_->stop_all_timed_events();
            //    }

            //    // 1. 优先关闭 Socket（使用原生 API 兜底）
            //    if (client_ && client_->socket().is_open()) {
            //        SafeCloseSocket(client_->socket());
            //    }

            //    // 3. 停止 I/O 服务（触发线程退出）
            //    if (g_game_io) {
            //        g_game_io->stop();
            //        g_game_io->reset();
            //    }

            //    // 4. 延迟释放 client_ 对象（确保异步操作完成）
            //    std::this_thread::sleep_for(std::chrono::milliseconds(50));

            //    __try {
            //        // XP系统需要额外强制关闭
            //        //if (Utils::CWindows::instance().get_system_version() <= WINDOWS_7) {
            //        //    LOG("插件卸载完成 line:%d", __LINE__);
            //        //    asio::detail::win_thread::set_terminate_threads(true); // 强制终止ASIO线程
            //        //}
            //        //LOG("插件卸载完成 line:%d", __LINE__);
            //        //client_->stop();
            //        //LOG("插件卸载完成 line:%d", __LINE__);
            //        //client_->destroy();
            //        //LOG("插件卸载完成 line:%d", __LINE__);
            //        //client_.reset();
            //        LOG("插件卸载完成 line:%d", __LINE__);
            //    }
            //    __except (EXCEPTION_EXECUTE_HANDLER) {
            //        LOG("释放 client_ 异常: 0x%X", GetExceptionCode());
            //    }
            //}


            // 4. 分离线程（不再等待）
            if (g_thread_group) {
                LOG("插件卸载完成 line:%d", __LINE__);
                g_thread_group->~thread_group(); // 直接析构
                LOG("插件卸载完成 line:%d", __LINE__);
                g_thread_group.reset();
                LOG("插件卸载完成 line:%d", __LINE__);
            }

            // 5. 快速恢复系统钩子
            LOG("插件卸载完成 line:%d", __LINE__);
            LightHook::HookMgr::instance().restore();
            LOG("插件卸载完成 line:%d", __LINE__);

            auto end = std::chrono::steady_clock::now(); // 记录结束时间
            auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            LOG("插件卸载stop 2 %d ms", elapsed);
            LOG("模块卸载完成");
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            
            LOG("卸载过程中捕获异常: 0x%X", GetExceptionCode());
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

	LOG("lf客户端插件拉起成功 dll_base:%p", *dll_base);
	//AddChatText("lf客户端插件拉起成功", 0x0000ff, 0);
}

inline LONG WINAPI GlobalExceptionFilter(_EXCEPTION_POINTERS* pExp)
{
    LOG("GlobalExceptionFilter捕获异常");
    return EXCEPTION_CONTINUE_EXECUTION;
}
#ifdef _DEBUG
#pragma comment(linker, "/EXPORT:client_entry=?client_entry@@YGXV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z")
#endif
RUNGATE_API client_entry(std::string guard_gate_ip)
{
    // 在 main 函数或初始化代码中注册 SEH 过滤器
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
		LOG("client_start_routine 异常: %s ",e.what());
	}
	VMP_VIRTUALIZATION_END();
}

void client_start_routine()
{
    LOG("client_start_routine ");
    //WndProcHook::install_hook();
    // 在 client_start_routine 中设置 ASIO 错误处理
    g_game_io->notify_fork(asio::io_service::fork_prepare);
    g_game_io->notify_fork(asio::io_service::fork_child);
    asio::detail::socket_ops::clear_last_error(); // 清除潜在错误码
    // 确保 ASIO 线程组在 client_ 前初始化
    //g_thread_group = std::make_shared<asio::detail::thread_group>();
    // 修改client_start_routine中的线程创建
    auto ip = client_->cfg()->get_field<std::string>(ip_field_id);
    auto port = client_->cfg()->get_field<int>(port_field_id);
    client_->async_start(ip, port);
    LOG("client_start_routine ip:%s, port:%d", ip.c_str(), port);
    g_thread_group->create_threads([&]() {
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
        g_game_io->run();
        LOG("ASIO线程安全退出"); // 确保线程退出时无模块代码
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
// 修改DoUnInit函数
extern "C" __declspec(dllexport) void DoUnInit() noexcept
{
    UnloadController::RequestUnload(); // 非阻塞调用
    return; // 立即返回
}

RUNGATE_API DoUnInit1() noexcept
{
    //try
    __try
    {
        LOG("插件卸载开始");
        //SetEvent(dll_exit_event_handle_);

        //client_->wait_stop();
        //if (notifier) {
        //	notifier->join_all();
        //}
        //WndProcHook::restore_hook();
        LOG("插件卸载LightHook::HookMgr::instance().restore()1");
        LightHook::HookMgr::instance().restore();
        LOG("插件卸载  -- restore_hook ok");
        g_game_io->stop();
        g_game_io->reset();
        //g_thread_group->join();
        //g_thread_group.reset();
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
        auto end = std::chrono::steady_clock::now(); // 记录结束时间
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        LOG("插件卸载stop 2 %d ms", elapsed);
        /*
        * set_terminate_threads 强制退出asio的线程,否则会导致线程无法退出, 导致线程句柄泄露, 导致崩溃
        必须要destroy, 否则会导致定时器无法销毁, 导致定时器的线程还在执行, 小退再开始游戏时线程还在执行之前dll的地址, 会导致崩溃
        */
        LOG("插件卸载完成1");
        client_->destroy();
        LOG("插件卸载完成2");
        client_.reset();
        LOG("插件卸载完成3");
        /*if(dll_exit_event_handle_){
            CloseHandle(dll_exit_event_handle_);
        }*/
        LOG("插件卸载完成");
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    //catch (...)
    {
        LOG("DoUnInit 异常");
    }
}

//RUNGATE_API DoUnInit() noexcept 
//{
//    __try {
//    //try {
//        LOG("插件卸载开始");
//
//        // 1. 停止JS线程池
//        if (g_js_thread_pool) {
//            g_js_thread_pool->stop(); // 停止接收新任务
//            g_js_thread_pool->join();  // 等待所有任务完成
//            g_js_thread_pool.reset();
//            g_js_semaphore.reset();
//            LOG("JS线程池已释放");
//        }
//
//        // 2. 恢复钩子
//        //WndProcHook::restore_hook();
//        //LightHook::HookMgr::instance().restore();
//
//        // 3. 强制关闭Socket和客户端
//        if (client_ && client_->is_started()) {
//            client_->socket().close();
//            client_->stop_all_timers();
//            asio::detail::win_thread::set_terminate_threads(true);
//            client_->stop();
//            client_->destroy();
//        }
//        client_.reset();
//
//        // 4. 清理Asio资源
//        if (g_game_io) {
//            g_game_io->stop();
//            g_game_io.reset();
//        }
//        g_thread_group.reset();
//
//        LOG("插件卸载完成");
//    }
//    __except (EXCEPTION_EXECUTE_HANDLER) {
//        LOG("DoUnInit 异常");
//    }
//    /*catch (...) {
//        LOG("DoUnInit 异常");
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
			//DoUnInit();不能在这里调用DoUnInit-->>不能在Windows Dll的DLL_PROCESS_DETACH块中调用asio2的server或client对象的stop函数，会导致stop函数永远阻塞无法返回。
			//原因是由于在DLL_PROCESS_DETACH时，通过PostQueuedCompletionStatus投递的IOCP事件永远得不到执行。
			LOG("DLL_PROCESS_DETACH"); break;
	}
	return TRUE;
}