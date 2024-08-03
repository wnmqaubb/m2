#include "anticheat.h"
#include <stdint.h>
#include <string>
#include "protocol.h"
#include "utils/utils.h"
#include "utils/windows_internal.h"
#include "utils/api_resolver.h"
#include <set>
#include <iostream>

PSYSTEM_HANDLE_INFORMATION GetSystemProcessHandleInfo();


namespace ShellCode
{
	class TaskStaticDetect : public Task
	{
	public:
		TaskStaticDetect()
		{
			Utils::CWindows::instance().power();
			set_interval(60 * 1000 * 5);
			set_package_id(SHELLCODE_PACKAGE_ID(13));
			initialize_handle_type();
			black_file_names = {
				L"BGM",
				L"record",
				L"≈‰÷√",
				L"…Û≈–"
			};
		}

		~TaskStaticDetect()
		{

		}

		void initialize_handle_type()
		{
			HANDLE_TYPE_PROCESS = 7;
			Utils::CWindows::SystemVersion sys_version = Utils::CWindows::instance().get_system_version();
			if (sys_version == Utils::CWindows::SystemVersion::WINDOWS_ANCIENT || sys_version == Utils::CWindows::SystemVersion::WINDOWS_XP)
			{
				HANDLE_TYPE_PROCESS = 5;
			}
			else if (sys_version == Utils::CWindows::SystemVersion::WINDOWS_VISTA)
			{
				HANDLE_TYPE_PROCESS = 6;
			}
			else
			{
				HANDLE_TYPE_PROCESS = 7;
			}
		}

		bool find_hidden_pid_from_csrss(std::wstring& reason)
		{
			auto OpenProcess = IMPORT(L"kernel32.dll", OpenProcess);
			auto DuplicateHandle = IMPORT(L"kernel32.dll", DuplicateHandle);
			auto GetCurrentProcess = IMPORT(L"kernel32.dll", GetCurrentProcess);
			auto CloseHandle = IMPORT(L"kernel32.dll", CloseHandle);
			auto GetProcessId = IMPORT(L"kernel32.dll", GetProcessId);

			if (!OpenProcess || !DuplicateHandle || !GetCurrentProcess || !CloseHandle || !GetProcessId)
				return false;
			SYSTEM_HANDLE_INFORMATION* pHandleInformation = NULL;
			bool hidden_processes = false;
			std::set<int> csrss_pid_set;
			Utils::CWindows::ProcessMap processes = Utils::CWindows::instance().enum_process();
			pHandleInformation = GetSystemProcessHandleInfo();

			if (!pHandleInformation)
			{
				return false;
			}

			for (auto &p : processes)
			{
				if (p.second.name == L"csrss.exe")
				{
					csrss_pid_set.insert(p.first);
				}
			}

			if (csrss_pid_set.size() == 0)
			{
				delete[](BYTE*)pHandleInformation;
				return hidden_processes;
			}

			int count = 1;
			for (ULONG i = 0; i < pHandleInformation->NumberOfHandles; i++)
			{
				DWORD current_handle_pid = pHandleInformation->Handles[i].UniqueProcessId;

				if (csrss_pid_set.find(current_handle_pid) == csrss_pid_set.end())
				{
					continue;
				}

				if (pHandleInformation->Handles[i].ObjectTypeIndex != HANDLE_TYPE_PROCESS)
				{
					continue;
				}

				HANDLE process_handle = OpenProcess(PROCESS_DUP_HANDLE, FALSE, current_handle_pid);

				if (process_handle == NULL)
				{
					continue;
				}

				HANDLE duplicated_handle = NULL;

				if (DuplicateHandle(process_handle,
					(HANDLE)pHandleInformation->Handles[i].HandleValue,
					GetCurrentProcess(),
					&duplicated_handle,
					Utils::CWindows::instance().ProcessQueryAccess | PROCESS_VM_READ, FALSE, 0) == NULL)
				{
					CloseHandle(process_handle);
					continue;
				}

				DWORD pid_from_duplicated_handle = GetProcessId(duplicated_handle);
				Utils::CWindows::ModuleList modules;
				if (pid_from_duplicated_handle != (DWORD)-1
					&& !Utils::CWindows::instance().is_64bits_process(duplicated_handle)
					&& (processes.find(pid_from_duplicated_handle) == processes.end()))
				{
					Utils::CWindows::instance().ldr_walk<uint32_t>(duplicated_handle, modules);
					if (!modules.empty())
					{
						std::error_code ec;
						auto walk_path = std::filesystem::path(modules.front().path).parent_path();
						auto itors = std::filesystem::directory_iterator(walk_path, ec);
						if (ec)
							break;
						for (auto& file : itors)
						{
							auto file_name = file.path().filename().wstring();
							for (auto& filter : black_file_names)
							{
								if (file_name.find(filter) != file_name.npos)
								{
									reason = file.path().wstring();
									hidden_processes = true;
									break;
								}
							}
						}
					}
				}

				CloseHandle(duplicated_handle);
				CloseHandle(process_handle);
				if (hidden_processes)
				{
					break;
				}
			}

			delete[](BYTE*)pHandleInformation;
			return hidden_processes;
		}

		virtual void on_time_proc(uint32_t curtime)
		{
			ProtocolShellCodeInstance proto;
			proto.m_type = Protocol::PackageType::PACKAGE_TYPE_ADMIN;
			proto.id = get_package_id();
			proto.is_cheat = false;
			if (find_hidden_pid_from_csrss(proto.reason))
			{
				proto.is_cheat = true;
			}
			if (proto.is_cheat)
			{
				AntiCheat::instance().send(proto);
			}
		}
		std::set<std::wstring> black_file_names;
		unsigned char HANDLE_TYPE_PROCESS;
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
			AntiCheat::instance().task_map_.erase(AntiCheat::instance().task_map_.find(task->get_package_id()));
			if (!AntiCheat::instance().add_task(task))
			{
				delete task;
				return 1;
			}
			return 0;
		}
	}
}


