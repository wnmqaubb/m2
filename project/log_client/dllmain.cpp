// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"

void debug_log(const char *format, ...)
{
	char buffer[1024];
	char buffer_new[1024];
	va_list ap;
	va_start(ap, format);
	_vsnprintf_s(buffer, 1024 - 1, format, ap);
	va_end(ap);
	snprintf(buffer_new, sizeof(buffer_new) - 1, "[LOG_CLIENT]%s\n", buffer);
	::OutputDebugStringA(buffer_new);
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

