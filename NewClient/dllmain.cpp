// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "ClientImpl.h"
#include "Tools/Packer/loader.h"
#include "CreateProcessHook.h"
#include "version.build"

share_data_ptr_t share_data = nullptr;

__declspec(dllexport) std::shared_ptr<HINSTANCE> dll_base;
__declspec(dllexport) std::shared_ptr<asio::io_service> g_game_io;
__declspec(dllexport) std::shared_ptr<asio::detail::thread_group> g_thread_group;
__declspec(dllexport) std::shared_ptr<int> g_client_rev_version;
__declspec(dllexport) std::shared_ptr<CClientImpl> client;

void __declspec(dllexport) reference_to_api()
{
    std::set<void*> ref;
    ref.emplace(get_ptr(&Utils::CWindows::instance));
    ref.emplace(get_ptr(&Utils::get_screenshot));
    ref.emplace(get_ptr(&Utils::PEScan::calc_pe_ico_hash));
    ref.emplace(get_ptr(&Utils::ImageProtect::instance));
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
#if LOG_SHOW
		char path[MAX_PATH];
		sprintf_s(path, MAX_PATH, "dll_base %08X", *dll_base);
		OutputDebugStringA(path);
#endif
        share_data = param;
		setlocale(LC_CTYPE, "");
		g_client_rev_version = std::make_shared<int>(REV_VERSION);
		g_game_io = std::make_shared<asio::io_service>();
		g_thread_group = std::make_shared<asio::detail::thread_group>();
        client = std::make_shared<CClientImpl>(std::move(ProtocolCFGLoader::load((char*)param->cfg, param->cfg_size)));
        if (client->cfg()->get_field<bool>(test_mode_field_id))
        {
            ::MessageBoxA(NULL, "test mode", "test", MB_OK);
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
        dll_base = std::make_shared<HINSTANCE>(hModule);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

