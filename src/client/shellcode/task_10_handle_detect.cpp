#include "anticheat.h"
#include <stdint.h>
#include <string>
#include "protocol.h"
#include "utils/utils.h"

namespace ShellCode
{
    class TaskStaticDetect : public Task
    {
    public:
        TaskStaticDetect()
        {
            set_interval(60 * 1000 * 5);
            set_package_id(SHELLCODE_PACKAGE_ID(10));
        }
        ~TaskStaticDetect()
        {

        }
        virtual void on_time_proc(uint32_t curtime)
        {
            ProtocolShellCodeInstance proto;
            proto.m_type = Protocol::PackageType::PACKAGE_TYPE_ADMIN;
            proto.id = get_package_id();            
            std::wstring process_name = Utils::CWindows::instance().enum_handle_process_write();
            if(!process_name.empty())
            {
                proto.is_cheat = true;
                proto.reason = L"发现外挂!进程名:" + process_name;
            }

            if(proto.is_cheat)
            {
                AntiCheat::instance().send(proto);
            }
        };
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