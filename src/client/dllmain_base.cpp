// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include <windows.hpp>
#include <process.h>
#include <lighthook.h>
#include <fstream>
#include <string>
#include <intrin.h>
#include "anticheat.h"
#include "peloader\peloader.h"
#include "utils\utils.h"
#include "utils\windows_internal.h"
#include <AsmJit\AsmJit.h>
#include <LDasm.h>
#include "VMProtectSDK.h"
#include <vector>
#include "game_local_funtion.h"
#include "cheat.h"

#define POINTER_IS_ALIGNED(Pointer, Alignment) \
    (((((ULONG_PTR)(Pointer)) & (((Alignment)-1))) == 0) ? TRUE : FALSE)

#ifdef IMPORT
#undef IMPORT
#define IMPORT(module_name,func_name) (decltype(&::func_name))ApiResolver::get_proc_address(ApiResolver::get_module_handle(CT_HASH(module_name)), CT_HASH(#func_name))
#endif

#ifdef _DEBUG
#define CONFIG_APP_NAME "内部测试"
#define CONFIG_WEBSITE "http://www.baidu.com/"
#endif
#ifdef BUILD_RELEASE
#define CONFIG_APP_NAME "★★=======二进制封挂加载成功=======★★"
#define CONFIG_WEBSITE  "★★===       www.7522.com       ===★★"
#endif
#ifdef BUILD_RELEASE2
#define CONFIG_APP_NAME "★★========审判封挂加载成功========★★"
#define CONFIG_WEBSITE  "★★===       www.pm76.com       ===★★"
#endif
void push_string(asmjit::X86Assembler& code, const char* str);
HMODULE dll_base;
bool cheat_hooked = false;
void main_thread(void*)
{
    AntiCheat& anticheat = AntiCheat::instance();
    AntiCheat::load_rsrc(dll_base);
    while (true)
    {
        anticheat.time_proc_dispatcher(::GetTickCount());
        ::SleepEx(1000, TRUE);
    }
}

namespace HookProc
{
#define FUNCTION_CHECK() auto GetModuleHandleExW = IMPORT(L"kernel32.dll", GetModuleHandleExW);\
    HMODULE module = NULL;\
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)_ReturnAddress(), &module);\
    if (module == dll_base)\
    {\
        SetLastError(ERROR_BAD_CONFIGURATION);\
        return FALSE;\
    }
    decltype(&::CreateProcessInternalW) create_process_internal_w = nullptr;
    decltype(&::ResumeThread) resume_thread = nullptr;
    HANDLE main_thread_handle = NULL;
    HANDLE process_handle = NULL;
    auto GetModuleFileNameW = IMPORT(L"kernel32.dll", GetModuleFileNameW); 
    BOOL WINAPI CreateProcessInternalW(
        HANDLE token_handle, LPCWSTR app_name, LPWSTR cmd_line,
        LPSECURITY_ATTRIBUTES process_attributes, LPSECURITY_ATTRIBUTES security_attributes,
        BOOL bih, DWORD flags, LPVOID env, LPCWSTR current_dir, LPSTARTUPINFOW startup_info,
        LPPROCESS_INFORMATION process_info, PHANDLE new_token)
    {
        VMP_VIRTUALIZATION_BEGIN();
        SetLastError(ERROR_SUCCESS);
        FUNCTION_CHECK();
        BOOL result = create_process_internal_w(token_handle,
            app_name,
            cmd_line,
            process_attributes,
            security_attributes,
            bih,
            flags,
            env,
            current_dir,
            startup_info,
            process_info,
            new_token);
        if(result && cmd_line != nullptr)
        {
            std::wstring main_module_file_name, cmd_line_str;
            std::string main_module_file_namea, cmd_line_stra;

            cmd_line_str = cmd_line;
            main_module_file_name.resize(MAX_PATH);
            GetModuleFileNameW(NULL, main_module_file_name.data(), MAX_PATH);

            main_module_file_namea = Utils::wstring2string(main_module_file_name);
            cmd_line_stra = Utils::wstring2string(cmd_line_str);
            if(cmd_line_stra.find(main_module_file_namea) != std::string::npos)
            {
                main_thread_handle = process_info->hThread;
                process_handle = process_info->hProcess;
            }
        }
        return result;

        VMP_VIRTUALIZATION_END();
    }

    DWORD WINAPI ResumeThread(HANDLE thread_handle)
    {
        VMP_VIRTUALIZATION_BEGIN();
        SetLastError(ERROR_SUCCESS);
        FUNCTION_CHECK();
        if (process_handle && main_thread_handle && thread_handle == main_thread_handle)
        {
            auto get_thread_context = IMPORT(L"kernel32.dll", GetThreadContext);
            CONTEXT context = {};
            context.ContextFlags = CONTEXT_FULL;
            if (!get_thread_context(thread_handle, &context))
            {
                ::MessageBoxA(NULL, "GetThreadContext Error", "Warning", MB_OK);
                return 0;
            }
            uintptr_t oep = context.Eax;
            {
                auto read_process_memory = IMPORT(L"kernel32.dll", ReadProcessMemory);
                uint8_t buffer[8] = {};
                DWORD bytes_of_read = 0;
                read_process_memory(process_handle, (LPVOID)oep, buffer, sizeof(buffer), &bytes_of_read);
                //17年登录器入口点兼容
                if (*(uint32_t*)buffer == 0xE8535056)
                {
                    oep = oep + 8 + *(uint32_t*)&buffer[4];
                }
            }
            {
                using namespace asmjit;
                JitRuntime rt;
                X86Assembler code(&rt);
                code.pusha();
                code.push(host::ebp);
                code.mov(host::ebp, host::esp);
                push_string(code, "client.dll");
                code.push(host::esp);
                code.mov(host::eax, (asmjit::Ptr)IMPORT(L"kernel32.dll", LoadLibraryA));
                code.call(host::eax);
                push_string(code, "AntiCheatEntry2");
                code.push(host::esp);
                code.push(host::eax);
                code.mov(host::eax, (asmjit::Ptr)IMPORT(L"kernel32.dll", GetProcAddress));
                code.call(host::eax);
                code.call(host::eax);
                code.mov(host::esp, host::ebp);
                code.pop(host::ebp);
                code.popa();
                std::string shellcode;
                shellcode.resize(code.getCodeSize());
                auto ptr = shellcode.data();
                code.relocCode((void*)shellcode.data());
                auto Sleep = IMPORT(L"kernel32.dll", Sleep);
                auto hooked = false;
                for (int i=0;i<30;i++)
                {
                    hooked = LightHook::HookMgr::instance().add_remote_inline_hook(process_handle, (void*)oep, shellcode);
                    if(hooked)
                    {
                        break;
                    }
                    Sleep(200);
                }

                if(!hooked)
                {
                    ::MessageBoxA(NULL, "add_remote_inline_hook error", "Warning", MB_OK);
                    ::TerminateProcess(main_thread_handle, 0);
                    return ERROR_ERRORS_ENCOUNTERED;
                }
            }
            main_thread_handle = NULL;
            process_handle = NULL;
        }
        return resume_thread(thread_handle);
        VMP_VIRTUALIZATION_END();
    }

    DWORD WINAPI check_hook_vaild_routine(LPVOID param)
    {
        VMP_VIRTUALIZATION_BEGIN();
        auto Sleep = IMPORT(L"kernel32.dll", Sleep);
        auto GetModuleHandleExW = IMPORT(L"kernel32.dll", GetModuleHandleExW);
        auto ExitProcess = IMPORT(L"kernel32.dll", ExitProcess);

        //LOG_EVENT("check_hook_vaild_routine 11");
        while (true)
        {
            auto hooks = LightHook::HookMgr::instance().get_hooks();
            for (auto &hook : hooks)
            {
                if (!hook->is_remote_hook() && hook->is_hooked())
                {
                    uint8_t* hook_addr = reinterpret_cast<uint8_t*>(hook->get_hook_addr());
                    uint8_t* src_addr = reinterpret_cast<uint8_t*>(ResolveJmp(hook->get_src()));
                    HMODULE module = NULL;
                    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)src_addr, &module);
                    if (*hook_addr != 0xE9 || module != dll_base || *(src_addr+0x2) == 0xE9)
                    {
                        *(void**)_AddressOfReturnAddress() = ExitProcess;
                        return 0;
                    }
                }
            }
            HookProc::CreateProcessInternalW(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
            if (GetLastError() != ERROR_BAD_CONFIGURATION)
            {
                *(void**)_AddressOfReturnAddress() = ExitProcess;
                return 0;
            }
            HookProc::ResumeThread(NULL);
            if (GetLastError() != ERROR_BAD_CONFIGURATION)
            {
                *(void**)_AddressOfReturnAddress() = ExitProcess;
                return 0;
            }
            Sleep(100);
        }
        return 0;
        VMP_VIRTUALIZATION_END();
    }
}
extern "C" BOOL APIENTRY AntiCheatEntry()
{
    VMP_VIRTUALIZATION_BEGIN();
    //登录器入口
    void* create_process_internal_w = IMPORT(L"kernelbase.dll", CreateProcessInternalW);
    if (!create_process_internal_w)
        create_process_internal_w = IMPORT(L"kernel32.dll", CreateProcessInternalW);
    auto QueueUserWorkItem = IMPORT(L"kernel32.dll", QueueUserWorkItem);
    QueueUserWorkItem(&HookProc::check_hook_vaild_routine, 0, WT_EXECUTEDEFAULT);
    /*
    因有的登录器带有加速盾(启动顺序为  盾-->登录器-->游戏),导致与此父进程检测规则冲突,所以暂时取消此父进程检测 
    
    auto& win = Utils::CWindows::instance();
    uint32_t _current_process_id = win.get_current_process_id();
    uint32_t _process_parent_id = win.get_process_parent(_current_process_id);
    Utils::CWindows::ProcessInfo parent_process, child_process;
    win.get_process(_current_process_id, child_process);
    if(win.get_process(_process_parent_id, parent_process) &&
        parent_process.name != child_process.name &&
        !win.is_process_open_from_explorer(_current_process_id))
    {
        auto msgbox = IMPORT(L"user32.dll", MessageBoxA);
        msgbox(NULL, "请把登录器放在桌面或者游戏目录运行", "提示", MB_OK);
        auto exit_process = IMPORT(L"kernel32.dll", ExitProcess);
        exit_process(0);
    }
    */
    LightHook::HookMgr::instance().add_inline_hook(create_process_internal_w, &HookProc::CreateProcessInternalW, &HookProc::create_process_internal_w);
    void* resume_thread = IMPORT(L"kernel32.dll", ResumeThread);
    LightHook::HookMgr::instance().add_inline_hook(resume_thread, &HookProc::ResumeThread, &HookProc::resume_thread);
    return TRUE;
    VMP_VIRTUALIZATION_END();
}

void self_remapping()
{
	static bool init = false;
	if (init)
		return;
	init = true;
	SYSTEM_INFO SystemInfo = {};
	GetSystemInfo(&SystemInfo);
	void* image = GetModuleHandleA(NULL);
	auto pNtHeaders = GET_IMAGE_NT_HEADER(image);
	auto pSectionHeader = IMAGE_FIRST_SECTION(pNtHeaders);

	for (WORD i = 0; i < pNtHeaders->FileHeader.NumberOfSections; ++i)
	{
		auto SectionBase =
			pNtHeaders->OptionalHeader.ImageBase +
			pSectionHeader[i].VirtualAddress;

		LOG_EVENT("    %-8.8s    0x%IX - 0x%IX,  0x%08X\n",
			pSectionHeader[i].Name,
			SectionBase,
			SectionBase + pSectionHeader[i].Misc.VirtualSize,
			pSectionHeader[i].Misc.VirtualSize);

		BOOL status = POINTER_IS_ALIGNED(
			SectionBase,
			SystemInfo.dwAllocationGranularity);
		if (!status)
		{
			LOG_EVENT("Unexpected section alignment. (SectionBase = 0x%IX)\n",
				SectionBase);
		}
	}
	LOG_EVENT("image size %08X", pNtHeaders->OptionalHeader.SizeOfImage);
	SIZE_T ImageViewSize = pNtHeaders->OptionalHeader.SizeOfImage;
	HANDLE hSection = NULL;
	NTSTATUS ntstatus = STATUS_SUCCESS;
	LARGE_INTEGER cbSectionSize = {};
	auto NtCreateSection = IMPORT(L"ntdll.dll", NtCreateSection);
	auto NtMapViewOfSection = IMPORT(L"ntdll.dll", NtMapViewOfSection);
	auto NtUnmapViewOfSection = IMPORT(L"ntdll.dll", NtUnmapViewOfSection);

	cbSectionSize.QuadPart = ImageViewSize;
	ntstatus = NtCreateSection(
		&hSection,
		SECTION_ALL_ACCESS,
		NULL,
		&cbSectionSize,
		PAGE_EXECUTE_READWRITE,
		SEC_COMMIT | SEC_NO_CHANGE,
		NULL);

	PVOID pViewBase = NULL;
	LARGE_INTEGER cbSectionOffset = {};
	SIZE_T cbViewSize = 0;

	ntstatus = NtMapViewOfSection(
		hSection,
		NtCurrentProcess(),
		&pViewBase,
		0,
		ImageViewSize,
		&cbSectionOffset,
		&cbViewSize,
		ViewUnmap,
		0,
		PAGE_READWRITE);
	if (!NT_SUCCESS(ntstatus))
	{
		LOG_EVENT("NtMapViewOfSection failed");
		return;
	}

	LOG_EVENT("NtMapViewOfSection %08X", pViewBase);
	SIZE_T bytes_of_write = 0;
	WriteProcessMemory(GetCurrentProcess(), pViewBase, image, ImageViewSize, &bytes_of_write);
	//NtUnmapViewOfSection(NtCurrentProcess(), pViewBase);
	BYTE* backup_base = (BYTE*)pViewBase;

	NtUnmapViewOfSection(NtCurrentProcess(), image);

	pViewBase = (PVOID)image;
	cbSectionOffset.HighPart = 0;
	DWORD code_size = 0x340000;
	cbViewSize = code_size;
	ntstatus = NtMapViewOfSection(
		hSection,
		NtCurrentProcess(),
		&pViewBase,
		0,
		0,
		&cbSectionOffset,
		&cbViewSize,
		ViewUnmap,
		SEC_NO_CHANGE,
		PAGE_EXECUTE_READ);

	LOG_EVENT("%08X", ntstatus);
	LPVOID rest = VirtualAlloc((BYTE*)image + code_size, ImageViewSize - code_size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	LOG_EVENT("%08X", rest);
	WriteProcessMemory(GetCurrentProcess(), (BYTE*)image + code_size, backup_base + code_size, ImageViewSize - code_size, &bytes_of_write);
	
	::MessageBoxA(NULL, "123", "123", MB_OK);
}

namespace WndProcHideHook
{
    LightHook::ContextHook* set_window_long_hook = nullptr;
    WNDPROC old_wnd_proc = nullptr;
    LRESULT WINAPI wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        VMP_VIRTUALIZATION_BEGIN();
		return old_wnd_proc(hwnd, msg, wparam, lparam);
        auto GetTickCount = IMPORT(L"kernel32.dll", GetTickCount);
        if((GetTickCount() - Singleton<TaskHeartBeat>::getInstance().get_last_tickcount()) > 60 * 1000 * 2
           // 心跳回包,防客户端拦截网关包,导致网关无法对玩家进行处罚,如果网关出问题会导致所有玩家卡死,先停用
           || (GetTickCount() - Singleton<TaskHeartBeat>::getInstance().get_last_recv_tick_count()) > 60 * 1000 * 1)
        {
            auto ExitProcess = IMPORT(L"kernel32.dll", ExitProcess);
            *(void**)_AddressOfReturnAddress() = ExitProcess;
            return 0;
        }
        /*if(!cheat_hooked)
        {
            cheat_hooked = Cheat::instance().hook_init();
        }*/
#if (CONFIG_GET_HOST_FROM_CONNECT_HOOK == 0)
        if (AntiCheat::instance().ip_ == L"*")
        {
            AntiCheat::instance().ip_ = Utils::string2wstring((char*)0x100BCCE/*0xFF3E0E*/);
        }
#endif
        return old_wnd_proc(hwnd, msg, wparam, lparam);
        VMP_VIRTUALIZATION_END();
    }
}

void show_welcome_msg(void*)
{
    VMP_VIRTUALIZATION_BEGIN();
    auto Sleep = IMPORT(L"kernel32.dll", Sleep);
    Sleep(10000);

    GameLocalFuntion& gameLocalFuntion = GameLocalFuntion::instance();
    gameLocalFuntion.hook_init();

    DWORD top = -1;
    strings welcomemsg = {top,41, CONFIG_APP_NAME};

    while(true)
    {
        if(Utils::Player::get_game_window_username().find(L" - ") != std::wstring::npos)
        {
            Sleep(10000);
            gameLocalFuntion.sendmsg_call(welcomemsg);
            break;
        }
        Sleep(500);
    }
    VMP_VIRTUALIZATION_END();
}

extern "C" BOOL APIENTRY AntiCheatEntry2()
{

    VMP_VIRTUALIZATION_BEGIN();
    //游戏入口
#ifdef _DEBUG
    ::MessageBoxA(NULL, "test", "test", MB_OK);
#endif
    using namespace WndProcHideHook;
	LightHook::HookMgr::instance().add_context_hook(&GetCommandLineA, [](LightHook::Context& ctx) {
		uint32_t ret_address = *(uint32_t*)ctx.esp;
		void* image = GetModuleHandleA(NULL);
		auto pNtHeaders = GET_IMAGE_NT_HEADER(image);
		if (((uint32_t)image < ret_address) &&
			(ret_address < ((uint32_t)image + pNtHeaders->OptionalHeader.SizeOfImage)))
		{
			if (ret_address == 0x00A17634)
			{
				LOG_EVENT("%08X", ret_address);
				self_remapping();
			}
		}
		
	});
    if (set_window_long_hook == nullptr)
    {
        set_window_long_hook = new LightHook::ContextHook();
        set_window_long_hook->install(IMPORT(L"user32.dll", SetWindowLongA), [](LightHook::Context& ctx) {
            uintptr_t* param = (uintptr_t*)ctx.esp;
            //LOG_EVENT("%d,%d,%d", param[1], param[2], param[3]);
            if (param[2] == GWLP_WNDPROC)
            {
                old_wnd_proc = (WNDPROC)param[3];
                param[3] = (uintptr_t)&wnd_proc;
                set_window_long_hook->restore();
            }
        }, NULL);
    }
    _beginthread(&main_thread, NULL, NULL);
    _beginthread(&show_welcome_msg, NULL, NULL);
    return TRUE;
    VMP_VIRTUALIZATION_END();
}

void push_string(asmjit::X86Assembler& code, const char* str)
{
    int len = strlen(str);
    int loop = len / sizeof(uint32_t);
    code.push(0);
    for (int n = 0; n <= loop; n++)
    {
        code.push(((uint32_t*)str)[loop - n]);
    }
}

BOOL APIENTRY DllMain(HMODULE module_instance,
    DWORD  ul_reason_for_call,
    LPVOID reserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        dll_base = module_instance;
        /*if(!cheat_hooked)
        {
            cheat_hooked = Cheat::instance().hook_init();
        }*/

        break;
    }
    case DLL_THREAD_ATTACH:
    {
        VMP_VIRTUALIZATION_BEGIN();
        using namespace Utils;
        auto GetCurrentThreadId = IMPORT(L"kernel32.dll", GetCurrentThreadId);
        auto GetModuleHandleExW = IMPORT(L"kernel32.dll", GetModuleHandleExW);
        auto VirtualProtect = IMPORT(L"kernel32.dll", VirtualProtect);
        CWindows::ProcessInfo process;
        CWindows::instance().get_process(CWindows::instance().get_current_process_id(), process);
        if(process.threads.size() == 0)
        {
            break;
        }
        LPVOID thread_start_address = (LPVOID)process.threads[GetCurrentThreadId()].start_address;
        if(thread_start_address)
        {
            HMODULE thread_module = nullptr;
            GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)thread_start_address, &thread_module);
            if(thread_module == nullptr)
            {
                DWORD oldprotect = NULL;
                VirtualProtect(thread_start_address, 1, PAGE_EXECUTE_READWRITE, &oldprotect);
                *(uint8_t*)thread_start_address = 0xC3;
                VirtualProtect(thread_start_address, 1, oldprotect, &oldprotect);
            }
        }
        break;
        VMP_VIRTUALIZATION_END();
    }
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

