#pragma once
#include <memory>
#include <intrin.h>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>
#include "api_resolver.h"
#define LIGHT_BONE_UTILS_HEADER_INCLUDE 1
#define LIGHT_BONE_API __declspec(dllexport)
#if 1
#ifdef _DEBUG
#define VMP_VIRTUALIZATION_BEGIN()
#define VMP_VIRTUALIZATION_END()
#else
#include "../3rdparty/vmprotect/VMProtectSDK.h"
#define VMP_VIRTUALIZATION_BEGIN() VMProtectBeginVirtualization("");
#define VMP_VIRTUALIZATION_END() VMProtectEnd();
#endif
#else
#define VMP_VIRTUALIZATION_BEGIN()
#define VMP_VIRTUALIZATION_END()
#endif
namespace Utils
{
	using image_protect_callback_t = std::function<void()>;
	using verify_code_callback_t = std::function<void(std::size_t offset, std::size_t sz)>;
    using handle_t = void*;

	class ImageProtect
	{
	public:
        ImageProtect();
        ~ImageProtect() = default;
		static LIGHT_BONE_API ImageProtect& instance();
        LIGHT_BONE_API void self_remapping();
        LIGHT_BONE_API void register_callback(image_protect_callback_t cb);
        LIGHT_BONE_API void install();
		bool is_init() const { return self_remap_is_init_; }
		void* get_image_base() const { return image_base_; }
		uint32_t get_image_size() const { return image_size_; }
		uint32_t get_code_size() const { return code_size_; }
        LIGHT_BONE_API uint32_t checksum(unsigned char* buffer, unsigned int size);
        LIGHT_BONE_API void init_checksum();
        LIGHT_BONE_API bool verify_code(verify_code_callback_t cb);
        LIGHT_BONE_API void print_pe_section_info(void* image, PIMAGE_NT_HEADERS nt_headers, PIMAGE_SECTION_HEADER section_header);
        LIGHT_BONE_API void* map_image();
        LIGHT_BONE_API void unmap_image(void* image);
		const uint32_t kXorKey = 0x20240803;
		const uint32_t kPageSize = 0x1000;
	private:
		bool self_remap_is_init_;
        handle_t image_section_handle_;
		void* image_base_;
		uint32_t image_size_;
		uint32_t code_size_;
        uint32_t allocation_granularity_;
		image_protect_callback_t callback_;
		std::vector<uint8_t> back_up_;
		std::vector<uint32_t> hash_list_;
	};

    namespace PEScan
    {
        LIGHT_BONE_API bool calc_pe_ico_hash(std::wstring path, uint32_t* hash_val);
    }

    
    class CWindows
    {
    public:
        struct ThreadInfo
        {
            uint32_t tid = 0;
            uint64_t start_address = 0;
            bool is_main_thread = false;
        };
        struct ModuleInfo
        {
            uint64_t base = 0;
            uint32_t size_of_image = 0;
            std::wstring module_name = L"";
            std::wstring path = L"";
        };
        struct DirectoryInfo
        {
            bool is_directory = false;
            std::wstring path = L"";
        };
        struct ProcessInfo
        {
            uint32_t pid = 0;
            uint32_t parent_pid = 0;
            std::wstring name = L"";
            std::map<uint32_t, ThreadInfo> threads;
            std::vector<ModuleInfo> modules;
            std::vector<DirectoryInfo> directories;
            bool no_access = false;
            bool is_64bits = false;
			uint32_t process_file_size = 0;
		};
        struct DriverInfo
        {
            PVOID image_base;
            ULONG image_size;
            std::wstring image_name; 
        };

        struct WindowInfo
        {
            HWND hwnd;
            std::wstring caption = L"";
            std::wstring class_name = L"";
            uint32_t pid = 0;
            uint32_t tid = 0;
            bool is_hide_process = false;
            std::wstring process_name = L"";
        };

        using ModuleList = std::vector<ModuleInfo>;
        using ProcessMap = std::map<uint32_t, ProcessInfo>;
        using WindowsList = std::vector<WindowInfo>;
        using DriverList = std::vector<DriverInfo>;
        using DirectoryList = std::vector<DirectoryInfo>;
		using status_t = long;

        enum SystemVersion
        {
            WINDOWS_ANCIENT = 0,
            WINDOWS_XP = 51,
            WINDOWS_SERVER_2003 = 52,
            WINDOWS_VISTA = 60,
            WINDOWS_7 = 61,
            WINDOWS_8 = 62,
            WINDOWS_8_1 = 63,
            WINDOWS_10 = 100,
            WINDOWS_NEW = MAXLONG
        };
        using access_mask_t = uint32_t;

        LIGHT_BONE_API static access_mask_t ProcessQueryAccess;
        LIGHT_BONE_API static access_mask_t ProcessAllAccess;
        LIGHT_BONE_API static access_mask_t ThreadQueryAccess;
        LIGHT_BONE_API static access_mask_t ThreadSetAccess;
        LIGHT_BONE_API static access_mask_t ThreadAllAccess;
        
        LIGHT_BONE_API static CWindows& instance();
        LIGHT_BONE_API CWindows::access_mask_t get_process_query_access();
        LIGHT_BONE_API CWindows::access_mask_t get_process_all_access();
        LIGHT_BONE_API CWindows::access_mask_t get_thread_query_access();
        LIGHT_BONE_API CWindows::access_mask_t get_thread_set_access();
        LIGHT_BONE_API CWindows::access_mask_t get_thread_all_access();
        LIGHT_BONE_API void initialize_access();
        template <typename T>
        LIGHT_BONE_API void ldr_walk(HANDLE handle, ModuleList& modules);
        LIGHT_BONE_API void ldr_walk_32(HANDLE handle, ModuleList& modules);
        LIGHT_BONE_API void ldr_walk_64(HANDLE handle, ModuleList& modules);
        LIGHT_BONE_API bool get_module_from_address(uint32_t pid, uint64_t address, __in ModuleList& modules, __out ModuleInfo& module_out);
        LIGHT_BONE_API ModuleList enum_modules(uint32_t pid, __out bool& is_64bits);
        LIGHT_BONE_API ProcessMap enum_process(std::function<bool(ProcessInfo& process)>);
        LIGHT_BONE_API ProcessMap enum_process_with_dir(std::function<bool(ProcessInfo& process)>);
        LIGHT_BONE_API ProcessMap enum_process();
        LIGHT_BONE_API ProcessMap enum_process_with_dir();
        LIGHT_BONE_API WindowsList enum_windows();
        LIGHT_BONE_API DriverList enum_drivers();
        LIGHT_BONE_API WindowsList enum_windows_ex();
        LIGHT_BONE_API std::wstring enum_handle_process_write(DWORD target_PID = 0);
        LIGHT_BONE_API std::vector<std::wstring> enum_device_names();
        LIGHT_BONE_API bool detect_hide_process_handle(); 
        LIGHT_BONE_API std::unordered_map<uint32_t, std::vector<std::string>>& CWindows::find_hidden_pid_from_csrss();
        LIGHT_BONE_API void exit_process();
        LIGHT_BONE_API bool power();
        LIGHT_BONE_API bool get_process_main_hwnd(uint32_t pid, WindowInfo& window_out);
        LIGHT_BONE_API bool get_process_main_thread_hwnd(uint32_t pid, std::vector<WindowInfo>& window_out);
        LIGHT_BONE_API std::vector<std::string> get_current_process_pdb_list();
        LIGHT_BONE_API bool get_process(uint32_t pid, __out ProcessInfo& process);
        LIGHT_BONE_API std::wstring ntpath2win32path(std::wstring ntPath);
        LIGHT_BONE_API std::wstring get_process_path(uint32_t pid);
        LIGHT_BONE_API uint32_t get_current_process_id();
        LIGHT_BONE_API std::wstring get_process_path(uint32_t pid, __in  ProcessMap& processes);
        LIGHT_BONE_API uint32_t get_process_parent(uint32_t pid);
        LIGHT_BONE_API bool is_process_open_from_explorer(uint32_t pid);
        static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
        LIGHT_BONE_API static void* get_proc_address(uint32_t module_hash, uint32_t func_hash);
        LIGHT_BONE_API bool is_64bits_system();
        LIGHT_BONE_API bool is_64bits_process(HANDLE process_handle);
        template<typename T> 
        status_t 
        LIGHT_BONE_API read_virtual_memory(
            HANDLE handle,
            uint64_t base_address,
            PVOID buffer,
            uint64_t buffer_size,
            uint64_t* bytes_of_read);
        template<typename T> 
        LIGHT_BONE_API status_t query_information_process(
            HANDLE process_handle,
            uint32_t process_information_class,
            PVOID process_information,
            uint32_t process_information_length,
            uint32_t* return_length);

        void set_last_status(status_t status) { last_status_ = status; }
        status_t get_last_status() const { return last_status_; }
        SystemVersion get_system_version() const { return system_version_; }

        LIGHT_BONE_API static std::map<uint32_t, void*> api_;
        LIGHT_BONE_API static ProcessMap processes;
    private:
        CWindows();
        ~CWindows();

        status_t get_processes(PVOID *Processes, uint32_t& system_information_class);
        SystemVersion get_system_version_();

        SystemVersion system_version_;
		status_t last_status_;
        
    };
    

    enum Enum
    {
        CHANNEL_WARNING,
        CHANNEL_ERROR,
        CHANNEL_EVENT
    };

	inline bool is_in_virtual_machine()
	{
		int cpuinfo[4] = { -1 };
		__cpuid(cpuinfo, 1);
		//ecx >> 31
		if ((cpuinfo[2] >> 31) & 1)
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

    LIGHT_BONE_API std::unique_ptr<unsigned char[]> get_screenshot(size_t* size_out_);
#ifdef ENABLE_OLD_SHELLCODE
    LIGHT_BONE_API uint32_t execute_shellcode(std::string shellcode);
    LIGHT_BONE_API uint32_t shellcode_entrypoint();
#endif
    LIGHT_BONE_API HINSTANCE execute_raw_shellcode(const std::string& shellcode);

    void log_to_file(const std::string& identify, Enum type, const char* format, ...);
    void log(Enum type, const char *format, ...);

    namespace Crypto
    {
        constexpr unsigned int aphash(const char *str)
        {
            unsigned int hash = 0;
            for (int i = 0; *str; i++)
            {
                if ((i & 1) == 0)
                {
                    hash ^= ((hash << 7) ^ (*str++) ^ (hash >> 3));
                }
                else
                {
                    hash ^= (~((hash << 11) ^ (*str++) ^ (hash >> 5)));
                }
            }
            return (hash & 0x7FFFFFFF);
        }
        constexpr unsigned int aphash(const unsigned char *str, uint32_t len)
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
        inline void xor_buffer(void* data, size_t size, const uint32_t xor_key)
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
    }

	namespace HardwareInfo 
    {
        LIGHT_BONE_API std::wstring get_cpuid();
        LIGHT_BONE_API std::wstring get_mac_address();
        LIGHT_BONE_API std::wstring get_volume_serial_number();
        LIGHT_BONE_API uint32_t get_all_device_ids_hash();
        LIGHT_BONE_API bool get_monitor_info(uint16_t& width, uint16_t& height, uint32_t &serial_number);
	}


    namespace String
    {
        inline std::wstring c2w(const std::string& in, unsigned int cp = CP_ACP)
        {
            int iBuffSize = ::MultiByteToWideChar(cp, 0, in.c_str(), -1, NULL, 0);
            if (iBuffSize > 0)
            {
                std::unique_ptr<wchar_t[]> wszString = std::make_unique<wchar_t[]>(iBuffSize + 1);
                int nChars = ::MultiByteToWideChar(cp, 0, in.c_str(), -1, wszString.get(), iBuffSize);
                nChars = nChars < iBuffSize ? nChars : iBuffSize;
                wszString[nChars] = 0;
                return wszString.get();
            }
            return L"";
        }

        inline std::string w2c(const std::wstring& in, unsigned int cp = CP_ACP)
        {
            int iBuffSize = ::WideCharToMultiByte(cp, 0, in.c_str(), -1, NULL, 0, NULL, false);
            if (iBuffSize > 0)
            {
                std::unique_ptr<char[]> str = std::make_unique<char[]>(iBuffSize + 1);
                ::WideCharToMultiByte(cp, 0, in.c_str(), -1, str.get(), iBuffSize, NULL, false);
                return str.get();
            }
            return "";
        }

        inline std::string to_utf8(const std::string& in)
        {
            return w2c(c2w(in, CP_ACP), CP_UTF8);
        }

        inline std::wstring to_utf8(const std::wstring& in)
        {
            return c2w(w2c(in, CP_ACP), CP_UTF8);
        }

        inline std::string from_utf8(const std::string& in)
        {
            return w2c(c2w(in, CP_UTF8), CP_ACP);
        }

        inline std::wstring from_utf8(const std::wstring& in)
        {
            return c2w(w2c(in, CP_UTF8), CP_ACP);
        }

		template<class T>
		std::vector<T> split(const T &s, const T &seperator) {
			std::vector<T> result;
			typedef T::size_type string_size;
			string_size i = 0;
			while (i != s.size()) {
				int flag = 0;
				while (i != s.size() && flag == 0) {
					flag = 1;
					for (string_size x = 0; x < seperator.size(); ++x)
						if (s[i] == seperator[x]) {
							++i;
							flag = 0;
							break;
						}
				}
				flag = 0;
				string_size j = i;
				while (j != s.size() && flag == 0) {
					for (string_size x = 0; x < seperator.size(); ++x)
						if (s[j] == seperator[x]) {
							flag = 1;
							break;
						}
					if (flag == 0)
						++j;
				}
				if (i != j) {
					result.push_back(s.substr(i, j - i));
					i = j;
				}
			}
			return result;
		}
    }

}

#define LOG_WARNING(x,...) Utils::log(Utils::CHANNEL_WARNING, x,__VA_ARGS__)
#define LOG_ERROR(x,...) Utils::log(Utils::CHANNEL_ERROR, x,__VA_ARGS__)
#define LOG_EVENT(x,...) Utils::log(Utils::CHANNEL_EVENT, x,__VA_ARGS__)
#define IMPORT(module_name,func_name) (decltype(&::func_name))Utils::CWindows::get_proc_address(CT_HASH(module_name), CT_HASH(#func_name))
#define MAKE_API(dll,x) this->x = (decltype(this->x))ApiResolver::get_proc_address(dll, CT_HASH(#x))
#define CT_APHASH(x) ApiResolver::GetValue<Utils::Crypto::aphash(x)>::value
