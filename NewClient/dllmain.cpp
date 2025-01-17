// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "ClientImpl.h"
#include "Tools/Packer/loader.h"
#include "CreateProcessHook.h"
#include "version.build"
#include <Lightbone/api_resolver.cpp>
share_data_ptr_t share_data = nullptr;

__declspec(dllexport) std::shared_ptr<HINSTANCE> dll_base;
__declspec(dllexport) std::shared_ptr<asio::io_service> g_game_io;
__declspec(dllexport) std::shared_ptr<asio::detail::thread_group> g_thread_group;
__declspec(dllexport) std::shared_ptr<int> g_client_rev_version;
__declspec(dllexport) std::shared_ptr<CClientImpl> client;

void __declspec(dllexport) reference_to_api()
{
    std::set<void*> ref;
    ref.emplace(get_ptr(&Utils::CWindows::instance));
    ref.emplace(get_ptr(&Utils::get_screenshot));
    ref.emplace(get_ptr(&Utils::PEScan::calc_pe_ico_hash));
    ref.emplace(get_ptr(&Utils::ImageProtect::instance));
}

namespace AntiDebugger {
    BOOL isDebuggerPresent() {
        // 多重调试器检测
        BOOL debuggerPresent = FALSE;
        // 使用动态加载API
        DWORD debugPort = 0;
        LONG status = Utils::CWindows::instance().query_information_process<uint32_t>(
            GetCurrentProcess(),
            7,  // ProcessDebugPort
            &debugPort,
            sizeof(debugPort),
            NULL
        );

        if (status >= 0 && debugPort != 0) {
            return TRUE;
        }

        // 标准方法
        if (IsDebuggerPresent()) return TRUE;

        // PEB检测（兼容性方案）
        __try {
            _PMPEB pPeb = (_PMPEB)__readfsdword(0x30);
            if (pPeb && pPeb->bBeingDebugged) return TRUE;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return FALSE;
        }

        return FALSE;
    }

    BOOL checkDebuggerFlags() {
        __try {
            _PMPEB pPeb = (_PMPEB)__readfsdword(0x30);
            if (pPeb) {
                DWORD* flags = (DWORD*)((BYTE*)pPeb + 0x68);
                return (*flags & 0x70) != 0;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return FALSE;
        }
        return FALSE;
    }

    BOOL checkTimingAttack() {
        LARGE_INTEGER start = { 0 }, end = { 0 }, freq = { 0 };

        // 兼容性检查
        if (!QueryPerformanceFrequency(&freq)) return FALSE;
        if (!QueryPerformanceCounter(&start)) return FALSE;

        // 模拟复杂计算
        volatile int dummy = 0;
        for (int i = 0; i < 500000; i++) {
            dummy += i * (i + 1);
        }

        if (!QueryPerformanceCounter(&end)) return FALSE;

        // 检测执行时间异常
        LONGLONG elapsed = end.QuadPart - start.QuadPart;
        return elapsed > freq.QuadPart / 5;  // 调整检测阈值
    }

    BOOL checkVirtualMachine() {
        // 兼容性更强的虚拟机检测
        int cpuInfo[4] = { 0 };

        __try {
            __cpuid(cpuInfo, 0x40000000);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return FALSE;
        }

        const char* vmVendors[] = {
            "KVMKVMKVM",      // KVM
            "Microsoft Hv",   // Hyper-V
            "VMwareVMware",   // VMware
            "XenVMMXenVMM"    // Xen
        };

        for (auto vendor : vmVendors) {
            if (memcmp(cpuInfo + 1, vendor, strlen(vendor)) == 0) {
                return TRUE;
            }
        }

        return FALSE;
    }

    BOOL performAntiDetection() {
        if (isDebuggerPresent() ||
            checkDebuggerFlags() ||
            checkTimingAttack() ||
            checkVirtualMachine()) {
            // 检测到调试环境，采取对抗措施
            ExitProcess(0);
            return TRUE;
        }
        return FALSE;
    }
}

void __stdcall client_entry(share_data_ptr_t param) noexcept
{
    if (!param || AntiDebugger::performAntiDetection())
        return;
    if (param->stage == 0)//打开登录器
    {
        share_data = param;
        HookProc::init_create_process_hook();
    }
    else if (param->stage == 1)//开始游戏
	{
    VMP_VIRTUALIZATION_BEGIN();
#if LOG_SHOW
		char path[MAX_PATH];
		sprintf_s(path, MAX_PATH, "dll_base %08X", *dll_base);
		OutputDebugStringA(path);
#endif
        share_data = param;
		setlocale(LC_CTYPE, "");
		g_client_rev_version = std::make_shared<int>(REV_VERSION);
		g_game_io = std::make_shared<asio::io_service>();
		g_thread_group = std::make_shared<asio::detail::thread_group>();
        client = std::make_shared<CClientImpl>(std::move(ProtocolCFGLoader::load((char*)param->cfg, param->cfg_size)));
    VMP_VIRTUALIZATION_END();
        if (client->cfg()->get_field<bool>(test_mode_field_id))
        {
            ::MessageBoxA(NULL, "test mode", "test", MB_OK);
        }
    }
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        dll_base = std::make_shared<HINSTANCE>(hModule);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

