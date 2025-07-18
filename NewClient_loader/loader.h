#pragma once
#include <lf_rungate_server_plug/lf_plug_sdk.h>

#ifndef LOADER_EXPORTS
#define LOADER_EXPORTS

#ifdef LOADER_EXPORTS
#define LOADER_API __declspec(dllexport)
#else
#define LOADER_API __declspec(dllimport)
#endif
using namespace lfengine::client;

extern "C" {
    LOADER_API void __stdcall Init(const PAppFuncDef AppFunc);
    LOADER_API void __stdcall HookRecv(lfengine::PTDefaultMessage defMsg, char* lpData, int dataLen);
    LOADER_API void __stdcall DoUnInit();
    __declspec(dllimport) void __stdcall InitEx(const PAppFuncDef AppFunc, const char* server_ip);
}

#endif