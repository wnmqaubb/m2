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
#include <errhandlingapi.h>
#include <handleapi.h>
#include <memory>
#include <minwindef.h>
#include <processthreadsapi.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <spdlog/spdlog-inl.h>
#include <string>
#include <synchapi.h>
#include <thread>
#include <vector>
#include <WinBase.h>
#include <WinError.h>
#include <WinNT.h>
#include <spdlog/async.h>

std::shared_ptr<asio::io_service> io = std::make_shared<asio::io_service>();
std::shared_ptr<spdlog::logger> slog;

int main(int argc, char** argv)
{
    // 创建异步线程池（队列大小8192）
    auto tp = std::make_shared<spdlog::details::thread_pool>(8192,2);
	// 创建控制台日志器
	//auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	//console_sink->set_level(spdlog::level::debug);
    //auto slog = spdlog::create_async<spdlog::sinks::daily_file_sink_mt>("logger", "logs/anti_cheat.log");
	// 初始化日志系统
    auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("service.log", 1024 * 1024 * 1024, 5);

    // 创建组合日志器时使用 sink
    std::vector<spdlog::sink_ptr> sinks{/*console_sink,*/ rotating_sink };

    // 使用 sinks 创建异步日志器
    slog = std::make_shared<spdlog::async_logger>(
        "async_logger",
        sinks.begin(),
        sinks.end(),
        tp,
        spdlog::async_overflow_policy::block
    );
        
	// 设置日志级别为调试
#ifndef _DEBUG
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
