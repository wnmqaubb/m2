<<<<<<< HEAD
﻿#include "pch.h"
#include <iostream>
#include "ObserverServer.h"

asio::detail::thread_group g_thread_group;
asio::io_service io;
CObserverServer server(io);

int main(int argc, char** argv)
{
    CreateMutex(NULL, FALSE, TEXT("mtx_service"));
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        return 0;
    }
    setlocale(LC_CTYPE, "");
    server.start("0.0.0.0", kDefaultServicePort);

    g_thread_group.create_thread([](){
        auto work_guard = asio::make_work_guard(io);
        io.run();
    });
    if (argc == 2 || argc == 3)
    {
        if (argc == 3)
        {
            char log_level = std::stoi(argv[2]);
            server.set_log_level(log_level);
        }
        unsigned int ppid = std::stoi(argv[1]);
        HANDLE phandle = OpenProcess(PROCESS_VM_OPERATION | SYNCHRONIZE, FALSE, ppid);
        WaitForSingleObject(phandle, INFINITE);
        server.stop();
        io.stop();
    }
    while (!server.is_stopped())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}
=======
﻿#include "pch.h"
#include <iostream>
#include "ObserverServer.h"

asio::detail::thread_group g_thread_group;
asio::io_service io;
CObserverServer server(io);

int main(int argc, char** argv)
{
    CreateMutex(NULL, FALSE, TEXT("mtx_service"));
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        return 0;
    }
    setlocale(LC_CTYPE, "");
    server.start("0.0.0.0", kDefaultServicePort);

    g_thread_group.create_thread([](){
        auto work_guard = asio::make_work_guard(io);
        io.run();
    });
    if (argc == 2 || argc == 3)
    {
        if (argc == 3)
        {
            char log_level = std::stoi(argv[2]);
            server.set_log_level(log_level);
        }
        unsigned int ppid = std::stoi(argv[1]);
        HANDLE phandle = OpenProcess(PROCESS_VM_OPERATION | SYNCHRONIZE, FALSE, ppid);
        WaitForSingleObject(phandle, INFINITE);
        server.stop();
        io.stop();
    }
    while (!server.is_stopped())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9
