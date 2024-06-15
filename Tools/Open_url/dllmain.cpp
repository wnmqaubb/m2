// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <ShellAPI.h>
#include "utils.h"
#include "xorstr.hpp"
bool is_open = false;

void open_url()
{
    std::string url = xorstr("http://www.6pkmir.com");
    if (!is_open)
    {
        if (GetModuleHandleA("Shell32.dll") == NULL)
            LoadLibraryA("Shell32.dll");

        is_open = true;
        // 方式1
        auto ShellExecuteA = IMPORT(L"shell32.dll", ShellExecuteA);
        if (ShellExecuteA)
        {
            if ((DWORD)ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOW) > 32)
            {
                return;
            }
        }

        // 方式2
        auto WinExec = IMPORT(L"kernel32.dll", WinExec);
        if (WinExec)
        {
            if (WinExec(("C:\\windows\\explorer.exe " + url).c_str(), SW_SHOW) > 31)
            {
                return;
            }
        }

        // 方式3
        auto CreateProcessA = IMPORT(L"kernel32.dll", CreateProcessA);
        if (CreateProcessA)
        {
            PROCESS_INFORMATION pi;
            STARTUPINFOA si;
            memset(&si, 0, sizeof(si));
            si.cb = sizeof(si);
            si.wShowWindow = SW_SHOW;
            si.dwFlags = STARTF_USESHOWWINDOW;
            if (CreateProcessA(NULL, (char*)("C:\\windows\\explorer.exe " + url).c_str(), NULL, FALSE, NULL, NULL, NULL, NULL, &si, &pi))
            {
                return;
            }
        }
    }

};

BOOL APIENTRY DllMain(HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        open_url();
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

