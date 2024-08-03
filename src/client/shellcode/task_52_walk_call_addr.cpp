#include "anticheat.h"
#include <stdint.h>
#include <string>
#include "protocol.h"
#include "utils/utils.h"
#include "utils/api_resolver.h"
#include <lighthook.h>
#include "memory.h"
#include "pattern.hpp"

namespace ShellCode
{
    uint32_t count = 10;
    uint32_t off_line_walk_count = 0;
    uint32_t last_mouse_point_x = 1;
    uint32_t last_mouse_point_y = 1;

	class TaskStaticDetect : public Task
	{
	public:
		TaskStaticDetect()
		{
            set_interval(60 * 1000);
			set_package_id(SHELLCODE_PACKAGE_ID(52));
            cheat = false;
		}
		~TaskStaticDetect()
		{

		}
		virtual void on_time_proc(uint32_t curtime)
		{
            if(cheat)
            {
                cheat = false;
                ProtocolShellCodeInstance proto;
                proto.id = get_package_id();
                proto.m_type = Protocol::PackageType::PACKAGE_TYPE_ADMIN;
                proto.is_cheat = true;
                proto.reason = L"发现脱机挂9052:";
                proto.reason.append(reason);
                AntiCheat::instance().send(proto);
            }
		};

        std::wstring reason;
        bool cheat;
	};

    void buffer2hexstr(void* pBuff, unsigned int nLen,OUT std::string& hexstr)
    {
        if(NULL == pBuff || 0 == nLen)
        {
            return;
        }

        const int nBytePerLine = 16;
        unsigned char* p = (unsigned char*)pBuff;
        hexstr.resize(3 * nBytePerLine);
        for(unsigned int i = 0; i < nLen; ++i)
        {
            int idx = 3 * (i % nBytePerLine);
            sprintf_s(&hexstr.data()[idx], 4, "%02X ", p[i]);
        }

    }


    float my_sqrt(float x)
    {
        float xhalf = 0.5f * x;
        int i = *(int*)&x;
        i = 0x5f375a86 - (i >> 1);
        x = *(float*)&i;
        x = x * (1.5f - xhalf * x*x);
        x = x * (1.5f - xhalf * x*x);
        x = x * (1.5f - xhalf * x*x);

        return 1 / x;
    }

    DWORD get_module_size(HMODULE module)
    {
        PBYTE image = (PBYTE)module;
        PIMAGE_DOS_HEADER image_dos_header;
        PIMAGE_NT_HEADERS image_nt_header;
        image_dos_header = (PIMAGE_DOS_HEADER)image;
        if(image_dos_header->e_magic != IMAGE_DOS_SIGNATURE)
        {
            return 0;
        }
        image_nt_header = (PIMAGE_NT_HEADERS)&image[image_dos_header->e_lfanew];
        if(image_nt_header->Signature != IMAGE_NT_SIGNATURE)
        {
            return 0;
        }
        return image_nt_header->OptionalHeader.SizeOfImage;
    }

    uint32_t calc_mouse_moved_distance()
    {
        auto GetCursorPos = IMPORT(L"user32.dll", GetCursorPos);
        POINT cp;
        GetCursorPos(&cp);
        if(cp.x == last_mouse_point_x && cp.y == last_mouse_point_y)
        {
            return 0;
        }
        uint32_t x = cp.x - last_mouse_point_x;
        uint32_t y = cp.y - last_mouse_point_y;
        uint32_t mouse_distance = my_sqrt(x * x + y * y);
        last_mouse_point_x = cp.x;
        last_mouse_point_y = cp.y;
        return mouse_distance;
    }



	uint32_t main()
    {
		TaskStaticDetect* task = new TaskStaticDetect();
        if(AntiCheat::instance().add_task(task))
        {
            uint32_t walk_call_addr_;

            Pattern walk_call_key1 = {0x55, 0x8B, 0xEC, 0x83, 0xC4, 0xE8, 0x53, 0x56, 0x57, 0x33, 0xDB, 0x89, 0x5D, 0xEC, 0x89,
                0x5D, 0xE8, 0x8B, 0xF9, 0x8B, 0xDA, 0x89, 0x45, 0xFC, 0x8B, 0x75, 0x0C, 0x8B, 0x45, 0x08, 0xE8};
            Pattern walk_call_key2 = {0x55, 0x8B, 0xEC, 0x83, 0xC4, 0xEC, 0x53, 0x56, 0x57, 0x33, 0xDB, 0x89, 0x5D, 0xEC, 0x8B, 0xF9, 0x8B, 0xDA,
                0x89, 0x45, 0xFC, 0x8B, 0x75, 0x08, 0x33, 0xC0, 0x55};

            auto GetModuleHandleW = IMPORT(L"kernel32.dll", GetModuleHandleW);
            auto CloseHandle = IMPORT(L"kernel32.dll", CloseHandle);
            HMODULE module = GetModuleHandleW(NULL);
            DWORD module_size = get_module_size(module);
            std::vector<ptr> result;

            if(walk_call_key1.search(module, module_size, result))
            {
                walk_call_addr_ = result.front();
            }
            else if(walk_call_key2.search(module, module_size, result))
            {
                walk_call_addr_ = result.front();
            }
            CloseHandle(module);
            // 走路栈回溯
            if(walk_call_addr_)
            {
                LightHook::HookMgr::instance().add_context_hook((uint32_t*)walk_call_addr_, [](LightHook::Context& ctx) {                    
                    uint32_t mouse_distance = calc_mouse_moved_distance();
                    if(count++ < 10) return;
                    count = 1;
                    std::vector<uint32_t> stack_call_back_list;
                    std::string stack_call_back = GameMemory::instance().stack_get_call_stack(ctx.esp, stack_call_back_list);
                    if(stack_call_back.empty()) return;
                    std::string reason = "", addr_hex_str;
                    uint32_t addr_hex = 0;
                    auto GetModuleHandleExW = IMPORT(L"kernel32.dll", GetModuleHandleExW);
                    HMODULE thread_module = nullptr;
                    for(uint32_t addr_hex : stack_call_back_list)
                    {                        
                        GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)addr_hex, &thread_module);
                        if(thread_module == nullptr && off_line_walk_count++ > 5)
                        {
                            off_line_walk_count = 0;
                            addr_hex_str = "";
                            uint32_t pBuff = addr_hex - 16, nLen = 16;
                            buffer2hexstr((PVOID)pBuff, nLen, addr_hex_str);
                            uint32_t reason_length = 70 + stack_call_back.length();
                            reason.resize(reason_length);
                            snprintf(&reason.data()[0], 
                                reason_length,
                                "%d|%08X|%s|%s",
                                mouse_distance,
                                addr_hex, 
                                addr_hex_str.c_str(), 
                                stack_call_back.c_str());
                            auto task = reinterpret_cast<ShellCode::TaskStaticDetect*>(AntiCheat::instance().task_map_[SHELLCODE_PACKAGE_ID(52)]);
                            task->cheat = true;
                            task->reason = Utils::string2wstring(reason);
                            return;
                        }
                    }
                    });
            }
			return 0;
		}
		else
		{
			delete task;
			return 1;
		}
	}
}