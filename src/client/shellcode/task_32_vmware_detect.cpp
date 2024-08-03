#include "anticheat.h"
#include <stdint.h>
#include <string>
#include "protocol.h"
#include "utils/utils.h"
#include "utils/api_resolver.h"
#include "utils/windows_internal.h"

#pragma warning(disable:4996)
#define GET_IMAGE_DOS_HEADER(image) ((PIMAGE_DOS_HEADER)(image))
#define GET_IMAGE_NT_HEADER(image) ((PIMAGE_NT_HEADERS)((ULONG_PTR)(GET_IMAGE_DOS_HEADER(image)->e_lfanew) + (ULONG_PTR)(image)))
#define ALIGN_LEFT_SIZE(virtual_size,align_size) ((virtual_size%align_size)?(align_size - (virtual_size%align_size)):0)
#define ALIGN(virtual_size,align_size) (virtual_size+ALIGN_LEFT_SIZE(virtual_size,align_size))
#define GET_DATA_DIRECTORY(nt_header,dir) (nt_header->OptionalHeader.DataDirectory[dir])
#define GET_DATA_DIRECTORY_VA(image_base, nt_header, dir) RVA2VA(image_base, GET_DATA_DIRECTORY(nt_header, dir).VirtualAddress)
#define GET_RELOC_DESC_TYPEOFFSET_SIZE(reloc_desc) ((reloc_desc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(TYPEOFFSET))
#define RVA2VA(image_base,rva) ((BYTE*)image_base+rva)

#define NB10_SIG	'01BN'
#define RSDS_SIG	'SDSR'
typedef struct  CV_HEADER
{
	DWORD Signature;
	DWORD Offset;
}CV_HEADER;

typedef struct CV_INFO_PDB20
{
	CV_HEADER	CvHeader;
	DWORD		Signature;
	DWORD		Age;
	BYTE		PdbFileName[1];
}CV_INFO_PDB20;

typedef struct CV_INFO_PDB70
{
	DWORD	CvSignature;
	GUID	Signature;
	DWORD	Age;
	BYTE	PdbFileName[1];
}CV_INFO_PDB70;


BOOL ptr_is_region_valid(PVOID Base, DWORD Size, PVOID Addr, DWORD RegionSize)
{
	return ((PBYTE)Addr >= (PBYTE)Base && ((PBYTE)Addr + RegionSize) <= ((PBYTE)Base + Size));
}

std::vector<std::string> get_current_process_pdb_list()
{
	decltype(&IsBadReadPtr) IsBadReadPtr = IMPORT(L"kernel32.dll", IsBadReadPtr);
	bool is_64bit = false;
	std::vector<std::string> result;
	auto modules = Utils::CWindows::instance().enum_modules(GetCurrentProcessId(), is_64bit);
	for (auto& module : modules)
	{
		uint32_t image_base = (uint32_t)module.base;
		if (GET_IMAGE_DOS_HEADER(image_base)->e_magic != IMAGE_DOS_SIGNATURE)
		{
			continue;
		}
		PIMAGE_NT_HEADERS nt_header = GET_IMAGE_NT_HEADER(image_base);
		if (!GET_DATA_DIRECTORY(nt_header, IMAGE_DIRECTORY_ENTRY_DEBUG).VirtualAddress)
		{
			continue;
		}

		PIMAGE_DEBUG_DIRECTORY dbg_dir = (PIMAGE_DEBUG_DIRECTORY)GET_DATA_DIRECTORY_VA(image_base, nt_header, IMAGE_DIRECTORY_ENTRY_DEBUG);
		if (!dbg_dir && IsBadReadPtr(dbg_dir, 1))
		{
			continue;
		}
		if (!dbg_dir->AddressOfRawData || dbg_dir->Type != IMAGE_DEBUG_TYPE_CODEVIEW)
			continue;

		CV_HEADER* cv_info = (CV_HEADER*)RVA2VA(image_base, dbg_dir->AddressOfRawData);
		if (!ptr_is_region_valid((unsigned char*)image_base, nt_header->OptionalHeader.SizeOfImage, cv_info, sizeof(CV_HEADER)))
			continue;
		std::string pdb_name;
		if (cv_info->Signature == NB10_SIG) //VC6.0 (GBK)
		{
			if (!ptr_is_region_valid((unsigned char*)image_base, nt_header->OptionalHeader.SizeOfImage, cv_info, sizeof(CV_INFO_PDB20) + MAX_PATH))
				break;
			pdb_name = (char*)((CV_INFO_PDB20*)cv_info)->PdbFileName;
		}
		else if (cv_info->Signature == RSDS_SIG) //VS2003+ (UTF-8)
		{
			if (!ptr_is_region_valid((unsigned char*)image_base, nt_header->OptionalHeader.SizeOfImage, cv_info, sizeof(CV_INFO_PDB70) + MAX_PATH))
				break;
			pdb_name = (char*)((CV_INFO_PDB70*)cv_info)->PdbFileName;
		}
		else
		{
			continue;
		}
		result.push_back(pdb_name);
	}
	return result;
}


namespace ShellCode
{
    class TaskStaticDetect : public Task
    {
    public:
        TaskStaticDetect()
        {
            set_interval(60 * 1000 * 3);
            set_package_id(SHELLCODE_PACKAGE_ID(32));
        }
        ~TaskStaticDetect()
        {

        }
        virtual void on_time_proc(uint32_t curtime)
        {
			decltype(&_splitpath) _splitpath = IMPORT(L"msvcrt.dll", _splitpath);
            ProtocolShellCodeInstance proto;
            proto.m_type = Protocol::PackageType::PACKAGE_TYPE_ADMIN;
            proto.id = get_package_id();

			auto pdb_list = get_current_process_pdb_list();
			for (auto& pdb_path : pdb_list)
			{
				char driver[_MAX_DRIVE] = { 0 };
				char dir[_MAX_DIR] = { 0 };
				char file_name[_MAX_FNAME] = { 0 };
				char ext[_MAX_EXT] = { 0 };
				_splitpath(pdb_path.c_str(), driver, dir, file_name, ext);
				if (*(DWORD*)file_name == 'd3mv')
				{
					proto.is_cheat = true;
					proto.reason += L"发现虚拟机环境:" + Utils::string2wstring(file_name);
					break;
				}
			}
            if(proto.is_cheat)
            {
                AntiCheat::instance().send(proto);
            }
        };
    };

    uint32_t main()
    {
        TaskStaticDetect* task = new TaskStaticDetect();
        if(AntiCheat::instance().add_task(task))
        {
            return 0;
        }
        else
        {
            delete task;
            return 1;
        }
    }
}