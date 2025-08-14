#include "pch.h"
#include "../lf_rungate_server_plug/lf_plug_sdk.h"
#include "ClientImpl.h"
#include "version.build"
#include <asio/detail/thread_group.hpp>
#include <asio/executor_work_guard.hpp>
#include <asio/io_service.hpp>
#include <asio2/base/timer.hpp>
#include <clocale>
#include <exception>
#include <../../yk/Lightbone/utils.h>
#include <memory>
#include <Service/NetUtils.h>
#include <Service/SubServicePackage.h>
#include <string>
#include <thread_manager.h>
#include <WinDef.h>
#include <Windows.h>
#include <WinError.h>

using namespace lfengine::client;

std::shared_ptr<TAppFuncDef> g_AppFunc;
HINSTANCE dll_base;
std::shared_ptr<asio::io_service> g_game_io;
std::shared_ptr<asio::detail::thread_group> g_thread_group;
std::shared_ptr<int> g_client_rev_version;
std::shared_ptr<CClientImpl> client_;
std::shared_ptr<asio2::timer> g_timer;
static std::wstring MUTEX_NAME;
HANDLE hMutex;

AntiCheatSystem::AntiCheatSystem() :
    m_running(false) {}

AntiCheatSystem::~AntiCheatSystem() {
}

void AntiCheatSystem::Init(const PAppFuncDef AppFunc, const char* server_ip) {
    m_server_ip = server_ip ? server_ip : "";
    g_client_rev_version = std::make_shared<int>(REV_VERSION);
    g_AppFunc = std::make_shared<TAppFuncDef>();
    g_game_io = std::make_shared<asio::io_service>();
    g_thread_group = std::make_shared<asio::detail::thread_group>();
    g_timer = std::make_shared<asio2::timer>();
    client_entry();
    m_running = true;
    return;
}

void AntiCheatSystem::UnInit() {
    m_running = false;

    // ������Դ
    client_->stop();
    if (g_thread_group) {
        g_thread_group->join();
        g_thread_group.reset();
    }
    if (g_game_io) {
        g_game_io->stop();
        g_game_io.reset();
    }
}

void AntiCheatSystem::client_entry()
{
    // �� main �������ʼ��������ע�� SEH ������
    /*SetUnhandledExceptionFilter(GlobalExceptionFilter);
    _se_translator_function old_seh = _set_se_translator([](unsigned code, EXCEPTION_POINTERS*) {
        throw std::runtime_error("SEH Exception: code=" + std::to_string(code));
    });*/
#if 0
    g_client_rev_version = std::make_shared<int>(REV_VERSION);
    g_AppFunc = std::make_shared<TAppFuncDef>();
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
        cfg.set_field<std::string>(ip_field_id, m_server_ip.empty() ? "127.0.0.1" : m_server_ip);
        cfg.set_field<int>(port_field_id, 23268);
        client_ = std::make_shared<CClientImpl>();
        client_->cfg() = std::make_unique<ProtocolCFGLoader>(cfg);
        client_->cfg()->set_field<bool>(test_mode_field_id, false);
        client_->cfg()->set_field<bool>(sec_no_change_field_id, false);
        client_->cfg()->set_field<std::wstring>(usrname_field_id, L"δ��¼�û�");
    #ifdef _DEBUG
        client_->cfg()->set_field<bool>(test_mode_field_id, true);
        client_->cfg()->set_field<bool>(sec_no_change_field_id, false);
    #endif
        LOG("client_start_routine ");
        //WndProcHook::install_hook();
        {
            auto ip = client_->cfg()->get_field<std::string>(ip_field_id);
            auto port = client_->cfg()->get_field<int>(port_field_id);
            client_->start(ip, port);
            g_thread_group->create_threads([this]() {
                // ����DLL_THREAD_ATTACH/DETACH֪ͨ
                if(dll_base)
                    DisableThreadLibraryCalls(dll_base);
                auto work_guard = asio::make_work_guard(*g_game_io);
                g_game_io->run();
                LOG("ASIO�̰߳�ȫ�˳�"); // ȷ���߳��˳�ʱ��ģ�����
            }, 2);
        }
    }
    catch (std::exception& e)
    {
        LOG("client_start_routine �쳣: %s ", e.what());
    }
    VMP_VIRTUALIZATION_END();
}

// ȫ�ֵ���
AntiCheatSystem& GetSystem() {
    static AntiCheatSystem instance;
    return instance;
}

__declspec(dllexport) void __stdcall InitEx(const PAppFuncDef AppFunc, const char* server_ip, const wchar_t* mutex_anti_cheat_module_name)
{
    //OutputDebugStringW(mutex_anti_cheat_module_name);
    MUTEX_NAME = mutex_anti_cheat_module_name;
    // ���Դ��Ѵ��ڵĻ�����       
    LOG("������Ϣ==============%s|%d", __FUNCTION__, __LINE__);
    hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEX_NAME.c_str());
    LOG("������Ϣ==============%s|%d", __FUNCTION__, __LINE__);
    if (hMutex != NULL) {
        CloseHandle(hMutex);
        return; // �Ѽ��أ���ֹ�ظ�����
    }

    LOG("������Ϣ==============%s|%d", __FUNCTION__, __LINE__);
    // �����»�����
    hMutex = CreateMutex(NULL, TRUE, MUTEX_NAME.c_str());
    if (hMutex == NULL || GetLastError() == ERROR_ALREADY_EXISTS) {
        LOG("CreateMutex ʧ�ܻ����������Ѵ���");
        return; // ����ʧ�ܻ����������Ѵ���
    }
    LOG("������Ϣ==============%s|%d", __FUNCTION__, __LINE__);
#ifdef LOG_SHOW
    if (dll_base) {
        LOG("DLL_PROCESS_ATTACH1 dll_base:%p", dll_base);

        auto now_time = std::time(nullptr);
        std::tm tm_;
        localtime_s(&tm_, &now_time);
        std::stringstream ss;
        ss << std::put_time(&tm_, "%m-%d %H:%M:%S");
        std::string time_str = ss.str();
        time_str += ": 0x";
        LOG("������Ϣ==============%s|%d", __FUNCTION__, __LINE__);
        std::stringstream ss_hex;
        ss_hex << std::hex << (uint32_t)dll_base;  // תΪʮ������
        time_str += ss_hex.str();
        // ��ȡģ���С
        // ��ȡDOSͷ
        LOG("������Ϣ==============%s|%d", __FUNCTION__, __LINE__);
        PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)dll_base;
        if (pDosHeader->e_magic == IMAGE_DOS_SIGNATURE) {
            // ��ȡNTͷ
            PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)((BYTE*)dll_base + pDosHeader->e_lfanew);
            if (pNtHeader->Signature == IMAGE_NT_SIGNATURE)
                time_str += "," + std::to_string(pNtHeader->OptionalHeader.SizeOfImage);
        }

        LOG("������Ϣ==============%s|%d", __FUNCTION__, __LINE__);
        std::filesystem::path file = std::filesystem::current_path() / "jsy.txt";
        std::ofstream output(file, std::ios::app);
        output << time_str.data() << std::endl;
        output.flush();
        LOG("������Ϣ==============%s|%d", __FUNCTION__, __LINE__);
    }
#endif
    LOG("������Ϣ==============%s|%d", __FUNCTION__, __LINE__);
    return GetSystem().Init(AppFunc, server_ip);
}

__declspec(dllexport) void __stdcall UnInitEx() {
    GetSystem().UnInit();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            //LOG("DLL_PROCESS_ATTACH1");
            dll_base = hModule;
            break;
        case DLL_THREAD_ATTACH:
            //LOG("DLL_THREAD_ATTACH"); 
            break;
        case DLL_THREAD_DETACH:
            //LOG("DLL_THREAD_DETACH"); 
            break;
        case DLL_PROCESS_DETACH:
            //DoUnInit();�������������DoUnInit-->>������Windows Dll��DLL_PROCESS_DETACH���е���asio2��server��client�����stop�������ᵼ��stop������Զ�����޷����ء�
            //ԭ����������DLL_PROCESS_DETACHʱ��ͨ��PostQueuedCompletionStatusͶ�ݵ�IOCP�¼���Զ�ò���ִ�С�
            //LOG("DLL_PROCESS_DETACH");
            hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEX_NAME.c_str());
            if (hMutex != NULL) {
                ReleaseMutex(hMutex);
                CloseHandle(hMutex);
            }
            break;
    }
    return TRUE;
}