<<<<<<< HEAD
#include "Tools/Packer/loader.h"
#include "Lightbone/peloader.cpp"

void __stdcall reflective_load(share_data_ptr_t param) noexcept
{
    if (param->magic != kShareDateMagicKey && param->magic != 0x90909090)
    {
        param = (share_data_ptr_t)((char*)param - offsetof(share_data_t, call_opcode));
    }
    HINSTANCE dll = NULL;
    if (peload((char*)param + sizeof(share_data_t), sizeof(IMAGE_DOS_HEADER), &dll, NULL) == ERROR_SUCCESS && dll)
    {
        enable_seh_on_shellcode();
        execute_tls_callback(dll, DLL_PROCESS_ATTACH, 0);
        execute_entrypoint(dll, DLL_PROCESS_ATTACH, 0);
        decltype(&client_entry) dll_export = (decltype(&client_entry))ApiResolver::get_proc_address(dll, CT_HASH("client_entry"));
        param->stage = 1;
        if (dll_export)
            dll_export(param);
        uint32_t oep = param->oep;
        if (oep)
        {
            if (param->stub_size)
            {
                auto VirtualFree = IMPORT(L"kernel32.dll", VirtualFree);
                VirtualFree(param, param->stub_size, MEM_RELEASE);
            }
            *(uint32_t*)_AddressOfReturnAddress() = oep;
        }
    }
=======
#include "Tools/Packer/loader.h"
#include "Lightbone/peloader.cpp"

void __stdcall reflective_load(share_data_ptr_t param) noexcept
{
    if (param->magic != kShareDateMagicKey && param->magic != 0x90909090)
    {
        param = (share_data_ptr_t)((char*)param - offsetof(share_data_t, call_opcode));
    }
    HINSTANCE dll = NULL;
    if (peload((char*)param + sizeof(share_data_t), sizeof(IMAGE_DOS_HEADER), &dll, NULL) == ERROR_SUCCESS && dll)
    {
        enable_seh_on_shellcode();
        execute_tls_callback(dll, DLL_PROCESS_ATTACH, 0);
        execute_entrypoint(dll, DLL_PROCESS_ATTACH, 0);
        decltype(&client_entry) dll_export = (decltype(&client_entry))ApiResolver::get_proc_address(dll, CT_HASH("client_entry"));
        param->stage = 1;
        if (dll_export)
            dll_export(param);
        uint32_t oep = param->oep;
        if (oep)
        {
            if (param->stub_size)
            {
                auto VirtualFree = IMPORT(L"kernel32.dll", VirtualFree);
                VirtualFree(param, param->stub_size, MEM_RELEASE);
            }
            *(uint32_t*)_AddressOfReturnAddress() = oep;
        }
    }
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9
}