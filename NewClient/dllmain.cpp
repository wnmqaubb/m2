// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "ClientImpl.h"
#include "Tools/Packer/loader.h"
#include "CreateProcessHook.h"
#include "version.build"
#include <Lightbone/api_resolver.cpp>
#include <Lightbone/windows_internal.h>
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
    // 检查是否支持高精度计时器
    BOOL IsHighResolutionTimerSupported() {
        LARGE_INTEGER freq;
        return QueryPerformanceFrequency(&freq);
    }

    // 改进的 PEB 读取方式
    BOOL IsDebuggerPresentByPEB() {
        __try {
            // 使用 NtQueryInformationProcess 获取 PEB 地址
            _PROCESS_BASIC_INFORMATION_T<uint32_t> pbi = { 0 };
            NTSTATUS status = Utils::CWindows::instance().query_information_process<uint32_t>(
                GetCurrentProcess(),
                ProcessBasicInformation,
                &pbi,
                sizeof(pbi),
                nullptr
            );

            if (status >= 0 && pbi.PebBaseAddress) {
                _PMPEB pPeb = reinterpret_cast<_PMPEB>(pbi.PebBaseAddress);
                return pPeb->bBeingDebugged;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return FALSE;
        }
        return FALSE;
    }

    // 改进的调试器检测
    BOOL isDebuggerPresent() {
        // 标准方法
        if (IsDebuggerPresent()) return TRUE;

        // 改进的 PEB 检测
        if (IsDebuggerPresentByPEB()) return TRUE;

        return FALSE;
    }

    // 改进的虚拟机检测
    BOOL checkVirtualMachine() {
        __try {
            int cpuInfo[4] = { 0 };

            // 检查 CPU 是否支持 CPUID 指令
            __cpuid(cpuInfo, 0);

            if (cpuInfo[0] >= 0x40000000) {
                __cpuid(cpuInfo, 0x40000000);

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
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return FALSE;
        }
        return FALSE;
    }

    // 改进的时间攻击检测
    BOOL checkTimingAttack() {
        if (!IsHighResolutionTimerSupported()) {
            return FALSE; // 不支持高精度计时器，跳过检测
        }

        LARGE_INTEGER start = { 0 }, end = { 0 }, freq = { 0 };
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&start);

        // 模拟复杂计算
        volatile int dummy = 0;
        for (int i = 0; i < 500000; i++) {
            dummy += i * (i + 1);
        }

        QueryPerformanceCounter(&end);

        // 检测执行时间异常
        LONGLONG elapsed = end.QuadPart - start.QuadPart;
        return elapsed > freq.QuadPart / 5;  // 调整检测阈值
    }

    // 改进的反检测逻辑
    BOOL performAntiDetection() {
        __try {
            if (isDebuggerPresent() || checkVirtualMachine() || checkTimingAttack()) {
                // 检测到调试环境，采取对抗措施
                ExitProcess(0);
                return TRUE;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return FALSE;
        }
        return FALSE;
    }
}

void __stdcall client_entry(share_data_ptr_t param) noexcept
{
    if (!param)
        return;
#ifndef _DEBUG
    if (AntiDebugger::performAntiDetection()) {
        OutputDebugStringA("AntiDebugger::performAntiDetection");
        return;
    }
#endif
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

