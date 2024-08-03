#include "pch.h"
namespace ApiResolverInline
{
#include "api_resolver.h"
#include "api_resolver.cpp"
}
#include <intrin.h>


using namespace ApiResolverInline;
using namespace ApiResolver;
#include "utils.h"
using NTSTATUS = long;

NTSYSCALLAPI
NTSTATUS
NTAPI
NtUnmapViewOfSection(
    _In_ HANDLE ProcessHandle,
    _In_opt_ PVOID BaseAddress
);

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
    DEFINEAPI(NtUnmapViewOfSection);

    VirtualAlloc = IMPORT(L"kernel32.dll", VirtualAlloc);
    GetModuleHandleA = IMPORT(L"kernel32.dll", GetModuleHandleA);
    LoadLibraryA = IMPORT(L"kernel32.dll", LoadLibraryA);
    VirtualFree = IMPORT(L"kernel32.dll", VirtualFree);
    GetProcAddress = IMPORT(L"kernel32.dll", GetProcAddress);
    IsBadWritePtr = IMPORT(L"kernel32.dll", IsBadWritePtr);
    NtUnmapViewOfSection = IMPORT(L"ntdll.dll", NtUnmapViewOfSection);

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
            if (instance && *instance && ApiResolver::hash(import_module_name) == CT_HASH("NewClient.dll"))
            {
                import_module_handle = *instance;
            }
            else
            {
                import_module_handle = LoadLibraryA(import_module_name);
                if (!import_module_handle)
                {
                    VirtualFree(image_base, nt_header->OptionalHeader.SizeOfImage, MEM_RELEASE);
                    return ERROR_MOD_NOT_FOUND;
                }
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
                VirtualFree(image_base, nt_header->OptionalHeader.SizeOfImage, MEM_RELEASE);
                return ERROR_FUNCTION_NOT_CALLED;
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

extern "C" HINSTANCE reflective_loader_entry()
{
    unsigned char* image = (unsigned char*)get_eip();
    while (!(*(uint16_t*)(&image[0]) == 0xBEEF && *(uint16_t*)(&image[2]) == 0xDEAD))
    {
        image++;
    }
    image += 4;
    HINSTANCE dll = NULL;

    if (peload(image, sizeof(IMAGE_DOS_HEADER), &dll, NULL) == ERROR_SUCCESS && dll)
    {
        enable_seh_on_shellcode();
        execute_tls_callback(dll, DLL_PROCESS_ATTACH, 0);
        execute_entrypoint(dll, DLL_PROCESS_ATTACH, 0);
        return dll;
    }
    return NULL;
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