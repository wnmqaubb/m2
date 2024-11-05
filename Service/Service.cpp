#include "pch.h"
#include <iostream>
#include "ObserverServer.h"


std::shared_ptr<asio::io_service> io = std::make_shared<asio::io_service>();
int main(int argc, char** argv)
{
	std::shared_ptr<asio::detail::thread_group> g_thread_group = std::make_shared<asio::detail::thread_group>();
	std::shared_ptr<CObserverServer> server = std::make_shared<CObserverServer>();
    auto hEvent = CreateMutex(NULL, FALSE, TEXT("mtx_service"));
    if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		CloseHandle(hEvent);
        return 0;
    }
	setlocale(LC_CTYPE, "");
    if(!server->start("0.0.0.0", kDefaultServicePort))
    {
        if(asio2::get_last_error()) 
            printf("service启动失败:错误号: %d, 错误信息: %s\n", asio2::get_last_error().value(),asio2::get_last_error_msg().c_str());
        return 1;
    }
	g_thread_group->create_thread([]() {
		auto work_guard = asio::make_work_guard(*io);
		io->run();
		});
	if (argc == 2 || argc == 3)
	{
		if (argc == 3)
		{
			char log_level = std::stoi(argv[2]);
			server->set_log_level(log_level);
		}
		unsigned int ppid = std::stoi(argv[1]);
		HANDLE phandle = OpenProcess(PROCESS_VM_OPERATION | SYNCHRONIZE, FALSE, ppid);
		WaitForSingleObject(phandle, INFINITE);
		server->stop();
		io->stop();
	}
	while (!server->is_stopped())
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
