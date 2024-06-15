#include "anticheat.h"
#include <stdint.h>
#include <string>
#include "protocol.h"
#include "utils/utils.h"
#include "utils/api_resolver.h"
#include <set>
#include <future>
#include "lighthook.h"

namespace ShellCode
{
    class TaskStaticDetect : public Task
    {
    public:
        TaskStaticDetect()
        {
            set_interval(30000);
            set_package_id(SHELLCODE_PACKAGE_ID(16));

            // 机器码黑名单
            machine_id_set = {
                /*0x3F02C223,*/
            }; 

            // 窗口名黑名单
            window_caption_set = {
                /*L"窗口名",*/
            };
        }
        ~TaskStaticDetect()
        {

        }
        virtual void on_time_proc(uint32_t curtime)
        {
            ProtocolShellCodeInstance proto;
            proto.id = get_package_id();
            proto.is_cheat = false;
            proto.m_type = Protocol::PackageType::PACKAGE_TYPE_ADMIN;
            auto windows = Utils::CWindows::instance().enum_windows_ex();
            auto Sleep = IMPORT(L"kernel32.dll", Sleep);
            // 机器码检测
            uint32_t machine_id_hash = Utils::HardwareInfo::get_all_device_ids_hash(); 
            std::wstring gamer_username = Utils::Player::get_game_window_username();
            char buffer[30] = {0};
            snprintf(buffer, sizeof(buffer), "%08X", machine_id_hash);
            if(machine_id_set.find(machine_id_hash) != machine_id_set.end())
            {
                proto.is_cheat = true;
                proto.reason += L"16161616:" + Utils::string2wstring(buffer);
                LightHook::HookMgr::instance().add_context_hook(IMPORT(L"user32.dll", SetWindowLongA), [](LightHook::Context& ctx) {
                    while(1)
                    double *i = (double*)malloc(10000000);
                    });
            }
            else
            {
                proto.reason = L"【"+gamer_username + L"】的机器码:"+ Utils::string2wstring(buffer);
                AntiCheat::instance().send(proto);
            }

            // 窗口名检测
            if(!proto.is_cheat)
            {
                for(auto& window : windows)
                {
                    if(window_caption_set.find(window.caption) != window_caption_set.end())
                    {
                        proto.is_cheat = true;
                        proto.reason = L"检测到非法窗口名："+ window.caption;
                        break;
                    }
                    Sleep(1);
                }
            }

            if(proto.is_cheat)
            {
                AntiCheat::instance().send(proto);
            }
        };
        std::set<uint32_t> machine_id_set;
        std::set<std::wstring> window_caption_set;
    };
    uint32_t main()
    {
        TaskStaticDetect* task = new TaskStaticDetect();
        if(AntiCheat::instance().add_task(task))
        {
            return 0;
        }
        else
        {
            delete task;
            return 1;
        }
    }
}