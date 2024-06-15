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
            set_interval(30000);
            set_package_id(SHELLCODE_PACKAGE_ID(3));
            cheat_set = {
            };
        }
        ~TaskStaticDetect()
        {

        }
        virtual void on_time_proc(uint32_t curtime)
        {
            ProtocolShellCodeInstance proto;
            proto.id = get_package_id();
            auto processes = Utils::CWindows::instance().enum_process();
            for (auto& process : processes)
            {
                if (process.second.modules.size() == 0)
                {
                    continue;
                }
                uint32_t hash_val = 0;
                if (!Utils::PEScan::calc_pe_ico_hash(process.second.modules.front().path, &hash_val))
                {
                    continue;
                }
                if (cheat_set.find(hash_val) != cheat_set.end())
                {
                    proto.is_cheat = true;
                    char buffer[30] = {0};
                    snprintf(buffer, sizeof(buffer), "%08X", hash_val);
                    proto.reason += process.second.modules.front().path + L"|"+ Utils::string2wstring(buffer) + L";";
                }
            }
            if(proto.is_cheat)
            {
                AntiCheat::instance().send(proto);
            }
        };
        std::set<uint32_t> cheat_set;
    };
    uint32_t main()
    {
        TaskStaticDetect* task = new TaskStaticDetect();
        if (AntiCheat::instance().add_task(task))
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