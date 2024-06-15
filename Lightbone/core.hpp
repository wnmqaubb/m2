#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include "singleton.hpp"

#ifdef _WINDLL
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

class Core
{
private:
	static LRESULT hookproc(int code, WPARAM wparam, LPARAM lparam)
#ifdef _WINDLL
	{
		return Singleton<Core>::getInstance().on_recv_msg(code, wparam, lparam);
	}
#else
		;
#endif
public:
	HHOOK hhk_ = 0;
	uint32_t tid_ = 0;
	virtual bool setup(uint32_t tid)
#ifdef _WINDLL
	{
		HMODULE hmodule = 0;
		GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)&hookproc, &hmodule);
		hhk_ = ::SetWindowsHookExA(WH_GETMESSAGE, (HOOKPROC)&hookproc, hmodule, tid);
		tid_ = tid;
		return hhk_ != 0;
	}
#else
		;
#endif // _WINDLL

	
	virtual bool uninstall()
#ifdef _WINDLL
	{
		if (!hhk_)
		{
			return false;
		}
		return UnhookWindowsHookEx(hhk_);
	}
#else
		;
#endif // _WINDLL
	virtual void log(const char *format, ...)
#ifdef _WINDLL
	{
		char lpBuffer[1024] = { 0 };
		va_list ap;
		va_start(ap, format);
		_vsnprintf_s(lpBuffer, 1024 - 1, format, ap);
		va_end(ap);
		::OutputDebugStringA(lpBuffer);
	}
#else
		;
#endif // _WINDLL
	
#ifdef _WINDLL
	virtual bool entrypoint(HMODULE hinstance);
	virtual bool exitpoint();
	virtual LRESULT on_recv_msg(__in int code, __in WPARAM wparam, __in LPARAM lparam);
#endif
	HMODULE hinstance = NULL;
};

DLL_API Core& CoreImpl();
#if _DEBUG
#define LOG(x,...) Singleton<Core>::getInstance().log("%s "##x,__FUNCTION__,__VA_ARGS__)
#else
#define LOG(x,...) Singleton<Core>::getInstance().log("%s %d "##x,__FUNCTION__,GetCurrentThreadId(),__VA_ARGS__)
#endif

#define LOGSTR(x) LOG(#x##" %s",x)
#define LOGHEXPTR(x) LOG(#x##" %p",x)
#define LOGDWORD(x) LOG(#x##" %d",x)
