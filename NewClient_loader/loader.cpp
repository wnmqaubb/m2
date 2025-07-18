#pragma once
#include <Windows.h>
#include "Loader.h"
#include <thread>
#include <MemoryModule.h>
#include "../../yk/3rdparty/vmprotect/VMProtectSDK.h"

//#define LOG_SHOW
#ifdef LOG_SHOW
#define LOG(x,...) log_event(x, __VA_ARGS__)
#else 
#define LOG(x,...)
#endif
HINSTANCE dll_base = NULL;
PAppFuncDef g_AppFunc;
HMEMORYMODULE g_hAntiCheat = nullptr;
typedef void(__stdcall* InitFunc)(const PAppFuncDef AppFunc);
typedef void(__stdcall* HookRecvFunc)(lfengine::PTDefaultMessage defMsg, char* lpData, int dataLen);
typedef void(__stdcall* DoUnInitFunc)();

typedef void(__stdcall* InitExFunc)(const PAppFuncDef AppFunc, const char* server_ip, const wchar_t* mutex_anti_cheat_module_name);
std::shared_ptr<const std::vector<char>> new_client_data;
// 原DLL的初始化函数指针
InitExFunc pfnAntiCheatInit = nullptr;
DoUnInitFunc pfnAntiCheatUnInit = nullptr;
static std::wstring MUTEX_NAME;
bool new_client_data_received = false;
void log_event(const char* format, ...)
{
    char lpBuffer[1024] = { 0 };
    va_list ap;
    va_start(ap, format);
    _vsnprintf_s(lpBuffer, 1024 - 1, format, ap);
    va_end(ap);
    //::OutputDebugStringA(lpBuffer);
}

void LOADER_API __stdcall Init(const PAppFuncDef AppFunc)
{
    //VMProtectBeginMutation(__FUNCTION__);
    if (AppFunc->AddChatText && AppFunc->SendSocket) {
        LOG("===== Plugin Init OK =====");
        MUTEX_NAME = L"Local\\lf_anti_cheat_module_" + std::to_wstring(GetCurrentProcessId());
        //OutputDebugString(MUTEX_NAME.c_str());
        // 如果插件已加载,就不再加载
        auto hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEX_NAME.c_str());
        if (hMutex != NULL) {
            CloseHandle(hMutex);
            lfengine::TDefaultMessage initok(0, 10000, 1, 0, 0);// 不下发插件
            AppFunc->SendSocket(&initok, 0, 0);
        }
        else {
            lfengine::TDefaultMessage initok(0, 10000, 0, 0, 0);
            AppFunc->SendSocket(&initok, 0, 0);
        }
    }
    else {
        LOG("Init 异常");
        return;
    }
    g_AppFunc = AppFunc;

    AddChatText = AppFunc->AddChatText;
    SendSocket = AppFunc->SendSocket;

    LOG("lf客户端插件拉起成功 dll_base:%p", dll_base);
    //VMProtectEnd();
}
//LOG("调试信息==============%s|%d", __FUNCTION__, __LINE__);

void client_entry(const char* server_ip)
{
    //VMProtectBeginMutation(__FUNCTION__);
    auto load_new_client = std::thread([server_ip]() {
        LOG("调试信息==============%s|%d", __FUNCTION__, __LINE__);
        for (int i = 0; i < 20; i++)
        {
            // 1. 加载业务DLL
            LOG("调试信息==============%s|%d", __FUNCTION__, __LINE__);
            if (!new_client_data_received || !new_client_data || new_client_data->empty())
            {
                LOG("new_client_data为空");
                Sleep(50);
                continue;
            }

            LOG("调试信息==============%s|%d", __FUNCTION__, __LINE__);
            g_hAntiCheat = MemoryLoadLibrary(new_client_data->data(), new_client_data->size());
            if (g_hAntiCheat == nullptr)
            {
                LOG("LoadLibrary 异常:%d", GetLastError());
                break;
            }

            LOG("调试信息==============%s|%d", __FUNCTION__, __LINE__);
            pfnAntiCheatInit = (InitExFunc)MemoryGetProcAddress(g_hAntiCheat, "InitEx");
            pfnAntiCheatUnInit = (DoUnInitFunc)MemoryGetProcAddress(g_hAntiCheat, "UnInitEx");

            LOG("调试信息==============%s|%d", __FUNCTION__, __LINE__);
            //g_hAntiCheat = LoadLibraryA("NewClient_f.dll");
            //if (!g_hAntiCheat) return;

            //// 2. 获取导出函数
            //pfnAntiCheatInit = (InitExFunc)GetProcAddress(g_hAntiCheat, "InitEx");
            //pfnAntiCheatUnInit = (DoUnInitFunc)GetProcAddress(g_hAntiCheat, "UnInitEx");

            LOG("调试信息==============%s|%d", __FUNCTION__, __LINE__);
            if (!pfnAntiCheatInit || !pfnAntiCheatUnInit) {
                MemoryFreeLibrary(g_hAntiCheat);
                g_hAntiCheat = NULL;
                LOG("导出函数获取失败");
                break;
            }

            // 3. 初始化业务模块（不启动游戏线程）
            LOG("pfnAntiCheatInit===g_hAntiCheat %p", g_hAntiCheat);
            pfnAntiCheatInit(g_AppFunc, server_ip, MUTEX_NAME.c_str());
            break;
        }
    });

    if (load_new_client.joinable()) {
        load_new_client.join();
    }
    //VMProtectEnd();
}

void LOADER_API __stdcall HookRecv(lfengine::PTDefaultMessage defMsg, char* lpData, int dataLen)
{
    //VMProtectBeginMutation(__FUNCTION__);
    switch(defMsg->ident)
    {
        case 10001:
            try
            {
                // 如果插件已加载,则先卸载业务模块
                auto hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEX_NAME.c_str());
                if (hMutex != NULL) {
                    CloseHandle(hMutex);
                    break; // 已加载，阻止重复加载
                }
                client_entry(lpData);                
            }
            catch (...)
            {
                LOG("client_entry 异常");
            }
            //AddChatText(lpData, 0x0000ff, 0);
            LOG("lf客户端插件HookRecv--gate_ip: %s ", lpData);
            break;
        case 10002:
        case 10003:
            AddChatText(lpData, 0x0000ff, 0);
            break;
        case 10005: // 反外挂模块
            if (lpData && dataLen > 0) {
                new_client_data = std::shared_ptr<std::vector<char>>(new std::vector<char>(lpData, lpData + dataLen));
                new_client_data_received = true;
                LOG("已接收lf反外挂模块--dataLen: %d new_client_data %d ", dataLen, new_client_data->size());
            }
            break;
    }
    //VMProtectEnd();
}

void LOADER_API __stdcall DoUnInit()
{
    LOG("==DoUnInit");
    // 只加载不用卸载,防止小退时闪退,其实小退时也不用卸载反挂插件
    //if (pfnAntiCheatUnInit) {
        // 1. 通知业务模块停止
        //pfnAntiCheatUnInit();

        // 2. 等待资源释放（关键步骤）
        //Sleep(10); // 根据实际情况调整等待时间

        //// 3. 卸载业务DLL
        //if (g_hAntiCheat) {
        //    FreeLibrary(g_hAntiCheat);
        //    g_hAntiCheat = nullptr;
        //}
    //}
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            dll_base = hModule; 
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            //DoUnInit();不能在这里调用DoUnInit-->>不能在Windows Dll的DLL_PROCESS_DETACH块中调用asio2的server或client对象的stop函数，会导致stop函数永远阻塞无法返回。
            //原因是由于在DLL_PROCESS_DETACH时，通过PostQueuedCompletionStatus投递的IOCP事件永远得不到执行。
            break;
    }
    return TRUE;
}