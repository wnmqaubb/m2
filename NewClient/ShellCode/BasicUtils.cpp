#include "../pch.h"
#include <Lightbone/utils.h>
#include <Lightbone/windows_internal.h>
#include <fstream>
#include <iphlpapi.h>
#include <WinSock.h>
#include <ShellAPI.h>
#include <Psapi.h>

using namespace ApiResolver;
using namespace Utils::Crypto;

void TimeOutCheckRoutine();

namespace BasicUtils
{

    uint32_t __declspec(noinline) manual_load_ntdll(void* buffer, size_t size, HINSTANCE* instance)
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

        if (is_image_import_directory_exist(image_base))
        {
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
                        VirtualFree(image_base, 0, MEM_RELEASE);
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
                        iat->u1.Function = (ULONG_PTR)GetProcAddress(import_module_handle, function_name);
                    }
                    if (iat->u1.Function == NULL)
                    {
                        VirtualFree(image_base, 0, MEM_RELEASE);
                        return ERROR_FUNCTION_NOT_CALLED;
                    }
                }
            }
        }

        //重定位修正
        if (get_data_directory(nt_header, IMAGE_DIRECTORY_ENTRY_BASERELOC).Size)
        {
            ULONG_PTR dist = 0;
            if (*instance)
            {
                dist = (ULONG_PTR)*instance - nt_header->OptionalHeader.ImageBase;
            }
            else
            {
                dist = (ULONG_PTR)image_base - nt_header->OptionalHeader.ImageBase;
            }

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

    void __declspec(noinline) manual_load_ntdll_and_bsod()
    {
        auto ntdll = GetModuleHandleA("ntdll.dll");
        char path[MAX_PATH] = { 0 };
        GetModuleFileNameA(ntdll, path, sizeof(path));
        std::ifstream file(path, std::ios::in | std::ios::binary);
        std::stringstream ss;
        ss << file.rdbuf();
        auto buf = std::move(ss.str());
        HINSTANCE ntdll_wrapper = ntdll;
        manual_load_ntdll(buf.data(), buf.size(), &ntdll_wrapper);
        auto RtlAdjustPrivilege = IMPORT(L"ntdll.dll", RtlAdjustPrivilege);
        auto NtRaiseHardError = IMPORT(L"ntdll.dll", NtRaiseHardError);
        RtlAdjustPrivilege = ApiResolver::rebase<decltype(RtlAdjustPrivilege)>(RtlAdjustPrivilege, (void*)ntdll, ntdll_wrapper);
        NtRaiseHardError = ApiResolver::rebase<decltype(NtRaiseHardError)>(NtRaiseHardError, (void*)ntdll, ntdll_wrapper);
        BOOLEAN was_enabled = 0;
        ULONG response = 0;
        RtlAdjustPrivilege(0x13, TRUE, FALSE, &was_enabled);
        NtRaiseHardError(0xC000021A, 4, 1, NULL, 6, &response);
        VirtualFree(ntdll_wrapper, 0, MEM_RELEASE);
    }

    static WNDPROC old_wnd_proc = NULL;
    WNDPROC set_old_wnd_proc(WNDPROC routine)
    {
        WNDPROC temp = old_wnd_proc;
        old_wnd_proc = routine;
        return temp;
    }

    void __declspec(noinline) infinite_exit()
    {
        auto ExitProcess = IMPORT(L"kernel32.dll", ExitProcess);
        set_old_wnd_proc((WNDPROC)ExitProcess);
    }

    LRESULT WINAPI wnd_proc_wrapper(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        VMP_VIRTUALIZATION_BEGIN()
        auto GetTickCount = IMPORT(L"kernel32.dll", GetTickCount);
        static DWORD last_tick_count = GetTickCount();
        if (GetTickCount() - last_tick_count > 5 * 60 * 1000)
        {
            TimeOutCheckRoutine();
            last_tick_count = GetTickCount();
        }
        VMP_VIRTUALIZATION_END()
        return old_wnd_proc(hwnd, msg, wparam, lparam);
    }

    void init_heartbeat_check(HWND hwnd)
    {
        VMP_VIRTUALIZATION_BEGIN()
        if (old_wnd_proc == NULL)
        {
            old_wnd_proc = (WNDPROC)GetWindowLongA(hwnd, GWLP_WNDPROC);
            SetWindowLongA(hwnd, GWLP_WNDPROC, (LONG)&wnd_proc_wrapper);
        }
        VMP_VIRTUALIZATION_END()
    }

    void __declspec(noinline) bsod()
    {
        HANDLE ntdll = LoadLibrary(TEXT("ntdll.dll"));
        auto RtlAdjustPrivilege = IMPORT(L"ntdll.dll", RtlAdjustPrivilege);
        auto RtlSetProcessIsCritical = IMPORT(L"ntdll.dll", RtlSetProcessIsCritical);
        BOOLEAN b;
        RtlAdjustPrivilege(20UL, TRUE, FALSE, &b);
        RtlSetProcessIsCritical(TRUE, NULL, FALSE);
        auto ExitProcess = IMPORT(L"kernel32.dll", ExitProcess);
        ExitProcess(0);
    }

    std::tuple<std::string, std::string> scan_tcp_table(std::vector<std::tuple<std::string, std::string>> black_ip_table)
    {
        std::tuple<std::string, std::string> empty_tuple{ "", "" };
        DWORD table_size = 0;
        std::string remote_ip;
        auto GetModuleHandleA = IMPORT(L"kernel32.dll", GetModuleHandleA);
        if (GetModuleHandleA("Iphlpapi.dll") == NULL)
            LoadLibraryA("Iphlpapi.dll");

        auto GetExtendedTcpTable = IMPORT(L"Iphlpapi.dll", GetExtendedTcpTable);
        if (!GetExtendedTcpTable)
            return empty_tuple;

        GetExtendedTcpTable(0, &table_size, false, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);


        PMIB_TCPTABLE_OWNER_PID ip_table = (PMIB_TCPTABLE_OWNER_PID)malloc(table_size);
        if(!ip_table) return empty_tuple;

        if (GetExtendedTcpTable(ip_table, &table_size, false, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) != NO_ERROR)
        {
            free(ip_table);
            return empty_tuple;
        }

        for (int i = 0; i < ip_table->dwNumEntries; ++i)
        {
            remote_ip = inet_ntoa(*(in_addr*)& ip_table->table[i].dwRemoteAddr);
            for (auto& t_ip : black_ip_table)
            {
                if (remote_ip == std::get<0>(t_ip))
                {
                    free(ip_table);
                    return t_ip;
                }
            }
        }
        
        free(ip_table);
        return empty_tuple;
    }
	std::map<uint32_t, std::tuple<std::wstring, uint32_t, uint32_t>> enum_memory(uint32_t phandle)
	{
		std::map<uint32_t, std::tuple<std::wstring, uint32_t, uint32_t>> items;
		SYSTEM_INFO info;
		GetSystemInfo(&info);
		uint32_t pagesize = info.dwPageSize;
		uint32_t lowerbound = (uint32_t)info.lpMinimumApplicationAddress;
		uint32_t upperbound = (uint32_t)info.lpMaximumApplicationAddress;

		MEMORY_BASIC_INFORMATION mbi;
		for (uint32_t addr = lowerbound; addr < upperbound;)
		{
			MEMORY_BASIC_INFORMATION mbi = { 0 };
			wchar_t buf[MAX_PATH];
			if (VirtualQueryEx((HANDLE)phandle, (LPVOID)addr, &mbi, sizeof(mbi)) == sizeof(mbi))
			{
				if (mbi.State != MEM_FREE)
				{
					DWORD num = GetMappedFileNameW((HANDLE)phandle, mbi.BaseAddress, buf, _countof(buf));
					if (num != 0)
					{
						items.emplace((uint32_t)mbi.BaseAddress, std::make_tuple(std::wstring(buf), mbi.Protect, mbi.RegionSize));
					}
					else
					{
						items.emplace((uint32_t)mbi.BaseAddress, std::make_tuple(std::wstring(L""), mbi.Protect, mbi.RegionSize));
					}
				}
				addr = (uint32_t)mbi.BaseAddress + mbi.RegionSize;
			}
			else
			{
				addr += pagesize;
			}
		}
		return items;
	}
    uint32_t read_virtual_memory(
        HANDLE handle,
        uint64_t base_address,
        PVOID buffer,
        uint64_t buffer_size,
        uint32_t* bytes_of_read)
    {
        auto NtReadVirtualMemory = IMPORT(L"ntdll.dll", NtReadVirtualMemory);
        return NtReadVirtualMemory(handle, (PVOID)base_address, buffer, buffer_size, (SIZE_T*)bytes_of_read);
    }

    bool calc_pe_ico_hash(std::wstring path, uint32_t* hash_val)
    {
        if (GetModuleHandleA("Shell32.dll") == NULL)
            LoadLibraryA("Shell32.dll");
        if (GetModuleHandleA("gdi32.dll") == NULL)
            LoadLibraryA("gdi32.dll");

        auto ExtractIconW = IMPORT(L"Shell32.dll", ExtractIconW);
        auto GetIconInfo = IMPORT(L"User32.dll", GetIconInfo);
        auto CopyImage = IMPORT(L"User32.dll", CopyImage);
        auto DestroyIcon = IMPORT(L"User32.dll", DestroyIcon);
        auto GetObjectW = IMPORT(L"gdi32.dll", GetObjectW);
        auto DeleteObject = IMPORT(L"gdi32.dll", DeleteObject);

        if (!ExtractIconW || !GetObjectW || !DeleteObject || !DestroyIcon || !CopyImage || !GetIconInfo)
            return false;
        bool result = false;
        HICON hicon = NULL;
        ICONINFO icon_info;
        try
        {
            HMODULE curr_hmodule = GetModuleHandle(NULL);
            if (curr_hmodule)
            {
                hicon = ExtractIconW(curr_hmodule, path.c_str(), 0);
                if (hicon)
                {
                    if (GetIconInfo(hicon, &icon_info))
                    {
                        HBITMAP hbitmap;
                        DIBSECTION ds;
                        int ds_size = GetObjectW(icon_info.hbmColor, sizeof(ds), &ds);
                        if (sizeof(ds) != ds_size)
                        {
                            hbitmap = (HBITMAP)CopyImage(icon_info.hbmColor, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
                            ds_size = GetObjectW(hbitmap, sizeof(ds), &ds);
                        }

                        std::unique_ptr<unsigned char[]> icon_buffer(new unsigned char[ds.dsBmih.biSizeImage]);
                        __movsb(icon_buffer.get(), (unsigned char*)ds.dsBm.bmBits, ds.dsBmih.biSizeImage);

                        if (hash_val)
                            *hash_val = aphash(icon_buffer.get(), ds.dsBmih.biSizeImage);

                        DestroyIcon(hicon);
                        DeleteObject(hbitmap);
                        DeleteObject(icon_info.hbmColor);
                        DeleteObject(icon_info.hbmMask);

                        result = true;
                    }
                }
            }
        }
        catch (...)
        {
            result = false;
        }

        if (hicon)
        {
            DestroyIcon(hicon);
        }

        if (icon_info.hbmColor)
        {
            DeleteObject(icon_info.hbmColor);
        }

        if (icon_info.hbmMask)
        {
            DeleteObject(icon_info.hbmMask);
        }

        return result;
    }

    std::string get_mac_by_ip(std::string &gateway_ip)
    {
        auto GetModuleHandleA = IMPORT(L"kernel32.dll", GetModuleHandleA);
        if (GetModuleHandleA("Ws2_32.dll") == NULL)
            LoadLibraryA("Ws2_32.dll");

        auto SendARP = IMPORT(L"Iphlpapi.dll", SendARP);
        auto inet_addr = IMPORT(L"Ws2_32.dll", inet_addr);
        if (!SendARP || !inet_addr)
            return "";
        unsigned char mac[6] = { 0 };
        ULONG MacLen = 6;
        ULONG DestIP = inet_addr(gateway_ip.c_str());
        int rs = SendARP(DestIP, (ULONG)NULL, (PULONG)mac, (PULONG)&MacLen);

        if (rs == 0)
        {
            char buf[32] = { 0 };
            sprintf_s(buf, "%02X-%02X-%02X-%02X-%02X-%02X",
                (unsigned int)mac[0],
                (unsigned int)mac[1],
                (unsigned int)mac[2],
                (unsigned int)mac[3],
                (unsigned int)mac[4],
                (unsigned int)mac[5]);
            return buf;
        }
        else
        {
            return "";
        }

    }

    std::vector<std::pair<std::string, std::string>> get_gateway_ip_macs()
    {
        std::vector<std::pair<std::string, std::string>> adapter_info_list;

        auto GetModuleHandleA = IMPORT(L"kernel32.dll", GetModuleHandleA);
        if (GetModuleHandleA("Iphlpapi.dll") == NULL)
            LoadLibraryA("Iphlpapi.dll");

        auto GetAdaptersInfo = IMPORT(L"Iphlpapi.dll", GetAdaptersInfo);
        if (!GetAdaptersInfo)
            return adapter_info_list;

        PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
        unsigned long stSize = sizeof(IP_ADAPTER_INFO);
        int nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
        int netCardNum = 0;
        int IPnumPerNetCard = 0;
        if (ERROR_BUFFER_OVERFLOW == nRel)
        {
            delete pIpAdapterInfo;
            pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[stSize];
            nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
        }
        if (ERROR_SUCCESS == nRel)
        {
            while (pIpAdapterInfo)
            {
                IP_ADDR_STRING *pIpAddrString = &(pIpAdapterInfo->IpAddressList);
                do
                {
                    std::string gateway_mac;
                    std::string gateway_ip = pIpAdapterInfo->GatewayList.IpAddress.String;
                    if (!gateway_ip.empty() && gateway_ip != "0.0.0.0")
                    {
                        gateway_mac = get_mac_by_ip(gateway_ip);
                        adapter_info_list.emplace_back(std::make_pair(gateway_ip, (gateway_mac.empty() ? "00-00-00-00-00-00" : gateway_mac)));
                    }
                    pIpAddrString = pIpAddrString->Next;
                }
                while (pIpAddrString);
                pIpAdapterInfo = pIpAdapterInfo->Next;
            }
        }
        if (pIpAdapterInfo)
        {
            delete pIpAdapterInfo;
        }
        return adapter_info_list;
    }

	uint32_t get_parent_process_id()
	{
		PROCESS_BASIC_INFORMATION pbi;
		auto NtQueryInformationProcess = IMPORT(L"ntdll.dll", NtQueryInformationProcess);
		NtQueryInformationProcess(GetCurrentProcess(), ProcessBasicInformation, (PVOID)&pbi, sizeof(PROCESS_BASIC_INFORMATION), NULL);
		return (uint32_t)pbi.InheritedFromUniqueProcessId;

	}
}