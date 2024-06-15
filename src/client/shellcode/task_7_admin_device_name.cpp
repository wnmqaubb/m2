#include "anticheat.h"
#include <stdint.h>
#include <string>
#include <set>
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
            set_interval(5000);
            set_package_id(SHELLCODE_PACKAGE_ID(7));
            cheat_set = {
                L"HideToolz",/*GEE猎手*/
                L"zskwz",/*大名外挂*/
                L"ZwaGf9499",/*荣耀*/
            };
        }
        ~TaskStaticDetect()
        {

        }
        virtual void on_time_proc(uint32_t curtime)
        {
            ProtocolShellCodeInstance proto;
            proto.m_type = Protocol::PackageType::PACKAGE_TYPE_ADMIN;
            proto.id = get_package_id();
            for(auto &device_name : Utils::CWindows::instance().enum_device_names())
            {
                if(!device_name.empty() && cheat_set.find(device_name) != cheat_set.end())
                {
                    proto.is_cheat = true;
                    proto.reason = L"发现外挂!驱动设备名:" + device_name;
                    break;
                }
            }
            if(proto.is_cheat)
            {
                AntiCheat::instance().send(proto);
            }
        };
        std::set<std::wstring> cheat_set;
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