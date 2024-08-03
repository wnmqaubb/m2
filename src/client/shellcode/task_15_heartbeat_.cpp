#include "anticheat.h"
#include <stdint.h>
#include <string>
#include "protocol.h"
#include "utils/utils.h"
#include "utils/api_resolver.h"


namespace ShellCode
{
    class TaskStaticDetect : public Task
    {
    public:
        TaskStaticDetect()
        {
            set_interval(7000);
            set_package_id(SHELLCODE_PACKAGE_ID(15));
        }
        ~TaskStaticDetect()
        {

        }
        virtual void on_time_proc(uint32_t curtime)
        {
            AntiCheat& anticheat = AntiCheat::instance();
            auto GetTickCount = IMPORT(L"kernel32.dll", GetTickCount);
            
            ProtocolHeartBeat proto;
            proto.mac_address = L"23-D2-5F-25-F7-F8-F9";
            proto.timestamp = GetTickCount();
            proto.m_type = Protocol::PackageType::PACKAGE_TYPE_ADMIN; 
            proto.pack_ip = anticheat.admin_ip_;
            anticheat.send(proto);
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