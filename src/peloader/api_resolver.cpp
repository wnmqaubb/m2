#include "api_resolver.h"
#include "peloader.h"
#include "peb.h"

#pragma code_seg(".shell")
//需要关闭增量链接，JMC，__RTC_CheckEsp

void* ApiResolver::get_proc_address(HMODULE module, uint32_t function_name_hash)
{
	if (module == NULL) return nullptr;
	PIMAGE_NT_HEADERS _nt_header = GET_IMAGE_NT_HEADER(module);
	PIMAGE_EXPORT_DIRECTORY _export = (PIMAGE_EXPORT_DIRECTORY)GET_DATA_DIRECTORY_VA(module, _nt_header, IMAGE_DIRECTORY_ENTRY_EXPORT);
	uint32_t* funcs = (uint32_t*)RVA2VA(module, _export->AddressOfFunctions);
	uint16_t* ordinals = (uint16_t*)RVA2VA(module, _export->AddressOfNameOrdinals);
	uint32_t* names = (uint32_t*)RVA2VA(module, _export->AddressOfNames);
	for (int n = 0; n < _export->NumberOfNames; n++)
	{
		LPCSTR name = (LPCSTR)RVA2VA(module, names[n]);
		if (hash(name) == function_name_hash)
		{
			return RVA2VA(module, funcs[ordinals[n]]);
		}
	}
	return nullptr;
}

HMODULE ApiResolver::get_module_handle(uint32_t module_name_hash)
{
    _PMPEB peb = (_PMPEB)__readfsdword(0x30);
    PLIST_ENTRY entry = &(peb->pLdr)->InMemoryOrderModuleList;
    for (PMLDR_DATA_TABLE_ENTRY module = (PMLDR_DATA_TABLE_ENTRY)entry->Flink;
        module && module != (PMLDR_DATA_TABLE_ENTRY)entry;
        module = *(PMLDR_DATA_TABLE_ENTRY*)module)
    {
        const uint32_t _hash = hash(module->BaseDllName.pBuffer);
        if (_hash == module_name_hash)
        {
            return (HMODULE)module->DllBase;
        }
    }
    return NULL;
}

void* ApiResolver::get_proc_address(uint32_t module_name_hash, uint32_t function_name_hash)
{
	return get_proc_address(get_module_handle(module_name_hash), function_name_hash);
}

#define MAKE_API(dll,x) api->x = (decltype(api->x))get_proc_address(dll, CT_HASH(#x))

bool ApiResolver::get_kernel32_function(PKERNEL32API api)
{
	HMODULE kernel32 = get_module_handle(CT_HASH(L"kernel32.dll"));
	MAKE_API(kernel32, GetProcAddress);
	MAKE_API(kernel32, GetModuleHandleA);
	MAKE_API(kernel32, VirtualAlloc);
	MAKE_API(kernel32, VirtualFree);
	MAKE_API(kernel32, CloseHandle);
	MAKE_API(kernel32, LoadLibraryA);
	MAKE_API(kernel32, GetSystemInfo);
    MAKE_API(kernel32, VirtualProtect);
    MAKE_API(kernel32, FindResourceA);
    MAKE_API(kernel32, LoadResource);
    MAKE_API(kernel32, SizeofResource);
    MAKE_API(kernel32, OutputDebugStringA);
    MAKE_API(kernel32, CreateFileA);
    MAKE_API(kernel32, ReadFile);
    MAKE_API(kernel32, WriteFile);
    MAKE_API(kernel32, GetFileSize);
    MAKE_API(kernel32, MessageBoxA);
    MAKE_API(kernel32, Sleep);
	return api->GetProcAddress && api->GetModuleHandleA && api->VirtualAlloc &&
		api->VirtualFree && api->CloseHandle && api->LoadLibraryA && api->GetSystemInfo &&
        api->VirtualProtect && api->FindResourceA && api->LoadResource && api->OutputDebugStringA &&
        api->SizeofResource;
}

bool ApiResolver::get_ntdll_function(PNTDLLAPI api)
{
    HMODULE ntdll = get_module_handle(CT_HASH(L"ntdll.dll"));
    MAKE_API(ntdll, NtCreateSection);
    MAKE_API(ntdll, NtMapViewOfSection);
    MAKE_API(ntdll, NtUnmapViewOfSection);
    MAKE_API(ntdll, NtClose);
    MAKE_API(ntdll, RtlImageNtHeader);
    return api->NtCreateSection && api->NtMapViewOfSection && api->NtUnmapViewOfSection
        && api->NtClose && api->RtlImageNtHeader;
}

namespace AntiCommon
{
	bool is_in_virtual_machine();
}

#pragma code_seg()