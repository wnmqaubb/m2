#include "NewClient/pch.h"
#include <Lightbone/utils.h>
#include <Lightbone/pattern.hpp>
#include "Service/AntiCheatClient.h"
#include "BasicUtils.h"
#include <iostream>
#include "3rdparty/wmic/wmic.h"
#include "3rdparty/wmic/dnscache.h"
#include <asio2/util/base64.hpp>
#include "ModuleCheckSum.h"


__declspec(dllimport) asio::detail::thread_group g_thread_group;
asio::io_service g_js_io;
qjs::Runtime g_runtime;
qjs::Context g_context(g_runtime);
NetUtils::CTimerMgr g_js_timer(g_js_io);
static CAntiCheatClient* g_client = nullptr;
static uint8_t ANONYMOUS_COUNT = 0;

class CSehException
{
public:
    CSehException(UINT code, PEXCEPTION_POINTERS pep)
    {
        m_exception_code = code;
        m_exception_record = *pep->ExceptionRecord;
        m_context = *pep->ContextRecord;
    }
    operator unsigned int() { return m_exception_code; }
    UINT m_exception_code;
    EXCEPTION_RECORD   m_exception_record;
    CONTEXT   m_context;
};

void translate_seh_to_ce(UINT code, PEXCEPTION_POINTERS pep)
{
    throw CSehException(code, pep);
}

static JSValue enum_device()
{
    auto device_names = Utils::CWindows::instance().enum_device_names();
    std::vector<std::string> result;
    for (auto& device_name : device_names)
    {
        result.push_back(Utils::String::w2c(device_name, CP_UTF8));
    }

	auto drivers = Utils::CWindows::instance().enum_drivers();
	for (auto& device : drivers)
	{
		result.push_back(Utils::String::w2c(device.image_name, CP_UTF8));
	}

    json x = result;
    auto str = x.dump();
    return g_context.fromJSON(str);
}
static uint32_t get_display_device_sig()
{
    static uint32_t device_sig_cache = 0;
    if (device_sig_cache != 0)
        return device_sig_cache;
    if (Utils::CWindows::instance().get_system_version() > WINDOWS_VISTA)
    {
        bool is_64bits = false;
        auto modules = Utils::CWindows::instance().enum_modules(GetCurrentProcessId(), is_64bits);
        for (auto& m : modules)
        {
            char wddm_api_name[] = { 'O', 'p', 'e', 'n', 'A', 'd', 'a', 'p', 't', 'e', 'r', 0 };
            void* openadapter_ptr = GetProcAddress((HMODULE)m.base, wddm_api_name);
            if (openadapter_ptr)
            {
                device_sig_cache = (uint32_t)openadapter_ptr - (uint32_t)m.base;
                return device_sig_cache;
            }
        }
    }
    device_sig_cache = -1;
    return device_sig_cache;
}
static JSValue enum_pdb()
{
    json x = Utils::CWindows::instance().get_current_process_pdb_list();
    auto str = x.dump();
    return g_context.fromJSON(str);
}

static void terminate_process(unsigned int pid)
{
    HANDLE handle = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    TerminateProcess(handle, 0);
    CloseHandle(handle);
}

static bool has_access(unsigned int pid)
{
    Utils::CWindows::ProcessInfo process;
    if (Utils::CWindows::instance().get_process(pid, process))
    {
        return !process.no_access;
    }
    return false;
}

static std::string get_process_name(unsigned int pid)
{
    Utils::CWindows::ProcessInfo process;
    if (Utils::CWindows::instance().get_process(pid, process))
    {
        return Utils::String::w2c(process.name, CP_UTF8);
    }
    return "";
}

static JSValue get_process_names()
{
    std::map<uint32_t, std::string> result;
    auto processes = Utils::CWindows::instance().enum_process();
    for (auto& process : processes)
    {
        result.emplace(std::make_pair(process.first, Utils::String::w2c(process.second.name, CP_UTF8)));
    }
    json x = result;
    auto str = x.dump();
    return g_context.fromJSON(str);
}

static JSValue enum_threads()
{
    auto processes = Utils::CWindows::instance().enum_process();
    std::vector<std::tuple<uint32_t, std::string, uint64_t>> result;
    for (auto& process : processes)
    {
        for (auto& thread : process.second.threads)
        {
            result.push_back(std::make_tuple(process.first, Utils::String::w2c(process.second.name, CP_UTF8), thread.second.start_address));
        }
    }
    json x = result;
    auto str = x.dump();
    return g_context.fromJSON(str);
}

static JSValue enum_windows()
{
    auto windows = Utils::CWindows::instance().enum_windows_ex();
    std::vector<std::tuple<unsigned int, std::string, std::string, bool, uint32_t>> result;
    for(auto& window : windows)
    {
        result.push_back(std::make_tuple(window.pid, Utils::String::w2c(window.caption, CP_UTF8), Utils::String::w2c(window.class_name, CP_UTF8), window.is_hide_process, (uint32_t)window.hwnd));
    }
    json x = result;
    auto str = x.dump();
    return g_context.fromJSON(str);
}

static JSValue enum_process_hash()
{
    std::vector<std::pair<std::string, uint32_t>> result;
    auto processes = Utils::CWindows::instance().enum_process();
    auto cur_pid = Utils::CWindows::instance().get_current_process_id();
    auto ppid = Utils::CWindows::instance().get_process_parent(cur_pid);
    for (auto& process : processes)
    {
        if (process.second.modules.size() == 0)
        {
            continue;
        }
        if (process.second.pid == cur_pid || process.second.pid == ppid)
        {
            continue;
        }
        if (process.second.is_64bits)
        {
            continue;
        }
        uint32_t hash_val = 0;
        if (!BasicUtils::calc_pe_ico_hash(process.second.modules.front().path, &hash_val))
        {
            continue;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        result.push_back(std::make_pair(Utils::String::w2c(process.second.modules.front().path, CP_UTF8), hash_val));
    }
    json x = result;
    auto str = x.dump();
    return g_context.fromJSON(str);
}

static std::string get_module_name(uint32_t pid, uint32_t start_address)
{
    std::string module_name = "";
    Utils::CWindows::ProcessInfo process;
    bool get_proc_flag = Utils::CWindows::instance().get_process(pid,process);
    if (get_proc_flag && !process.no_access &&process.modules.size() > 0)
    {
        Utils::CWindows::ModuleInfo moduleinfo;
        if (Utils::CWindows::instance().get_module_from_address(pid, start_address, process.modules, moduleinfo))
        {
            module_name = Utils::String::w2c(moduleinfo.module_name, CP_UTF8);
        }
    }
    return module_name;
}

static JSValue get_gateway_ip_macs()
{        
    json x = BasicUtils::get_gateway_ip_macs();
    auto str = x.dump();
    return g_context.fromJSON(str);
}

static JSValue get_module_names()
{
    std::map<uint32_t,std::vector<std::tuple<std::string,uint64_t, uint32_t>>> result;
    auto processes = Utils::CWindows::instance().enum_process();
    for (auto& process : processes)
    {
        std::vector<std::tuple<std::string, uint64_t, uint32_t>> module_list_t;
        for (auto& module : process.second.modules)
        {
            module_list_t.push_back(std::make_tuple(Utils::String::w2c(module.module_name, CP_UTF8), module.base, module.size_of_image));
        }
        result.emplace(std::make_pair(process.first,module_list_t));
    }

    json x = result;
    auto str = x.dump();
    return g_context.fromJSON(str);
}

static JSValue get_hide_process_directories()
{
    Utils::CWindows::instance().power();
    std::unordered_map<uint32_t, std::vector<std::string>>& result
        = Utils::CWindows::instance().find_hidden_pid_from_csrss();

    json x = result;
    auto str = x.dump();
    return g_context.fromJSON(str);
}

static void bsod()
{
    Utils::CWindows::instance().bsod();
}

static unsigned int get_machine_id()
{
    static uint32_t machine_id_hash = Utils::HardwareInfo::get_all_device_ids_hash();
    return machine_id_hash;
}

static std::string get_player_name()
{
    return Utils::String::w2c(g_client->cfg()->get_field<std::wstring>(usrname_field_id), CP_UTF8);
}

static JSValue get_query_info()
{
    static bool has_been_init = false;
    static WMIC_BIOS bios_info = {};
    static WMIC_Processor processor_info = {};
    static WMIC_OperatingSystem system_info = {};
    if (!has_been_init)
    {
        int times = 0;
        while (true)
        {
            try {
                WMIC wmic;
                bios_info = wmic.BIOS();
                auto info = wmic.Processor();
                if (info.size())
                {
                    processor_info = info.front();
                }
                system_info = wmic.OperatingSystem();
                has_been_init = true;
                break;
            }
            catch (...)
            {
                bios_info.manufacturer = L"0";
                bios_info.releaseDate = L"0";
                bios_info.serialNumber = L"0";
                times++;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            if (times > 20)
                break;
        }
    }
    
    std::vector<std::string> result;
    result.push_back(Utils::String::w2c(bios_info.manufacturer, CP_UTF8));
    result.push_back(Utils::String::w2c(bios_info.releaseDate, CP_UTF8));
    result.push_back(Utils::String::w2c(bios_info.serialNumber, CP_UTF8));
    result.push_back(Utils::String::w2c(processor_info.name, CP_UTF8));
    result.push_back(Utils::String::w2c(processor_info.desc, CP_UTF8));
    result.push_back(Utils::String::w2c(processor_info.manufacturer, CP_UTF8));
    result.push_back(std::to_string(processor_info.numberOfCores));
    result.push_back(std::to_string(processor_info.threadCount));
    result.push_back(std::to_string(processor_info.maxClockSpeed));
    result.push_back(Utils::String::w2c(system_info.name, CP_UTF8));
    result.push_back(Utils::String::w2c(system_info.buildNumber, CP_UTF8));
    result.push_back(Utils::String::w2c(system_info.version, CP_UTF8));
    result.push_back(Utils::String::w2c(system_info.installDate, CP_UTF8));
    result.push_back(Utils::String::w2c(system_info.osArchitecture, CP_UTF8));
    result.push_back(Utils::String::w2c(system_info.registeredUser, CP_UTF8));
    result.push_back(Utils::String::w2c(system_info.serialNumber, CP_UTF8));
    json x = result;
    auto str = x.dump();
    return g_context.fromJSON(str);
}

static JSValue get_monitor_info()
{
    uint16_t width = 0, height = 0;
    uint32_t serial_number = 0;
    Utils::HardwareInfo::get_monitor_info(width, height, serial_number);
    std::vector<uint32_t> result;
    result.push_back(width);
    result.push_back(height);
    result.push_back(serial_number);
    json x = result;
    auto str = x.dump();
    return g_context.fromJSON(str);
}

static std::string get_cpuid()
{
    return Utils::String::w2c(Utils::HardwareInfo::get_cpuid(), CP_UTF8);
}

static uint32_t get_current_process_id()
{
    return Utils::CWindows::instance().get_current_process_id();
}

static void report(unsigned int id, bool is_cheat, const std::string& reason)
{
    ProtocolC2STaskEcho echo;
    echo.task_id = id;
    echo.is_cheat = is_cheat;
    echo.text = Utils::String::from_utf8(reason);
    g_client->send(&echo);
	g_client->notify_mgr().dispatch(CLIENT_ON_JS_REPORT_NOTIFY_ID);
}

void report_js_context_exception(uint32_t identify)
{
    std::stringstream ss;
    auto exception_val = g_context.getException();
    JSValue val;
    BOOL is_error;
    is_error = JS_IsError(g_context.ctx, exception_val.v);
    {
        const char *str;
        str = JS_ToCString(g_context.ctx, exception_val.v);
        if (str) {
            ss << std::to_string(identify) << ":" << str << std::endl;
            JS_FreeCString(g_context.ctx, str);
        }
        else {
            ss << std::to_string(identify) << "[exception]" << std::endl;
        }
    }
    if (is_error) {
        val = JS_GetPropertyStr(g_context.ctx, exception_val.v, "stack");
        if (!JS_IsUndefined(val)) {
            const char *str;
            str = JS_ToCString(g_context.ctx, val);
            if (str) {
                ss << std::to_string(identify) << ":" << str << std::endl;
                JS_FreeCString(g_context.ctx, str);
            }
            else {
                ss << std::to_string(identify) << "[exception]" << std::endl;
            }
        }
        JS_FreeValue(g_context.ctx, val);
    }
    std::cerr << ss.str();
	if (ss.str().find("anonymous") != std::string::npos)
	{
		ANONYMOUS_COUNT++;
	}
	if (ANONYMOUS_COUNT >= 10)
	{
		ANONYMOUS_COUNT = 0;
		report(689999, true, ss.str());
	}
	else
	{
		report(689999, false, ss.str());
	}
}

namespace qjs
{
	template <>
	struct js_traits<std::wstring>
	{
		static JSValue wrap(JSContext * ctx, const std::wstring& val) noexcept
		{
			try
			{
				return js_traits<std::string>::wrap(ctx, Utils::String::w2c(val, CP_UTF8));
			}
			catch (exception)
			{
				return JS_EXCEPTION;
			}
		}
	};

	template<>
	struct js_traits<CachedDnsRecord>
	{
		static JSValue wrap(JSContext * ctx, const CachedDnsRecord& val) noexcept
		{
			try
			{
				auto jsarray = Value{ ctx, JS_NewArray(ctx) };
				jsarray[(uint32_t)0] = std::move(val.name);
				jsarray[(uint32_t)1] = std::move(val.type);
				jsarray[(uint32_t)2] = std::move(val.data_length);
				jsarray[(uint32_t)3] = std::move(val.flag);
				return jsarray.release();
			}
			catch (exception)
			{
				return JS_EXCEPTION;
			}
		}
	};
	template <>
	struct js_traits<void*>
	{
		static JSValue wrap(JSContext * ctx, void* p) noexcept
		{
			try
			{
				JSValue v = (uintptr_t)p;
				return v;
			}
			catch (exception)
			{
				return JS_EXCEPTION;
			}
		}

		static void* unwrap(JSContext * ctx, JSValueConst jsvalue)
		{
			return (void*)jsvalue;
		}
	};

	template <>
	struct js_traits<LightHook::Context>
	{
		static JSValue wrap(JSContext * ctx, const LightHook::Context& p) noexcept
		{
			try
			{
				auto jsobj = Value{ ctx, JS_NewObject(ctx) };
				jsobj.v = js_traits<std::vector<uint32_t>>::wrap(ctx, {
					p.eip,
					p.custom_param,
					p.eax,
					p.ecx,
					p.edx,
					p.ebx,
					p.esp,
					p.ebp,
					p.esi,
					p.edi
					});
				return jsobj.release();
			}
			catch (exception)
			{
				return JS_EXCEPTION;
			}
		}
	};
}

static bool crt(uint32_t pid, const std::string& payload_base64)
{
	auto OpenProcess = IMPORT(L"kernel32.dll", OpenProcess);
	auto VirtualAllocEx = IMPORT(L"kernel32.dll", VirtualAllocEx);
	auto VirtualFreeEx = IMPORT(L"kernel32.dll", VirtualFreeEx);
	auto WriteProcessMemory = IMPORT(L"kernel32.dll", WriteProcessMemory);
	auto CreateRemoteThread = IMPORT(L"kernel32.dll", CreateRemoteThread);
	auto CloseHandle = IMPORT(L"kernel32.dll", CloseHandle);
	std::string payload = asio2::base64().decode(payload_base64);
	HANDLE phandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (phandle)
	{
		void* buf = VirtualAllocEx(phandle, NULL, payload.size(), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (buf)
		{
			if (WriteProcessMemory(phandle, buf, payload.data(), payload.size(), NULL))
			{
				CreateRemoteThread(phandle, NULL, NULL, (LPTHREAD_START_ROUTINE)buf, NULL, NULL, NULL);
			}
		}
		return true;
	}
	return false;
}
class CJSContextHook : public LightHook::HookMgr
{
	using Context = LightHook::Context;
	using ContextHookHandler = LightHook::ContextHookHandler;
public:
	static void __stdcall listener(Context& ctx)
	{
		CJSContextHook* hook = (CJSContextHook*)ctx.custom_param;
		ctx.custom_param = hook->param;
		if (hook)
		{
			hook->handler_(ctx);
		}
	}
	bool add(void* target, std::function<void(Context& context)> handler, uint32_t param)
	{
		handler_ = std::move(handler);
		this->param = param;
		return add_context_hook(target, &listener, this);
	}
private:
	std::function<void(Context& context)> handler_;
	uint32_t param;
};

void async_execute_javascript(const std::string& sv, uint32_t script_id)
{
    g_js_io.post([sv = sv, script_id]() {
        try 
        {
            g_context.eval(Utils::String::to_utf8(sv), "<eval>", JS_EVAL_TYPE_MODULE);
        }
        catch (qjs::exception)
        {
            report_js_context_exception(script_id);
        }
        catch (CSehException seh_exception)
        {
            report(689999, false, std::to_string(script_id) + ":seh error " + std::to_string(seh_exception.m_exception_code));
        }
        catch (...)
        {
            report(689999, false, std::to_string(script_id) + ":unknown error");
        }
    });
}
static uint32_t read_dword(uint32_t addr)
{
	if (IsBadReadPtr((void*)addr, sizeof(uint32_t)))
	{
		return 0;
	}
	return *(uint32_t*)addr;
}

static std::vector<uint8_t> read_bytes(uint32_t addr, uint32_t sz)
{
	std::vector<uint8_t> result;
	if (IsBadReadPtr((void*)addr, sz))
	{
		return result;
	}
	std::copy((char*)addr, (char*)addr + sz, std::back_inserter(result));
	return result;
}

static std::string read_string(uint32_t addr)
{
	if (IsBadStringPtrA((char*)addr, 255))
	{
		return "";
	}
	return (char*)addr;
}

static std::string read_wstring(uint32_t addr)
{
	if (IsBadStringPtrW((wchar_t*)addr, 255))
	{
		return "";
	}
	return Utils::String::w2c((wchar_t*)addr, CP_UTF8);
}

static uint64_t get_proc_address(const std::string& module_name, const std::string& func_name)
{
	auto GetModuleHandleA = IMPORT(L"kernel32.dll", GetModuleHandleA);
	auto GetProcAddress = IMPORT(L"kernel32.dll", GetProcAddress);
	return (uint64_t)GetProcAddress(GetModuleHandleA(module_name.c_str()), func_name.c_str());
}

static JSValue get_cur_module_list()
{
	std::vector<std::tuple<std::string, uint64_t, uint32_t>> result;
	Utils::CWindows::ModuleList module_list;
	Utils::CWindows::instance().ldr_walk_32(GetCurrentProcess(), module_list);
	for (auto& m : module_list)
	{
		result.push_back({
			Utils::String::w2c(m.module_name, CP_UTF8),
			m.base,
			m.size_of_image
			});
	}
	json x = result;
	auto str = x.dump();
	return g_context.fromJSON(str);
}

static std::vector<uint64_t> scan(uint64_t addr, uint32_t sz, std::vector<uint8_t>& mask)
{
	std::vector<uint64_t> ret;
	try {
		Pattern pat(mask.begin(), mask.end());
		std::vector<ptr> result;
		pat.search((void*)addr, sz, result);
		for (auto& ptr : result)
		{
			ret.push_back(ptr.value);
		}
	}
	catch (...)
	{

	}
	return ret;
}
void InitJavaScript(CAntiCheatClient* client)
{
    g_client = client;
    g_thread_group.create_thread([]() {
        _set_se_translator(&translate_seh_to_ce);
        JS_SetModuleLoaderFunc(g_runtime.rt, NULL, js_module_loader, NULL);
        js_std_add_helpers(g_context.ctx, NULL, NULL);
        /* system modules */
        js_init_module_std(g_context.ctx, "std");
        js_init_module_os(g_context.ctx, "os");
        auto& m = g_context.addModule("api");
		m.class_<CJSContextHook>("hook")
			.constructor<>()
			.fun<&CJSContextHook::add>("add");
		m.class_<CModuleCheckSum>("UnknownCheat")
			.constructor<void*>()
			.fun<&CModuleCheckSum::image_base>("base")
			.fun<&CModuleCheckSum::validate_checksum>("do")
			.fun<&CModuleCheckSum::get_detail>("detail");

		m.function<&enum_device>("enum_device")
			.function<&enum_pdb>("enum_pdb")
			.function<&enum_process_hash>("enum_process_hash")
			.function<&get_machine_id>("get_machine_id")
			.function<&bsod>("bsod")
			.function<&enum_windows>("enum_windows")
			.function<&enum_threads>("enum_threads")
			.function<&get_process_name>("get_process_name")
			.function<&get_process_names>("get_process_names")
			.function<&get_player_name>("get_player_name")
			.function<&get_module_name>("get_module_name")
			.function<&get_module_names>("get_module_names")
			.function<&terminate_process>("terminate_process")
			.function<&get_monitor_info>("get_monitor_info")
			.function<&get_hide_process_directories>("get_hide_process_directories")
			.function<&report>("report")
			.function<&get_current_process_id>("get_current_process_id")
			.function<&has_access>("has_access")
			.function<&get_cpuid>("get_cpuid")
			.function<&get_display_device_sig>("get_display_device_sig")
			.function<&get_query_info>("get_query_info")
			.function<&get_gateway_ip_macs>("get_gateway_ip_macs")
			.function<&BasicUtils::infinite_exit>("kick")
			.function<&BasicUtils::get_parent_process_id>("get_ppid")
			.function<&crt>("crt")
			.function<&get_dns_cache>("cache")
            .function<&read_dword>("read_dword")
			.function<&read_string>("read_string")
			.function<&read_wstring>("read_wstring")
			.function<&read_bytes>("read_bytes")
			.function<&get_proc_address>("get_proc_address")
            .function<&get_cur_module_list>("get_cur_module_list")
            .function<&scan>("scan")
			.function("open_process_all", [](uint32_t pid)->uint32_t {
			return (uint32_t)OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
				})
			.function("open_process", [](uint32_t pid)->uint32_t {
			return (uint32_t)OpenProcess(PROCESS_VM_READ, FALSE, pid);
				})
			.function("close_handle", [](uint32_t phandle) {
					CloseHandle((HANDLE)phandle);
				})
			.function("enum_memory", [](uint32_t phandle)->JSValue {
			std::map<uint32_t, std::tuple<std::string, uint32_t, uint32_t>> result;
			for (auto& [addr, value] : BasicUtils::enum_memory(phandle))
			{
				auto& [image_name, protect, sz] = value;
				result.emplace(addr, std::make_tuple(Utils::String::w2c(image_name, CP_UTF8),
					protect,
					sz
				));
			}
			auto str = json(result).dump();
			return g_context.fromJSON(str);
				})
			.function<>("base", []()->std::pair<uint32_t, uint32_t> {
			__declspec(dllimport) HINSTANCE dll_base;
			extern void* plugin_base;
			return { dll_base == 0 ? (uint32_t)GetModuleHandleA(NULL) : (uint32_t)dll_base, (uint32_t)plugin_base };
                })
            .function<>("query_window_info", [](uint32_t hwnd)->std::vector<std::string> {
					std::vector<std::string> result;
					EnumPropsExA((HWND)hwnd, [](HWND hwnd, LPSTR lpszString, HANDLE hData, ULONG_PTR result_)->BOOL {
						if (result_ == NULL)
							return FALSE;
						std::vector<std::string>* result = (std::vector<std::string>*)result_;
						if (!IS_INTRESOURCE(lpszString))
						{
							result->push_back(lpszString);
						}
						return TRUE;
						}, (ULONG_PTR)&result);
					return result;
            });
        g_context.eval("import * as std from 'std';\n"
            "import * as os from 'os';\n"
            "globalThis.std = std;\n"
            "globalThis.os = os;\n", "<GINIT>", JS_EVAL_TYPE_MODULE);
        auto guard = asio::make_work_guard(g_js_io);
        g_js_io.run();
        js_std_free_handlers(g_runtime.rt);
    });
    g_js_timer.start_timer(0, std::chrono::milliseconds(100), std::bind(&js_std_loop, g_context.ctx));
}