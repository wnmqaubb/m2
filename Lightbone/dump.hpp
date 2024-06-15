<<<<<<< HEAD
#pragma once
#include <singleton.hpp>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

class DumpService
{
public:
	DumpService()
	{
		SetUnhandledExceptionFilter(UnhandledExceptionFilterExImpl);
	}
protected:
private:
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
			MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, pException ? &ExInfo : NULL, NULL, NULL);
			CloseHandle(hDumpFile);
		}
	}
	static LONG WINAPI UnhandledExceptionFilterExImpl(struct _EXCEPTION_POINTERS *pException)
	{
		Singleton<DumpService>::getInstance().CreateDumpFile(TEXT("minidump.dmp"), pException);
		return EXCEPTION_EXECUTE_HANDLER;
	}
=======
#pragma once
#include <singleton.hpp>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

class DumpService
{
public:
	DumpService()
	{
		SetUnhandledExceptionFilter(UnhandledExceptionFilterExImpl);
	}
protected:
private:
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
			MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, pException ? &ExInfo : NULL, NULL, NULL);
			CloseHandle(hDumpFile);
		}
	}
	static LONG WINAPI UnhandledExceptionFilterExImpl(struct _EXCEPTION_POINTERS *pException)
	{
		Singleton<DumpService>::getInstance().CreateDumpFile(TEXT("minidump.dmp"), pException);
		return EXCEPTION_EXECUTE_HANDLER;
	}
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9
};