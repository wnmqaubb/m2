#include "pch.h"
#include "CreateProcessHook.h"
#include <Lightbone/windows_internal.h>
#include <Lightbone/utils.h>
#include <Lightbone/lighthook.h>
#include "Lightbone/LDasm.h"
#include "Tools/Packer/loader.h"

using namespace ApiResolver;
extern HINSTANCE dll_base;
extern share_data_ptr_t share_data;
extern void __stdcall reflective_load(share_data_ptr_t param) noexcept;
extern asio::detail::thread_group g_thread_group;

namespace HookProc
{
    bool is_address_in_module(void* addr, HINSTANCE dll)
    {
        return ((uintptr_t)dll <= (uintptr_t)addr) && ((uintptr_t)addr <= ((uintptr_t)dll + get_image_nt_header(dll)->OptionalHeader.SizeOfImage));
    }
#define FUNCTION_CHECK() \
     if (is_address_in_module(_ReturnAddress(), dll_base))\
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

    DWORD WINAPI ResumeThread(HANDLE thread_handle)
    {
        VMP_VIRTUALIZATION_BEGIN();
        SetLastError(ERROR_SUCCESS);
        FUNCTION_CHECK();
        if (process_handle && main_thread_handle && thread_handle == main_thread_handle)
        {
            auto get_thread_context = IMPORT(L"kernel32.dll", GetThreadContext);
            CONTEXT context = {};
            context.ContextFlags = CONTEXT_FULL;
            if (!get_thread_context(thread_handle, &context))
            {
                ::MessageBoxA(NULL, "GetThreadContext Error", "Warning", MB_OK);
                return 0;
            }
            uintptr_t oep = context.Eax;
            {
                auto read_process_memory = IMPORT(L"kernel32.dll", ReadProcessMemory);
                uint8_t buffer[8] = {};
                DWORD bytes_of_read = 0;
                read_process_memory(process_handle, (LPVOID)oep, buffer, sizeof(buffer), &bytes_of_read);
                //17年登录器入口点兼容
                if (*(uint32_t*)buffer == 0xE8535056)
                {
                    oep = oep + 8 + *(uint32_t*)&buffer[4];
                }
            }
            {
                std::string shellcode;
                auto image = GetModuleHandleA(NULL);
                crypt_code_ptr_t crypt_code = rva2va<crypt_code_ptr_t>(image, share_data->crypt_code_rva);
                DWORD old_protect = 0;
                VirtualProtect(rva2va<void*>(image, share_data->crypt_code_rva), share_data->crypt_code_size, PAGE_READWRITE, &old_protect);
                Utils::Crypto::xor_buffer(rva2va<void*>(image, share_data->crypt_code_rva), share_data->crypt_code_size, share_data->xor_key);
                VirtualProtect(rva2va<void*>(image, share_data->crypt_code_rva), share_data->crypt_code_size, old_protect, &old_protect);

                const size_t reloc_size = crypt_code->reloc_size;
                const size_t bind_dll_size = crypt_code->bind_dll_size;
                const size_t text_code_size = crypt_code->text_code_size;
                const size_t text_code_offset = offsetof(CryptCode, reloc) + sizeof(RelocInfo) * reloc_size;
                const size_t bind_dll_offset = text_code_offset + text_code_size;

                shellcode.resize(sizeof(*share_data) + crypt_code->bind_dll_size);
                memcpy((void*)shellcode.data(), share_data, sizeof(*share_data));
                memcpy((char*)shellcode.data() + sizeof(*share_data), rva2va<void*>(image, share_data->crypt_code_rva + bind_dll_offset), crypt_code->bind_dll_size);

                VirtualProtect(rva2va<void*>(image, share_data->crypt_code_rva), share_data->crypt_code_size, PAGE_READWRITE, &old_protect);
                Utils::Crypto::xor_buffer(rva2va<void*>(image, share_data->crypt_code_rva), share_data->crypt_code_size, share_data->xor_key);
                VirtualProtect(rva2va<void*>(image, share_data->crypt_code_rva), share_data->crypt_code_size, old_protect, &old_protect);

                share_data_ptr_t new_param = (share_data_ptr_t)shellcode.data();
                new_param->magic = 0x90909090;
                new_param->entry_point_dist = sizeof(share_data_t) - offsetof(share_data_t, ret_opcode) + va2fa(dll_base, (size_t)&reflective_load);
                new_param->stub_size = shellcode.size();

                auto Sleep = IMPORT(L"kernel32.dll", Sleep);
                auto hooked = false;
                for (int i = 0; i < 30; i++)
                {
                    hooked = LightHook::HookMgr::instance().add_remote_inline_hook(process_handle, (void*)oep, shellcode, offsetof(share_data_t, oep));
                    if (hooked)
                    {
                        break;
                    }
                    Sleep(200);
                }

                if (!hooked)
                {
                    ::MessageBoxA(NULL, "add_remote_inline_hook error", "Warning", MB_OK);
                    ::TerminateProcess(main_thread_handle, 0);
                    return ERROR_ERRORS_ENCOUNTERED;
                }
            }
            main_thread_handle = NULL;
            process_handle = NULL;
        }
        VMP_VIRTUALIZATION_END();
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
                    if (*hook_addr != 0xE9 || !is_address_in_module(src_addr, dll_base) || *(src_addr + 0x2) == 0xE9)
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

    bool init_create_process_hook()
    {
        //登录器入口
        void* create_process_internal_w = IMPORT(L"kernelbase.dll", CreateProcessInternalW);
        if (!create_process_internal_w)
            create_process_internal_w = IMPORT(L"kernel32.dll", CreateProcessInternalW);
        LightHook::HookMgr::instance().add_inline_hook(create_process_internal_w, &HookProc::CreateProcessInternalW, &HookProc::create_process_internal_w);
        void* resume_thread = IMPORT(L"kernel32.dll", ResumeThread);
        LightHook::HookMgr::instance().add_inline_hook(resume_thread, &HookProc::ResumeThread, &HookProc::resume_thread);
        g_thread_group.create_thread(&check_hook_vaild_routine);
        return true;
    }
}
