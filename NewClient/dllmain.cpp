// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <iostream>
#include "ClientImpl.h"
#include "Tools/Packer/loader.h"
#include "CreateProcessHook.h"
#include "WndProcHook.h"
#include "version.build"
#include <regex>

__declspec(dllexport) HINSTANCE dll_base = NULL;
share_data_ptr_t share_data = nullptr;

__declspec(dllexport) asio::io_service g_io;
__declspec(dllexport) asio::io_service g_game_io;
__declspec(dllexport) asio::detail::thread_group g_thread_group;
__declspec(dllexport) int g_client_rev_version = REV_VERSION;
__declspec(dllexport) NetUtils::CTimerMgr g_game_timer_mgr(g_game_io);

void __declspec(dllexport) reference_to_api()
{
    std::set<void*> ref;
    ref.emplace(get_ptr(&Utils::CWindows::instance));
    ref.emplace(get_ptr(&Utils::get_screenshot));
    ref.emplace(get_ptr(&Utils::PEScan::calc_pe_ico_hash));
}

void client_start_routine(std::shared_ptr<CClientImpl> client)
{
    WndProcHook::install_hook();
    std::string ip = client->cfg()->get_field<std::string>(ip_field_id);
    std::transform(ip.begin(), ip.end(), ip.begin(), ::tolower);
    if (ip[0] == '0' && ip[1] == 'x')
    {
        char* ip_ptr = NULL;
        sscanf_s(ip.c_str(), "0x%x", &ip_ptr);
        g_thread_group.create_thread([ip_ptr, client]() {
			std::regex ip_regex(R"((\d{1,3}\.){3}\d{1,3})");
			while (!(!IsBadReadPtr(ip_ptr, 1) && *ip_ptr != 0 && std::regex_match(std::string(ip_ptr), ip_regex)))
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}
            client->cfg()->set_field<std::string>(ip_field_id, ip_ptr);
            client->start(client->cfg()->get_field<std::string>(ip_field_id), client->cfg()->get_field<unsigned int>(port_field_id));
            auto work_guard = asio::make_work_guard(g_io);
            g_io.run();
        });
    }
    else
    {
        client->start(client->cfg()->get_field<std::string>(ip_field_id), client->cfg()->get_field<unsigned int>(port_field_id));
        g_thread_group.create_thread([]() {
            auto work_guard = asio::make_work_guard(g_io);
            g_io.run();
        });
    }
    
#if 0
    g_thread_group.create_thread([client](){
        static std::vector<std::shared_ptr<CClientImpl>> client_vec;
        for (int i = 0; i < 200; i++)
        {
            auto client_ = std::make_shared<CClientImpl>(g_io);
            client_->cfg_ = std::make_unique<ProtocolCFGLoader>();
            client_->cfg_->data = client->cfg_->data;
            client_->cfg_->json = client_->cfg_->json;/*
            std::this_thread::sleep_for(std::chrono::seconds(1));*/
            client_->start(client->cfg_->get_field<std::string>(ip_field_id), client->cfg_->get_field<unsigned int>(port_field_id));
            client_vec.push_back(std::move(client_));
        }
    });
    g_thread_group.create_thread([]() {
        auto work_guard = asio::make_work_guard(g_io);
        g_io.run();
    });
#endif
}

void __stdcall client_entry(share_data_ptr_t param) noexcept
{
    VMP_VIRTUALIZATION_BEGIN();
    if (!param)
        return;
    if (param->stage == 0)//打开登录器
    {
        share_data = param;
        HookProc::init_create_process_hook();
    }
    else if (param->stage == 1)//开始游戏
    {
        share_data = param;
        setlocale(LC_CTYPE, "");
        std::shared_ptr<CClientImpl> client(std::make_shared<CClientImpl>(g_io));
        client->cfg() = ProtocolCFGLoader::load((char*)param->cfg, param->cfg_size);
        if (client->cfg()->get_field<bool>(test_mode_field_id))
        {
            ::MessageBoxA(NULL, "test mode", "test", MB_OK);
        }

        if (client->cfg()->get_field<bool>(sec_no_change_field_id))
        {
            Utils::ImageProtect::instance().register_callback(std::bind(&client_start_routine, std::move(client)));
            Utils::ImageProtect::instance().install();
        }
        else
        {
            client_start_routine(std::move(client));
        }
    }
    VMP_VIRTUALIZATION_END();
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        dll_base = hModule;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

