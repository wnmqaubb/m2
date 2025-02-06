#include "pch.h"
#include "NetUtils.h"
#include "ObserverServer.h"
#include <asio/detail/thread_group.hpp>
#include <asio/executor_work_guard.hpp>
#include <asio/io_service.hpp>
#include <asio2/base/error.hpp>
#include <chrono>
#include <clocale>
#include <cstdio>
#include <memory>
#include <string>
#include <thread>
#include <WinBase.h>
#include <WinDef.h>
#include <WinError.h>
#include <WinNT.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <client/windows/crash_generation/client_info.h>
#include <client/windows/crash_generation/crash_generation_server.h>
#include <client/windows/handler/exception_handler.h>
#include <client/windows/common/ipc_protocol.h>
using namespace google_breakpad;

bool ShowDumpResults(const wchar_t* dump_path,
                     const wchar_t* minidump_id,
                     void* context,
                     EXCEPTION_POINTERS* exinfo,
                     MDRawAssertionInfo* assertion,
                     bool succeeded)
{
    ::MessageBox(nullptr, L"程序可能被劫持，若频繁出现，请使用360急救箱或重装系统", L"提示", MB_OK | MB_ICONWARNING);
    return succeeded;
}

void InitMiniDump()
{
    static auto handler = new ExceptionHandler(L".\\",
                                               NULL,
                                               ShowDumpResults,
                                               NULL,
                                               ExceptionHandler::HANDLER_ALL,
                                               MiniDumpValidTypeFlags,
                                               (HANDLE)NULL,
                                               NULL);
}
std::shared_ptr<asio::io_service> io = std::make_shared<asio::io_service>();
std::shared_ptr<spdlog::logger> slog;

int main(int argc, char** argv)
{
    InitMiniDump();
	// 创建控制台日志器
	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	//console_sink->set_level(spdlog::level::debug);

	// 初始化日志系统
	auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("service_log.log", 1024 * 1024 * 1024, 5);

	// 创建组合日志器
	std::vector<spdlog::sink_ptr> sinks{/*console_sink,*/ rotating_sink};
    slog = std::make_shared<spdlog::logger>("Service_logger", sinks.begin(), sinks.end());
        
	// 设置日志级别为调试
#ifdef _DEBUG
	spdlog::set_level(spdlog::level::trace);
#else
	spdlog::set_level(spdlog::level::warn);
#endif

	// 注册为全局日志器
	spdlog::register_logger(slog);
	spdlog::set_default_logger(slog);

	// 设置日志消息的格式模式
	spdlog::set_pattern("%^[%m-%d %H:%M:%S][%l]%$ %v");
	slog->flush_on(spdlog::level::warn);
	spdlog::flush_every(std::chrono::seconds(1));
	slog->info("Logger initialized successfully");

	std::shared_ptr<asio::detail::thread_group> g_thread_group = std::make_shared<asio::detail::thread_group>();
	//std::shared_ptr<CObserverServer> server = std::make_shared<CObserverServer>();
	auto hEvent = CreateMutex(NULL, FALSE, TEXT("mtx_service"));
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		CloseHandle(hEvent);
		return 0;
	}
	setlocale(LC_CTYPE, "");
    CObserverServer::instance().start("0.0.0.0", kDefaultServicePort);
	//if (!server->start("0.0.0.0", kDefaultServicePort))
	//{
	//	if (asio2::get_last_error())
	//		printf("service启动失败:错误号: %d, 错误信息: %s\n", asio2::get_last_error().value(), asio2::get_last_error_msg().c_str());
	//	//return 1;
	//}
	g_thread_group->create_thread([]() {
		auto work_guard = asio::make_work_guard(*io);
		io->run();
		});
	if (argc == 2 || argc == 3)
	{
		if (argc == 3)
		{
			char log_level = std::stoi(argv[2]);
            CObserverServer::instance().set_log_level(log_level);
		}
		unsigned int ppid = std::stoi(argv[1]);
		HANDLE phandle = OpenProcess(PROCESS_VM_OPERATION | SYNCHRONIZE, FALSE, ppid);
		WaitForSingleObject(phandle, INFINITE);
        CObserverServer::instance().stop();
		io->stop();
	}
	while (!CObserverServer::instance().is_stopped())
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	if (asio2::get_last_error())
		printf("service 已停止 错误号: %d, 错误信息: %s\n", asio2::get_last_error().value(), asio2::get_last_error_msg().c_str());

	if (hEvent)
	{
		ReleaseMutex(hEvent);
		CloseHandle(hEvent);
	}
	return 0;
}
