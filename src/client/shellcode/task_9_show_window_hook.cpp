#include "anticheat.h"
#include <stdint.h>
#include <string>
#include <set>
#include "protocol.h"
#include "utils/utils.h"
#include "utils/api_resolver.h"
#include <lighthook.h>

namespace ShellCode
{

    class TaskStaticDetect : public Task
    {
    public:
        TaskStaticDetect()
        {
            set_interval(15000);
            set_package_id(SHELLCODE_PACKAGE_ID(9));
            cheat = false;
        }
        ~TaskStaticDetect()
        {

        }
        virtual void on_time_proc(uint32_t curtime)
        {
            if(cheat)
            {
                ProtocolShellCodeInstance proto;
                proto.id = get_package_id();
                proto.is_cheat = true;
                proto.reason = L"非法窗口:" + reason;
                AntiCheat::instance().send(proto);
            }
        };

        std::wstring reason;
        bool cheat;
    };
    uint32_t main()
    {
        TaskStaticDetect* task = new TaskStaticDetect();
        if(AntiCheat::instance().add_task(task))
        {
            auto ShowWindow = IMPORT(L"user32.dll", ShowWindow);
            LightHook::HookMgr::instance().add_context_hook(ShowWindow, [](LightHook::Context& ctx) {
                uintptr_t* param = (uintptr_t*)ctx.esp;
                const uintptr_t return_address = param[0];
                const HWND hwnd = reinterpret_cast<HWND>(param[1]);
                const int nCmdShow = param[2];
                auto GetModuleHandleExW = IMPORT(L"kernel32.dll", GetModuleHandleExW);

                if(return_address)
                {
                    HMODULE thread_module = nullptr;
                    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)return_address, &thread_module);
                    if(thread_module == nullptr)
                    {
                        auto task = reinterpret_cast<ShellCode::TaskStaticDetect*>(AntiCheat::instance().task_map_[SHELLCODE_PACKAGE_ID(9)]);
                        task->cheat = true;
                        task->reason = L"外挂窗口";
                        return;
                    }
                }

                if(nCmdShow != SW_SHOWNORMAL)
                {
                    return;
                }

                auto GetClassNameW = IMPORT(L"user32.dll", GetClassNameW);
                std::wstring class_name;
                class_name.resize(MAX_PATH);
                GetClassNameW(hwnd, (PWCHAR)class_name.data(), MAX_PATH);
                if(class_name[0] == '#')
                {
                    return;
                }
                for(auto& ch : class_name)
                {
                    if('0' <= ch && ch <= '9')
                    {
                        auto task = reinterpret_cast<ShellCode::TaskStaticDetect*>(AntiCheat::instance().task_map_[SHELLCODE_PACKAGE_ID(9)]);
                        task->cheat = true;
                        task->reason = class_name;
                        break;
                    }
                }
            });

            return 0;
        }
        else
        {
            delete task;
            return 1;
        }
    }
}