#include "anticheat.h"
#include <stdint.h>
#include <string>
#include "protocol.h"
#include "utils/utils.h"
#include "utils/api_resolver.h"
#include "utils/windows_internal.h"

PSYSTEM_HANDLE_INFORMATION GetSystemProcessHandleInfo();
DWORD detect_hide_process_handle()
{
	SYSTEM_HANDLE_TABLE_ENTRY_INFO* CurHandle;
	SYSTEM_HANDLE_INFORMATION *pInfo = GetSystemProcessHandleInfo();
	Utils::CWindows::ProcessMap processes = Utils::CWindows::instance().enum_process();
	if (pInfo)
	{
		for (DWORD i = 0; i < pInfo->NumberOfHandles; i++)
		{

			CurHandle = &(pInfo->Handles[i]);
			if (CurHandle->GrantedAccess & PROCESS_VM_WRITE)
			{
				if (processes.find(CurHandle->UniqueProcessId) == processes.end())
				{
					delete[] pInfo;
					return CurHandle->UniqueProcessId;
				}
				else
				{
					auto& process = processes[CurHandle->UniqueProcessId];
					if (process.name == L"explorer.exe" && process.no_access)
					{
						return CurHandle->UniqueProcessId;
					}
				}
			}
		}
	}
	return 0;
}

namespace ShellCode
{
	class TaskStaticDetect : public Task
	{
	public:
		TaskStaticDetect()
		{
			set_interval(60 * 1000 * 3);
			set_package_id(SHELLCODE_PACKAGE_ID(31));
		}
		~TaskStaticDetect()
		{

		}
		virtual void on_time_proc(uint32_t curtime)
		{
			decltype(&_splitpath) _splitpath = IMPORT(L"msvcrt.dll", _splitpath);
			ProtocolShellCodeInstance proto;
			proto.m_type = Protocol::PackageType::PACKAGE_TYPE_ADMIN;
			proto.id = get_package_id();
			DWORD hide_pid = detect_hide_process_handle();
			if (hide_pid != 0)
			{
				proto.is_cheat = true;
				proto.reason = L"发现隐藏进程外挂!:" + std::to_wstring(hide_pid);
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