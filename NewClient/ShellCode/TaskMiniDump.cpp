#include "../pch.h"
#if 0
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")


void CreateDumpFile(LPCTSTR lpstrDumpFilePathName, EXCEPTION_POINTERS *pException)
{
    HANDLE hDumpFile = CreateFile(lpstrDumpFilePathName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (INVALID_HANDLE_VALUE != hDumpFile)
    {
        MINIDUMP_EXCEPTION_INFORMATION ExInfo;
        if (pException)
        {
            ExInfo.ThreadId = ::GetCurrentThreadId();
            ExInfo.ExceptionPointers = pException;
            ExInfo.ClientPointers = TRUE;
        }
        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpValidTypeFlags, pException ? &ExInfo : NULL, NULL, NULL);
        CloseHandle(hDumpFile);
    }
}

LONG WINAPI UnhandledExceptionFilterExImpl(struct _EXCEPTION_POINTERS *pException)
{
    CreateDumpFile(TEXT("./cache/minidump.dmp"), pException);
    return EXCEPTION_EXECUTE_HANDLER;
}

void InitMiniDump()
{
    SetUnhandledExceptionFilter(UnhandledExceptionFilterExImpl);
}
#endif


#include <client/windows/crash_generation/client_info.h>
#include <client/windows/crash_generation/crash_generation_server.h>
#include <client/windows/handler/exception_handler.h>
#include <client/windows/common/ipc_protocol.h>
using namespace google_breakpad;

bool ShowDumpResults(const wchar_t* dump_path,
    const wchar_t* minidump_id,
    void* context,
    EXCEPTION_POINTERS* exinfo,
    MDRawAssertionInfo* assertion,
    bool succeeded)
{
    MessageBoxA(NULL, "程序可能被劫持，若频繁出现，请使用360急救箱或重装系统", "提示", MB_OK | MB_ICONERROR);
    return succeeded;
}

void InitMiniDump()
{
	static auto handler = new ExceptionHandler(L".\\cache\\",
		NULL,
		ShowDumpResults,
		NULL,
		ExceptionHandler::HANDLER_ALL,
		MiniDumpValidTypeFlags,
		(HANDLE)NULL,
		NULL);
}