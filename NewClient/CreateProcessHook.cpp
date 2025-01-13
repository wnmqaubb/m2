#include "pch.h"
#include "CreateProcessHook.h"
#include <Lightbone/windows_internal.h>
#include <Lightbone/utils.h>
#include <Lightbone/lighthook.h>
#include "Lightbone/LDasm.h"
#include "Tools/Packer/loader.h"

using namespace ApiResolver;
extern std::shared_ptr<HINSTANCE> dll_base;
extern share_data_ptr_t share_data;
extern void __stdcall reflective_load(share_data_ptr_t param) noexcept;
extern std::shared_ptr <asio::detail::thread_group> g_thread_group;

namespace HookProc
{
    bool is_address_in_module(void* addr, HINSTANCE dll)
    {
        return ((uintptr_t)dll <= (uintptr_t)addr) && ((uintptr_t)addr <= ((uintptr_t)dll + get_image_nt_header(dll)->OptionalHeader.SizeOfImage));
    }
#define FUNCTION_CHECK() \
     if (is_address_in_module(_ReturnAddress(), *dll_base))\
     {\
         SetLastError(ERROR_BAD_CONFIGURATION);\
         return FALSE;\
     }

    decltype(&::CreateProcessInternalW) create_process_internal_w = nullptr;
    decltype(&::ResumeThread) resume_thread = nullptr;
    HANDLE main_thread_handle = NULL;
    HANDLE process_handle = NULL;
    BOOL WINAPI CreateProcessInternalW(
        HANDLE token_handle, LPCWSTR app_name, LPWSTR cmd_line,
        LPSECURITY_ATTRIBUTES process_attributes, LPSECURITY_ATTRIBUTES security_attributes,
        BOOL bih, DWORD flags, LPVOID env, LPCWSTR current_dir, LPSTARTUPINFOW startup_info,
        LPPROCESS_INFORMATION process_info, PHANDLE new_token)
    {
        VMP_VIRTUALIZATION_BEGIN();
        SetLastError(ERROR_SUCCESS);
        FUNCTION_CHECK();
        BOOL result = create_process_internal_w(token_handle,
            app_name,
            cmd_line,
            process_attributes,
            security_attributes,
            bih,
            flags,
            env,
            current_dir,
            startup_info,
            process_info,
            new_token);
        if (result && cmd_line != nullptr)
        {
            std::wstring main_module_file_name, cmd_line_str;
            std::string main_module_file_namea, cmd_line_stra;

            cmd_line_str = cmd_line;
            main_module_file_name.resize(MAX_PATH);
            GetModuleFileNameW(NULL, main_module_file_name.data(), MAX_PATH);
			
            main_module_file_namea = Utils::String::w2c(main_module_file_name);
            cmd_line_stra = Utils::String::w2c(cmd_line_str);
            if (cmd_line_stra.find(main_module_file_namea) != std::string::npos)
            {
				do 
				{
					auto args = Utils::String::split<std::string>(cmd_line_stra, " ");
					if (args.size() && args.size() >= 1 && args[1] == std::to_string(GetCurrentProcessId()))
					{
						LOG_EVENT("launcher copy");
						break;
					}
					else
					{
						main_thread_handle = process_info->hThread;
						process_handle = process_info->hProcess;
					}
				} while (0);
            }
        }
        VMP_VIRTUALIZATION_END();
        return result;
    }

    /**
     * @brief 恢复线程的钩子函数
     * 
     * 该函数是ResumeThread的钩子函数，用于在恢复线程时执行一些额外的操作。
     * 
     * @param thread_handle 线程句柄
     * @return 返回值表示操作是否成功
     */
    DWORD WINAPI ResumeThread(HANDLE thread_handle)
    {
        VMP_VIRTUALIZATION_BEGIN();
        // 设置最后一个错误码为ERROR_SUCCESS，表示操作成功
        SetLastError(ERROR_SUCCESS);
        // 检查当前地址是否在模块内，如果是则返回FALSE
        FUNCTION_CHECK();
        // 如果当前进程句柄、主线程句柄和传入的线程句柄都有效，并且传入的线程句柄等于主线程句柄
        if (process_handle && main_thread_handle && thread_handle == main_thread_handle)
        {
            // 导入kernel32.dll中的GetThreadContext函数
            auto get_thread_context = IMPORT(L"kernel32.dll", GetThreadContext);
            // 定义一个CONTEXT结构体，用于存储线程上下文
            CONTEXT context = {};
            // 设置上下文标志为CONTEXT_FULL，表示获取完整的上下文信息
            context.ContextFlags = CONTEXT_FULL;
            // 如果获取线程上下文失败，则弹出一个消息框提示错误，并返回0
            if (!get_thread_context(thread_handle, &context))
            {
                ::MessageBoxA(NULL, "GetThreadContext Error", "Warning", MB_OK);
                return 0;
            }
            // 获取线程的入口点地址
            uintptr_t oep = context.Eax;
            {
                // 导入kernel32.dll中的ReadProcessMemory函数
                auto read_process_memory = IMPORT(L"kernel32.dll", ReadProcessMemory);
                // 定义一个8字节的缓冲区，用于存储读取的数据
                uint8_t buffer[8] = {};
                // 定义一个DWORD类型的变量，用于存储实际读取的字节数
                DWORD bytes_of_read = 0;
                // 从进程内存中读取数据到缓冲区
                read_process_memory(process_handle, (LPVOID)oep, buffer, sizeof(buffer), &bytes_of_read);
                // 如果读取的数据的前4个字节等于0xE8535056，则计算新的入口点地址
                if (*(uint32_t*)buffer == 0xE8535056)
                {
                    oep = oep + 8 + *(uint32_t*)&buffer[4];
                }
            }
            {
                // 定义一个字符串变量，用于存储shellcode
                std::string shellcode;
                // 获取当前模块的句柄
                auto image = GetModuleHandleA(NULL);
                // 根据crypt_code_rva计算出加密代码的指针
                crypt_code_ptr_t crypt_code = rva2va<crypt_code_ptr_t>(image, share_data->crypt_code_rva);
                // 定义一个DWORD类型的变量，用于存储内存保护属性
                DWORD old_protect = 0;
                // 修改crypt_code_rva指向的内存区域的保护属性为PAGE_READWRITE
                VirtualProtect(rva2va<void*>(image, share_data->crypt_code_rva), share_data->crypt_code_size, PAGE_READWRITE, &old_protect);
                // 使用xor_buffer函数对crypt_code_rva指向的内存区域进行异或解密
                Utils::Crypto::xor_buffer(rva2va<void*>(image, share_data->crypt_code_rva), share_data->crypt_code_size, share_data->xor_key);
                // 恢复crypt_code_rva指向的内存区域的原始保护属性
                VirtualProtect(rva2va<void*>(image, share_data->crypt_code_rva), share_data->crypt_code_size, old_protect, &old_protect);

                // 计算重定位信息的大小
                const size_t reloc_size = crypt_code->reloc_size;
                // 计算绑定DLL的大小
                const size_t bind_dll_size = crypt_code->bind_dll_size;
                // 计算文本代码的大小
                const size_t text_code_size = crypt_code->text_code_size;
                // 计算文本代码的偏移量
                const size_t text_code_offset = offsetof(CryptCode, reloc) + sizeof(RelocInfo) * reloc_size;
                // 计算绑定DLL的偏移量
                const size_t bind_dll_offset = text_code_offset + text_code_size;

                // 调整shellcode的大小，使其能够容纳共享数据和绑定的DLL
                shellcode.resize(sizeof(*share_data) + crypt_code->bind_dll_size);
                // 将共享数据复制到shellcode中
                memcpy((void*)shellcode.data(), share_data, sizeof(*share_data));
                // 将绑定的DLL复制到shellcode中
                memcpy((char*)shellcode.data() + sizeof(*share_data), rva2va<void*>(image, share_data->crypt_code_rva + bind_dll_offset), crypt_code->bind_dll_size);

                // 再次修改crypt_code_rva指向的内存区域的保护属性为PAGE_READWRITE
                VirtualProtect(rva2va<void*>(image, share_data->crypt_code_rva), share_data->crypt_code_size, PAGE_READWRITE, &old_protect);
                // 再次使用xor_buffer函数对crypt_code_rva指向的内存区域进行异或加密
                Utils::Crypto::xor_buffer(rva2va<void*>(image, share_data->crypt_code_rva), share_data->crypt_code_size, share_data->xor_key);
                // 恢复crypt_code_rva指向的内存区域的原始保护属性
                VirtualProtect(rva2va<void*>(image, share_data->crypt_code_rva), share_data->crypt_code_size, old_protect, &old_protect);

                // 计算共享数据在shellcode中的指针
                share_data_ptr_t new_param = (share_data_ptr_t)shellcode.data();
                // 设置共享数据的magic字段为0x90909090
                new_param->magic = 0x90909090;
                // 计算入口点的偏移量
                new_param->entry_point_dist = sizeof(share_data_t) - offsetof(share_data_t, ret_opcode) + va2fa(*dll_base, (size_t)&reflective_load);
                // 设置shellcode的大小
                new_param->stub_size = shellcode.size();

                // 导入kernel32.dll中的Sleep函数
                auto Sleep = IMPORT(L"kernel32.dll", Sleep);
                // 定义一个布尔变量，用于标记是否成功添加远程内联钩子
                auto hooked = false;
                // 尝试最多30次添加远程内联钩子
                for (int i = 0; i < 30; i++)
                {
                    // 添加远程内联钩子，将shellcode插入到指定的地址
                    hooked = LightHook::HookMgr::instance().add_remote_inline_hook(process_handle, (void*)oep, shellcode, offsetof(share_data_t, oep));
                    // 如果添加成功，则跳出循环
                    if (hooked)
                    {
                        break;
                    }
                    // 等待200毫秒后再次尝试
                    Sleep(200);
                }

                // 如果添加远程内联钩子失败，则弹出一个消息框提示错误，并终止主线程
                if (!hooked)
                {
                    ::MessageBoxA(NULL, "add_remote_inline_hook error", "Warning", MB_OK);
                    ::TerminateProcess(main_thread_handle, 0);
                    return ERROR_ERRORS_ENCOUNTERED;
                }
            }
            // 清除主线程句柄和进程句柄
            main_thread_handle = NULL;
            process_handle = NULL;
        }
        VMP_VIRTUALIZATION_END();
        // 调用原始的ResumeThread函数，恢复线程的执行
        return resume_thread(thread_handle);
    }

    DWORD WINAPI check_hook_vaild_routine()
    {
        auto Sleep = IMPORT(L"kernel32.dll", Sleep);
        auto GetModuleHandleExW = IMPORT(L"kernel32.dll", GetModuleHandleExW);
        auto ExitProcess = IMPORT(L"kernel32.dll", ExitProcess);
        while (true)
        {
            VMP_VIRTUALIZATION_BEGIN();
            auto hooks = LightHook::HookMgr::instance().get_hooks();
            for (auto &hook : hooks)
            {
                if (!hook->is_remote_hook() && hook->is_hooked())
                {
                    uint8_t* hook_addr = reinterpret_cast<uint8_t*>(hook->get_hook_addr());
                    uint8_t* src_addr = reinterpret_cast<uint8_t*>(ResolveJmp(hook->get_src()));
                    if (*hook_addr != 0xE9 || !is_address_in_module(src_addr, *dll_base) || *(src_addr + 0x2) == 0xE9)
                    {
                        *(void**)_AddressOfReturnAddress() = ExitProcess;
                        return 0;
                    }
                }
            }
            VMP_VIRTUALIZATION_END();
            HookProc::CreateProcessInternalW(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
            if (GetLastError() != ERROR_BAD_CONFIGURATION)
            {
                *(void**)_AddressOfReturnAddress() = ExitProcess;
                return 0;
            }
            HookProc::ResumeThread(NULL);
            if (GetLastError() != ERROR_BAD_CONFIGURATION)
            {
                *(void**)_AddressOfReturnAddress() = ExitProcess;
                return 0;
            }
            Sleep(100);
        }
        return 0;
    }

    /**
     * @brief 初始化CreateProcessInternalW和ResumeThread的钩子
     * 
     * 该函数尝试从kernelbase.dll或kernel32.dll中导入CreateProcessInternalW和ResumeThread函数，并为它们设置钩子。
     * 
     * @return 如果成功初始化钩子，则返回true，否则返回false。
     */
    bool init_create_process_hook()
    {
        //登录器入口
        // 尝试从kernelbase.dll中导入CreateProcessInternalW函数
        void* create_process_internal_w = IMPORT(L"kernelbase.dll", CreateProcessInternalW);
        // 如果导入失败，则尝试从kernel32.dll中导入
        if (!create_process_internal_w)
        {
            create_process_internal_w = IMPORT(L"kernel32.dll", CreateProcessInternalW);
        }
        // 为CreateProcessInternalW函数设置钩子，将其重定向到HookProc::CreateProcessInternalW
        LightHook::HookMgr::instance().add_inline_hook(create_process_internal_w, &HookProc::CreateProcessInternalW, &HookProc::create_process_internal_w);

        // 尝试从kernelbase.dll中导入ResumeThread函数
        void* resume_thread = IMPORT(L"kernelbase.dll", ResumeThread);
        // 如果导入失败，则尝试从kernel32.dll中导入
        if (!resume_thread)
        {
            resume_thread = IMPORT(L"kernel32.dll", ResumeThread);
        }
        // 为ResumeThread函数设置钩子，将其重定向到HookProc::ResumeThread
        LightHook::HookMgr::instance().add_inline_hook(resume_thread, &HookProc::ResumeThread, &HookProc::resume_thread);
        
        // 创建一个新线程来检查钩子的有效性
        //g_thread_group->create_thread(&check_hook_vaild_routine);
        // 返回初始化成功的标志
        return true;
    }

}
