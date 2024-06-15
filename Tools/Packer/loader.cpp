#include "Lightbone/windows_internal.h"
namespace ApiResolverInline
{
#include "Lightbone/api_resolver.h"
#include "Lightbone/api_resolver.cpp"
}
#include <intrin.h>

using namespace ApiResolverInline;
using namespace ApiResolver;
#include "Lightbone/utils.h"
#include "loader.h"
using namespace Utils::Crypto;
using NTSTATUS = long;

NTSYSCALLAPI
NTSTATUS
NTAPI
NtUnmapViewOfSection(
    _In_ HANDLE ProcessHandle,
    _In_opt_ PVOID BaseAddress
);

#ifdef IMPORT
#undef IMPORT
#endif

#define IMPORT(module_name,func_name) (decltype(&::func_name))get_proc_address(CT_HASH(module_name), CT_HASH(#func_name))

typedef VOID(*PFNExeMain)();
typedef BOOL(WINAPI* PFNDllMain)(HINSTANCE hModule, DWORD dwReason, LPVOID); /* DllMain */
uint32_t peload(void* buffer, size_t size, HINSTANCE* instance, void* params);
void execute_tls_callback(HINSTANCE instance, uint32_t reason, void* param);
void execute_entrypoint(HINSTANCE instance, uint32_t reason, void* param);
void enable_seh_on_shellcode();


uint32_t peload(void* buffer, size_t size, HINSTANCE* instance, void* params)
{
    DEFINEAPI(VirtualAlloc);
    DEFINEAPI(GetModuleHandleA);
    DEFINEAPI(LoadLibraryA);
    DEFINEAPI(VirtualFree);
    DEFINEAPI(GetProcAddress);
    DEFINEAPI(IsBadWritePtr);

    VirtualAlloc = IMPORT(L"kernel32.dll", VirtualAlloc);
    GetModuleHandleA = IMPORT(L"kernel32.dll", GetModuleHandleA);
    LoadLibraryA = IMPORT(L"kernel32.dll", LoadLibraryA);
    VirtualFree = IMPORT(L"kernel32.dll", VirtualFree);
    GetProcAddress = IMPORT(L"kernel32.dll", GetProcAddress);
    IsBadWritePtr = IMPORT(L"kernel32.dll", IsBadWritePtr);

    if (size < sizeof(IMAGE_DOS_HEADER))
        return ERROR_INVALID_DATA;
    PIMAGE_DOS_HEADER dos_header = get_image_dos_header(buffer);
    if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
        return ERROR_INVALID_MODULETYPE;
    PIMAGE_NT_HEADERS nt_header = get_image_nt_header(buffer);

    if (params)
    {
        int n = nt_header->FileHeader.NumberOfSections - 1;
        IMAGE_SECTION_HEADER* section = image_first_section(nt_header);
        size_t copy_size = section[n].SizeOfRawData;
        *(uintptr_t*)params = (uintptr_t)buffer + section[n].PointerToRawData + copy_size;
    }

    void* image_base = nullptr;

    do
    {
        image_base = VirtualAlloc(NULL,
            nt_header->OptionalHeader.SizeOfImage,
            MEM_RESERVE | MEM_COMMIT,
            PAGE_EXECUTE_READWRITE);
    } while (0);


    if (!image_base)
        return ERROR_OUTOFMEMORY;

    __movsb((BYTE*)image_base, (BYTE*)buffer, nt_header->OptionalHeader.SizeOfHeaders);

    IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(nt_header);
    for (size_t n = 0; n < nt_header->FileHeader.NumberOfSections; n++)
    {
        size_t copy_size = section[n].SizeOfRawData;
        if (copy_size == 0)
        {
            if (section[n].Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA)
            {
                copy_size = nt_header->OptionalHeader.SizeOfInitializedData;
            }
            else if (section[n].Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA)
            {
                copy_size = nt_header->OptionalHeader.SizeOfUninitializedData;
            }
            else
            {
                continue;
            }
        }

        __movsb(rva2va<unsigned char*>(image_base, section[n].VirtualAddress),
            (BYTE*)buffer + section[n].PointerToRawData,
            copy_size);

    }

    for (PIMAGE_IMPORT_DESCRIPTOR import_desc = get_image_import_directory(image_base);
        import_desc->Name;
        import_desc++)
    {
        char* import_module_name = rva2va<char*>(image_base, import_desc->Name);
        HMODULE import_module_handle = GetModuleHandleA(import_module_name);
        if (!import_module_handle)
        {
            import_module_handle = LoadLibraryA(import_module_name);
            if (!import_module_handle)
            {
                VirtualFree(image_base, nt_header->OptionalHeader.SizeOfImage, MEM_RELEASE);
                return ERROR_MOD_NOT_FOUND;
            }
        }

        PIMAGE_THUNK_DATA iat = rva2va<PIMAGE_THUNK_DATA>(image_base, import_desc->FirstThunk);
        PIMAGE_THUNK_DATA thunk = import_desc->OriginalFirstThunk ?
            rva2va<PIMAGE_THUNK_DATA>(image_base, import_desc->OriginalFirstThunk) :
            rva2va<PIMAGE_THUNK_DATA>(image_base, import_desc->FirstThunk);

        for (;
            iat && thunk && thunk->u1.AddressOfData;
            iat++, thunk++)
        {
            auto function_name = (LPCSTR)((rva2va<PIMAGE_IMPORT_BY_NAME>(image_base, thunk->u1.AddressOfData))->Name);
            iat->u1.Function = (ULONG_PTR)GetProcAddress(import_module_handle,
                IMAGE_SNAP_BY_ORDINAL(thunk->u1.Ordinal) ?
                (LPCSTR)IMAGE_ORDINAL(thunk->u1.Ordinal)
                :
                function_name
            );
            if (iat->u1.Function == NULL)
            {
                iat->u1.Function = (ULONG_PTR)get_proc_address(import_module_handle, hash(function_name));
            }
            if (iat->u1.Function == NULL)
            {
                __debugbreak();
            }
        }
    }


    //重定位修正
    if (get_data_directory(nt_header, IMAGE_DIRECTORY_ENTRY_BASERELOC).Size)
    {
        ULONG_PTR dist = (ULONG_PTR)image_base - nt_header->OptionalHeader.ImageBase;

        for (PIMAGE_BASE_RELOCATION reloc_desc = (PIMAGE_BASE_RELOCATION)get_data_directory_va(image_base, IMAGE_DIRECTORY_ENTRY_BASERELOC);
            reloc_desc->SizeOfBlock;
            reloc_desc = (PIMAGE_BASE_RELOCATION)((BYTE*)reloc_desc + reloc_desc->SizeOfBlock))
        {

            PTYPEOFFSET offset = (PTYPEOFFSET)&reloc_desc[1];
            for (size_t n = 0; n < get_reloc_desc_typeoffset_size(reloc_desc); n++)
            {
                ULONG_PTR* fix = (ULONG_PTR*)rva2va(image_base, reloc_desc->VirtualAddress + offset[n].offset);
                if (IsBadWritePtr(fix, 4))
                    continue;
                if (offset[n].Type == IMAGE_REL_BASED_DIR64 ||
                    offset[n].Type == IMAGE_REL_BASED_HIGHLOW)
                {
                    *fix += dist;
                }
                else if (offset[n].Type == IMAGE_REL_BASED_HIGHLOW)
                {
                    *fix += HIWORD(dist);
                }
                else if (offset[n].Type == IMAGE_REL_BASED_LOW)
                {
                    *fix += LOWORD(dist);
                }
            }
        }
    }

    //修正ImageBase
    get_image_nt_header(image_base)->OptionalHeader.ImageBase = (ULONG_PTR)image_base;

    *instance = (HINSTANCE)image_base;
    return ERROR_SUCCESS;
}

void execute_tls_callback(HINSTANCE instance, uint32_t reason, void* param)
{
    //tlscallback调用
    PIMAGE_NT_HEADERS nt_header = get_image_nt_header(instance);

    if (get_data_directory(nt_header, IMAGE_DIRECTORY_ENTRY_TLS).Size)
    {
        PIMAGE_TLS_DIRECTORY tls_dir = get_image_tls_directory(instance);
        PIMAGE_TLS_CALLBACK* callback = (PIMAGE_TLS_CALLBACK*)tls_dir->AddressOfCallBacks;
        while (callback && *callback)
        {
            (*callback)(instance, reason, param);
            callback++;
        }
    }
}

void execute_entrypoint(HINSTANCE instance, uint32_t reason, void* param)
{

    PIMAGE_NT_HEADERS nt_header = get_image_nt_header(instance);
    void* entrypoint = rva2va<void*>(instance, nt_header->OptionalHeader.AddressOfEntryPoint);
    if (nt_header->FileHeader.Characteristics & IMAGE_FILE_DLL)
    {
        ((PFNDllMain)entrypoint)(instance, DLL_PROCESS_ATTACH, 0);
    }
    else
    {
        ((PFNExeMain)entrypoint)();
    }

}
#pragma optimize("",off)
void* get_eip()
{
    return _ReturnAddress();
}
#pragma optimize("",on)


PIMAGE_SECTION_HEADER get_current_section(uint32_t base_module)
{
    uint32_t return_rva = (uint32_t)_ReturnAddress() - base_module;
    PIMAGE_NT_HEADERS nt_header = get_image_nt_header(base_module);
    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(nt_header);
    for (size_t n = 0; n < nt_header->FileHeader.NumberOfSections; n++)
    {
        const uint32_t virtual_size = align(section[n].Misc.VirtualSize,
            nt_header->OptionalHeader.SectionAlignment);
        if (section[n].VirtualAddress <= return_rva
            && return_rva <= (section[n].VirtualAddress + virtual_size))
        {
            return &section[n];
        }
    }
    return nullptr;
}
extern "C" void __stdcall stub_entry(share_data_ptr_t param)
{
    auto GetModuleHandleA = IMPORT(L"kernel32.dll", GetModuleHandleA);
    auto VirtualProtect = IMPORT(L"kernel32.dll", VirtualProtect);

    void* image = GetModuleHandleA(NULL);
    crypt_code_ptr_t crypt_code = rva2va<crypt_code_ptr_t>(image, param->crypt_code_rva);
    DWORD old_protect = 0;
    VirtualProtect(rva2va<void*>(image, param->crypt_code_rva), param->crypt_code_size, PAGE_READWRITE, &old_protect);
    xor_buffer(rva2va<void*>(image, param->crypt_code_rva), param->crypt_code_size, param->xor_key);
    VirtualProtect(rva2va<void*>(image, param->crypt_code_rva), param->crypt_code_size, old_protect, &old_protect);

    const size_t reloc_size = crypt_code->reloc_size;
    const size_t bind_dll_size = crypt_code->bind_dll_size;
    const size_t text_code_size = crypt_code->text_code_size;
    const size_t text_code_offset = offsetof(CryptCode, reloc) + sizeof(RelocInfo) * reloc_size;
    const size_t bind_dll_offset = text_code_offset + text_code_size;
    VirtualProtect(rva2va<void*>(image, param->current_section_rva), param->current_section_virtual_size, PAGE_EXECUTE_READWRITE, &old_protect);
    __movsb(rva2va<unsigned char*>(image, param->current_section_rva), rva2va<unsigned char*>(image, param->crypt_code_rva + text_code_offset), text_code_size);
    ULONG_PTR dist = (ULONG_PTR)image - param->origin_image_base;
    if (dist)
    {
        for (int n = 0; n < reloc_size; n++)
        {
            ULONG_PTR* fix = rva2va<ULONG_PTR*>(image, (crypt_code->reloc[n].rva^param->xor_key));
            const uint32_t type = crypt_code->reloc[n].type^param->xor_key;
            if (type == IMAGE_REL_BASED_DIR64 ||
                type == IMAGE_REL_BASED_HIGHLOW)
            {
                *fix += dist;
            }
            else if (type == IMAGE_REL_BASED_HIGH)
            {
                *fix += HIWORD(dist);
            }
            else if (type == IMAGE_REL_BASED_LOW)
            {
                *fix += LOWORD(dist);
            }
        }
    }

    VirtualProtect(rva2va<void*>(image, param->current_section_rva), param->current_section_virtual_size, old_protect, &old_protect);

    if (bind_dll_size)
    {
        HINSTANCE dll = NULL;
        if (peload(rva2va<void*>(image, param->crypt_code_rva + bind_dll_offset), sizeof(IMAGE_DOS_HEADER), &dll, NULL) == ERROR_SUCCESS && dll)
        {
            enable_seh_on_shellcode();
            execute_tls_callback(dll, DLL_PROCESS_ATTACH, 0);
            execute_entrypoint(dll, DLL_PROCESS_ATTACH, 0);
            decltype(&client_entry) dll_export = (decltype(&client_entry))get_proc_address(dll, CT_HASH("client_entry"));
            if (dll_export)
                dll_export(param);
        }
    }

    VirtualProtect(rva2va<void*>(image, param->crypt_code_rva), param->crypt_code_size, PAGE_READWRITE, &old_protect);
    xor_buffer(rva2va<void*>(image, param->crypt_code_rva), param->crypt_code_size, param->xor_key);
    VirtualProtect(rva2va<void*>(image, param->crypt_code_rva), param->crypt_code_size, old_protect, &old_protect);
    PFNExeMain oep = rva2va<PFNExeMain>(image, param->oep);
    oep();
}

extern "C" void __stdcall loader_entry(uint8_t* eip)
{
    auto GetModuleHandleA = IMPORT(L"kernel32.dll", GetModuleHandleA);
    auto VirtualAlloc = IMPORT(L"kernel32.dll", VirtualAlloc);
    share_data_ptr_t param = (share_data_ptr_t)(eip - offsetof(share_data_t, call_opcode));
    void* image = GetModuleHandleA(NULL);
    unsigned char* stub = (unsigned char*)VirtualAlloc(NULL, param->stub_size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    PIMAGE_SECTION_HEADER current_section = get_current_section((uint32_t)image);
    __movsb(stub, rva2va<unsigned char*>(rva2va(image, current_section->VirtualAddress), param->stub_rva), param->stub_size);
    decltype(&stub_entry) stub_entry_routine = rva2va<decltype(&stub_entry)>(stub, param->stub_entry_rva);
    share_data_ptr_t stub_data = rva2va<share_data_ptr_t>(stub, ((uint32_t)param - rva2va(rva2va(image, current_section->VirtualAddress), param->stub_rva)));
    stub_data->current_section_rva = current_section->VirtualAddress;
    stub_data->current_section_virtual_size = align(current_section->Misc.VirtualSize,
        get_image_nt_header(image)->OptionalHeader.SectionAlignment);
    stub_entry_routine(stub_data);
}

void enable_seh_on_shellcode()
{
    //Vista 以上进程IMAGE在启动时如果有DEP,内核会设置Permanent属性永久生效，无法再次修改
    //https://sudonull.com/post/97504-Exceptions-for-hardcore-Features-of-processing-executions-in-dynamically-allocated-code-Blog-Company
    ULONG ExecuteFlags = MEM_EXECUTE_OPTION_ENABLE;
    auto NtSetInformationProcess = IMPORT(L"ntdll.dll", NtSetInformationProcess);
    ExecuteFlags = MEM_EXECUTE_OPTION_ENABLE;
    NtSetInformationProcess(
        NtCurrentProcess(),
        ProcessExecuteFlags,
        &ExecuteFlags,
        sizeof(ExecuteFlags));
    ExecuteFlags = MEM_EXECUTE_OPTION_IMAGE_DISPATCH_ENABLE;
    NtSetInformationProcess(
        NtCurrentProcess(),
        ProcessExecuteFlags,
        &ExecuteFlags,
        sizeof(ExecuteFlags));
    ExecuteFlags = MEM_EXECUTE_OPTION_DISABLE_EXCEPTIONCHAIN_VALIDATION;
    NtSetInformationProcess(
        NtCurrentProcess(),
        ProcessExecuteFlags,
        &ExecuteFlags,
        sizeof(ExecuteFlags));
}