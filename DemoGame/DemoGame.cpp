
#define WIN32_LEAN_AND_MEAN
#include "pch.h"
#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "Lightbone/utils.h"
#include "Service/SubServicePackage.h"
#include "Service/NetUtils.h"
#include "asio2/http/http_client.hpp"
#include "asio2/util/base64.hpp"
#include "Service/AntiCheatClient.h"
#include "lf_rungate_server_plug/lf_plug_sdk.h"
#include "NewClient/ShellCode/TaskBasic.h"
#include "NewClient/ShellCode/anti_monitor_directory/ReadDirectoryChanges.h"
#include "wmic/wmic.h"
#include <NewClient/ShellCode/BasicUtils.h>
#include <iphlpapi.h>
#include <DbgHelp.h>
#include <Psapi.h>
#include <Uxtheme.h>
//#include "NewClient/loader.h"
#pragma comment(lib, "iphlpapi.lib") 

extern void __stdcall client_entry(std::string guard_gate_ip) noexcept;
extern void __stdcall DoUnInit();
using client_entry_t = decltype(&client_entry);
using uninit_t = decltype(&DoUnInit);
namespace fs = std::filesystem;
//extern void LoadPlugin(CAntiCheatClient* client);
extern uint32_t peload(void* buffer, size_t size, HINSTANCE* instance, void* params);
extern void execute_tls_callback(HINSTANCE instance, uint32_t reason, void* param);
extern void execute_entrypoint(HINSTANCE instance, uint32_t reason, void* param);
extern void enable_seh_on_shellcode();
extern std::shared_ptr<asio::io_service> g_game_io;
void init_client_entry();
void test_task_basic_dll(fs::path path);
bool IsThreadFromModule(DWORD tid); 
typedef void(__stdcall* InitExFunc)(const void* AppFunc, const char* server_ip);
// 原DLL的初始化函数指针
InitExFunc pfnAntiCheatInit = nullptr;
void GetWindowFontInfo(HWND hWnd) {
    // 获取当前字体句柄
    HFONT hFont = (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0);

    if (hFont == NULL) {
        printf("Window uses system default font\n");
        return;
    }

    // 获取字体详细信息
    LOGFONT lf;
    if (GetObject(hFont, sizeof(LOGFONT), &lf)) {
        printf("Font Face: %s\n", lf.lfFaceName);
        printf("Height: %d (pixels)\n", abs(lf.lfHeight));
        printf("Weight: %d (400=normal, 700=bold)\n", lf.lfWeight);
        printf("Italic: %s\n", lf.lfItalic ? "Yes" : "No");
        printf("CharSet: %d\n", lf.lfCharSet);
    }
    else {
        printf("Failed to get font info\n");
    }
}

int main(int argc, char** argv)
{

    HWND hNotepad = (HWND)201570;
    if (hNotepad) {
        GetWindowFontInfo(hNotepad);
    }
    else {
        printf("Notepad not found\n");
    }
	//init_client_entry();
	//wchar_t* user_profile = nullptr;
	//size_t len = 0;
	//FileChangeNotifier notifier;
	//notifier.add_filter_role(L"wpe", L"WPE.INI");
	//notifier.add_filter_role(L"CE", L"ADDRESSES.FIRST");
	//notifier.add_filter_role(L"CE", L"MEMORY.FIRST");

	//if (_wdupenv_s(&user_profile, &len, L"USERPROFILE") == 0 && user_profile != nullptr) {
	//	notifier.start_directory_and_monitor(user_profile, true, [](const std::wstring& filter_role, int action, const std::wstring& file_path) {

	//		std::cout << Utils::String::w2c(filter_role + L"|" + explain_action(action) + L"|" + file_path) << std::endl;

	//		});
	//	free(user_profile);  // 释放内存
	//}

	//notifier.start_directory_and_monitor(L"D:\\work\\temp\\2024", true, [](const std::wstring& filter_role, int action, const std::wstring& file_path) {

	//	std::cout << Utils::String::w2c(filter_role + L"|" + explain_action(action) + L"|" + file_path) << std::endl;

	//	});

	
	//fs::path path("d:\\");
	//test_task_basic_dll(path);
    /*auto windows = get_tcp_table();
    for (auto& [ip,port] : windows)
    {
        std::cout << ip.c_str() << ":" << port << std::endl;

    }*/
    //auto processes = Utils::CWindows::instance().enum_process();
    //int nub = 0;
    //auto signature = Utils::CWindows::instance().verify_signature(L"e:\\sihost.exe");
    //for (auto& i : processes) {
    //    auto path = GetProcessPath(i.first);
    //    if(path.empty()) continue;
    //    auto signature = Utils::CWindows::instance().verify_signature(path);
    //    Utils::CWindows::SignatureInfo sign;
    //    if (!signature.has_value()) {
    //        std::cout << ++nub << ": " << i.first << " - " << Utils::String::w2c(i.second.name).c_str() << "|无签名|" << Utils::String::w2c(path).c_str() << std::endl;
    //        continue;
    //    }
    //    //else {
    //    //    sign = signature.value();
    //    //    std::cout << ++nub << ": " << i.first << " - " << Utils::String::w2c(i.second.name).c_str() << "|" << sign.issuer.c_str() 
    //    //        << "|" << sign.subject.c_str() << "|" << FileTimeToString(sign.timestamp).c_str() << Utils::String::w2c(path).c_str() << std::endl;
    //    //}
    //}


    //auto processes = Utils::CWindows::instance().enum_process();
    //int nub = 0;
    ////auto signature = Utils::CWindows::instance().verify_signature(L"e:\\sihost.exe");
    //for (auto& i : processes) {
    //    auto path = GetProcessPath(i.first);
    //    if(path.empty()) continue;
    //    //std::wstring filePath = L"e:\\sihost.exe";
    //    std::wstring errorMsg;

    //    if (!VerifySystemFileSignatureEx(path, errorMsg)) {
    //        std::cout << ++nub << ": " << i.first << " - " << Utils::String::w2c(i.second.name).c_str() << "|无签名|" << Utils::String::w2c(errorMsg).c_str() << std::endl;
    //        continue;
    //    }
    //}
    //std::wstring filePath = L"e:\\sihost.exe";
    //std::wstring errorMsg;

    //if (VerifyFileSignature(filePath, errorMsg))
    //{
    //    MessageBoxW(nullptr, L"签名验证成功", L"结果", MB_OK);
    //}
    //else
    //{
    //    MessageBoxW(nullptr, errorMsg.c_str(), L"验证失败", MB_OK);
    //}
    getchar();
    std::cout << "Hello World!\n";
}
#pragma comment(lib, "DbgHelp.lib")
bool IsThreadFromModule(DWORD tid) {
    HANDLE hThread = ::OpenThread(THREAD_GET_CONTEXT, FALSE, tid);
    CONTEXT ctx = { 0 };
    ctx.ContextFlags = CONTEXT_CONTROL;

    // 获取线程上下文
    if (!::GetThreadContext(hThread, &ctx)) return false;

    // 获取线程起始地址
    STACKFRAME64 stack = { 0 };
    stack.AddrPC.Offset = ctx.Eip;
    stack.AddrPC.Mode = AddrModeFlat;

    // 符号解析
    DWORD modBase = 0;
    if (!::SymGetModuleBase(::GetCurrentProcess(), modBase)) {
        return false;
    }

    // 检查模块地址范围
    MODULEINFO modInfo;
    if (::GetModuleInformation(
        ::GetCurrentProcess(),
        reinterpret_cast<HMODULE>(modBase),
        &modInfo,
        sizeof(MODULEINFO))
        ) {
        return (stack.AddrPC.Offset >= reinterpret_cast<DWORD_PTR>(modInfo.lpBaseOfDll) &&
                stack.AddrPC.Offset <= reinterpret_cast<DWORD_PTR>(modInfo.lpBaseOfDll) + modInfo.SizeOfImage);
    }
    return false;
}
bool ProcessMainWindow() {
    std::vector<Utils::CWindows::WindowInfo> windows;
    //auto pid = Utils::CWindows::instance().get_current_process_id();
    //printf("pid: %d\n", pid);
    if (!Utils::CWindows::instance().get_process_main_thread_hwnd(Utils::CWindows::instance().get_current_process_id(), windows)) {
        return false;
    }
    printf("windows.size(): %d\n", windows.size());
    // 使用范围for循环和auto引用避免拷贝
    for (auto& window : windows) {
        // 转换类名为小写进行统一比较
        printf("transform1");
        std::transform(window.class_name.begin(), window.class_name.end(),
                       window.class_name.begin(), ::towlower);
        printf("transform2");

        if (window.class_name == L"tfrmmain") {
            // 直接存储HWND无需智能指针，除非需要共享所有权
            return true;
        }
        printf("transform3");
    }
    GetDesktopWindow();
        printf("transform4");
    return false;
}
void init_client_entry() {
    printf("init_client_entry==============\n");
    ProcessMainWindow();
    //MessageBoxA(nullptr, "init_client_entry", "init_client_entry", MB_OK);
	auto hmodule = LoadLibraryA(".\\NewClient_f.dll");
    printf("hmodule: %p\n", hmodule);
    //pfnAntiCheatInit = (InitExFunc)GetProcAddress(g_hAntiCheat, "InitEx");
    //client_entry_t entry = (client_entry_t)ApiResolver::get_proc_address(hmodule, CT_HASH("client_entry"));
    InitExFunc pfnAntiCheatInit = (InitExFunc)ApiResolver::get_proc_address(hmodule, CT_HASH("InitEx"));
    //printf("client_entry: %p\n", entry);
    printf("client_entry: %p\n", pfnAntiCheatInit);
	//uninit_t uninit = (uninit_t)ApiResolver::get_proc_address(hmodule, CT_HASH("DoUnInit"));
    //printf("uninit: %p\n", uninit);
    pfnAntiCheatInit(nullptr ,"140.210.20.215");
	//entry("");
	//Sleep(5000);
	//uninit();
	/*if (FreeLibrary(hmodule))
		std::cout << "FreeLibrary ok!\n";*/
}

void test_task_basic_dll(fs::path path) {
	std::ifstream file(path.parent_path() / "taskbasic.dll", std::ios::in | std::ios::binary);
	if (file.is_open())
	{
		std::cout << "start running\n";
		std::stringstream ss;
		ss << file.rdbuf();
		xor_buffer(ss.str().data(), ss.str().size(), kProtocolXorKey);

		/*std::ofstream file1("d:\\work\\Repos\\M2\\taskbasic1.dll", std::ios::out | std::ios::binary);
		file1.write(ss.str().data(), ss.str().size());
		file1.close();*/

		//plugin_handle = dll_base;
		auto a = peload(ss.str().data(), sizeof(IMAGE_DOS_HEADER), nullptr, NULL);
		if (a == ERROR_SUCCESS)
		{
			std::cout << "start running 11\n";
			enable_seh_on_shellcode();
			//execute_tls_callback(plugin_handle, DLL_PROCESS_ATTACH, 0);
			//execute_entrypoint(plugin_handle, DLL_PROCESS_ATTACH, 0);
			//instance_->log(0, TEXT("CClientPluginMgr::load_plugin 加载插件>>> 3"));
			//decltype(&LoadPlugin) plugin_entry = (decltype(&LoadPlugin))ApiResolver::get_proc_address(plugin_handle, ApiResolver::hash("LoadPlugin"));
			//OutputDebugStringA((std::to_string(*(int*)plugin_entry) + "<<< LoadPlugin").c_str());
			std::cout << "start running 22\n";
		}
		std::cout << "start running ok\n";
	}
	else
	{
		std::cout << "client.bin not exist\n";
	}
}
