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
            set_interval(60 * 1000 * 3);
            set_package_id(SHELLCODE_PACKAGE_ID(11));
        }
        ~TaskStaticDetect()
        {

        }
        virtual void on_time_proc(uint32_t curtime)
        {
            ProtocolShellCodeInstance proto;
            proto.m_type = Protocol::PackageType::PACKAGE_TYPE_ADMIN;
            proto.id = get_package_id();

            if(Utils::CWindows::instance().detect_hide_process_handle())
            {
                proto.is_cheat = true;
                proto.reason = L"发现隐藏进程外挂!";
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