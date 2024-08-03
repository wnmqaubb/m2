#include "anticheat.h"
#include <stdint.h>
#include <string>
#include "protocol.h"
#include "utils/utils.h"
#include "utils/windows_internal.h"
#include "utils/api_resolver.h"
#include <set>


namespace ShellCode
{
    class TaskStaticDetect : public Task
    {
    public:
        TaskStaticDetect()
        {
            set_interval(60 * 1000 * 5);
            set_package_id(SHELLCODE_PACKAGE_ID(14));
        }
        ~TaskStaticDetect()
        {

        }
        virtual void on_time_proc(uint32_t curtime)
        {
            ProtocolShellCodeInstance proto;
            proto.m_type = Protocol::PackageType::PACKAGE_TYPE_ADMIN;
            proto.id = get_package_id();
            proto.is_cheat = false;
            auto GetWindowLongW = IMPORT(L"User32.dll", GetWindowLongW);
            auto IsWindow = IMPORT(L"User32.dll", IsWindow);
            auto GetWindowThreadProcessId = IMPORT(L"User32.dll", GetWindowThreadProcessId); 
            if(!GetWindowLongW || !IsWindow || !GetWindowThreadProcessId)
                return;
            DWORD process_id = -1;
            std::set<DWORD> pid_set;
            Utils::CWindows::ProcessMap processes = Utils::CWindows::instance().enum_process();
            for(size_t i = 0; i < 0xFFFFFFF; i++)
            {
                if(!IsWindow((HWND)i) || GetWindowLongW((HWND)i, GWL_STYLE) == 0) continue;
                process_id = -1;
                GetWindowThreadProcessId((HWND)i, &process_id);
                if(process_id != (DWORD)-1)
                {
                    pid_set.insert(process_id);
                }
                Sleep(1);
            }

            for(DWORD process_id : pid_set)
            {
                if(processes.find(process_id) == processes.end())
                {
                    proto.is_cheat = true;
                    proto.reason = L"find hidden pid:" + process_id;
                    break;
                }
            }

            if(proto.is_cheat)
            {
                AntiCheat::instance().send(proto);
            }
        }

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