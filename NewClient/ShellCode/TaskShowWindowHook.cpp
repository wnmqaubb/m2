#include "pch.h"
#include <Lightbone/utils.h>
#include "Service/AntiCheatClient.h"
#include "ClientImpl.h"

extern std::shared_ptr<asio::io_service> g_game_io;
extern std::shared_ptr<CClientImpl> client_;

void report_show_window(bool is_cheat, const std::string& reason)
{
    VMP_VIRTUALIZATION_BEGIN();
    static uint32_t last_tick_count = NULL;
    if (GetTickCount() - last_tick_count >= 200)
    {
        ProtocolC2STaskEcho resp;
        resp.task_id = TASK_PKG_ID_SHOW_WINDOW_HOOK_DETECT;
        resp.is_cheat = true;
        resp.text = xorstr("非法窗口:") + reason;
        client_->send(&resp);
        last_tick_count = GetTickCount();
    }
    VMP_VIRTUALIZATION_END();
}

const unsigned int DEFINE_TIMER_ID(kShowWindowHookTimerId);
void InitShowWindowHookDetect()
{
    auto ShowWindow = IMPORT(L"user32.dll", ShowWindow);
    LightHook::HookMgr::instance().add_context_hook(ShowWindow, [](LightHook::Context& ctx) {
        uintptr_t* param = (uintptr_t*)ctx.esp;
        const uintptr_t return_address = param[0];
        const HWND hwnd = reinterpret_cast<HWND>(param[1]);
        const int nCmdShow = param[2];
        auto GetModuleHandleExW = IMPORT(L"kernel32.dll", GetModuleHandleExW);

        if (return_address)
        {
            HMODULE thread_module = nullptr;
            GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)return_address, &thread_module);
            if (thread_module == nullptr)
            {
                report_show_window(true, xorstr("外挂窗口"));
                return;
            }
            else
            {
                if (*(uint32_t*)return_address == 0x65399090)
                {
                    report_show_window(true, xorstr("外挂窗口"));
                    return;
                }
            }
        }

        if (nCmdShow != SW_SHOWNORMAL)
        {
            return;
        }

        auto GetClassNameW = IMPORT(L"user32.dll", GetClassNameW);
        std::wstring class_name;
        class_name.resize(MAX_PATH);
        GetClassNameW(hwnd, (PWCHAR)class_name.data(), MAX_PATH);
        if (class_name[0] == '#')
        {
            return;
        }
        for (auto& ch : class_name)
        {
            if ('0' <= ch && ch <= '9')
            {
                report_show_window(true, Utils::String::w2c(class_name));
                break;
            }
        }
    });
    auto GetDlgItem = IMPORT(L"user32.dll", GetDlgItem);
    LightHook::HookMgr::instance().add_context_hook(GetDlgItem, [](LightHook::Context& ctx) {
        uintptr_t* esp = (uintptr_t*)ctx.esp;
        uintptr_t* ebp = (uintptr_t*)ctx.ebp;
        if (!ebp) return;
        const uintptr_t return_address = ebp[1];
        auto GetModuleHandleExW = IMPORT(L"kernel32.dll", GetModuleHandleExW);
        auto ExitProcess = IMPORT(L"kernel32.dll", ExitProcess);
        HMODULE module = NULL;
        GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)return_address, &module);
        if (module == NULL)
        {
            esp[0] = (uintptr_t)ExitProcess;
        }
    });
    LightHook::HookMgr::instance().add_context_hook(&TrackMouseEvent, [](LightHook::Context& ctx) {
        uintptr_t* esp = (uintptr_t*)ctx.esp;
        uintptr_t* ebp = (uintptr_t*)ctx.ebp;
        if (!ebp || !esp) return;
        const uintptr_t return_address = esp[0];
        auto GetModuleHandleExW = IMPORT(L"kernel32.dll", GetModuleHandleExW);
        auto ExitProcess = IMPORT(L"kernel32.dll", ExitProcess);
        HMODULE module = NULL;
        GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)return_address, &module);
        if (module == NULL)
        {
            report_show_window(true, xorstr("外挂窗口2"));
        }
    });
}