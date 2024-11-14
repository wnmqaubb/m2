#pragma once
#include "Protocol.h"

enum ServicePackageId
{
    SPKG_ID_START = 681000,
    SPKG_SERIALIZE_TYPE_PROCESS_INFO,
    SPKG_SERIALIZE_TYPE_THREAD_INFO,
    SPKG_SERIALIZE_TYPE_MODULE_INFO,
    SPKG_ID_S2C_QUERY_PROCESS,
    SPKG_ID_C2S_QUERY_PROCESS,
    SPKG_SERIALIZE_TYPE_DRIVERINFO,
    SPKG_SERIALIZE_TYPE_WINDOWINFO,
    SPKG_ID_S2C_QUERY_DRIVERINFO,
    SPKG_ID_C2S_QUERY_DRIVERINFO,
    SPKG_ID_S2C_QUERY_WINDOWSINFO,
    SPKG_ID_C2S_QUERY_WINDOWSINFO,
    SPKG_ID_S2C_QUERY_SCREENSHOT,
    SPKG_ID_C2S_QUERY_SCREENSHOT,
    SPKG_SERIALIZE_TYPE_DIRECTORY_INFO,
    SPKG_ID_CUR_PACKAGE_ID_SIZE,
    SPKG_ID_MAX_PACKAGE_ID_SIZE = 681999
};

//static_assert(SPKG_ID_C2S_QUERY_WINDOWSINFO == 15, "ServicePackageId只允许在后面加入新ID");

struct ProtocolThreadInfo : ProtocolBase<SPKG_SERIALIZE_TYPE_THREAD_INFO>
{
    uint32_t tid = 0;
    uint64_t start_address = 0;
    bool is_main_thread = false;
    std::wstring owner_module = L"";
    MSGPACK_DEFINE(package_id, tid, start_address, is_main_thread, owner_module);
};
struct ProtocolModuleInfo : ProtocolBase<SPKG_SERIALIZE_TYPE_MODULE_INFO>
{
    uint64_t base = 0;
    uint32_t size_of_image = 0;
    std::wstring module_name = L"";
    std::wstring path = L"";
    MSGPACK_DEFINE(package_id, base, size_of_image, module_name, path);
};
struct ProtocolDirectoryInfo : ProtocolBase<SPKG_SERIALIZE_TYPE_DIRECTORY_INFO>
{
    bool is_directory = false;
    std::wstring path = L"";
    MSGPACK_DEFINE(package_id, is_directory, path);
};

struct ProtocolProcessInfo : ProtocolBase<SPKG_SERIALIZE_TYPE_PROCESS_INFO>
{
    uint32_t pid = 0;
    uint32_t parent_pid = 0;
    std::wstring name = L"";
    std::map<uint32_t, ProtocolThreadInfo> threads;
    std::vector<ProtocolModuleInfo> modules;
    bool no_access = false;
	bool is_64bits = false;
    std::vector<ProtocolDirectoryInfo> directories;
	uint32_t process_file_size = 0;
    MSGPACK_DEFINE(package_id, pid, parent_pid, name, threads, modules, no_access, is_64bits, directories, process_file_size);
};

struct ProtocolDriverInfo : ProtocolBase<SPKG_SERIALIZE_TYPE_DRIVERINFO>
{
    uint64_t image_base;
    uint32_t image_size;
    std::wstring image_name;
    MSGPACK_DEFINE(package_id, image_base, image_size, image_name);
};

struct ProtocolWindowInfo : ProtocolBase<SPKG_SERIALIZE_TYPE_WINDOWINFO>
{
    uint32_t hwnd;
    std::wstring caption = L"";
    std::wstring class_name = L"";
    uint32_t pid = 0;
    uint32_t tid = 0;
    bool is_hide_process = false;
    std::wstring process_name = L"";
    MSGPACK_DEFINE(package_id, hwnd, caption, class_name, pid, tid, is_hide_process, process_name);
};

struct ProtocolS2CQueryProcess : ProtocolBase<SPKG_ID_S2C_QUERY_PROCESS>
{
    MSGPACK_DEFINE(package_id);
};

struct ProtocolC2SQueryProcess : ProtocolBase<SPKG_ID_C2S_QUERY_PROCESS>
{
    std::map<uint32_t, ProtocolProcessInfo> data;
    MSGPACK_DEFINE(package_id, data);
};

struct ProtocolS2CQueryDriverInfo : ProtocolBase<SPKG_ID_S2C_QUERY_DRIVERINFO>
{
    MSGPACK_DEFINE(package_id);
};

struct ProtocolC2SQueryDriverInfo : ProtocolBase<SPKG_ID_C2S_QUERY_DRIVERINFO>
{
    std::vector<ProtocolDriverInfo> data;
    MSGPACK_DEFINE(package_id, data);
};

struct ProtocolS2CQueryWindows : ProtocolBase<SPKG_ID_S2C_QUERY_WINDOWSINFO>
{
    MSGPACK_DEFINE(package_id);
};

struct ProtocolC2SQueryWindowsInfo : ProtocolBase<SPKG_ID_C2S_QUERY_WINDOWSINFO>
{
    std::vector<ProtocolWindowInfo> data;
    MSGPACK_DEFINE(package_id, data);
};

struct ProtocolS2CQueryScreenShot : ProtocolBase<SPKG_ID_S2C_QUERY_SCREENSHOT>
{
    MSGPACK_DEFINE(package_id);
};

struct ProtocolC2SQueryScreenShot : ProtocolBase<SPKG_ID_C2S_QUERY_SCREENSHOT>
{
    std::vector<unsigned char> data;
    MSGPACK_DEFINE(package_id, data);
};

#if (LIGHT_BONE_UTILS_HEADER_INCLUDE)
namespace Utils {
inline std::vector<ProtocolModuleInfo> cast(const std::vector<CWindows::ModuleInfo>& src)
{
    std::vector<ProtocolModuleInfo> result;
    std::transform(src.begin(),
        src.end(),
        std::insert_iterator<std::vector<ProtocolModuleInfo>>(result, result.begin()),
        [](CWindows::ModuleInfo child)->ProtocolModuleInfo {
        ProtocolModuleInfo new_child;
        new_child.base = child.base;
        new_child.module_name = child.module_name;
        new_child.path = child.path;
        new_child.size_of_image = child.size_of_image;
        return new_child;
    });
    return std::move(result);
}

inline std::vector<ProtocolWindowInfo> cast(const std::vector<CWindows::WindowInfo>& src)
{
    std::vector<ProtocolWindowInfo> result;
    std::transform(src.begin(),
        src.end(),
        std::insert_iterator<std::vector<ProtocolWindowInfo>>(result, result.begin()),
        [](CWindows::WindowInfo child)->ProtocolWindowInfo {
        ProtocolWindowInfo new_child;
        new_child.hwnd = (uint32_t)child.hwnd;
        new_child.caption = child.caption;
        new_child.class_name = child.class_name;
        new_child.pid = child.pid;
        new_child.tid = child.tid;
        new_child.is_hide_process = child.is_hide_process;
        new_child.process_name = child.process_name;
        return new_child;
    });
    return std::move(result);
}

inline std::vector<ProtocolDriverInfo> cast(const std::vector<CWindows::DriverInfo>& src)
{
    std::vector<ProtocolDriverInfo> result;
    std::transform(src.begin(),
        src.end(),
        std::insert_iterator<std::vector<ProtocolDriverInfo>>(result, result.begin()),
        [](CWindows::DriverInfo child)->ProtocolDriverInfo {
        ProtocolDriverInfo new_child;
        new_child.image_base = (uint64_t)child.image_base;
        new_child.image_size = (uint32_t)child.image_size;
        new_child.image_name = child.image_name;
        return new_child;
    });
    return std::move(result);
}

inline std::map<uint32_t, ProtocolThreadInfo> cast(const std::map<uint32_t, CWindows::ThreadInfo>& src, std::vector<CWindows::ModuleInfo>& moduleinfos)
{
    std::map<uint32_t, ProtocolThreadInfo> result;
    std::transform(src.begin(),
        src.end(),
        std::insert_iterator<std::map<uint32_t, ProtocolThreadInfo>>(result, result.begin()),
        [&moduleinfos](std::pair<uint32_t, CWindows::ThreadInfo> child)->std::pair<uint32_t, ProtocolThreadInfo> {
        ProtocolThreadInfo new_child;
        new_child.is_main_thread = child.second.is_main_thread;
        new_child.start_address = child.second.start_address;
        new_child.tid = child.second.tid;
        Utils::CWindows::ModuleInfo module_info;
        Utils::CWindows::instance().get_module_from_address(child.first, child.second.start_address, moduleinfos, module_info);
        new_child.owner_module = module_info.path;
        return std::pair<uint32_t, ProtocolThreadInfo>(child.first, new_child);
    });
    return std::move(result);
}
inline std::vector<ProtocolDirectoryInfo> cast(const std::vector<CWindows::DirectoryInfo>& src)
{
    std::vector<ProtocolDirectoryInfo> result;
    std::transform(src.begin(),
        src.end(),
        std::insert_iterator<std::vector<ProtocolDirectoryInfo>>(result, result.begin()),
        [](CWindows::DirectoryInfo child)->ProtocolDirectoryInfo {
        ProtocolDirectoryInfo new_child;
        new_child.is_directory = child.is_directory;
        new_child.path = child.path;
        return new_child;
    });
    return std::move(result);
}

inline std::map<uint32_t, ProtocolProcessInfo> cast(const std::map<uint32_t, CWindows::ProcessInfo>& src)
{
    std::map<uint32_t, ProtocolProcessInfo> result;
    std::transform(src.begin(),
        src.end(),
        std::insert_iterator<std::map<uint32_t, ProtocolProcessInfo>>(result, result.begin()),
        [](std::pair<uint32_t, CWindows::ProcessInfo> child)->std::pair<uint32_t, ProtocolProcessInfo> {
        ProtocolProcessInfo new_child;
        new_child.pid = child.second.pid;
        new_child.parent_pid = child.second.parent_pid;
        new_child.name = child.second.name;
        new_child.threads = cast(child.second.threads, child.second.modules);
        new_child.modules = cast(child.second.modules);
        new_child.directories = cast(child.second.directories);
        new_child.no_access = child.second.no_access;
        new_child.is_64bits = child.second.is_64bits;
        return std::pair<uint32_t, ProtocolProcessInfo>(child.first, new_child);
    });
    return std::move(result);
}
};
#endif