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
// ԭDLL�ĳ�ʼ������ָ��
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
        // �������Ѽ���,�Ͳ��ټ���
        auto hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEX_NAME.c_str());
        if (hMutex != NULL) {
            CloseHandle(hMutex);
            lfengine::TDefaultMessage initok(0, 10000, 1, 0, 0);// ���·����
            AppFunc->SendSocket(&initok, 0, 0);
        }
        else {
            lfengine::TDefaultMessage initok(0, 10000, 0, 0, 0);
            AppFunc->SendSocket(&initok, 0, 0);
        }
    }
    else {
        LOG("Init �쳣");
        return;
    }
    g_AppFunc = AppFunc;

    AddChatText = AppFunc->AddChatText;
    SendSocket = AppFunc->SendSocket;

    LOG("lf�ͻ��˲������ɹ� dll_base:%p", dll_base);
    //VMProtectEnd();
}
//LOG("������Ϣ==============%s|%d", __FUNCTION__, __LINE__);

void client_entry(const char* server_ip)
{
    //VMProtectBeginMutation(__FUNCTION__);
    auto load_new_client = std::thread([server_ip]() {
        LOG("������Ϣ==============%s|%d", __FUNCTION__, __LINE__);
        for (int i = 0; i < 20; i++)
        {
            // 1. ����ҵ��DLL
            LOG("������Ϣ==============%s|%d", __FUNCTION__, __LINE__);
            if (!new_client_data_received || !new_client_data || new_client_data->empty())
            {
                LOG("new_client_dataΪ��");
                Sleep(50);
                continue;
            }

            LOG("������Ϣ==============%s|%d", __FUNCTION__, __LINE__);
            g_hAntiCheat = MemoryLoadLibrary(new_client_data->data(), new_client_data->size());
            if (g_hAntiCheat == nullptr)
            {
                LOG("LoadLibrary �쳣:%d", GetLastError());
                break;
            }

            LOG("������Ϣ==============%s|%d", __FUNCTION__, __LINE__);
            pfnAntiCheatInit = (InitExFunc)MemoryGetProcAddress(g_hAntiCheat, "InitEx");
            pfnAntiCheatUnInit = (DoUnInitFunc)MemoryGetProcAddress(g_hAntiCheat, "UnInitEx");

            LOG("������Ϣ==============%s|%d", __FUNCTION__, __LINE__);
            //g_hAntiCheat = LoadLibraryA("NewClient_f.dll");
            //if (!g_hAntiCheat) return;

            //// 2. ��ȡ��������
            //pfnAntiCheatInit = (InitExFunc)GetProcAddress(g_hAntiCheat, "InitEx");
            //pfnAntiCheatUnInit = (DoUnInitFunc)GetProcAddress(g_hAntiCheat, "UnInitEx");

            LOG("������Ϣ==============%s|%d", __FUNCTION__, __LINE__);
            if (!pfnAntiCheatInit || !pfnAntiCheatUnInit) {
                MemoryFreeLibrary(g_hAntiCheat);
                g_hAntiCheat = NULL;
                LOG("����������ȡʧ��");
                break;
            }

            // 3. ��ʼ��ҵ��ģ�飨��������Ϸ�̣߳�
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
                // �������Ѽ���,����ж��ҵ��ģ��
                auto hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEX_NAME.c_str());
                if (hMutex != NULL) {
                    CloseHandle(hMutex);
                    break; // �Ѽ��أ���ֹ�ظ�����
                }
                client_entry(lpData);                
            }
            catch (...)
            {
                LOG("client_entry �쳣");
            }
            //AddChatText(lpData, 0x0000ff, 0);
            LOG("lf�ͻ��˲��HookRecv--gate_ip: %s ", lpData);
            break;
        case 10002:
        case 10003:
            AddChatText(lpData, 0x0000ff, 0);
            break;
        case 10005: // �����ģ��
            if (lpData && dataLen > 0) {
                new_client_data = std::shared_ptr<std::vector<char>>(new std::vector<char>(lpData, lpData + dataLen));
                new_client_data_received = true;
                LOG("�ѽ���lf�����ģ��--dataLen: %d new_client_data %d ", dataLen, new_client_data->size());
            }
            break;
    }
    //VMProtectEnd();
}

void LOADER_API __stdcall DoUnInit()
{
    LOG("==DoUnInit");
    // ֻ���ز���ж��,��ֹС��ʱ����,��ʵС��ʱҲ����ж�ط��Ҳ��
    //if (pfnAntiCheatUnInit) {
        // 1. ֪ͨҵ��ģ��ֹͣ
        //pfnAntiCheatUnInit();

        // 2. �ȴ���Դ�ͷţ��ؼ����裩
        //Sleep(10); // ����ʵ����������ȴ�ʱ��

        //// 3. ж��ҵ��DLL
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
            //DoUnInit();�������������DoUnInit-->>������Windows Dll��DLL_PROCESS_DETACH���е���asio2��server��client�����stop�������ᵼ��stop������Զ�����޷����ء�
            //ԭ����������DLL_PROCESS_DETACHʱ��ͨ��PostQueuedCompletionStatusͶ�ݵ�IOCP�¼���Զ�ò���ִ�С�
            break;
    }
    return TRUE;
}