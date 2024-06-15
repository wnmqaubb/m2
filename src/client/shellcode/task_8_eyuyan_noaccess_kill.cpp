#include "anticheat.h"
#include <stdint.h>
#include <string>
#include "protocol.h"
#include "utils/utils.h"
#include "utils/api_resolver.h"
#include <lighthook.h>
#include <set>

namespace ShellCode
{
	class TaskStaticDetect : public Task
	{
	public:
		TaskStaticDetect()
		{
			set_interval(15000);
            set_package_id(SHELLCODE_PACKAGE_ID(8)); 
		}
		~TaskStaticDetect()
		{

		}
		virtual void on_time_proc(uint32_t curtime)
		{
			ProtocolShellCodeInstance proto;
            proto.m_type = Protocol::PackageType::PACKAGE_TYPE_ADMIN;
			proto.id = get_package_id();
            auto OpenProcess = IMPORT(L"kernel32.dll", OpenProcess);
            auto CloseHandle = IMPORT(L"kernel32.dll", CloseHandle);
            auto Sleep = IMPORT(L"kernel32.dll", Sleep);
            auto windows = Utils::CWindows::instance().enum_windows();
            for (auto& window : windows)
            {
                if (window.class_name == L"_EL_HideOwner")
                {
                    Utils::CWindows::ProcessInfo process;
                    Utils::CWindows::instance().get_process(window.pid, process);
                    HANDLE handle = OpenProcess(PROCESS_TERMINATE, FALSE, window.pid);
                    if (handle == NULL)
                    {
                        proto.is_cheat = true;
                        proto.reason = L"发现易语言无权限窗口，进程为:" + process.name;
                        break;
                    }
                    else
                    {
                        CloseHandle(handle);
                    }
                }
                Sleep(1);
            }

            if (proto.is_cheat)
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