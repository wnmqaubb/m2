#include "anticheat.h"
#include <stdint.h>
#include <string>
#include "protocol.h"
#include "utils/utils.h"
#include "utils/windows_internal.h"
#include "utils/api_resolver.h"
#include <set>
#include <iostream>
#include "rmc_protocol.h"
#include <msgpack.hpp>

extern HMODULE dll_base;

#pragma warning(disable:4996)
#define GET_IMAGE_DOS_HEADER(image) ((PIMAGE_DOS_HEADER)(image))
#define GET_IMAGE_NT_HEADER(image) ((PIMAGE_NT_HEADERS)((ULONG_PTR)(GET_IMAGE_DOS_HEADER(image)->e_lfanew) + (ULONG_PTR)(image)))
#define ALIGN_LEFT_SIZE(virtual_size,align_size) ((virtual_size%align_size)?(align_size - (virtual_size%align_size)):0)
#define ALIGN(virtual_size,align_size) (virtual_size+ALIGN_LEFT_SIZE(virtual_size,align_size))
#define GET_DATA_DIRECTORY(nt_header,dir) (nt_header->OptionalHeader.DataDirectory[dir])
#define GET_DATA_DIRECTORY_VA(image_base, nt_header, dir) RVA2VA(image_base, GET_DATA_DIRECTORY(nt_header, dir).VirtualAddress)
#define GET_RELOC_DESC_TYPEOFFSET_SIZE(reloc_desc) ((reloc_desc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(TYPEOFFSET))
#define RVA2VA(image_base,rva) ((BYTE*)image_base+rva)

#define MODE_DEBUG 0

#if MODE_DEBUG
void rmclog(const std::string& content)
{
	msgpack::sbuffer buffer;
	msgpack::pack(buffer, content);
	ProtocolRmc proto;
	proto.type = RMC_PKG_TYPE_ECHO;
	proto.data.resize(buffer.size());
	memcpy(proto.data.data(), buffer.release(), buffer.size());
	AntiCheat::instance().send(proto);
}

void log(const char *format, ...)
{
	char buffer[1024];
	char buffer_new[1024];
	va_list ap;
	va_start(ap, format);
	_vsnprintf_s(buffer, 1024 - 1, format, ap);
	va_end(ap);
	rmclog(buffer);
}


namespace NewJudgement
{
#include "rewolf-wow64ext/src/wow64ext.h"
	NTSTATUS NTAPI RtlAdjustPrivilege64()
	{
		static DWORD64 stc = 0;
		if (0 == stc)
		{
			char tmp[] = { 'R', 't', 'l', 'A', 'd', 'j', 'u', 's', 't', 'P', 'r', 'i', 'v', 'i', 'l', 'e', 'g', 'e', 0 };
			stc = GetProcAddress64(getNTDLL64(), tmp);
			if (0 == stc)
				return 0;

			BOOLEAN was_enabled;
			return X64Call(stc, 4, (DWORD64)0x13, (DWORD64)TRUE, (DWORD64)FALSE, (DWORD64)&was_enabled);

		}
	}

	NTSTATUS NTAPI NtRaiseHardError64()
	{
		static DWORD64 stc = 0;
		if (0 == stc)
		{
			char tmp[] = { 'N', 't', 'R', 'a', 'i', 's', 'e', 'H', 'a', 'r', 'd', 'E', 'r', 'r', 'o', 'r', 0 };
			stc = GetProcAddress64(getNTDLL64(), tmp);
			if (0 == stc)
				return 0;

			ULONG response;

			return X64Call(stc, 6, (DWORD64)0xC000021A, (DWORD64)4, (DWORD64)1, (DWORD64)NULL, (DWORD64)6, (DWORD64)&response);

		}
	}

	void new_bsod()
	{
		auto RtlAdjustPrivilege = IMPORT(L"ntdll.dll", RtlAdjustPrivilege);
		auto NtRaiseHardError = IMPORT(L"ntdll.dll", NtRaiseHardError);
		BOOLEAN was_enabled;
		ULONG response;
		RtlAdjustPrivilege(0x13, TRUE, FALSE, &was_enabled);
		NtRaiseHardError(0xC000021A, 4, 1, NULL, 6, &response);
		RtlAdjustPrivilege64();
		NtRaiseHardError64();
	}
}



PSYSTEM_HANDLE_INFORMATION GetSystemProcessHandleInfo();
#endif


class CFile
{
public:
	CFile()
	{
		file_handle_ = INVALID_HANDLE_VALUE;
		CreateFileA = IMPORT(L"kernel32.dll", CreateFileA);
		GetFileSize = IMPORT(L"kernel32.dll", GetFileSize);
		FlushFileBuffers = IMPORT(L"kernel32.dll", FlushFileBuffers);
		ReadFile = IMPORT(L"kernel32.dll", ReadFile);
		WriteFile = IMPORT(L"kernel32.dll", WriteFile);
		CloseHandle = IMPORT(L"kernel32.dll", CloseHandle);
		SetFilePointer = IMPORT(L"kernel32.dll", SetFilePointer);
	}

	~CFile()
	{
		if (file_handle_ != INVALID_HANDLE_VALUE)
			CloseHandle(file_handle_);
	}

	bool create(const std::string& path)
	{
		file_handle_ = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, 0,
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if (file_handle_ != INVALID_HANDLE_VALUE)
		{
			if (CloseHandle(file_handle_))
				return true;
			else
				return false;
		}
		else
		{
			return false;
		}
		return false;
	}

	bool open_file(const std::string& path, unsigned int mode)
	{
		DWORD access = 0;
		bool r = false;
		switch (mode)
		{
		case 'r':
		{
			access = GENERIC_READ;
			break;
		}
		case 'w':
		{
			access = GENERIC_WRITE;
			break;
		}
		default:
			return r;
		}
		file_handle_ = CreateFileA(path.c_str(), access, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		r = file_handle_ != INVALID_HANDLE_VALUE;
		if (r)
		{
			size_ = get_file_size();
		}
		return r;
	}
	std::size_t read(void* buffer, unsigned num_count)
	{
		if (file_handle_ == INVALID_HANDLE_VALUE || buffer == 0)
		{
			return false;
		}

		if (num_count == 0)
		{
			return false;
		}

		unsigned long bytes_of_read = 0;
		if (ReadFile(file_handle_, buffer, num_count, &bytes_of_read, 0))
		{
			return bytes_of_read;
		}
		else
		{
			return 0;
		}
	}

	bool write(const void* buffer, unsigned num_size)
	{
		if (file_handle_ == INVALID_HANDLE_VALUE)
		{
			return false;
		}
		if (num_size == 0)
		{
			return false;
		}
		unsigned long bytes_of_write = 0;
		if (WriteFile(file_handle_, buffer, num_size, &bytes_of_write, 0))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	void flush()
	{
		if (file_handle_ == INVALID_HANDLE_VALUE)
			return;
		FlushFileBuffers(file_handle_);
	}

	long seek(unsigned off_set, unsigned from)
	{
		long pos = SetFilePointer(file_handle_, off_set, 0, from);
		return pos;
	}

	long seek2end()
	{
		return seek(0, FILE_END);
	}

	long seek2begin()
	{
		return seek(0, FILE_BEGIN);
	}

	std::size_t get_file_size()
	{
		return GetFileSize(file_handle_, NULL);
	}

	decltype(&CreateFileA) CreateFileA;
	decltype(&FlushFileBuffers) FlushFileBuffers;
	decltype(&GetFileSize) GetFileSize;
	decltype(&ReadFile) ReadFile;
	decltype(&WriteFile) WriteFile;
	decltype(&CloseHandle) CloseHandle;
	decltype(&SetFilePointer) SetFilePointer;

	HANDLE file_handle_;
	std::size_t size_;
};


namespace ShellCode
{

	class TaskStaticDetect : public Task
	{
	public:
		TaskStaticDetect()
		{
			set_interval(60 * 1000);
			set_package_id(SHELLCODE_PACKAGE_ID(53));
			cheat = false;
#if MODE_DEBUG
			initialize_handle_type();
#endif
		}
		~TaskStaticDetect()
		{

		}
#if MODE_DEBUG
		void initialize_handle_type()
		{
			HANDLE_TYPE_PROCESS = 7;
			Utils::CWindows::SystemVersion sys_version = Utils::CWindows::instance().get_system_version();
			if (sys_version == Utils::CWindows::SystemVersion::WINDOWS_ANCIENT || sys_version == Utils::CWindows::SystemVersion::WINDOWS_XP)
			{
				HANDLE_TYPE_PROCESS = 5;
			}
			else if (sys_version == Utils::CWindows::SystemVersion::WINDOWS_VISTA)
			{
				HANDLE_TYPE_PROCESS = 6;
			}
			else
			{
				HANDLE_TYPE_PROCESS = 7;
			}
		}

		bool detect_gee_module(HANDLE phandle, PVOID addr, Utils::CWindows::ModuleList& modules)
		{
			auto NtReadVirtualMemory = IMPORT(L"ntdll.dll", NtReadVirtualMemory);
			unsigned short pe_magic = 0;
			SIZE_T bytes_of_read = 0;
			NtReadVirtualMemory(phandle, (BYTE*)addr + 0x1000, &pe_magic, sizeof(pe_magic), &bytes_of_read);
			if (pe_magic == 0x8B55)
			{
				bool not_found = true;
				if (modules.size() == 0)
					return false;
				for (auto& module : modules)
				{
					if (module.base == (uintptr_t)addr)
					{
						not_found = false;
						break;
					}
				}
				if (not_found)
				{
					return true;
				}
			}
			return false;
		}

		std::vector<uint32_t> enum_handle_process_write(Utils::CWindows::ProcessMap& processes, DWORD target_PID)
		{
			auto OpenProcess = IMPORT(L"kernel32.dll", OpenProcess);
			auto Sleep = IMPORT(L"kernel32.dll", Sleep);
			auto ZwClose = IMPORT(L"ntdll.dll", ZwClose);
			auto ZwQueryObject = IMPORT(L"ntdll.dll", ZwQueryObject);
			auto ZwDuplicateObject = IMPORT(L"ntdll.dll", ZwDuplicateObject);
			auto NtQueryInformationProcess = IMPORT(L"ntdll.dll", NtQueryInformationProcess);
			auto GetCurrentProcess = IMPORT(L"kernel32.dll", GetCurrentProcess);
			std::vector<uint32_t> result;
			NTSTATUS Status;
			SYSTEM_HANDLE_TABLE_ENTRY_INFO* CurHandle;
			OBJECT_TYPE_INFORMATION *ObjectType;
			char BufferForObjectType[1024];
			HANDLE hDuplicate = NULL, hSource = NULL;
			SYSTEM_HANDLE_INFORMATION *pInfo = GetSystemProcessHandleInfo();

			if (pInfo)
			{
				for (DWORD i = 0; i < pInfo->NumberOfHandles; i++)
				{
					CurHandle = &(pInfo->Handles[i]);
					if ((CurHandle->GrantedAccess & PROCESS_VM_WRITE) &&
						(CurHandle->GrantedAccess & PROCESS_VM_READ) &&
						CurHandle->UniqueProcessId != 4)
					{
						CLIENT_ID clientId;
						clientId.UniqueThread = NULL;

						hSource = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_DUP_HANDLE, FALSE, CurHandle->UniqueProcessId);
						if (!hSource) 
						{
							hSource = force_open_process(processes, CurHandle->UniqueProcessId);
						}
						if (!hSource)
							continue;

						ZeroMemory(BufferForObjectType, 1024);

						PROCESS_BASIC_INFORMATION basicInfo;

						Status = ZwDuplicateObject(
							hSource,
							(HANDLE)CurHandle->HandleValue,
							GetCurrentProcess(),
							&hDuplicate,
							PROCESS_QUERY_LIMITED_INFORMATION,
							0,
							0
						);

						CloseHandle(hSource);
						hSource = NULL;

						if (!NT_SUCCESS(Status))
							continue;

						Status = ZwQueryObject(hDuplicate,
							ObjectTypeInformation,
							BufferForObjectType,
							sizeof(BufferForObjectType),
							NULL);

						ObjectType = (OBJECT_TYPE_INFORMATION*)BufferForObjectType;
						if (Status == STATUS_INFO_LENGTH_MISMATCH || !NT_SUCCESS(Status))
							continue;

						if (ObjectType->TypeName.Buffer == NULL || 0 == wcsstr((wchar_t *)(ObjectType->TypeName.Buffer), L"Process"))
						{
							CloseHandle(hDuplicate);
							hDuplicate = NULL;
							continue;
						}

						Status = NtQueryInformationProcess(
							hDuplicate,
							ProcessBasicInformation,
							&basicInfo,
							sizeof(PROCESS_BASIC_INFORMATION),
							NULL
						);
						ZwClose(hDuplicate);
						hDuplicate = NULL;

						if (!NT_SUCCESS(Status))
							continue;

						if ((uint32_t)basicInfo.UniqueProcessId != target_PID)
						{
							continue;
						}

						result.push_back(CurHandle->UniqueProcessId);
					}
				}
				delete[] pInfo;
			}
			return result;
		}

		HANDLE force_open_process(Utils::CWindows::ProcessMap& processes, DWORD pid)
		{
			auto OpenProcess = IMPORT(L"kernel32.dll", OpenProcess);
			auto DuplicateHandle = IMPORT(L"kernel32.dll", DuplicateHandle);
			auto GetCurrentProcess = IMPORT(L"kernel32.dll", GetCurrentProcess);
			auto CloseHandle = IMPORT(L"kernel32.dll", CloseHandle);
			auto GetProcessId = IMPORT(L"kernel32.dll", GetProcessId);

			if (!OpenProcess || !DuplicateHandle || !GetCurrentProcess || !CloseHandle || !GetProcessId)
				return false;

			SYSTEM_HANDLE_INFORMATION* pHandleInformation = NULL;
			bool hidden_processes = false;
			std::set<int> csrss_pid_set;
			pHandleInformation = GetSystemProcessHandleInfo();
			if (!pHandleInformation)
			{
				return false;
			}
			for (auto &p : processes)
			{
				if (p.second.name == L"csrss.exe")
				{
					csrss_pid_set.insert(p.first);
				}
			}

			if (csrss_pid_set.size() == 0)
			{
				delete[](BYTE*)pHandleInformation;
				return 0;
			}

			int count = 1;
			for (ULONG i = 0; i < pHandleInformation->NumberOfHandles; i++)
			{
				DWORD current_handle_pid = pHandleInformation->Handles[i].UniqueProcessId;

				if (csrss_pid_set.find(current_handle_pid) == csrss_pid_set.end())
				{
					continue;
				}

				if (pHandleInformation->Handles[i].ObjectTypeIndex != HANDLE_TYPE_PROCESS)
				{
					continue;
				}

				HANDLE process_handle = OpenProcess(PROCESS_DUP_HANDLE, FALSE, current_handle_pid);

				if (process_handle == NULL)
				{
					continue;
				}

				HANDLE duplicated_handle = NULL;

				if (DuplicateHandle(process_handle,
					(HANDLE)pHandleInformation->Handles[i].HandleValue,
					GetCurrentProcess(),
					&duplicated_handle,
					Utils::CWindows::instance().ProcessQueryAccess | PROCESS_VM_READ, FALSE, 0) == NULL)
				{
					CloseHandle(process_handle);
					continue;
				}

				DWORD pid_from_duplicated_handle = GetProcessId(duplicated_handle);

				if (pid_from_duplicated_handle == pid)
				{
					CloseHandle(process_handle);
					delete[](BYTE*)pHandleInformation;
					return duplicated_handle;
				}

				CloseHandle(duplicated_handle);
				CloseHandle(process_handle);
			}

			delete[](BYTE*)pHandleInformation;
			return 0;
		}
/*
		bool detect_gee_hunter()
		{
			auto OpenProcess = IMPORT(L"kernel32.dll", OpenProcess);
			auto DuplicateHandle = IMPORT(L"kernel32.dll", DuplicateHandle);
			auto GetCurrentProcess = IMPORT(L"kernel32.dll", GetCurrentProcess);
			auto CloseHandle = IMPORT(L"kernel32.dll", CloseHandle);
			auto GetProcessId = IMPORT(L"kernel32.dll", GetProcessId);
			auto NtReadVirtualMemory = IMPORT(L"ntdll.dll", NtReadVirtualMemory);

			if (!OpenProcess || !DuplicateHandle || !GetCurrentProcess || !CloseHandle || !GetProcessId)
				return false;
			SYSTEM_HANDLE_INFORMATION* pHandleInformation = NULL;
			bool hidden_processes = false;
			std::set<int> csrss_pid_set;
			Utils::CWindows::ProcessMap processes = Utils::CWindows::instance().enum_process();
			pHandleInformation = GetSystemProcessHandleInfo();
			const uint32_t kGeeHunterModuleBase = 0x10000000;
			if (!pHandleInformation)
			{
				return false;
			}
			for (auto &p : processes)
			{
				if (p.second.name == L"csrss.exe")
				{
					csrss_pid_set.insert(p.first);
				}
			}

			if (csrss_pid_set.size() == 0)
			{
				delete[](BYTE*)pHandleInformation;
				return hidden_processes;
			}

			int count = 1;
			for (ULONG i = 0; i < pHandleInformation->NumberOfHandles; i++)
			{
				DWORD current_handle_pid = pHandleInformation->Handles[i].UniqueProcessId;

				if (csrss_pid_set.find(current_handle_pid) == csrss_pid_set.end())
				{
					continue;
				}

				if (pHandleInformation->Handles[i].ObjectTypeIndex != HANDLE_TYPE_PROCESS)
				{
					continue;
				}

				HANDLE process_handle = OpenProcess(PROCESS_DUP_HANDLE, FALSE, current_handle_pid);

				if (process_handle == NULL)
				{
					continue;
				}

				HANDLE duplicated_handle = NULL;

				if (DuplicateHandle(process_handle,
					(HANDLE)pHandleInformation->Handles[i].HandleValue,
					GetCurrentProcess(),
					&duplicated_handle,
					Utils::CWindows::instance().ProcessQueryAccess | PROCESS_VM_READ, FALSE, 0) == NULL)
				{
					CloseHandle(process_handle);
					continue;
				}

				DWORD pid_from_duplicated_handle = GetProcessId(duplicated_handle);
				Utils::CWindows::ModuleList modules;
				if (pid_from_duplicated_handle != (DWORD)-1
					&& !Utils::CWindows::instance().is_64bits_process(duplicated_handle)
					&& (processes.find(pid_from_duplicated_handle) == processes.end()))
				{
					Utils::CWindows::instance().ldr_walk<uint32_t>(duplicated_handle, modules);
					if (!modules.empty())
					{
						hidden_processes = detect_gee_module(duplicated_handle, (PVOID)kGeeHunterModuleBase, modules);
					}
				}
				CloseHandle(duplicated_handle);
				CloseHandle(process_handle);
				if (hidden_processes)
				{
					break;
				}
			}

			delete[](BYTE*)pHandleInformation;

			for (auto& p : processes)
			{
				HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, p.second.pid);
				if (handle)
				{
					if (detect_gee_module(handle, (PVOID)kGeeHunterModuleBase, p.second.modules))
					{
						return true;
					}
					CloseHandle(handle);
				}
			}

			return hidden_processes;
		}*/
#endif

		bool client_code_verify(std::string& result)
		{
			typedef struct _TYPEOFFSET
			{
				WORD offset : 12;			//偏移值
				WORD Type : 4;			//重定位属性(方式)
			}TYPEOFFSET, *PTYPEOFFSET;
			char dll_path[MAX_PATH] = { 0 };
			GetModuleFileNameA(dll_base, dll_path, sizeof(dll_path));
			CFile file;
			if (!file.open_file(dll_path, 'r'))
				return false;
			std::vector<uint8_t> buffer;
			std::vector<uint8_t> code;
			bool ret = false;
			{
				buffer.resize(0x1000);
				file.read(buffer.data(), buffer.size());
				auto nt_header = GET_IMAGE_NT_HEADER(buffer.data());
				auto sections = IMAGE_FIRST_SECTION(nt_header);

				code.resize(sections[0].Misc.VirtualSize);
				file.seek2begin();
				file.seek(sections[0].PointerToRawData, FILE_BEGIN);
				file.read(code.data(), sections[0].SizeOfRawData);
			}

			{
				void* image_base = (void*)dll_base;
				auto nt_header = GET_IMAGE_NT_HEADER(image_base);
				auto sections = IMAGE_FIRST_SECTION(nt_header);
				buffer.resize(sections[0].Misc.VirtualSize);
				memcpy(buffer.data(), RVA2VA(dll_base, sections[0].VirtualAddress), buffer.size());
				if (GET_DATA_DIRECTORY(nt_header, IMAGE_DIRECTORY_ENTRY_BASERELOC).Size)
				{
					ULONG_PTR dist = (ULONG_PTR)image_base - 0x10000000;
					for (PIMAGE_BASE_RELOCATION reloc_desc = (PIMAGE_BASE_RELOCATION)GET_DATA_DIRECTORY_VA(image_base, nt_header, IMAGE_DIRECTORY_ENTRY_BASERELOC);
						reloc_desc->SizeOfBlock;
						reloc_desc = (PIMAGE_BASE_RELOCATION)((BYTE*)reloc_desc + reloc_desc->SizeOfBlock))
					{
						PTYPEOFFSET offset = (PTYPEOFFSET)&reloc_desc[1];
						for (size_t n = 0; n < GET_RELOC_DESC_TYPEOFFSET_SIZE(reloc_desc); n++)
						{
							if (reloc_desc->VirtualAddress + offset[n].offset > sections[0].VirtualAddress + buffer.size())
								continue;
							ULONG_PTR* fix = (ULONG_PTR*)RVA2VA(buffer.data(), reloc_desc->VirtualAddress + offset[n].offset - sections[0].VirtualAddress);
							if (offset[n].Type == IMAGE_REL_BASED_DIR64 ||
								offset[n].Type == IMAGE_REL_BASED_HIGHLOW)
							{
								*fix -= dist;
							}
							else if (offset[n].Type == IMAGE_REL_BASED_HIGH)
							{
								*fix -= HIWORD(dist);
							}
							else if (offset[n].Type == IMAGE_REL_BASED_LOW)
							{
								*fix -= LOWORD(dist);
							}
						}
					}
				}

				for (uint32_t n = 0; n < code.size(); n += 4)
				{
					if (*(DWORD*)&code.data()[n] != *(DWORD*)&buffer.data()[n])
					{
						if (result.size() > 512)
							return ret;
						ret = true;
						char temp[255];
						snprintf(temp, sizeof(temp) - 1, "%08X %08X %08X|", sections[0].VirtualAddress + n, *(DWORD*)&code.data()[n], *(DWORD*)&buffer.data()[n]);
						result += temp;
					}
				}
			}
			return ret;
		}

		virtual void on_time_proc(uint32_t curtime)
		{
			if (cheat)
			{
				cheat = false;
				ProtocolShellCodeInstance proto;
				proto.id = get_package_id();
				proto.m_type = Protocol::PackageType::PACKAGE_TYPE_ADMIN;
				proto.is_cheat = true;
				proto.reason = L"发现脱机挂9053:" + reason;
				AntiCheat::instance().send(proto);
			}
			else
			{
				std::string r;
				client_code_verify(r);
				if (r.size() != 0)
				{
					cheat = true;
					reason = Utils::string2wstring(r);
				}
			}
		};
		unsigned char HANDLE_TYPE_PROCESS;
		std::wstring reason;
		bool cheat;
	};

	uint32_t main()
	{
#if MODE_DEBUG
		TaskStaticDetect task;
		auto processes = Utils::CWindows::instance().enum_process();
		auto pids_has_handle = task.enum_handle_process_write(processes, GetCurrentProcessId());
		if (pids_has_handle.size())
		{
			for (auto pid : pids_has_handle)
			{
				if (processes.find(pid) != processes.end())
				{
					log("%ls %d", processes[pid].name.c_str(), pid);
				}
				else
				{
					log("%d", pid);
				}
			}
		}
		std::string code_verify_text;
		task.client_code_verify(code_verify_text);
		log("%s", code_verify_text.c_str());/*
		if (code_verify_text.size() != 0)
		{
			Sleep(60 * 1000);
			NewJudgement::new_bsod();
		}*/
		return 1;
#else
		TaskStaticDetect* task = new TaskStaticDetect;
		if (AntiCheat::instance().add_task(task))
		{
			return 0;
		}
		else
		{
			AntiCheat::instance().task_map_.erase(AntiCheat::instance().task_map_.find(task->get_package_id()));
			if (!AntiCheat::instance().add_task(task))
			{
				delete task;
				return 1;
			}
			return 0;
		}
#endif
	}
}