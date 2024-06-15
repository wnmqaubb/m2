#include "pch.h"
#include "task.h"
#include "utils\utils.h"
#include "utils\api_resolver.h"
#ifdef CLIENT
#include <lighthook.h>
#endif
TaskApiHook::TaskApiHook()
{
	set_interval(0);
	set_package_id(Protocol::PackageId::PACKAGE_ID_API_HOOK);
#ifdef CLIENT
    //show_window_hook();
#endif
}

TaskApiHook::~TaskApiHook()
{

}

void TaskApiHook::show_window_hook()
{
#ifdef CLIENT
    /*auto ShowWindow = IMPORT(L"user32.dll", ShowWindow);
    LightHook::HookMgr::instance().add_context_hook(ShowWindow, [](LightHook::Context& ctx) {
        uintptr_t* param = (uintptr_t*)ctx.esp;
        const uintptr_t return_address = param[0];
        const HWND hwnd = reinterpret_cast<HWND>(param[1]);
        const int nCmdShow = param[2];
        if(nCmdShow != SW_SHOWNORMAL)
        {
            return;
        }

        auto GetModuleHandleExW = IMPORT(L"kernel32.dll", GetModuleHandleExW);
        auto GetModuleHandleA = IMPORT(L"kernel32.dll", GetModuleHandleA);
        auto GetClassNameW = IMPORT(L"user32.dll", GetClassNameW);
        auto GetWindowTextW = IMPORT(L"user32.dll", GetWindowTextW);
        auto FindWindowExA = IMPORT(L"user32.dll", FindWindowExA);

        std::wstring caption;
        caption.resize(MAX_PATH);
        std::wstring class_name;
        class_name.resize(MAX_PATH);
        GetWindowTextW(hwnd, caption.data(), MAX_PATH);
        GetClassNameW(hwnd, class_name.data(), MAX_PATH);
        ProtocolApiHook proto;
        proto.api_hash = CT_HASH("ShowWindow");
        proto.params.push_back(caption);
        proto.params.push_back(class_name);
        AntiCheat::instance().send(proto);
    });*/
#endif
}

void TaskApiHook::on_recv_package(void* sender, uintptr_t conn_id, json& package, Protocol::PackageType channel)
{
#ifdef SERVER
    /*ProtocolApiHook proto;
    proto.from_json(package);
    Utils::log_to_file(conn_id, Utils::CHANNEL_EVENT, GlobalString::LOG::FORMAT_S.c_str(), proto.dump().c_str());
    if(proto.api_hash == CT_HASH("ShowWindow"))
    {
        if(proto.params.size() == 2)
        {
            if(proto.params[1][0] == '#')
            {
                return;
            }
            for(auto& ch : proto.params[1])
            {
                if('0' <= ch && ch <= '9')
                {
                    Policy policy;
                    policy.policy_id = 3001;
                    policy.policy_type = ENM_POLICY_TYPE_MAX;
                    policy.punish_type = ENM_PUNISH_TYPE_KICK;
                    Singleton<TaskPolicy>::getInstance().punish_player(conn_id, policy, L"·Ç·¨´°¿Ú:" + proto.params[1]);
                    return;
                }
            }
        }
    }*/
#endif
}
