#include "peloader.h"
#include "api_resolver.h"
#include <intrin.h>
#include <fstream>


#pragma comment(linker, "/merge:.shelld=.shell") 
#pragma comment(linker, "/section:.shell,RWE")

#pragma data_seg(".shelld")
PELoader::ShareData share = { PELOADER_MAGICKEY,
{0xE8,0,0,0,0},
0xE8,
(uint32_t)&PELoader::entrypoint - (uint32_t)&share.entry_point_dist - 4,
0xC3 };
#pragma data_seg()

#pragma code_seg(".shell")

wchar_t* string2wstring(char* str, UINT CodePage)
{
    int iBuffSize = ::MultiByteToWideChar(CodePage, 0, str, -1, NULL, 0);
    if(iBuffSize > 0)
    {
        std::unique_ptr<wchar_t[]> wszString = std::make_unique<wchar_t[]>(iBuffSize + 1);
        int nChars = ::MultiByteToWideChar(CodePage, 0, str, -1, wszString.get(), iBuffSize);
        nChars = nChars < iBuffSize ? nChars : iBuffSize;
        wszString[nChars] = 0;
        return wszString.get();
    }
    return L"";
}

char* wstring2string(wchar_t* wstr, UINT CodePage)
{
    int iBuffSize = ::WideCharToMultiByte(CodePage, 0, wstr, -1, NULL, 0, NULL, false);
    if(iBuffSize > 0)
    {
        std::unique_ptr<char[]> wszString = std::make_unique<char[]>(iBuffSize + 1);
        ::WideCharToMultiByte(CodePage, 0, wstr, -1, wszString.get(), iBuffSize, NULL, false);
        return wszString.get();
    }
    return "";
}

void PELoader::xor_buffer(void* data, size_t size, const uint32_t xor_key)
{
    const int length = size / sizeof(xor_key);
    uint32_t* buffer = (uint32_t*)data;
    for (int n = 0; n < length; n++)
    {
        buffer[n] ^= xor_key;
    }
    const int last_bytes_size = size % sizeof(xor_key);
    if (last_bytes_size)
    {
        uint32_t last_dword = 0;
        __movsb((unsigned char*)&last_dword, (unsigned char*)&buffer[length], last_bytes_size);
        last_dword ^= xor_key;
        __movsb((unsigned char*)&buffer[length], (unsigned char*)&last_dword, last_bytes_size);
    }
}

void obfuscation1()
{
    __asm
    {
        xor eax, eax
        xor eax, eax
        xor eax, eax
    }
}

unsigned int PELoader::aphash(const unsigned char *str, uint32_t len)
{
    unsigned int hash = 0;
    for (uint32_t i = 0; i < len; i++)
    {
        if ((i & 1) == 0)
        {
            hash ^= ((hash << 7) ^ (str[i]) ^ (hash >> 3));
        }
        else
        {
            hash ^= (~((hash << 11) ^ (str[i]) ^ (hash >> 5)));
        }
    }
    return (hash & 0x7FFFFFFF);
}

bool __stdcall PELoader::write_file(LPCSTR filename, LPVOID buffer, SIZE_T file_size)
{
    PELoaderAPI api;
    ApiResolver::get_kernel32_function(&api.kernel32);

    DWORD bytes_of_write = 0;
    bool should_write_file = false;
    HANDLE file_handle = api.kernel32.CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file_handle != INVALID_HANDLE_VALUE)
    {
        DWORD file_size_ = api.kernel32.GetFileSize(file_handle, NULL);
        LPVOID buffer_ = api.kernel32.VirtualAlloc(NULL, file_size_, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (!buffer_)
        {
            api.kernel32.CloseHandle(file_handle);
            char text[] = { 'O','u','t',' ','o','f',' ','M','e','m','o','r','y',0 };
            char caption[] = { 'E','r','r','o','r',0 };
            api.kernel32.MessageBoxA(NULL, text, caption, MB_OK);
            return FALSE;
        }
        DWORD bytes_of_read = 0;
        if (!api.kernel32.ReadFile(file_handle, buffer_, file_size_, &bytes_of_read, NULL))
        {
            api.kernel32.CloseHandle(file_handle);
            char text[] = { 'R','e','a','d',' ','o','c','c','u','p','a','t','e',0 };
            char caption[] = { 'E','r','r','o','r',0 };
            api.kernel32.MessageBoxA(NULL, text, caption, MB_OK);
            return FALSE;
        }
        DWORD file_hash = aphash((unsigned char*)buffer_, file_size_);
        api.kernel32.VirtualFree(buffer_, file_size_, MEM_RELEASE);
        if (aphash((unsigned char*)buffer, file_size) != file_hash)
        {
            should_write_file = true;
        }
        api.kernel32.CloseHandle(file_handle);
    }
    else
    {
        should_write_file = true;
    }

    if (should_write_file)
    {
        file_handle = api.kernel32.CreateFileA(filename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (file_handle == INVALID_HANDLE_VALUE)
        {
            char text[] = { 'W','r','i','t','e',' ','o','c','c','u','p','a','t','e',0 };
            char caption[] = { 'E','r','r','o','r',0 };
            api.kernel32.MessageBoxA(NULL, text, caption, MB_OK);
            return FALSE;
        }
        if (!api.kernel32.WriteFile(file_handle, buffer, file_size, &bytes_of_write, NULL))
        {
            api.kernel32.CloseHandle(file_handle);
            char text[] = { 'W','r','i','t','e',' ','o','c','c','u','p','a','t','e','2',0 };
            char caption[] = { 'E','r','r','o','r',0 };
            api.kernel32.MessageBoxA(NULL, text, caption, MB_OK);
            return FALSE;
        }
        api.kernel32.CloseHandle(file_handle);
    }
    return TRUE;
}

PIMAGE_SECTION_HEADER PELoader::get_current_section(uint32_t base_module)
{
    uint32_t return_rva = (uint32_t)_ReturnAddress() - base_module;
    PIMAGE_NT_HEADERS nt_header = GET_IMAGE_NT_HEADER(base_module);
    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(nt_header);
    for(size_t n = 0; n < nt_header->FileHeader.NumberOfSections; n++)
    {
        const uint32_t virtual_size = ALIGN(section[n].Misc.VirtualSize,
            nt_header->OptionalHeader.SectionAlignment);
        if(section[n].VirtualAddress <= return_rva
            && return_rva <= (section[n].VirtualAddress + virtual_size))
        {
            return &section[n];
        }
    }
    return nullptr;
}

char *__stdcall PELoader::memcpy(char *dst, const char* src, int cnt)
{
	char *ret = dst;
	if(dst >= src && dst <= src + cnt - 1)
	{
		dst = dst + cnt - 1;
		src = src + cnt - 1;
		while(cnt--)
			*dst-- = *src--;
	}
	else
	{
		while(cnt--)
			*dst++ = *src++;
	}

	return ret;
}

void __stdcall PELoader::get_game_path(char* gamepath)
{
	PELoaderAPI api;
	ApiResolver::get_kernel32_function(&api.kernel32);
	decltype(&::GetModuleFileNameA) GetModuleFileNameA = (decltype(&::GetModuleFileNameA))ApiResolver::get_proc_address(ApiResolver::get_module_handle(CT_HASH(L"kernel32.dll")), CT_HASH("GetModuleFileNameA"));
	char temp[MAX_PATH];
	DWORD size = GetModuleFileNameA(NULL, temp, MAX_PATH);
	char filename[] = {'c', 'l', 'i', 'e', 'n', 't', '.', 'd', 'l', 'l', 0};
	if(!size)
	{
		strcpy(gamepath, filename, 10);
		return;
	}
	char* p = temp + size;
	while(p > temp)
	{
		if(*p == '\\')
		{
			*++p = 0;
			break;
		}
		p--;
	}
	int strlen = p - temp;
	strcpy(gamepath, temp, strlen);
	strcpy(gamepath + strlen, filename, 10);
}

void __stdcall PELoader::stub_entry(ShareDataPtr data)
{
    PELoaderAPI api;
    ApiResolver::get_kernel32_function(&api.kernel32);
    void* image = api.kernel32.GetModuleHandleA(NULL);
    CryptCodePtr crypt_code = (CryptCodePtr)RVA2VA(image, data->crypt_code_rva);
    DWORD old_protect = 0;
    api.kernel32.VirtualProtect(RVA2VA(image, data->crypt_code_rva), data->crypt_code_size, PAGE_READWRITE,&old_protect);
    xor_buffer(RVA2VA(image, data->crypt_code_rva), data->crypt_code_size, data->xor_key);
    api.kernel32.VirtualProtect(RVA2VA(image, data->crypt_code_rva), data->crypt_code_size, old_protect, &old_protect);

    const size_t reloc_size = crypt_code->reloc_size;
    const size_t bind_dll_size = crypt_code->bind_dll_size;
    const size_t text_code_size = crypt_code->text_code_size;
    const size_t text_code_offset = offsetof(CryptCode, reloc) + sizeof(RelocInfo) * reloc_size;
    const size_t bind_dll_offset = text_code_offset + text_code_size;
    api.kernel32.VirtualProtect(RVA2VA(image, data->current_section_rva), data->current_section_virtual_size, PAGE_EXECUTE_READWRITE, &old_protect);
    __movsb(RVA2VA(image, data->current_section_rva), RVA2VA(image, data->crypt_code_rva + text_code_offset), text_code_size);
    ULONG_PTR dist = (ULONG_PTR)image - data->origin_image_base;
    if (dist)
    {
        for (int n = 0; n < reloc_size; n++)
        {
            ULONG_PTR* fix = (ULONG_PTR*)RVA2VA(image, (crypt_code->reloc[n].rva^data->xor_key));
            const uint32_t type = crypt_code->reloc[n].type^data->xor_key;
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

    api.kernel32.VirtualProtect(RVA2VA(image, data->current_section_rva), data->current_section_virtual_size, old_protect, &old_protect);
    
    if (bind_dll_size)
    {
		if(is_in_virtual_machine())
		{
			decltype(&::GetCursorPos) GetCursorPos = (decltype(&::GetCursorPos))ApiResolver::get_proc_address(ApiResolver::get_module_handle(CT_HASH(L"user32.dll")), CT_HASH("GetCursorPos"));
			POINT last_point = {};
			GetCursorPos(&last_point);
			while(true)
			{
				POINT point = {};
				GetCursorPos(&point);
				if(point.x != last_point.x || point.y != last_point.y)
				{
					break;
				}
				api.kernel32.Sleep(100);
			}
		}
        
        char procname[] = { 'A','n','t','i','C','h','e','a','t','E','n','t','r','y',0 };
		char gamepath[MAX_PATH];
		PELoader::get_game_path(gamepath);
        PELoader::write_file(gamepath, RVA2VA(image, data->crypt_code_rva + bind_dll_offset), bind_dll_size);
        typedef BOOL (APIENTRY* AntiCheatEntry)();
        AntiCheatEntry anti_cheat_entry = (AntiCheatEntry)api.kernel32.GetProcAddress(api.kernel32.LoadLibraryA(gamepath), procname);
        anti_cheat_entry();
    }

    api.kernel32.VirtualProtect(RVA2VA(image, data->crypt_code_rva), data->crypt_code_size, PAGE_READWRITE, &old_protect);
    xor_buffer(RVA2VA(image, data->crypt_code_rva), data->crypt_code_size, data->xor_key);
    api.kernel32.VirtualProtect(RVA2VA(image, data->crypt_code_rva), data->crypt_code_size, old_protect, &old_protect);

    PFNExeMain oep = (PFNExeMain)RVA2VA(image, data->oep);
    oep();
}

char *__stdcall PELoader::strcpy(char *dst, const char *src, int strlen)
{
    char *ret = dst;
    memcpy(dst, src, strlen + 1);
    return ret;
}

bool __stdcall PELoader::entrypoint(BYTE* eip)
{
    PELoaderAPI api;
    ApiResolver::get_kernel32_function(&api.kernel32);
    ShareDataPtr data = (ShareDataPtr)(eip - offsetof(ShareData, call_opcode));
    void* image = api.kernel32.GetModuleHandleA(NULL);
    unsigned char* stub = (unsigned char*)api.kernel32.VirtualAlloc(NULL, data->stub_size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    PIMAGE_SECTION_HEADER current_section = get_current_section((uint32_t)image);
    __movsb(stub, RVA2VA(RVA2VA(image, current_section->VirtualAddress), data->stub_rva), data->stub_size);
    decltype(&PELoader::stub_entry) stub_entry_routine = (decltype(&PELoader::stub_entry))RVA2VA(stub, data->stub_entry_rva);
    ShareDataPtr stub_data = (ShareDataPtr)RVA2VA(stub, ((unsigned char*)data - RVA2VA(RVA2VA(image, current_section->VirtualAddress), data->stub_rva)));
    stub_data->current_section_rva = current_section->VirtualAddress;
    stub_data->current_section_virtual_size = ALIGN(current_section->Misc.VirtualSize,
        GET_IMAGE_NT_HEADER(image)->OptionalHeader.SectionAlignment);
    stub_entry_routine(stub_data);
    /*obfuscation1();
    obfuscation1();
    obfuscation1();

    if (eip == nullptr)
    {
        PELoaderAPI api;
        if (!ApiResolver::get_ntdll_function(&api.ntdll) || !ApiResolver::get_kernel32_function(&api.kernel32))
            return false;
        char module_name[] = { 'c','l','i','e','n','t','.','d','l','l',0 };
        api.kernel32.LoadLibraryA(module_name);
        return false;
    }

    ShareDataPtr data = (ShareDataPtr)(eip - offsetof(ShareData, call_opcode));
    HINSTANCE instance = nullptr;
    PELoaderAPI api;
    if (!ApiResolver::get_ntdll_function(&api.ntdll) || !ApiResolver::get_kernel32_function(&api.kernel32))
        return false;

    char str_client[] = { 'C','L','I','E','N','T', 0 };
    char str_dll[] = { 'D','L','L', 0 };
    if (HRSRC rsrc_handle = api.kernel32.FindResourceA(NULL, str_client, str_dll))
    {
        HGLOBAL memory = api.kernel32.LoadResource(NULL, rsrc_handle);
        DWORD size = api.kernel32.SizeofResource(NULL, rsrc_handle);
        if (peload(&api,
            &instance,
            memory,
            size) != ERROR_SUCCESS)
            return false;
        PFNDllMain entrypoint = (PFNDllMain)ApiResolver::get_proc_address(instance, CT_HASH("UnPackerEntry"));
        entrypoint(instance, DLL_PROCESS_ATTACH, data);
    }
    else
    {
        if (peload(&api,
            &instance,
            RVA2VA(api.kernel32.GetModuleHandleA(NULL), get_current_section(&api)->VirtualAddress + data->dll_rva),
            data->dll_size) != ERROR_SUCCESS)
            return false;
        execute_tls_callback(&api, instance);
        execute_entrypoint(instance, data);
        PFNExeMain oep = (PFNExeMain)RVA2VA(api.kernel32.GetModuleHandleA(NULL), data->oep);
        oep();
    }*/
    return true;
}


PIMAGE_SECTION_HEADER PELoader::get_current_section(PELoaderAPIPtr api)
{
	uint32_t base_module = (uint32_t)api->kernel32.GetModuleHandleA(NULL);
	uint32_t return_rva = (uint32_t)_ReturnAddress() - base_module;
	PIMAGE_NT_HEADERS nt_header = GET_IMAGE_NT_HEADER(base_module);
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(nt_header);
	for (size_t n = 0; n < nt_header->FileHeader.NumberOfSections; n++)
	{
		const uint32_t virtual_size = ALIGN(section[n].Misc.VirtualSize,
			nt_header->OptionalHeader.SectionAlignment);
		if (section[n].VirtualAddress <= return_rva
			&& return_rva <= (section[n].VirtualAddress + virtual_size))
		{
			return &section[n];
		}
	}
	return nullptr;
}

uint32_t PELoader::peload(PELoaderAPIPtr api, HINSTANCE* instance, void* buffer, size_t size)
{
    if (size < sizeof(IMAGE_DOS_HEADER))
        return ERROR_INVALID_DATA;
    PIMAGE_DOS_HEADER dos_header = GET_IMAGE_DOS_HEADER(buffer);
    if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
        return ERROR_INVALID_MODULETYPE;
    PIMAGE_NT_HEADERS nt_header = GET_IMAGE_NT_HEADER(buffer);
    void* image_base = api->kernel32.VirtualAlloc(NULL,
        nt_header->OptionalHeader.SizeOfImage,
        MEM_RESERVE | MEM_COMMIT,
        PAGE_EXECUTE_READWRITE);
    if (!image_base)
        return ERROR_OUTOFMEMORY;

    //¿½±´PEÍ·
    __movsb((BYTE*)image_base, (BYTE*)buffer, nt_header->OptionalHeader.SizeOfHeaders);

    //sectionÕ¹¿ª
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
            else if (section[n].Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA)
            {
                copy_size = nt_header->OptionalHeader.SizeOfUninitializedData;
            }
            else
            {
                continue;
            }
        }

         __movsb(RVA2VA(image_base, section[n].VirtualAddress),
            (BYTE*)buffer + section[n].PointerToRawData,
            copy_size);
    }

    ////IATÐÞÕý
    //for (PIMAGE_IMPORT_DESCRIPTOR import_desc = (PIMAGE_IMPORT_DESCRIPTOR)GET_DATA_DIRECTORY_VA(image_base, nt_header, IMAGE_DIRECTORY_ENTRY_IMPORT);
    //    import_desc->Name;
    //    import_desc++)
    //{
    //    char* import_module_name = (char*)RVA2VA(image_base, import_desc->Name);
    //    HMODULE import_module_handle = api->kernel32.GetModuleHandleA(import_module_name);
    //    if (!import_module_handle)
    //    {
    //        import_module_handle = api->kernel32.LoadLibraryA(import_module_name);
    //        if (!import_module_handle)
    //        {
    //            api->kernel32.VirtualFree(image_base, nt_header->OptionalHeader.SizeOfImage, MEM_RELEASE);
    //            return ERROR_MOD_NOT_FOUND;
    //        }
    //    }

    //    PIMAGE_THUNK_DATA iat = (PIMAGE_THUNK_DATA)RVA2VA(image_base, import_desc->FirstThunk);
    //    PIMAGE_THUNK_DATA thunk = import_desc->OriginalFirstThunk ?
    //        (PIMAGE_THUNK_DATA)RVA2VA(image_base, import_desc->OriginalFirstThunk) :
    //        (PIMAGE_THUNK_DATA)RVA2VA(image_base, import_desc->FirstThunk);

    //    for (;
    //        iat && thunk && thunk->u1.AddressOfData;
    //        iat++, thunk++)
    //    {
    //        iat->u1.Function = (ULONG_PTR)api->kernel32.GetProcAddress(import_module_handle,
    //            IMAGE_SNAP_BY_ORDINAL(thunk->u1.Ordinal) ?
    //            (LPCSTR)IMAGE_ORDINAL(thunk->u1.Ordinal) /*Í¨ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Å»ï¿½È?/
    //            :
    //            (LPCSTR)(((PIMAGE_IMPORT_BY_NAME)RVA2VA(image_base, thunk->u1.AddressOfData))->Name)/*Í¨ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½È¡*/
    //        );
    //    }
    //}

    //ÖØ¶¨Î»ÐÞÕý
    if (GET_DATA_DIRECTORY(nt_header, IMAGE_DIRECTORY_ENTRY_BASERELOC).Size)
    {
        ULONG_PTR dist = (ULONG_PTR)image_base - nt_header->OptionalHeader.ImageBase;
        for (PIMAGE_BASE_RELOCATION reloc_desc = (PIMAGE_BASE_RELOCATION)GET_DATA_DIRECTORY_VA(image_base, nt_header, IMAGE_DIRECTORY_ENTRY_BASERELOC);
            reloc_desc->SizeOfBlock;
            reloc_desc = (PIMAGE_BASE_RELOCATION)((BYTE*)reloc_desc + reloc_desc->SizeOfBlock))
        {
            PTYPEOFFSET offset = (PTYPEOFFSET)&reloc_desc[1];
            for (size_t n = 0; n < GET_RELOC_DESC_TYPEOFFSET_SIZE(reloc_desc); n++)
            {
                ULONG_PTR* fix = (ULONG_PTR*)RVA2VA(image_base, reloc_desc->VirtualAddress + offset[n].offset);
                if (offset[n].Type == IMAGE_REL_BASED_DIR64 ||
                    offset[n].Type == IMAGE_REL_BASED_HIGHLOW)
                {
                    *fix += dist;
                }
                else if (offset[n].Type == IMAGE_REL_BASED_HIGH)
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

    //ÐÞÕýImageBase
    GET_IMAGE_NT_HEADER(image_base)->OptionalHeader.ImageBase = (ULONG_PTR)image_base;
    
    *instance = (HINSTANCE)image_base;
    return ERROR_SUCCESS;
}

bool PELoader::is_in_virtual_machine()
{
    int cpuinfo[4] = {-1};
    __cpuid(cpuinfo, 1);
    //ecx >> 31
    if((cpuinfo[2] >> 31) & 1)
        return true;
    __cpuid(cpuinfo, 0x40000000);
    char szHypervisorVendor[0x40];
    __stosb((unsigned char*)szHypervisorVendor, 0, sizeof(szHypervisorVendor));
    __movsb((unsigned char*)szHypervisorVendor, (const unsigned char*)&cpuinfo[1], 12);
    uint32_t hash = ApiResolver::hash(szHypervisorVendor);
    return hash == CT_HASH("KVMKVMKVM") || hash == CT_HASH("Microsoft Hv")
        || hash == CT_HASH("VMwareVMware") || hash == CT_HASH("XenVMMXenVMM")
        || hash == CT_HASH("prl hyperv  ") || hash == CT_HASH("VBoxVBoxVBox");
}

void PELoader::execute_tls_callback(PELoaderAPIPtr api, HINSTANCE instance)
{
    DWORD* thread_local_storage = (DWORD*)__readfsdword(0x2C);
    //tlscallbackµ÷ÓÃ
    PIMAGE_NT_HEADERS nt_header = GET_IMAGE_NT_HEADER(instance);
    if (GET_DATA_DIRECTORY(nt_header, IMAGE_DIRECTORY_ENTRY_TLS).Size)
    {
        PIMAGE_TLS_DIRECTORY tls_dir = (PIMAGE_TLS_DIRECTORY)GET_DATA_DIRECTORY_VA(instance, nt_header, IMAGE_DIRECTORY_ENTRY_TLS);
        DWORD tlsindex = *(DWORD*)RVA2VA(instance, tls_dir->AddressOfIndex - nt_header->OptionalHeader.ImageBase);
        
        DWORD tls_data_size = tls_dir->EndAddressOfRawData - tls_dir->StartAddressOfRawData;
        LPVOID tls_index_new_heap = api->kernel32.VirtualAlloc(NULL, sizeof(DWORD) + tls_data_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);       
        if (!tls_index_new_heap)
        {
            return;
        }
        __movsb((BYTE*)tls_index_new_heap + sizeof(DWORD),
            RVA2VA(instance, tls_dir->StartAddressOfRawData - nt_header->OptionalHeader.ImageBase),
            tls_data_size);
        *(DWORD*)(tls_index_new_heap) = (DWORD)tls_index_new_heap + sizeof(DWORD);
        thread_local_storage[tlsindex] = (DWORD)tls_index_new_heap;

       PIMAGE_TLS_CALLBACK* callback = (PIMAGE_TLS_CALLBACK*)RVA2VA(instance, tls_dir->AddressOfCallBacks - nt_header->OptionalHeader.ImageBase);
        while (callback && *callback)
        {
            (*callback)(instance, DLL_PROCESS_ATTACH, 0);
            callback++;
        }
    }
}

PIMAGE_SECTION_HEADER PELoader::get_shell_section()
{
    return get_current_section();
}

void PELoader::execute_entrypoint(HINSTANCE instance, LPVOID param)
{
    PIMAGE_NT_HEADERS nt_header = GET_IMAGE_NT_HEADER(instance);
    void* entrypoint = RVA2VA(instance, GET_IMAGE_NT_HEADER(instance)->OptionalHeader.AddressOfEntryPoint);
    if(nt_header->FileHeader.Characteristics & IMAGE_FILE_DLL)
    {
        ((PFNDllMain)entrypoint)(instance, DLL_PROCESS_ATTACH, param);
    }
    else
    {
        ((PFNExeMain)entrypoint)();
    }

}
#pragma code_seg()

char* PELoader::loadfile(std::string& path, size_t* lpsize)
{
    std::streampos size;
    std::fstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
    if (file.is_open())
    {
        size = file.tellg();
        *lpsize = size;
        char* buffer = new char[size];
        file.seekg(0, std::ios::beg);
        file.read(buffer, size);
        file.close();
        return buffer;
    }
    *lpsize = 0;
    return nullptr;

}

PIMAGE_SECTION_HEADER PELoader::get_current_section()
{
    uint32_t base_module = (uint32_t)::GetModuleHandleA(NULL);
    uint32_t return_rva = (uint32_t)_ReturnAddress() - base_module;
    PIMAGE_NT_HEADERS nt_header = GET_IMAGE_NT_HEADER(base_module);
    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(nt_header);
    for (size_t n = 0; n < nt_header->FileHeader.NumberOfSections; n++)
    {
        const uint32_t virtual_size = ALIGN(section[n].Misc.VirtualSize,
            nt_header->OptionalHeader.SectionAlignment);
        if (section[n].VirtualAddress <= return_rva
            && return_rva <= (section[n].VirtualAddress + virtual_size))
        {
            return &section[n];
        }
    }
    return nullptr;
}