#pragma once
#include <string>
#include <atomic>
#include "../lf_rungate_server_plug/lf_plug_sdk.h"
using namespace lfengine::client;
class AntiCheatSystem {
public:
    AntiCheatSystem();
    ~AntiCheatSystem();

    void Init(const PAppFuncDef AppFunc, const char* server_ip);
    void UnInit();
    void client_entry();

private:
    std::atomic_bool m_running;
    std::string m_server_ip;
};

// 导出函数声明
extern "C" __declspec(dllexport) void __stdcall InitEx(const PAppFuncDef AppFunc, const char* server_ip, const wchar_t* mutex_anti_cheat_module_name);
extern "C" __declspec(dllexport) void __stdcall UnInitEx();