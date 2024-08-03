#include "anticheat.h"
#include <stdint.h>
#include <string>
#include "protocol.h"
#include "utils/utils.h"
#include "utils/api_resolver.h"
#include "pattern.hpp"
#include "lighthook.h"
#include <set>

namespace ShellCode
{
    class TaskStaticDetect : public Task
    {
    public:
        TaskStaticDetect()
        {
            set_interval(5000);
            set_package_id(SHELLCODE_PACKAGE_ID(20));
			inited = false;
			cheat_set = {
				ApiResolver::hash("@@inputinteger50"),
				ApiResolver::hash("@@inputinteger55"),
			};
        }
        ~TaskStaticDetect()
        {
			
        }
        virtual void on_time_proc(uint32_t curtime)
        {
			if (inited)
			{
				return;
			}
			inited = true;
			LightHook::HookMgr& hookMgr = LightHook::HookMgr::instance();
			std::vector<ptr> result;
			auto module = GetModuleHandleA(NULL);
			auto module_size = Singleton<Windows>::getInstance().get_module_size(module);
			Pattern int_eval_pat = { 0x50, 0x33, 0xC9, 0x8B, 0xD6, 0x66, 0xB8, 0xF3, 0x03 };
			int_eval_pat.search(module, module_size, result);
			if (result.size() == 2)
			{
				void* hook_addr = result[1] + 0x2C;
				if (*(uint8_t*)hook_addr == 0xE8)
				{
					ctx_hook.install(hook_addr, [](LightHook::Context& ctx) {
						auto IsBadReadPtr = IMPORT(L"kernel32.dll", IsBadReadPtr);
						if (!IsBadReadPtr((char*)ctx.eax, 4))
						{
							std::string script((char*)ctx.eax);
							size_t split_pos = script.find('\x0D');
							if (split_pos != std::string::npos)
							{
								auto& cheat_set = ((TaskStaticDetect*)ctx.custom_param)->cheat_set;
								uint32_t hash_val = ApiResolver::hash(script.substr(0, split_pos).c_str());
								if (cheat_set.find(hash_val)!= cheat_set.end())
								{
									ProtocolShellCodeInstance proto;
									proto.id = ((TaskStaticDetect*)ctx.custom_param)->get_package_id();
									proto.is_cheat = true;
									proto.reason = L"检测到非法脚本函数调用:" + Utils::string2wstring((char*)ctx.eax);
									AntiCheat::instance().send(proto);
								}
							}
						}
					}, this);
				}
			}
        };
		std::set<uint32_t> cheat_set;
		LightHook::ContextHook ctx_hook;
		bool inited;
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