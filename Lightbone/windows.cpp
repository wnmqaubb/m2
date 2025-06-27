#include "pch.h"
#include "utils.h"
#include "api_resolver.h"
#include <shared_mutex>
#include <filesystem>
#include <set>
#include <wincrypt.h>
#include <optional>
#include <mscat.h>
#include <Uxtheme.h>
#include <Psapi.h>
#pragma comment(lib, "Wintrust.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "Uxtheme.lib")

using namespace Utils;
using namespace Utils::String;

CWindows::access_mask_t CWindows::ProcessQueryAccess;
CWindows::access_mask_t CWindows::ProcessAllAccess;
CWindows::access_mask_t CWindows::ThreadQueryAccess;
CWindows::access_mask_t CWindows::ThreadSetAccess;
CWindows::access_mask_t CWindows::ThreadAllAccess;
std::map<uint32_t, void*> CWindows::api_;
// 静态成员初始化
CWindows::WindowsList CWindows::cachedWindowsList;
std::chrono::steady_clock::time_point CWindows::lastUpdateTime = std::chrono::steady_clock::now();
bool CWindows::lastExcludeSystem = false;
bool CWindows::lastExcludeSigned = false;
std::mutex CWindows::cacheMutex;
std::unordered_map<DWORD, std::pair<std::wstring, bool>> CWindows::process_cache;
CWindows::CWindows()
{
    system_version_ = get_system_version_();
    initialize_access();
    if (!GetModuleHandleW(L"Wintrust.dll")) LoadLibraryW(L"Wintrust.dll");
    if (!GetModuleHandleW(L"advapi32.dll")) LoadLibraryW(L"advapi32.dll");
    if (!GetModuleHandleW(L"crypt32.dll")) LoadLibraryW(L"crypt32.dll");
    // 禁用XP的主题样式以获得更好兼容性
    if (system_version_ <= WINDOWS_XP) {
        SetThemeAppProperties(STAP_ALLOW_NONCLIENT | STAP_ALLOW_CONTROLS);
    }
}

CWindows::~CWindows()
{

}

CWindows& CWindows::instance()
{
    static CWindows instance_;
    return instance_;
}
CWindows::access_mask_t CWindows::get_process_query_access()
{
    return ProcessQueryAccess;
}
CWindows::access_mask_t CWindows::get_process_all_access()
{
    return ProcessAllAccess;
}
CWindows::access_mask_t CWindows::get_thread_query_access()
{
    return ThreadQueryAccess;
}
CWindows::access_mask_t CWindows::get_thread_set_access()
{
    return ThreadSetAccess;
}
CWindows::access_mask_t CWindows::get_thread_all_access()
{
    return ThreadAllAccess;
}

void CWindows::initialize_access()
{
    if (system_version_ >= WINDOWS_VISTA)
    {
        ProcessQueryAccess = PROCESS_QUERY_LIMITED_INFORMATION;
        ProcessAllAccess = STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0x1fff;
        ThreadQueryAccess = THREAD_QUERY_LIMITED_INFORMATION;
        ThreadSetAccess = THREAD_SET_LIMITED_INFORMATION;
        ThreadAllAccess = STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0xfff;
    }
    else if (system_version_ >= WINDOWS_XP) {  // XP专用设置
        ProcessQueryAccess = PROCESS_QUERY_INFORMATION | PROCESS_VM_READ;
        ProcessAllAccess = STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0xFFF;
        ThreadQueryAccess = THREAD_QUERY_INFORMATION;
        ThreadSetAccess = THREAD_SET_INFORMATION;
        ThreadAllAccess = STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0x3FF;
    }
    else
    {
        ProcessQueryAccess = PROCESS_QUERY_INFORMATION;
        ProcessAllAccess = STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0xfff;
        ThreadQueryAccess = THREAD_QUERY_INFORMATION;
        ThreadSetAccess = THREAD_SET_INFORMATION;
        ThreadAllAccess = STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0x3ff;
    }
}

void* CWindows::get_proc_address(uint32_t module_hash, uint32_t func_hash)
{
    if (api_.find(module_hash ^ func_hash) != api_.end())
    {
        return api_[module_hash ^ func_hash];
    }
    HMODULE module_handle = ApiResolver::get_module_handle(module_hash);
    if (!module_handle)
    {
        return nullptr;
    }
    return api_[module_hash ^ func_hash] = ApiResolver::get_proc_address(module_handle, func_hash);
}

CWindows::SystemVersion CWindows::get_system_version_()
{
    auto RtlGetVersion = IMPORT(L"ntdll.dll", RtlGetVersion);
    RTL_OSVERSIONINFOEXW version_info;
    ULONG major_version;
    ULONG minor_version;

    version_info.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);

    if (!NT_SUCCESS(RtlGetVersion(&version_info)))
    {
        return WINDOWS_NEW;
    }

    major_version = version_info.dwMajorVersion;
    minor_version = version_info.dwMinorVersion;

    if (major_version == 5 && minor_version < 1 || major_version < 5)
    {
        return WINDOWS_ANCIENT;
    }
    /* Windows XP */
    else if (major_version == 5 && minor_version == 1)
    {
        return WINDOWS_XP;
    }
    /* Windows Server 2003 */
    else if (major_version == 5 && minor_version == 2)
    {
        return WINDOWS_SERVER_2003;
    }
    /* Windows Vista, Windows Server 2008 */
    else if (major_version == 6 && minor_version == 0)
    {
        return WINDOWS_VISTA;
    }
    /* Windows 7, Windows Server 2008 R2 */
    else if (major_version == 6 && minor_version == 1)
    {
        return WINDOWS_7;
    }
    /* Windows 8 */
    else if (major_version == 6 && minor_version == 2)
    {
        return WINDOWS_8;
    }
    /* Windows 8.1 */
    else if (major_version == 6 && minor_version == 3)
    {
        return WINDOWS_8_1;
    }
    /* Windows 10 */
    else if (major_version == 10 && minor_version == 0)
    {
        return WINDOWS_10;
    }
    else if (major_version == 10 && minor_version > 0 || major_version > 10)
    {
        return WINDOWS_NEW;
    }

    return WINDOWS_NEW;
}
bool CWindows::is_64bits_system()
{
#ifndef _WIN64
    auto IsWow64Process = IMPORT(L"kernel32.dll", IsWow64Process);
    if (!IsWow64Process)
        return false;
    BOOL is_64bit = FALSE;
    IsWow64Process(GetCurrentProcess(), &is_64bit);
    return is_64bit;
#else
    return true;
#endif
}

bool CWindows::is_64bits_process(HANDLE process_handle)
{
    auto IsWow64Process = IMPORT(L"kernel32.dll", IsWow64Process);
    if (is_64bits_system() == false)
        return false;
    if (!IsWow64Process)
        return false;
    BOOL is_32bit = FALSE;
    IsWow64Process(process_handle, &is_32bit);
    return !is_32bit;
}
using SmartHandle = std::unique_ptr<void, decltype(&::NtClose)>;
#pragma warning(push)
#pragma warning(disable:4244)
template<>
NTSTATUS CWindows::read_virtual_memory<uint64_t>(
    HANDLE handle,
    uint64_t base_address,
    PVOID buffer,
    uint64_t buffer_size,
    uint64_t* bytes_of_read)
{
    // XP需要更严格的内存对齐检查
    if (system_version_ <= WINDOWS_XP && (base_address % 4 != 0)) {
        return STATUS_DATATYPE_MISALIGNMENT;
    }
    auto NtWow64ReadVirtualMemory64 = IMPORT(L"ntdll.dll", NtWow64ReadVirtualMemory64);
    return NtWow64ReadVirtualMemory64(handle, (PVOID64)base_address, buffer, buffer_size, bytes_of_read);
}

template<>
NTSTATUS CWindows::read_virtual_memory<uint32_t>(
    HANDLE handle,
    uint64_t base_address,
    PVOID buffer,
    uint64_t buffer_size,
    uint64_t* bytes_of_read)
{
    auto NtReadVirtualMemory = IMPORT(L"ntdll.dll", NtReadVirtualMemory);
    return NtReadVirtualMemory(handle, (PVOID)base_address, buffer, buffer_size, (SIZE_T*)bytes_of_read);
}
template<>
NTSTATUS
Utils::CWindows::query_information_process<uint32_t>(
    HANDLE process_handle,
    uint32_t process_information_class,
    PVOID process_information,
    uint32_t process_information_length,
    uint32_t* return_length)
{
    auto NtQueryInformationProcess = IMPORT(L"ntdll.dll", NtQueryInformationProcess);
    return NtQueryInformationProcess(process_handle, (PROCESSINFOCLASS)process_information_class, process_information, process_information_length, (PULONG)return_length);
}


template<>
NTSTATUS
Utils::CWindows::query_information_process<uint64_t>(
    HANDLE process_handle,
    uint32_t process_information_class,
    PVOID process_information,
    uint32_t process_information_length,
    uint32_t* return_length)
{
    auto NtWow64QueryInformationProcess64 = IMPORT(L"ntdll.dll", NtWow64QueryInformationProcess64);
    return NtWow64QueryInformationProcess64(process_handle, (PROCESSINFOCLASS)process_information_class, process_information, process_information_length, (PULONG)return_length);
}
#pragma warning(pop)

template <typename T>
void CWindows::ldr_walk(HANDLE handle, ModuleList& modules)
{
    _PROCESS_BASIC_INFORMATION_T<T> pbi;
    uint64_t bytes_of_read = 0;
    NTSTATUS status = 0;

    status = query_information_process<T>(handle, ProcessBasicInformation, &pbi, sizeof(pbi), NULL);
    if (!NT_SUCCESS(status))
        return;
    T ldr_ptr = 0;
    status = read_virtual_memory<T>(handle, pbi.PebBaseAddress + offsetof(_PEB_T<T>, Ldr), &ldr_ptr, sizeof(ldr_ptr), &bytes_of_read);
    if (!NT_SUCCESS(status))
        return;

    T list_entry_ptr = 0;

    const T ofst = offsetof(_LDR_DATA_TABLE_ENTRY_BASE_T<T>, InMemoryOrderLinks);
    T head = ldr_ptr + offsetof(_PEB_LDR_DATA2_T<T>, InMemoryOrderModuleList);

    status = read_virtual_memory<T>(handle, head, &list_entry_ptr, sizeof(list_entry_ptr), &bytes_of_read);

    if (!NT_SUCCESS(status))
        return;
    while (list_entry_ptr != head)
    {
        _LDR_DATA_TABLE_ENTRY_BASE_T<T> ldr_data_table;
        status = read_virtual_memory<T>(handle, list_entry_ptr - ofst, &ldr_data_table, sizeof(ldr_data_table), &bytes_of_read);
        if (!NT_SUCCESS(status))
            break;
        ModuleInfo module;
        wchar_t buffer[MAX_PATH] = { 0 };
        module.base = ldr_data_table.DllBase;
        read_virtual_memory<T>(handle, ldr_data_table.BaseDllName.Buffer, buffer, min(ldr_data_table.BaseDllName.Length, MAX_PATH), &bytes_of_read);
        module.module_name = buffer;
        read_virtual_memory<T>(handle, ldr_data_table.FullDllName.Buffer, buffer, min(ldr_data_table.FullDllName.Length, MAX_PATH), &bytes_of_read);
        module.path = buffer;
        module.size_of_image = ldr_data_table.SizeOfImage;
        modules.push_back(module);
        list_entry_ptr = ldr_data_table.InMemoryOrderLinks.Flink;
    }
}

bool CWindows::get_module_from_address(uint32_t pid, uint64_t address, __in ModuleList& modules, __out ModuleInfo& module_out)
{
    ModuleList::iterator itor = std::find_if(modules.begin(), modules.end(), [address](ModuleInfo& module)->bool {
        if (module.base <= address && address <= module.base + module.size_of_image)
        {
            return true;
        }
        return false;
    });
    if (itor == modules.end())
    {
        return false;
    }
    module_out = *itor;
    return true;
}
CWindows::ModuleList CWindows::enum_modules(uint32_t pid, bool& is_64bits)
{
    auto NtOpenProcess = IMPORT(L"ntdll.dll", NtOpenProcess);
    auto NtClose = IMPORT(L"ntdll.dll", NtClose);

    CWindows::ModuleList modules;

    SmartHandle handle(nullptr, NtClose);

    CLIENT_ID client_id = { (PVOID)pid, 0 };
    NTSTATUS status = 0;
    HANDLE handle_ = 0;
    OBJECT_ATTRIBUTES object_attributes = { sizeof(OBJECT_ATTRIBUTES) };
    status = NtOpenProcess(&handle_, ProcessQueryAccess | PROCESS_VM_READ, &object_attributes, &client_id);
    handle.reset(handle_);
    if (!handle)
    {
        set_last_status(status);
        return modules;
    }

    do
    {
        is_64bits = is_64bits_process(handle.get());
        if (!is_64bits)
        {
            ldr_walk<uint32_t>(handle.get(), modules);
        }
        else
        {
            ldr_walk<uint64_t>(handle.get(), modules);
        }
    } while (0);
    set_last_status(STATUS_SUCCESS);
    return modules;
}

NTSTATUS CWindows::get_processes(PVOID* processes, uint32_t& system_information_class)
{
    auto NtQuerySystemInformation = IMPORT(L"ntdll.dll", NtQuerySystemInformation);
    NTSTATUS status;
    ULONG bytes_of_read = 0x4000;

    system_information_class = SystemExtendedProcessInformation;
    uint8_t* buffer = new uint8_t[bytes_of_read];

    while (TRUE)
    {
        status = NtQuerySystemInformation((SYSTEM_INFORMATION_CLASS)system_information_class, buffer, bytes_of_read, &bytes_of_read);

        if (status == STATUS_BUFFER_TOO_SMALL || status == STATUS_INFO_LENGTH_MISMATCH)
        {
            delete[] buffer;
            buffer = new uint8_t[bytes_of_read];
        }
        else if (!NT_SUCCESS(status) && system_information_class == SystemExtendedProcessInformation)
        {
            system_information_class = SystemProcessInformation;
        }
        else
        {
            break;
        }
    }

    if (!NT_SUCCESS(status))
    {
        delete[] buffer;
        return status;
    }

    *processes = buffer;

    return status;
}

bool CWindows::is_system_process(DWORD pid) {
    auto OpenProcess = IMPORT(L"kernel32.dll", OpenProcess);

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!hProcess) return false;

    HANDLE hToken;
    if (!OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) {
        CloseHandle(hProcess);
        return false;
    }

    DWORD dwSize = 0;
    GetTokenInformation(hToken, TokenUser, NULL, 0, &dwSize);
    PTOKEN_USER pTokenUser = (PTOKEN_USER)malloc(dwSize);
    GetTokenInformation(hToken, TokenUser, pTokenUser, dwSize, &dwSize);

    char szAccount[256] = { 0 };
    char szDomain[256] = { 0 };
    DWORD dwAccountLen = 256, dwDomainLen = 256;
    SID_NAME_USE sidType;

    LookupAccountSidA(
        NULL,
        pTokenUser->User.Sid,
        szAccount,
        &dwAccountLen,
        szDomain,
        &dwDomainLen,
        &sidType
    );

    free(pTokenUser);
    CloseHandle(hToken);
    CloseHandle(hProcess);

    return (strcmp(szAccount, "SYSTEM") == 0) ||
        (strcmp(szAccount, "LOCAL SERVICE") == 0) ||
        (strcmp(szAccount, "NETWORK SERVICE") == 0);
}

CWindows::ProcessMap CWindows::enum_process(std::function<bool(ProcessInfo& process)> callback, bool exclude_system_process, bool exclude_signed)
{
    static std::mutex mtx;
    std::lock_guard<std::mutex> lck(mtx);
    auto NtQueryInformationThread = IMPORT(L"ntdll.dll", NtQueryInformationThread);
    auto OpenThread = IMPORT(L"kernel32.dll", OpenThread);
    ProcessMap process_map;
    power();
    // XP使用旧版API获取进程信息
    uint32_t system_information_class = SystemProcessInformation;
    // XP需要不同的系统信息类
    if (system_version_ >= WINDOWS_VISTA) {
        system_information_class = SystemExtendedProcessInformation;
    }
    NTSTATUS status;
    static PVOID processes = NULL;
    static ULONG64 lastProcessesTickCount = 0;
    PWCHAR name;
    ULONG64 tickCount;
    tickCount = GetTickCount64();
    if (tickCount - lastProcessesTickCount >= 5000)
    {
        if (processes)
        {
            delete[] processes;
            processes = NULL;
        }
        if (!NT_SUCCESS(get_processes(&processes, system_information_class)))
        {
            return process_map;
        }
        lastProcessesTickCount = tickCount;
    }
    if (!processes)
    {
        return process_map;
    }

    for (SYSTEM_PROCESS_INFORMATION* info = (SYSTEM_PROCESS_INFORMATION*)processes;;)
    {
        ProcessInfo process = { 0 };
        process.pid = reinterpret_cast<uint32_t>(info->UniqueProcessId);
        // 排除系统进程
        if (exclude_system_process && is_system_process(process.pid)) {
            if (info->NextEntryOffset)
            {
                info = reinterpret_cast<SYSTEM_PROCESS_INFORMATION*>((uint8_t*)info + info->NextEntryOffset);
                continue;
            }
            else
                break;
        }
        // 排除有签名的进程
        //if (system_version_ >= WINDOWS_VISTA && exclude_signed && verify_signature(process.pid)) {
        //    if (info->NextEntryOffset)
        //    {
        //        info = reinterpret_cast<SYSTEM_PROCESS_INFORMATION*>((uint8_t*)info + info->NextEntryOffset);
        //        continue;
        //    }
        //    else
        //        break;
        //}
        // 获取进程名（XP兼容方式）
        if (info->ImageName.Buffer) {
            process.name = std::wstring(info->ImageName.Buffer, info->ImageName.Length / sizeof(WCHAR));
        }
        process.parent_pid = reinterpret_cast<uint32_t>(info->InheritedFromUniqueProcessId);
        process.modules = enum_modules(process.pid, process.is_64bits);
        if (!NT_SUCCESS(get_last_status()))
        {
            process.no_access = true;
        }
        uint64_t min_time = (std::numeric_limits<uint64_t>::max)();
        uint32_t main_thread_tid = 0;

        for (uint32_t i = 0; i < info->NumberOfThreads; i++)
        {
            if (system_information_class == SystemExtendedProcessInformation)
            {
                ThreadInfo thread;
                auto& thd = info->Threads[i].ThreadInfo;

                thread.tid = reinterpret_cast<uint32_t>(thd.ClientId.UniqueThread);
                if (system_version_ >= WINDOWS_VISTA) {
                    if (process.is_64bits && process.no_access == false)
                    {
                        HANDLE thread_handle = OpenThread(THREAD_QUERY_INFORMATION, FALSE, thread.tid);
                        uint64_t start_address = 0;
                        ULONG return_length = 0;
                        status = NtWow64QueryInformationThread64(thread_handle, ThreadQuerySetWin32StartAddress, &start_address, sizeof(start_address), &return_length);
                        CloseHandle(thread_handle);
                        if (NT_SUCCESS(status))
                        {
                            thread.start_address = start_address;
                        }
                    }
                    else
                    {
                        thread.start_address = reinterpret_cast<uintptr_t>(info->Threads[i].Win32StartAddress);
                    }
                }


                if (thd.CreateTime.QuadPart < min_time)
                {
                    min_time = thd.CreateTime.QuadPart;
                    main_thread_tid = thread.tid;
                }
                process.threads.emplace(std::make_pair(thread.tid, thread));
            }
            else {
                ThreadInfo thread;
                SYSTEM_THREAD_INFORMATION thd = ((PSYSTEM_THREAD_INFORMATION)(info->Threads))[i];

                thread.tid = reinterpret_cast<uint32_t>(thd.ClientId.UniqueThread);
                if (system_version_ >= WINDOWS_VISTA) {
                    if (thread.tid > 0)
                    {
                        HANDLE thread_handle = OpenThread(THREAD_QUERY_INFORMATION, FALSE, thread.tid);
                        uint32_t start_address = 0;
                        ULONG return_length = 0;
                        if (thread_handle != NULL)
                        {
                            status = NtQueryInformationThread(thread_handle, ThreadQuerySetWin32StartAddress, &start_address, sizeof(start_address), &return_length);
                            CloseHandle(thread_handle);
                            if (NT_SUCCESS(status))
                            {
                                thread.start_address = start_address;
                            }
                        }
                    }
                }

                if (thd.CreateTime.QuadPart < min_time)
                {
                    min_time = thd.CreateTime.QuadPart;
                    main_thread_tid = thread.tid;
                }
                process.threads.emplace(std::make_pair(thread.tid, thread));
            }

        }
        process.threads[main_thread_tid].is_main_thread = true;
        process_map.emplace(std::make_pair(process.pid, process));
        if (callback(process) == false)
        {
            return process_map;
        }

        if (info->NextEntryOffset)
            info = reinterpret_cast<SYSTEM_PROCESS_INFORMATION*>((uint8_t*)info + info->NextEntryOffset);
        else
            break;
    }
    return process_map;
}

CWindows::ProcessMap CWindows::enum_process_with_dir(std::function<bool(ProcessInfo& process)> callback, bool exclude_system_process, bool exclude_signed)
{
    static std::mutex mtx;
    std::lock_guard<std::mutex> lck(mtx);
    auto NtQueryInformationThread = IMPORT(L"ntdll.dll", NtQueryInformationThread);
    auto OpenThread = IMPORT(L"kernel32.dll", OpenThread);
    auto GetFileSizeEx = IMPORT(L"kernel32.dll", GetFileSizeEx);
    ProcessMap process_map;
    power();
    // XP使用旧版API获取进程信息
    uint32_t system_information_class = SystemProcessInformation;
    // 根据系统版本选择信息类
    if (system_version_ >= WINDOWS_VISTA) {
        system_information_class = SystemExtendedProcessInformation;
    }
    NTSTATUS status;
    static PVOID processes = NULL;
    static ULONG64 lastProcessesTickCount = 0;
    PWCHAR name;
    ULONG64 tickCount;
    tickCount = GetTickCount64();
    if (tickCount - lastProcessesTickCount >= 5000)
    {
        if (processes)
        {
            delete[] processes;
            processes = NULL;
        }

        if (!NT_SUCCESS(get_processes(&processes, system_information_class)))
        {
            return process_map;
        }
        lastProcessesTickCount = tickCount;
    }
    if (!processes)
    {
        return process_map;
    }

    for (SYSTEM_PROCESS_INFORMATION* info = (SYSTEM_PROCESS_INFORMATION*)processes;;)
    {
        ProcessInfo process = { 0 };
        process.pid = reinterpret_cast<uint32_t>(info->UniqueProcessId);
        // 排除系统进程
        if (exclude_system_process && is_system_process(process.pid)) {
            if (info->NextEntryOffset)
            {
                info = reinterpret_cast<SYSTEM_PROCESS_INFORMATION*>((uint8_t*)info + info->NextEntryOffset);
                continue;
            }
            else
                break;
        }
        // 排除有签名的进程
        //if (system_version_ >= WINDOWS_VISTA && exclude_signed && verify_signature(process.pid)) {
        //    if (info->NextEntryOffset)
        //    {
        //        info = reinterpret_cast<SYSTEM_PROCESS_INFORMATION*>((uint8_t*)info + info->NextEntryOffset);
        //        continue;
        //    }
        //    else
        //        break;
        //}
        process.name = info->ImageName.Buffer ? info->ImageName.Buffer : L"";
        process.parent_pid = reinterpret_cast<uint32_t>(info->InheritedFromUniqueProcessId);
        process.modules = enum_modules(process.pid, process.is_64bits);
        if (!NT_SUCCESS(get_last_status()))
        {
            process.no_access = true;
        }
        uint64_t min_time = (std::numeric_limits<uint64_t>::max)();
        uint32_t main_thread_tid = 0;

        for (uint32_t i = 0; i < info->NumberOfThreads; i++)
        {
            if (system_information_class == SystemExtendedProcessInformation)
            {
                ThreadInfo thread;
                auto& thd = info->Threads[i].ThreadInfo;

                thread.tid = reinterpret_cast<uint32_t>(thd.ClientId.UniqueThread);

                if (system_version_ >= WINDOWS_VISTA) {
                    if (process.is_64bits && process.no_access == false)
                    {
                        HANDLE thread_handle = OpenThread(THREAD_QUERY_INFORMATION, FALSE, thread.tid);
                        uint64_t start_address = 0;
                        ULONG return_length = 0;
                        status = NtWow64QueryInformationThread64(thread_handle, ThreadQuerySetWin32StartAddress, &start_address, sizeof(start_address), &return_length);
                        CloseHandle(thread_handle);
                        if (NT_SUCCESS(status))
                        {
                            thread.start_address = start_address;
                        }
                    }
                    else
                    {
                        thread.start_address = reinterpret_cast<uintptr_t>(info->Threads[i].Win32StartAddress);
                    }
                }


                if (thd.CreateTime.QuadPart < min_time)
                {
                    min_time = thd.CreateTime.QuadPart;
                    main_thread_tid = thread.tid;
                }
                process.threads.emplace(std::make_pair(thread.tid, thread));
            }
            else {
                ThreadInfo thread;
                SYSTEM_THREAD_INFORMATION thd = ((PSYSTEM_THREAD_INFORMATION)(info->Threads))[i];

                thread.tid = reinterpret_cast<uint32_t>(thd.ClientId.UniqueThread);

                if (system_version_ >= WINDOWS_VISTA) {
                    if (thread.tid > 0)
                    {
                        HANDLE thread_handle = OpenThread(THREAD_QUERY_INFORMATION, FALSE, thread.tid);
                        uint32_t start_address = 0;
                        ULONG return_length = 0;
                        if (thread_handle != NULL)
                        {
                            status = NtQueryInformationThread(thread_handle, ThreadQuerySetWin32StartAddress, &start_address, sizeof(start_address), &return_length);
                            CloseHandle(thread_handle);
                            if (NT_SUCCESS(status))
                            {
                                thread.start_address = start_address;
                            }
                        }
                    }
                }

                if (thd.CreateTime.QuadPart < min_time)
                {
                    min_time = thd.CreateTime.QuadPart;
                    main_thread_tid = thread.tid;
                }
                process.threads.emplace(std::make_pair(thread.tid, thread));
            }

        }
        process.threads[main_thread_tid].is_main_thread = true;
        if (!process.modules.empty())
        {
            auto process_path = process.modules.front().path;
            auto walk_path = std::filesystem::path(process_path).parent_path();
            std::error_code ec;
            size_t file_count = 0;
            if (!process_path.empty() && std::filesystem::exists(process_path, ec))
            {
                process.process_file_size = std::filesystem::file_size(process_path);
                if (process.process_file_size <= 0)
                {
                    LARGE_INTEGER fileSize;
                    HANDLE hFile = CreateFile(process_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                    if (hFile != INVALID_HANDLE_VALUE)
                    {
                        if (GetFileSizeEx(hFile, &fileSize))
                        {
                            process.process_file_size = fileSize.QuadPart;
                        }
                        CloseHandle(hFile);
                    }
                }
            }
            for (auto& file : std::filesystem::directory_iterator(walk_path, ec))
            {
                if (file_count++ >= 50)
                {
                    break;
                }
                DirectoryInfo directory;
                directory.is_directory = file.is_directory();
                directory.path = file.path().filename();
                process.directories.emplace_back(directory);
            }
        }
        process_map.emplace(std::make_pair(process.pid, process));
        if (callback(process) == false)
        {
            return process_map;
        }

        if (info->NextEntryOffset)
            info = reinterpret_cast<SYSTEM_PROCESS_INFORMATION*>((uint8_t*)info + info->NextEntryOffset);
        else
            break;
    }
    return process_map;
}

void Utils::CWindows::ldr_walk_32(HANDLE handle, ModuleList& modules)
{
    return ldr_walk<uint32_t>(handle, modules);
}

void Utils::CWindows::ldr_walk_64(HANDLE handle, ModuleList& modules)
{
    return ldr_walk<uint64_t>(handle, modules);
}

CWindows::ProcessMap CWindows::enum_process(bool exclude_system_process, bool exclude_signed)
{
    return enum_process([](ProcessInfo&)->bool {
        return true;
    }, exclude_system_process, exclude_signed);
}

CWindows::ProcessMap CWindows::enum_process_with_dir(bool exclude_system_process, bool exclude_signed)
{
    return enum_process_with_dir([](ProcessInfo&)->bool {
        return true;
    }, exclude_system_process, exclude_signed);
}

bool Utils::CWindows::get_process(uint32_t pid, __out ProcessInfo& process)
{
    bool found = false;
    enum_process([&found, pid, &process](ProcessInfo& target)->bool {
        if (target.pid == pid)
        {
            found = true;
            process = target;
            return false;
        }
        return true;
    });
    return found;
}

bool CWindows::power()
{
    auto GetCurrentProcess = IMPORT(L"kernel32.dll", GetCurrentProcess);
    auto OpenProcessToken = IMPORT(L"advapi32.dll", OpenProcessToken);
    auto LookupPrivilegeValueW = IMPORT(L"advapi32.dll", LookupPrivilegeValueW);
    auto AdjustTokenPrivileges = IMPORT(L"advapi32.dll", AdjustTokenPrivileges);

    if (!GetCurrentProcess || !OpenProcessToken || !LookupPrivilegeValueW || !AdjustTokenPrivileges) {
        return false;
    }
    TOKEN_PRIVILEGES tp;
    HANDLE hToken;
    LUID luid;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken)) {
        if (LookupPrivilegeValueW(nullptr, SE_DEBUG_NAME, &luid)) {
            tp.PrivilegeCount = 1;
            tp.Privileges[0].Luid = luid;
            if (TRUE) {
                tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            }
            else {
                tp.Privileges[0].Attributes = 0;
            }
            if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL)) {
                return false;
            }
            return true;
        }
    }
    return false;
}

std::wstring CWindows::get_process_path(uint32_t pid, ProcessMap& processes)
{
    if (processes.find(pid) == processes.end())
    {
        return L"not found";
    }
    if (processes[pid].no_access)
    {
        return processes[pid].name + L"(no access)";
    }
    if (processes[pid].modules.size() == 0)
    {
        return processes[pid].name + L"(unknown)";
    }
    return processes[pid].modules.front().path;
}

std::wstring CWindows::get_process_path(uint32_t pid)
{
    ProcessInfo process;
    if (!get_process(pid, process))
    {
        return L"not found";
    }
    if (process.no_access)
    {
        return process.name + L"(no access)";
    }
    if (process.modules.size() == 0)
    {
        return process.name + L"(unknown)";
    }
    return process.modules.front().path;
}
uint32_t CWindows::get_current_process_id()
{
    auto GetCurrentProcessId = IMPORT(L"kernel32.dll", GetCurrentProcessId);
    return GetCurrentProcessId();
}
uint32_t CWindows::get_process_parent(uint32_t pid)
{
    ProcessInfo process;
    if (!get_process(pid, process))
    {
        return -1;
    }
    return process.parent_pid;
}

bool CWindows::is_process_open_from_explorer(uint32_t pid)
{
    uint32_t parent_pid = get_process_parent(pid);
    ProcessInfo parent_process;
    if (!get_process(parent_pid, parent_process))
    {
        return false;
    }
    std::vector<std::wstring> desktop_list = {
        {'e', 'x', 'p', 'l', 'o', 'r', 'e', 'r', '.', 'e', 'x', 'e'},
        {'d', 'e', 's', 'k', 't', 'o', 'p', 'm', 'g', 'r', '6', '4', '.', 'e', 'x', 'e'},
        {'d', 'e', 's', 'k', 't', 'o', 'p', 'm', 'g', 'r', '.', 'e', 'x', 'e'},
        {'3', '6', '0'},
        {'g', 'a', 'm', 'e', 'c', 'a', 'p', '.', 'e', 'x', 'e'},
        {'w', 'i', 'n', 'r', 'a', 'r', '.', 'e', 'x', 'e'},
        {'7', 'z', 'f', 'm', '.', 'e', 'x', 'e'}
    };
    transform(parent_process.name.begin(), parent_process.name.end(), parent_process.name.begin(), ::towlower);
    for (std::wstring desktop : desktop_list)
    {
        if (parent_process.name.find(desktop) == 0)
        {
            return true;
        }
    }
    return false;
}

CWindows::WindowsList CWindows::enum_windows(bool exclude_system_process, bool exclude_signed)
{
    std::lock_guard<std::mutex> lock(cacheMutex);
    auto now = std::chrono::steady_clock::now();

    // 检查缓存有效性（5秒内且参数相同）
    if (exclude_system_process == lastExcludeSystem &&
        //exclude_signed == lastExcludeSigned &&
        std::chrono::duration_cast<std::chrono::minutes>(now - lastUpdateTime).count() < 3 &&
        !cachedWindowsList.empty()) {
        return cachedWindowsList;
    }

    // 重新枚举窗口
    std::vector<HWND> hwnds;
    if (auto EnumDesktopWindows = IMPORT(L"user32.dll", EnumDesktopWindows)) {
        EnumDesktopWindows(nullptr, &EnumWindowsProc, reinterpret_cast<LPARAM>(&hwnds));
    }

    WindowsList result;
    result.reserve(hwnds.size());  // 关键优化：预分配

    //auto GetWindowTextW = IMPORT(L"user32.dll", GetWindowTextW);
    auto GetClassNameW = IMPORT(L"user32.dll", GetClassNameW);
    auto GetWindowThreadProcessId = IMPORT(L"user32.dll", GetWindowThreadProcessId);
    try {
        for (HWND hwnd : hwnds) {
            // 获取进程信息
            DWORD process_id = 0;
            DWORD thread_id = GetWindowThreadProcessId(hwnd, &process_id);

            // 系统进程过滤
            if (exclude_system_process && is_system_process(process_id)) {
                continue;
            }

            // 签名验证过滤
            //if (system_version_ >= WINDOWS_VISTA && exclude_signed && verify_signature(process_id)) {
            //    continue;
            //}

            // 获取窗口基础信息
            wchar_t className[MAX_PATH] = { };
            if (!GetClassNameW(hwnd, className, MAX_PATH) || className[0] == L'\0'
                || wcscmp(className, L"TFrmJSYDlg") == 0) continue;

            wchar_t title[MAX_PATH] = { };
            // 获取窗口基础信息
            LRESULT gettext_result = SendMessageTimeoutW(
                hwnd,
                WM_GETTEXT,
                MAX_PATH,
                reinterpret_cast<LPARAM>(title),
                SMTO_ABORTIFHUNG | SMTO_ERRORONEXIT,
                30,  // 50ms超时
                nullptr
            );

            if (gettext_result == 0) title[0] = L'\0'; // 超时或失败

            // 快速跳过不可见窗口和空窗口
            if ((title[0] == L'\0')
                //|| !IsWindowVisible(hwnd) ||
                //IsIconic(hwnd)
                ) {
                continue;
            }

            // 签名验证过滤
            //if (system_version_ >= WINDOWS_VISTA && exclude_signed && verify_signature(process_id)) {
            //    continue;
            //}

            // 构建窗口信息
            result.emplace_back(WindowInfo{
                /*hwnd*/      hwnd,
                /*caption*/    title,
                /*class_name*/ className,
                /*pid*/       process_id,
                /*tid*/       thread_id
                                });
        }
    }
    catch (const std::exception& e) {
        printf("enum_windows 异常：", e.what());
    }
    catch (...) {
        printf("enum_windows 未知异常");
    }

    // 更新缓存
    cachedWindowsList = std::move(result);
    lastExcludeSystem = exclude_system_process;
    lastExcludeSigned = exclude_signed;
    lastUpdateTime = now;

    return cachedWindowsList;
};

std::wstring CWindows::ntpath2win32path(std::wstring ntPath)
{
    wchar_t system_path[MAX_PATH] = L"";
    GetWindowsDirectory(system_path, MAX_PATH);
    if (ntPath._Starts_with(L"\\\\?\\"))
    {
        ntPath.erase(ntPath.begin(), ntPath.begin() + 4);
        return ntPath;
    }
    if (ntPath._Starts_with(L"\\??\\"))
    {
        ntPath.erase(ntPath.begin(), ntPath.begin() + 4);
    }
    if (ntPath._Starts_with(L"\\"))
    {
        ntPath.erase(ntPath.begin(), ntPath.begin() + 1);
    }
    if (ntPath._Starts_with(L"globalroot\\"))
    {
        ntPath.erase(ntPath.begin(), ntPath.begin() + 11);
    }
    if (ntPath._Starts_with(L"SystemRoot"))
    {
        ntPath.replace(ntPath.begin(), ntPath.begin() + 10, system_path);
    }
    if (ntPath._Starts_with(L"Windows"))
    {
        ntPath.replace(ntPath.begin(), ntPath.begin() + 7, system_path);
    }
    return ntPath;
}

CWindows::DriverList CWindows::enum_drivers()
{
    auto NtQuerySystemInformation = IMPORT(L"ntdll.dll", NtQuerySystemInformation);
    auto OpenThread = IMPORT(L"kernel32.dll", OpenThread);

    DriverList drivers;
    ULONG bytes_of_read = 0;
    NTSTATUS status;
    std::unique_ptr<uint8_t[]> buffer(new uint8_t[0x100]);
    status = NtQuerySystemInformation(SystemModuleInformation, buffer.get(), 0x100, &bytes_of_read);
    if (!NT_SUCCESS(status))
    {
        buffer.reset(new uint8_t[bytes_of_read]);
        status = NtQuerySystemInformation(SystemModuleInformation, buffer.get(), bytes_of_read, &bytes_of_read);
        if (!NT_SUCCESS(status))
        {
            return drivers;
        }
    }
    SYSTEM_MODULE_INFORMATION* info = (SYSTEM_MODULE_INFORMATION*)buffer.get();
    for (uint32_t i = 0; i < info->ModuleCount; i++)
    {
        DriverInfo driver = { 0 };
        SYSTEM_MODULE module_info = info->Module[i];
        driver.image_name = module_info.ImageName ? c2w(module_info.ImageName) : L"";
        driver.image_name = ntpath2win32path(driver.image_name);
        driver.image_base = module_info.ImageBase;
        driver.image_size = module_info.ImageSize;
        drivers.push_back(driver);
    }
    return drivers;
}
#pragma optimize("",off)
std::vector<std::wstring> CWindows::enum_device_names()
{
    auto ZwOpenDirectoryObject = IMPORT(L"ntdll.dll", ZwOpenDirectoryObject);
    auto ZwQueryDirectoryObject = IMPORT(L"ntdll.dll", ZwQueryDirectoryObject);
    auto ZwClose = IMPORT(L"ntdll.dll", ZwClose);

    HMODULE hNtdll = NULL;
    UNICODE_STRING     strDirName = {};
    OBJECT_ATTRIBUTES  oba = {};
    NTSTATUS           ntStatus = 0;
    HANDLE             hDirectory = 0;
    std::vector<std::wstring> device_names;
    if (!ZwOpenDirectoryObject || !ZwQueryDirectoryObject || !RtlInitUnicodeString)
        return device_names;
    RtlInitUnicodeString(&strDirName, L"\\??\\Global");
    InitializeObjectAttributes(&oba, &strDirName, OBJ_CASE_INSENSITIVE, NULL, NULL);
    ntStatus = ZwOpenDirectoryObject(&hDirectory, DIRECTORY_QUERY, &oba);
    if (ntStatus != STATUS_SUCCESS)
    {
        return device_names;
    }

    PDIRECTORY_BASIC_INFORMATION   pBuffer = NULL;
    PDIRECTORY_BASIC_INFORMATION   pBuffer2 = NULL;
    ULONG    ulLength = 0x800;    // 2048  
    ULONG    ulContext = 0;
    ULONG    ulRet = 0;
    // 查询目录对象  
    do
    {
        if (pBuffer != NULL)
        {
            free(pBuffer);
        }
        ulLength = ulLength * 2;
        pBuffer = (PDIRECTORY_BASIC_INFORMATION)malloc(ulLength);
        memset(pBuffer, 0, ulLength);
        if (NULL == pBuffer)
        {
            if (pBuffer != NULL)
            {
                free(pBuffer);
            }
            if (hDirectory != NULL)
            {
                ZwClose(hDirectory);
            }

            return device_names;
        }
        ntStatus = ZwQueryDirectoryObject(hDirectory, pBuffer, ulLength, FALSE, TRUE, &ulContext, &ulRet);
    } while (ntStatus == STATUS_MORE_ENTRIES || ntStatus == STATUS_BUFFER_TOO_SMALL);

    if (STATUS_SUCCESS == ntStatus)
    {
        pBuffer2 = pBuffer;
        while ((pBuffer2->ObjectName.Length != 0) && (pBuffer2->ObjectTypeName.Length != 0))
        {
            std::wstring strDriverName;
            strDriverName = pBuffer2->ObjectName.Buffer;
            device_names.push_back(strDriverName);
            pBuffer2++;
        }
    }
    if (pBuffer != NULL)
    {
        free(pBuffer);
    }
    if (hDirectory != NULL)
    {
        ZwClose(hDirectory);
    }
    return device_names;
}
#pragma optimize("",on)

LIGHT_BONE_API PSYSTEM_HANDLE_INFORMATION get_system_handle_info()
{
    auto NtQuerySystemInformation = IMPORT(L"ntdll.dll", NtQuerySystemInformation);
    DWORD buffLen = 0x1000;
    NTSTATUS status;
    BYTE* buff = new BYTE[buffLen];
    do
    {
        status = NtQuerySystemInformation(SystemHandleInformation, buff, buffLen, &buffLen);
        if (status == STATUS_INFO_LENGTH_MISMATCH)
        {
            delete[] buff;
            buff = new BYTE[buffLen];
        }
        else
            break;

    } while (TRUE);
    return (PSYSTEM_HANDLE_INFORMATION)buff;
}

std::wstring CWindows::enum_handle_process_write(DWORD target_pid_)
{
    std::wstring process_name = L"";
    auto OpenProcess = IMPORT(L"kernel32.dll", OpenProcess);
    auto Sleep = IMPORT(L"kernel32.dll", Sleep);
    auto ZwClose = IMPORT(L"ntdll.dll", ZwClose);
    auto ZwQueryObject = IMPORT(L"ntdll.dll", ZwQueryObject);
    auto ZwDuplicateObject = IMPORT(L"ntdll.dll", ZwDuplicateObject);
    auto NtQueryInformationProcess = IMPORT(L"ntdll.dll", NtQueryInformationProcess);
    auto GetCurrentProcess = IMPORT(L"kernel32.dll", GetCurrentProcess);

    DWORD target_pid = target_pid_ == 0 ? GetCurrentProcessId() : target_pid_;
    NTSTATUS Status;
    SYSTEM_HANDLE_TABLE_ENTRY_INFO* CurHandle;
    OBJECT_TYPE_INFORMATION* ObjectType;
    char BufferForObjectType[1024];
    HANDLE hDuplicate = NULL, hSource = NULL;
    SYSTEM_HANDLE_INFORMATION* system_handle_info = get_system_handle_info();
    if (system_handle_info)
    {
        auto parent_processID = get_process_parent(target_pid);
        for (DWORD i = 0; i < system_handle_info->NumberOfHandles; i++)
        {
            CurHandle = &(system_handle_info->Handles[i]);
            if (CurHandle->GrantedAccess & PROCESS_VM_WRITE)
            {
                CLIENT_ID clientId;
                clientId.UniqueThread = NULL;

                hSource = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_DUP_HANDLE, FALSE, CurHandle->UniqueProcessId);
                if (!hSource) continue;

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

                if (ObjectType->TypeName.Buffer == NULL || 0 == wcsstr((wchar_t*)(ObjectType->TypeName.Buffer), L"Process"))
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

                if (CurHandle->UniqueProcessId == parent_processID ||
                    (uint32_t)basicInfo.UniqueProcessId != target_pid)
                {
                    continue;
                }

                clientId.UniqueProcess = (HANDLE)CurHandle->UniqueProcessId;

                PSYSTEM_PROCESS_INFORMATION processInfo = NULL;
                /*GetClientIdName(&clientId, &processInfo);*/

                std::wstring processName((PWCHAR)processInfo->ImageName.Buffer, processInfo->ImageName.Length / sizeof(WCHAR));

                if (processInfo && processName != L"csrss.exe" && processName != L"lsass.exe" && processName != L"svchost.exe" /*&& processName != L"sesvc.exe"*/)
                {
                    process_name = processName;
                    break;
                }
            }
        }
        delete[] system_handle_info;
    }
    return process_name;
}

// 检测进程隐藏的可写句柄
bool CWindows::detect_hide_process_handle()
{
    SYSTEM_HANDLE_TABLE_ENTRY_INFO* CurHandle;
    SYSTEM_HANDLE_INFORMATION* pInfo = get_system_handle_info();
    ProcessMap processes = enum_process();
    if (pInfo)
    {
        for (DWORD i = 0; i < pInfo->NumberOfHandles; i++)
        {
            CurHandle = &(pInfo->Handles[i]);
            if (processes.find(CurHandle->UniqueProcessId) == processes.end())
            {
                delete[] pInfo;
                return true;
            }
        }
    }
    return false;
}

CWindows::WindowsList CWindows::enum_windows_ex(bool exclude_system_process, bool exclude_signed)
{
    ProcessMap processes = enum_process(exclude_system_process, exclude_signed);
    WindowsList windows = enum_windows(exclude_system_process, exclude_signed);
    for (auto& window : windows)
    {
        if (processes.find(window.pid) != processes.end())
        {
            window.is_hide_process = false;
            window.process_name = processes[window.pid].name;
        }
        else
        {
            window.is_hide_process = true;
            window.process_name = L"not found";
        }
    }
    return windows;
}
void CWindows::exit_process()
{
    auto ExitProcess = IMPORT(L"kernel32.dll", ExitProcess);
    ExitProcess(0);
}

bool CWindows::get_process_main_hwnd(uint32_t pid, WindowInfo& window_out)
{
    ProcessInfo process;
    if (!get_process(pid, process))
    {
        return false;
    }
    if (process.threads.size() == 0)
    {
        return false;
    }
    uint32_t main_tid = 0;
    for (auto& itor : process.threads)
    {
        if (itor.second.is_main_thread)
        {
            main_tid = itor.second.tid;
            break;
        }
    }
    for (auto& window : enum_windows())
    {
        if (window.tid == main_tid)
        {
            window_out = window;
            return true;
        }
    }
    return false;
}


bool CWindows::get_process_main_thread_hwnd(uint32_t pid, std::vector<WindowInfo>& window_out)
{
    ProcessInfo process;
    if (!get_process(pid, process))
    {
        return false;
    }
    if (process.threads.size() == 0)
    {
        return false;
    }
    uint32_t main_tid = 0;
    for (auto& itor : process.threads)
    {
        if (itor.second.is_main_thread)
        {
            main_tid = itor.second.tid;
            break;
        }
    }
    for (auto& window : enum_windows())
    {
        if (window.tid == main_tid)
        {
            window_out.push_back(window);
        }
    }
    return window_out.size() > 0;
}



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

// 新增通用解析方法
std::string get_pdb_from_pe(uint8_t* image_base) {
    decltype(&IsBadReadPtr) IsBadReadPtr = IMPORT(L"kernel32.dll", IsBadReadPtr);
    std::string pdb_name = "";
    if (!IsBadReadPtr) {
        return pdb_name;
    }
    try
    {
        if (ApiResolver::get_image_dos_header(image_base)->e_magic != IMAGE_DOS_SIGNATURE)
        {
            return pdb_name;
        }
        PIMAGE_NT_HEADERS nt_header = ApiResolver::get_image_nt_header(image_base);
        if (!ApiResolver::get_data_directory(nt_header, IMAGE_DIRECTORY_ENTRY_DEBUG).VirtualAddress)
        {
            return pdb_name;
        }

        PIMAGE_DEBUG_DIRECTORY dbg_dir = (PIMAGE_DEBUG_DIRECTORY)ApiResolver::get_data_directory_va(image_base, IMAGE_DIRECTORY_ENTRY_DEBUG);
        if (!dbg_dir && IsBadReadPtr(dbg_dir, 1))
        {
            return pdb_name;
        }
        if (!dbg_dir->AddressOfRawData || dbg_dir->Type != IMAGE_DEBUG_TYPE_CODEVIEW)
            return pdb_name;
        CV_HEADER* cv_info = (CV_HEADER*)ApiResolver::rva2va(image_base, dbg_dir->AddressOfRawData);
        if (!ptr_is_region_valid((unsigned char*)image_base, nt_header->OptionalHeader.SizeOfImage, cv_info, sizeof(CV_HEADER)))
            return pdb_name;
        if (cv_info->Signature == NB10_SIG) //VC6.0 (GBK)
        {
            if (!ptr_is_region_valid((unsigned char*)image_base, nt_header->OptionalHeader.SizeOfImage, cv_info, sizeof(CV_INFO_PDB20) + MAX_PATH))
                return pdb_name;
            pdb_name = (char*)((CV_INFO_PDB20*)cv_info)->PdbFileName;
        }
        else if (cv_info->Signature == RSDS_SIG) //VS2003+ (UTF-8)
        {
            if (!ptr_is_region_valid((unsigned char*)image_base, nt_header->OptionalHeader.SizeOfImage, cv_info, sizeof(CV_INFO_PDB70) + MAX_PATH))
                return pdb_name;
            pdb_name = (char*)((CV_INFO_PDB70*)cv_info)->PdbFileName;
        }
        else
        {
            return pdb_name;
        }
        return pdb_name;
    }
    catch (...)
    {
        return pdb_name;
    }
}

// 新增驱动路径PDB获取方法
std::string CWindows::get_pdb_from_driver(const std::wstring& driver_path) {
    auto CreateFile = IMPORT(L"kernel32.dll", CreateFile);
    auto CreateFileMapping = IMPORT(L"kernel32.dll", CreateFileMapping);
    auto MapViewOfFile = IMPORT(L"kernel32.dll", MapViewOfFile);
    std::string pdb = "";
    if (!CreateFile || !CreateFileMapping || !MapViewOfFile) return pdb;

    HANDLE hFile = CreateFile(driver_path.c_str(), GENERIC_READ, FILE_SHARE_READ,
                              NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) return pdb;

    HANDLE hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!hMapping) {
        CloseHandle(hFile);
        return pdb;
    }

    uint8_t* pe_base = (uint8_t*)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    if (!pe_base) {
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return pdb;
    }

    pdb = get_pdb_from_pe(pe_base);

    UnmapViewOfFile(pe_base);
    CloseHandle(hMapping);
    CloseHandle(hFile);

    return pdb;
}

std::vector<std::string> CWindows::get_current_process_pdb_list()
{
    auto IsBadReadPtr = IMPORT(L"kernel32.dll", IsBadReadPtr);
    bool is_64bit = false;
    std::vector<std::string> result;
    auto modules = Utils::CWindows::instance().enum_modules(GetCurrentProcessId(), is_64bit);
    for (auto& module : modules)
    {
        auto pdb_name = get_pdb_from_pe((uint8_t*)module.base);
        if (!pdb_name.empty())
        {
            result.push_back(pdb_name);
        }
    }
    return result;
}

std::unordered_map<uint32_t, std::vector<std::string>>& CWindows::find_hidden_pid_from_csrss()
{
    static std::unordered_map<uint32_t, std::vector<std::string>> pid_directories;
    static ULONG64 lastProcessesTickCount = 0;
    ULONG64 tickCount = GetTickCount64();
    if (tickCount - lastProcessesTickCount >= 60 * 1000)
    {
        if (!pid_directories.empty())
        {
            pid_directories.clear();
        }
        lastProcessesTickCount = tickCount;
    }
    else
    {
        return pid_directories;
    }
    unsigned char handle_type_process;

    handle_type_process = 7;
    SystemVersion sys_version = get_system_version();
    if (sys_version == WINDOWS_ANCIENT || sys_version == WINDOWS_XP)
    {
        handle_type_process = 5;
    }
    else if (sys_version == WINDOWS_VISTA)
    {
        handle_type_process = 6;
    }
    else
    {
        handle_type_process = 7;
    }

    auto OpenProcess = IMPORT(L"kernel32.dll", OpenProcess);
    auto DuplicateHandle = IMPORT(L"kernel32.dll", DuplicateHandle);
    auto GetCurrentProcess = IMPORT(L"kernel32.dll", GetCurrentProcess);
    auto CloseHandle = IMPORT(L"kernel32.dll", CloseHandle);
    auto GetProcessId = IMPORT(L"kernel32.dll", GetProcessId);
    CWindows::ModuleList modules;

    if (!OpenProcess || !DuplicateHandle || !GetCurrentProcess || !CloseHandle || !GetProcessId)
        return pid_directories;
    SYSTEM_HANDLE_INFORMATION* pHandleInformation = NULL;
    std::set<int> csrss_pid_set;
    Utils::CWindows::ProcessMap processes = enum_process();
    pHandleInformation = get_system_handle_info();

    if (!pHandleInformation)
    {
        return pid_directories;
    }

    for (auto& p : processes)
    {
        if (p.second.name == L"csrss.exe")
        {
            csrss_pid_set.insert(p.first);
        }
    }

    if (csrss_pid_set.size() == 0)
    {
        delete[](BYTE*)pHandleInformation;
        return pid_directories;
    }

    int count = 1;
    for (ULONG i = 0; i < pHandleInformation->NumberOfHandles; i++)
    {
        DWORD current_handle_pid = pHandleInformation->Handles[i].UniqueProcessId;

        std::vector<std::string> directories;
        if (csrss_pid_set.find(current_handle_pid) == csrss_pid_set.end())
        {
            continue;
        }

        if (pHandleInformation->Handles[i].ObjectTypeIndex != handle_type_process)
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
                            get_process_query_access() | PROCESS_VM_READ, FALSE, 0) == NULL)
        {
            CloseHandle(process_handle);
            continue;
        }

        DWORD pid_from_duplicated_handle = GetProcessId(duplicated_handle);

        if (pid_from_duplicated_handle != (DWORD)-1
            && !is_64bits_process(duplicated_handle)
            && (processes.find(pid_from_duplicated_handle) == processes.end()))
        {
            ldr_walk_32(duplicated_handle, modules);
            if (!modules.empty())
            {
                std::error_code ec;
                auto walk_path = std::filesystem::path(modules.front().path).parent_path();
                auto itors = std::filesystem::directory_iterator(walk_path, ec);
                if (ec)
                    break;
                for (auto& file : itors)
                {
                    directories.emplace_back(file.path().filename().u8string());
                }
                pid_directories.emplace(std::make_pair(pid_from_duplicated_handle, directories));
            }
        }
        CloseHandle(duplicated_handle);
        CloseHandle(process_handle);
    }

    delete[](BYTE*)pHandleInformation;
    return pid_directories;
}

std::optional<CWindows::SignatureInfo> CWindows::verify_embedded_signature(const std::wstring& path) {
    auto CryptQueryObject = IMPORT(L"Crypt32.dll", CryptQueryObject);
    auto CryptMsgGetParam = IMPORT(L"Crypt32.dll", CryptMsgGetParam);
    auto CertFindCertificateInStore = IMPORT(L"Crypt32.dll", CertFindCertificateInStore);
    auto CertCloseStore = IMPORT(L"Crypt32.dll", CertCloseStore);
    auto CertGetCertificateChain = IMPORT(L"Crypt32.dll", CertGetCertificateChain);
    auto CertGetNameStringA = IMPORT(L"Crypt32.dll", CertGetNameStringA);
    auto CertFreeCertificateChain = IMPORT(L"Crypt32.dll", CertFreeCertificateChain);
    auto CertFreeCertificateContext = IMPORT(L"Crypt32.dll", CertFreeCertificateContext);
    auto Wow64EnableWow64FsRedirection = IMPORT(L"Kernel32.dll", Wow64EnableWow64FsRedirection);
    auto Wow64DisableWow64FsRedirection = IMPORT(L"Kernel32.dll", Wow64DisableWow64FsRedirection);

    if (!CryptQueryObject || !CryptMsgGetParam || !CertFindCertificateInStore || !CertCloseStore || !CertGetCertificateChain
        || !CertGetNameStringA || !CertFreeCertificateChain || !CertFreeCertificateContext || !Wow64EnableWow64FsRedirection
        || !Wow64DisableWow64FsRedirection) {
        return std::nullopt;
    }
    HCERTSTORE hStore = nullptr;
    HCRYPTMSG hMsg = nullptr;
    PCCERT_CONTEXT pCertContext = nullptr;
    CERT_INFO CertInfo{};
    DWORD dwEncoding, dwContentType, dwFormatType;
    PVOID oldValue = nullptr;
    Wow64DisableWow64FsRedirection(&oldValue);
    std::wstring real_path = path;
    auto pos = real_path.find(L"System32\\");
    if (pos != std::wstring::npos) {
        real_path.replace(pos, 8, L"Sysnative");
    }

    if (!CryptQueryObject(CERT_QUERY_OBJECT_FILE, real_path.c_str(),
                          CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED | CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED | CERT_QUERY_CONTENT_FLAG_PKCS7_UNSIGNED,
                          CERT_QUERY_FORMAT_FLAG_BINARY, 0, &dwEncoding,
                          &dwContentType, &dwFormatType, &hStore, &hMsg, nullptr)) {
        Wow64EnableWow64FsRedirection(true);
        return std::nullopt;
    }

    // 获取签名信息
    DWORD dwSignerInfoSize = 0;
    CryptMsgGetParam(hMsg, CMSG_SIGNER_INFO_PARAM, 0, nullptr, &dwSignerInfoSize);
    auto pSignerInfo = std::make_unique<BYTE[]>(dwSignerInfoSize);
    CryptMsgGetParam(hMsg, CMSG_SIGNER_INFO_PARAM, 0, pSignerInfo.get(), &dwSignerInfoSize);

    CertInfo.Issuer = reinterpret_cast<CMSG_SIGNER_INFO*>(pSignerInfo.get())->Issuer;
    CertInfo.SerialNumber = reinterpret_cast<CMSG_SIGNER_INFO*>(pSignerInfo.get())->SerialNumber;

    pCertContext = CertFindCertificateInStore(hStore, dwEncoding, 0, CERT_FIND_SUBJECT_CERT, &CertInfo, nullptr);

    if (!pCertContext) {
        CertCloseStore(hStore, 0);
        Wow64EnableWow64FsRedirection(true);
        return std::nullopt;
    }

    char buffer[256]{};
    CERT_CHAIN_PARA ChainPara = { sizeof(ChainPara) };
    PCCERT_CHAIN_CONTEXT pChainContext = nullptr;
    CertGetCertificateChain(nullptr, pCertContext, nullptr, hStore, &ChainPara,
                            CERT_CHAIN_REVOCATION_CHECK_CHAIN, nullptr, &pChainContext);

    SignatureInfo info{};
    if (pChainContext) {
        if (pChainContext->TrustStatus.dwErrorStatus == CERT_TRUST_NO_ERROR) {
            char buffer[256]{};
            CertGetNameStringA(pCertContext, CERT_NAME_SIMPLE_DISPLAY_TYPE, 0, nullptr, buffer, sizeof(buffer));
            info.issuer = buffer;
            CertGetNameStringA(pCertContext, CERT_NAME_SIMPLE_DISPLAY_TYPE, CERT_NAME_ISSUER_FLAG,
                               nullptr, buffer, sizeof(buffer));
            info.subject = buffer;
            info.timestamp = pCertContext->pCertInfo->NotBefore;  // 确保证书有效
        }
        CertFreeCertificateChain(pChainContext);  // 先释放链
    }

    if (pCertContext) CertFreeCertificateContext(pCertContext);  // 再释放证书
    if (hStore) CertCloseStore(hStore, 0);  // 最后关闭存储
    Wow64EnableWow64FsRedirection(true);
    return pChainContext ? std::optional(info) : std::nullopt;
}

std::optional<CWindows::SignatureInfo> CWindows::verify_catalog_signature(const std::wstring& path) {
    if (path.empty()) return std::nullopt;
    auto CreateFileW = IMPORT(L"Kernel32.dll", CreateFileW);
    auto CloseHandle = IMPORT(L"Kernel32.dll", CloseHandle);
    auto Wow64EnableWow64FsRedirection = IMPORT(L"Kernel32.dll", Wow64EnableWow64FsRedirection);
    auto Wow64DisableWow64FsRedirection = IMPORT(L"Kernel32.dll", Wow64DisableWow64FsRedirection);
    auto CryptCATAdminAcquireContext = IMPORT(L"Wintrust.dll", CryptCATAdminAcquireContext);
    auto CryptCATAdminCalcHashFromFileHandle = IMPORT(L"Wintrust.dll", CryptCATAdminCalcHashFromFileHandle);
    auto CryptCATAdminReleaseContext = IMPORT(L"Wintrust.dll", CryptCATAdminReleaseContext);
    auto CryptCATAdminEnumCatalogFromHash = IMPORT(L"Wintrust.dll", CryptCATAdminEnumCatalogFromHash);
    auto CryptCATCatalogInfoFromContext = IMPORT(L"Wintrust.dll", CryptCATCatalogInfoFromContext);
    auto CryptCATAdminReleaseCatalogContext = IMPORT(L"Wintrust.dll", CryptCATAdminReleaseCatalogContext);

    if (!CreateFileW || !CloseHandle || !Wow64EnableWow64FsRedirection || !Wow64DisableWow64FsRedirection ||
        !CryptCATAdminAcquireContext || !CryptCATAdminCalcHashFromFileHandle || !CryptCATAdminReleaseContext ||
        !CryptCATAdminEnumCatalogFromHash || !CryptCATCatalogInfoFromContext || !CryptCATAdminReleaseCatalogContext) {
        return std::nullopt;
    }

    PVOID oldValue = nullptr;
    Wow64DisableWow64FsRedirection(&oldValue);
    std::wstring real_path = path;
    auto pos = real_path.find(L"System32\\");
    if (pos != std::wstring::npos) {
        real_path.replace(pos, 8, L"Sysnative");
    }
    HANDLE hFile = CreateFileW(real_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return std::nullopt;

    HCATADMIN hCatAdmin = nullptr;
    if (!CryptCATAdminAcquireContext(&hCatAdmin, nullptr, 0)) {
        CloseHandle(hFile);
        return std::nullopt;
    }

    BYTE hash[100];
    DWORD hashSize = sizeof(hash);
    if (!CryptCATAdminCalcHashFromFileHandle(hFile, &hashSize, hash, 0)) {
        CryptCATAdminReleaseContext(hCatAdmin, 0);
        CloseHandle(hFile);
        return std::nullopt;
    }

    HCATINFO hCatInfo = CryptCATAdminEnumCatalogFromHash(hCatAdmin, hash, hashSize, 0, nullptr);
    if (!hCatInfo) {
        CryptCATAdminReleaseContext(hCatAdmin, 0);
        CloseHandle(hFile);
        return std::nullopt;
    }

    CATALOG_INFO catalogInfo = { sizeof(catalogInfo) };
    if (!CryptCATCatalogInfoFromContext(hCatInfo, &catalogInfo, 0)) {
        CryptCATAdminReleaseCatalogContext(hCatAdmin, hCatInfo, 0);
        CryptCATAdminReleaseContext(hCatAdmin, 0);
        CloseHandle(hFile);
        return std::nullopt;
    }

    // 验证目录文件的嵌入式签名
    auto catalogSignature = verify_signature(catalogInfo.wszCatalogFile);
    if (!catalogSignature) {
        CryptCATAdminReleaseCatalogContext(hCatAdmin, hCatInfo, 0);
        CryptCATAdminReleaseContext(hCatAdmin, 0);
        CloseHandle(hFile);
        return std::nullopt;
    }

    // 使用目录文件的签名信息
    SignatureInfo info;
    info.issuer = catalogSignature->issuer;
    info.subject = catalogSignature->subject;
    info.timestamp = catalogSignature->timestamp;

    CryptCATAdminReleaseCatalogContext(hCatAdmin, hCatInfo, 0);
    CryptCATAdminReleaseContext(hCatAdmin, 0);
    CloseHandle(hFile);
    return info;
}

bool CWindows::verify_signature(uint32_t pid) {
    if (auto it = process_cache.find(pid); it != process_cache.end()) {
        if (it->second.second) return true;
    }
    else {
        auto path = get_process_path_(pid);
        bool is_signed = !path.empty() && verify_signature(path);
        process_cache[pid] = { path, is_signed };
        return is_signed;
    }
    return false;
}

std::optional<CWindows::SignatureInfo> CWindows::verify_signature(const std::wstring& path) {
    if (path.empty()) return std::nullopt;
    auto Wow64EnableWow64FsRedirection = IMPORT(L"Kernel32.dll", Wow64EnableWow64FsRedirection);
    power();
    std::optional<CWindows::SignatureInfo> embeddedInfo;
    try {
        embeddedInfo = verify_embedded_signature(path);
        if (embeddedInfo) {
            Wow64EnableWow64FsRedirection(true);
            return embeddedInfo;
        }

        embeddedInfo = verify_catalog_signature(path);
        Wow64EnableWow64FsRedirection(true);
        return embeddedInfo ? embeddedInfo : std::nullopt;
    }
    catch (const std::exception& e)
    {
        return std::nullopt;
    }
}

std::wstring CWindows::get_process_path_(DWORD pid) {
    auto OpenProcess = IMPORT(L"kernel32.dll", OpenProcess);
    auto GetModuleFileNameExW = IMPORT(L"psapi.dll", GetModuleFileNameExW); // XP 兼容
    auto CloseHandle = IMPORT(L"kernel32.dll", CloseHandle);

    if (!OpenProcess || !GetModuleFileNameExW || !CloseHandle) return L"";
    HANDLE hProcess = OpenProcess(ProcessQueryAccess, FALSE, pid);
    if (!hProcess) return L"";

    wchar_t path[MAX_PATH]{};
    if (GetModuleFileNameExW(hProcess, NULL, path, MAX_PATH) == 0) {
        CloseHandle(hProcess);
        return L"";
    }
    CloseHandle(hProcess);
    return path;
}