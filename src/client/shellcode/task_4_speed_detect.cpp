#include "anticheat.h"
#include <stdint.h>
#include <string>
#include <set>
#include "protocol.h"
#include "utils/utils.h"
#include "utils/api_resolver.h"
namespace ShellCode
{
	class TaskSpeedDetect : public Task
	{
	public:
		TaskSpeedDetect()
		{
			set_interval(5000);
			set_package_id(SHELLCODE_PACKAGE_ID(4)); 
		}
		~TaskSpeedDetect()
		{

		}

		virtual void on_time_proc(uint32_t curtime)
		{
			ProtocolShellCodeInstance proto;
			proto.id = get_package_id();
			auto Sleep = IMPORT(L"kernel32.dll", Sleep);
			auto QueryPerformanceFrequency = IMPORT(L"kernel32.dll", QueryPerformanceFrequency);
			auto QueryPerformanceCounter = IMPORT(L"kernel32.dll", QueryPerformanceCounter);
			auto SetThreadAffinityMask = IMPORT(L"kernel32.dll", SetThreadAffinityMask);
			auto GetCurrentThread = IMPORT(L"kernel32.dll", GetCurrentThread);
			auto GetTickCount = IMPORT(L"kernel32.dll", GetTickCount);
			auto timeGetTime = IMPORT(L"winmm.dll", timeGetTime);
			std::vector<void*> api_check_list = {GetTickCount, QueryPerformanceCounter, timeGetTime, SetThreadAffinityMask, QueryPerformanceFrequency};
			for(auto api : api_check_list)
			{
				if(api)
				{
					if(*(uint8_t*)api == 0xE9)
					{
						proto.reason = L"检测到加速作弊";
						proto.is_cheat = true;
					}
				}
			}

			SetThreadAffinityMask(GetCurrentThread(), 1);
			LARGE_INTEGER frequency;
			QueryPerformanceFrequency(&frequency);
			LARGE_INTEGER start, end;
			double seconds = 0.0L;
			int iterations = 0;
			for(int i = 0; i < 5; i++)
			{
				QueryPerformanceCounter(&start);
				Sleep(500);
				QueryPerformanceCounter(&end);

				seconds += double(end.QuadPart - start.QuadPart) / frequency.QuadPart;
				iterations++;
			}

			SetThreadAffinityMask(GetCurrentThread(), -1);
			seconds = seconds / iterations;
			if(seconds <= 0.5 * 0.99)
			{
				char buffer[255] = {0};
				snprintf(buffer, sizeof(buffer), "检测到驱动变速:%f", seconds);
				proto.reason = Utils::string2wstring(buffer);
				proto.is_cheat = true;
			}
            if(proto.is_cheat)
            {
                AntiCheat::instance().send(proto);
            }
		};
	};
	uint32_t main()
	{
		TaskSpeedDetect* task = new TaskSpeedDetect();
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