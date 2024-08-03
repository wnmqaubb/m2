#include "anticheat.h"
#include <stdint.h>
#include <string>
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
			set_package_id(SHELLCODE_PACKAGE_ID(6));
		}
		~TaskStaticDetect()
		{

		}
		virtual void on_time_proc(uint32_t curtime)
		{
			ProtocolShellCodeInstance proto;
			proto.id = get_package_id();
            auto OpenProcess = IMPORT(L"kernel32.dll", OpenProcess);
            auto CloseHandle = IMPORT(L"kernel32.dll", CloseHandle);
            auto TerminateProcess = IMPORT(L"kernel32.dll", TerminateProcess);
            auto windows = Utils::CWindows::instance().enum_windows();
            for (auto& window : windows)
            {
                if (window.class_name == L"_EL_HideOwner")
                {
                    Utils::CWindows::ProcessInfo process;
                    Utils::CWindows::instance().get_process(window.pid, process);
                    HANDLE handle = OpenProcess(PROCESS_TERMINATE, FALSE, window.pid);
                    TerminateProcess(handle, 0);
                    CloseHandle(handle);
                    proto.is_cheat = true;
                    proto.reason = L"发现易语言窗口，进程为:" + process.name;
                    break;
                }
            }

            if(!proto.is_cheat)
            {
                for(auto &process : Utils::CWindows::instance().enum_process())
                {
                    if(process.second.threads.size() == 0)
                    {
                        continue;
                    }
                    for(auto &thread : process.second.threads)
                    {
                        if(!thread.second.is_main_thread)
                        {
                            continue;
                        }
                        if(thread.second.start_address == 0xBA6C74)
                        {
                            proto.is_cheat = true;
                            proto.reason = L"发现猎手外挂，进程为:" + process.second.name;
                            break;
                        }
                    }
                }
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
            auto GetDlgItem = IMPORT(L"user32.dll", GetDlgItem);
            LightHook::HookMgr::instance().add_context_hook(GetDlgItem, [](LightHook::Context& ctx) {
                uintptr_t* esp = (uintptr_t*)ctx.esp;
                uintptr_t* ebp = (uintptr_t*)ctx.ebp;
                const uintptr_t return_address = ebp[1];
                auto GetModuleHandleExW = IMPORT(L"kernel32.dll", GetModuleHandleExW);
                auto ExitProcess = IMPORT(L"kernel32.dll", ExitProcess);
                HMODULE module = NULL;
                GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)return_address, &module);
                if (module == NULL)
                {
                    esp[0] = (uintptr_t)ExitProcess;
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