#pragma once
#include "api_resolver.h"
#include <string>


#define GET_IMAGE_DOS_HEADER(image) ((PIMAGE_DOS_HEADER)(image))
#define GET_IMAGE_NT_HEADER(image) ((PIMAGE_NT_HEADERS)((ULONG_PTR)(GET_IMAGE_DOS_HEADER(image)->e_lfanew) + (ULONG_PTR)(image)))
#define ALIGN_LEFT_SIZE(virtual_size,align_size) ((virtual_size%align_size)?(align_size - (virtual_size%align_size)):0)
#define ALIGN(virtual_size,align_size) (virtual_size+ALIGN_LEFT_SIZE(virtual_size,align_size))
#define GET_DATA_DIRECTORY(nt_header,dir) (nt_header->OptionalHeader.DataDirectory[dir])
#define GET_DATA_DIRECTORY_VA(image_base, nt_header, dir) RVA2VA(image_base, GET_DATA_DIRECTORY(nt_header, dir).VirtualAddress)
#define GET_RELOC_DESC_TYPEOFFSET_SIZE(reloc_desc) ((reloc_desc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(TYPEOFFSET))
#define RVA2VA(image_base,rva) ((BYTE*)image_base+rva)

#define PELOADER_MAGICKEY 0xCAFEBABE
namespace PELoader
{
    typedef struct _TYPEOFFSET
    {
        WORD offset : 12;			//偏移值
        WORD Type : 4;			//重定位属性(方式)
    }TYPEOFFSET, *PTYPEOFFSET;

	typedef struct _PELoaderAPI
	{
		ApiResolver::NTDLLAPI ntdll;
		ApiResolver::KERNEL32API kernel32;
	}PELoaderAPI, *PELoaderAPIPtr;
#pragma pack(push)
#pragma pack(1)
    typedef struct _ShareData
    {
        DWORD magic;
        BYTE  eip_opcode[5];
        BYTE  call_opcode;
        DWORD entry_point_dist;
        BYTE  ret_opcode;
        DWORD oep;
        DWORD stub_rva;
        DWORD stub_size;
        DWORD stub_entry_rva;
        DWORD crypt_code_rva;
        DWORD crypt_code_size;
        DWORD xor_key;
        DWORD current_section_rva;
        DWORD current_section_virtual_size;
        DWORD origin_image_base;
    }ShareData, *ShareDataPtr;

    typedef struct _RelocInfo
    {
        DWORD type;
        DWORD rva;
    }RelocInfo, *RelocInfoPtr;

    typedef struct _CryptCode
    {
        size_t text_code_size;
        size_t bind_dll_size;
        size_t reloc_size;
        RelocInfo reloc[1];
    }CryptCode, *CryptCodePtr;

#pragma pack(pop)

    typedef VOID(*PFNExeMain)();
    typedef BOOL(WINAPI *PFNDllMain)(HINSTANCE hModule, DWORD dwReason, LPVOID); /* DllMain */

    void xor_buffer(void* data, size_t size, const uint32_t xor_key);
    PIMAGE_SECTION_HEADER get_current_section(PELoaderAPIPtr api);
    PIMAGE_SECTION_HEADER get_current_section(uint32_t base_module);
    unsigned int aphash(const unsigned char *str, uint32_t len);
    bool __stdcall write_file(LPCSTR filename, LPVOID buffer, SIZE_T file_size);
	char *__stdcall memcpy(char *dst, const char* src, int cnt);
	char *__stdcall strcpy(char *dst, const char *src, int strlen);
	uint32_t peload(PELoaderAPIPtr api, HINSTANCE* instance, void* buffer, size_t size);
    void execute_tls_callback(PELoaderAPIPtr api, HINSTANCE instance);
    void execute_entrypoint(HINSTANCE instance, LPVOID param);
    PIMAGE_SECTION_HEADER get_shell_section();
    bool is_in_virtual_machine();
    void __stdcall stub_entry(ShareDataPtr data);
	void __stdcall get_game_path(char * gamepath);
    bool __stdcall entrypoint(BYTE* eip);

    PIMAGE_SECTION_HEADER get_current_section();
    char* loadfile(std::string& path, size_t* lpsize);
}