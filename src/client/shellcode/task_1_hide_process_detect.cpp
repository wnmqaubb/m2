#include "anticheat.h"
#include <stdint.h>
#include <string>
#include "protocol.h"
#include "utils/utils.h"
#include "utils/api_resolver.h"
#include <set>

namespace ShellCode
{
    class TaskStaticDetect : public Task
    {
    public:
		TaskStaticDetect()
		{
			set_interval(30000);
			set_package_id(SHELLCODE_PACKAGE_ID(1));
			cheat_dll_set = {

			};
			cheat_exe_set = {
				0x9D6B,/*GEE大师*/
			};
			cheat_set = {
				0x40B76F,/*猎手外挂*/
				0x44FD84,/*定制外挂*/
				0x401214,/*定制外挂*/
				0xBAE089,/*刺客外挂*/
				0x14413D2,/*红牛大退*/
				0x548290,/*守望者2.03*/
				0x49D9FA,
				0x409E00,
				0x4A3AA5,
				0x405DF0,
				0x46CBC3,
				0x1FBB35E,
				0x418101,
				0x449C71,
				0x405810,
				0x44EA94,
				0x410780,/*CE5410*/
				0x464FD7,
				0x44C4A2,
				0x466D95,
				0x131E022,
				0x1010DD1F,
				0x455580,
				0x455188,
				0x468881,
				0xE87B43,
				0xD07A0B,
				0x421456,
				0x40BC0E,
				0x4012DC,/*简单外挂*/
				0x4D4960,/*土豪金多开器*/
				0xEBCF00,/*BRCTR*/
				0xEBC2D1,/*BRCTR*/
				0x44E076,/*WPE*/
				0x44EA94,/*WPE*/
				0x44533D,/*WPE*/
				0x445CD4,/*WPE*/
				0x7D6B38,/*WPE*/
				0xC78F17,/*GEE猎手*/
				0xDF3E94,/*GEE猎手*/
				0x806D24,/*小叮当*/
				0x4151F0,/*CE6.5*/
				0x404FE0,/*独立团CE*/
				0x63D336,/*外挂作坊CE*/
				0x413900,/*寡人CE*/
				0x404DFC,/*爱神专用CE*/
				0x419800,/*CE6.5*/
				0x415A40,/*CE6.6*/
				0x41A380,/*CE6.7*/
				0x415C50,/*CE6.7*/
				0x40B3C1,/*全哥CE*/
				0x414260,/*CE伪装QQ*/
				0x418400,/*CE伪装QQ*/
				0x44C740,/*叠加封包*/
				0x4A9BB4,/*叠加封包*/
				0x5073FC,/*易语言CE*/
				0x12C6EFE,/*QE*/
				0x495CA4,/*封包*/
				0x12DE787,/*荣耀*/
				0x44C9B8,/*荣耀*/
				0x40D5AF,/*键盘鼠标录制*/
				0x18D3209,/*荣耀*/
				0x40110C,/*未知*/
				0x4165C1,/*BOIS虚你话*/
				0x233F3386,/*一刀*/
				0x622FFD,/*南方天成*/
				0x114FC7A,/*Gee荣耀*/
				0x4AA37E,/*简单破解*/
				0x594017,/*GEE荣耀*/
				0x4298F6,/*GEE猎手*/
				0x4FD970,/*GEE猎手*/
				0x47B172,/*GEE猎手*/
				0x6B6EB40,/*GEE荣耀*/
				0x25F0EF2,/*刀锋外挂*/
				0x40A735,/*进程隐藏*/
				0x0209F0,/*GEE大师*/
				0x2C800A,/*老骨灰小腿*/
				0x10070B75,/*GEE荣耀*/
				0x4A3C45,/*简单外挂*/
				0x40148C,/*简单外挂*/
				0x545C69,
			};

			cheat_gee_set = {
				0x03A72,/*GEE猎手*/
				0x608E7,/*GEE猎手*/
				0xF7B63,/*GEE猎手*/
				0xE30BD,/*GEE猎手*/
				0x23C5,/*GEE猎手*/
				0x3DBD,/*GEE猎手*/
				0x30C1,/*GEE猎手*/
				0x60D9,/*GEE猎手*/
				0x60A7,/*GEE猎手*/
				0x60D1,/*GEE猎手*/
				0x7AD7,/*GEE猎手*/
				0x5417,/*GEE猎手*/
				0x0363,/*GEE猎手*/
				0xE18C,/*GEE猎手*/
				0xEBAE,/*定制挂机*/
				0x94DD,/*GEE猎手*/
				0x4AC4,/*GEE猎手*/
				0xECCA,/*GEEPK*/
				0x1006FB55,
				0x4B45EF,
				0x402644,
				0x192FB9A,
				0xE09808,
				0xE65E94,
				0xE60ACF,
				0xD0E9,
				0x5247,
				0x33ED,
				0x115D7F,
				0xEBAE,
				0x6F659,
				0x2256,
				0x3C3A,
				0xD36B,
				0x443090,
				0xF33A,
			    0x03A72,
			    0x23C5,
				0x2644,
				0x8063,
				0x545D39,
				0x545B89,/*守望者*/
				0x475DA8,/*GEE定制*/
				0x406393,/*GEE定制*/
				0x40AFC8,/*GEE定制*/
				0x4106F9,/*GEE定制*/
				0x406345,/*GEE定制*/
				0x3C7EB40,/*GEE大师*/
				0x6841F0,/*GEE定制*/
				0x17A4
			};
		}
		virtual void on_time_proc(uint32_t curtime)
		{		 
			ProtocolShellCodeInstance proto;
			proto.id = get_package_id();
			proto.is_cheat = false;
			auto windows = Utils::CWindows::instance().enum_windows();
            auto process_maps = Utils::CWindows::instance().enum_process();
            auto Sleep = IMPORT(L"kernel32.dll", Sleep);
            for (auto& window : windows)
            {
                if ((window.class_name.find(L"MSCTFIME") == std::wstring::npos
                    && window.class_name.find(L"tooltips_class32") == std::wstring::npos
                    && window.class_name.find(L"FORM_PROJECT1_FORM1_CLASS:0") == std::wstring::npos
                    && window.class_name.find(L"QQPinyinImageCandWndTSF") == std::wstring::npos
                    && window.class_name.length() != 10) ||
					process_maps.find(window.pid) == process_maps.end())
                {
                    continue;
                }
                auto& process = process_maps[window.pid];
				if (!process.threads.size())
					continue;
                for (auto& thread : process.threads)
                {
					if ((window.class_name.find(L"tooltips_class32") != std::wstring::npos || window.class_name.find(L"MSCTFIME"))
						&& (cheat_gee_set.find(thread.second.start_address & 0xFFFFFFFF) != cheat_gee_set.end() ||
							cheat_gee_set.find(thread.second.start_address & 0xFFFFFF) != cheat_gee_set.end()))
					{
						proto.is_cheat = true;
						proto.reason = L"发现GEE猎手或者荣耀外挂请封号处理【1号特征】，进程为:" + process.name;
						break;
					}
                    Utils::CWindows::ModuleInfo module;
                    if (!process.modules.size() || !Utils::CWindows::instance().get_module_from_address(process.pid, thread.second.start_address, process.modules, module))
                    {
						// ***************无模块的加在这***************
						if (cheat_gee_set.find(thread.second.start_address & 0xFFFF) != cheat_gee_set.end() ||
							cheat_gee_set.find(thread.second.start_address & 0xFFFFF) != cheat_gee_set.end() )
						{
							proto.is_cheat = true;
							proto.reason = L"发现GEE猎手或者荣耀外挂请封号处理【1号特征】，进程为:" + process.name;
							break;
						}
                        continue;
                    }

                    if (module.module_name.find(L".exe") != std::wstring::npos)
                    {
						if (window.class_name.find(L"QQPinyinImageCandWndTSF") != std::wstring::npos
							&& (thread.second.start_address & 0xFFFF) == 0x3020)
						{
							proto.is_cheat = true;
							proto.reason = L"GEE大师外挂请封号处理【4号特征】，进程为:" + process.name;
							break;
						}
						else if (window.class_name.find(L"FORM_PROJECT1_FORM1_CLASS:0") != std::wstring::npos
							&& (thread.second.start_address & 0xFFFF) == 0x2256)
						{
							proto.is_cheat = true;
							proto.reason = L"发现GEE猎手或者荣耀外挂请封号处理【5号特征】，进程为:" + process.name;
							break;
						}
                    }
					else if (module.module_name.find(L".dll") != std::wstring::npos)
					{
						if (window.class_name.length() == 10
							&& ((thread.second.start_address & 0xFFFF) == 0xE070
								|| (thread.second.start_address & 0xFFFF) == 0xE480
								|| (thread.second.start_address & 0xFFFF) == 0x8B86
								|| (thread.second.start_address & 0xFFFF) == 0xE750
								|| (thread.second.start_address & 0xFFFFFF) == 0x4012DC))
						{
							proto.is_cheat = true;
								proto.reason = L"发现简单类外挂，进程为:" + process.name;
								break;
						}
					}
                    else if (module.module_name.find(L".tap") != std::wstring::npos
                        && ((thread.second.start_address & 0xFFFF) == 0xDD1F
                            || (thread.second.start_address & 0xFFFF) == 0x58F0))
                    {
                        proto.is_cheat = true;
                        proto.reason = L"发现荣耀外挂请封号处理，进程为【7号特征】:" + process.name;
                        break;
                    }
                }
                if (proto.is_cheat)
                {
                    break;
                }
                Sleep(1);
            }


            if (!proto.is_cheat)
            {
                for (auto &process : process_maps)
                {
					auto& threads = process.second.threads;
					if (!threads.size())
						continue;
                    for (auto &thread : threads)
                    {
                        Utils::CWindows::ModuleInfo module;
						uint32_t pid = process.second.pid;
						uint64_t thread_start_address = thread.second.start_address;
						std::vector<Utils::CWindows::ModuleInfo>& modules = process.second.modules;

                        if (cheat_set.find(thread_start_address) != cheat_set.end())
                        {
                            proto.is_cheat = true;
                            proto.reason = L"发现封包工具或者外挂！，进程为:" + process.second.name;
                            break;
                        }

                        if (!modules.size() || !Utils::CWindows::instance().get_module_from_address(pid, thread_start_address, modules, module))
                        {
                            continue;
                        }


                        if (module.module_name.find(L".exe") != std::wstring::npos)
                        {
                            if (cheat_exe_set.find(thread.second.start_address & 0xFFFF) != cheat_exe_set.end())
                            {
                                proto.is_cheat = true;
                                proto.reason = L"发现外挂，进程为:" + process.second.name;
                                break;
                            }
                        }
                        else if (module.module_name.find(L".dll") != std::wstring::npos)
                        {
                        }
                    }
                    if (proto.is_cheat)
                    {
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
        std::set<uint32_t> cheat_dll_set;
        std::set<uint32_t> cheat_exe_set;
        std::set<uint32_t> cheat_set;
		std::set<uint32_t> cheat_gee_set;
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