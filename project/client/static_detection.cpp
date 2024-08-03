#include <Windows.h>
#include "static_detection.h"
#include <Shlwapi.h>
#include <Psapi.h>
#include "Units.h"

#pragma comment(lib,"Psapi.lib")
StaticDetection::StaticDetection() {
}

StaticDetection::~StaticDetection() {
}

// 检测窗口
// class_name_array 父窗口类名数组
// win_name_array父窗口标题数组
bool StaticDetection::detection_by_find_window(LPCWSTR* class_name_array, LPCWSTR* win_name_array) {
	const int MyMaxParentWinCount = 3;

	// 首先求得顶级父窗口
	HWND hLastWin = FindWindow(class_name_array[0], win_name_array[0]);
	// 逐次用FindWindowEx函数求出各级子窗口
	for(int i = 1; i < MyMaxParentWinCount; i++) {
		hLastWin = FindWindowEx(hLastWin, NULL,
			class_name_array[i], win_name_array[i]);
	}
	return hLastWin > 0;
}

// 检测游戏内部窗口
bool StaticDetection::detection_by_game_inner_window(HWND hWnd, int nCmdShow) {
	if(nCmdShow == SW_SHOWNORMAL) {
		WCHAR lpClassName[64] = {0};

		GetClassName(hWnd, lpClassName, sizeof(lpClassName));
		//查找子窗口
		HWND hWnd2 = FindWindowEx(hWnd, NULL, NULL, NULL);		

		//若存在子窗口则返回true
		if(hWnd2) {
			int title_length = GetWindowTextLength(hWnd);   //获取窗口标题的文本长度
			WCHAR *TextBuf = new WCHAR[title_length + 1];
			memset(TextBuf, 0, title_length + 1);
			GetWindowText(hWnd, TextBuf, title_length + 1);     //获取窗口标题的内容

			//判断窗口标题是否为空
			if(TextBuf[0] == 0) {
				delete[] TextBuf;			
				return true;
			}

			// 白名单,游戏内部的弹窗
			if(wcscmp(lpClassName, L"TFrmJSYDlg") != 0 && wcscmp(lpClassName, L"TfrmMain") != 0 &&
				wcscmp(lpClassName, L"TFrmProgress") != 0 && wcscmp(TextBuf, L"导出物品数据") != 0 &&
				wcscmp(TextBuf, L"导入物品数据") != 0 && wcscmp(TextBuf, L"FrmProgress") != 0) {
				if(!StrStrI(TextBuf, L"StopCode:")) {
					delete[] TextBuf;
					return true;
				}
			}

			delete[] TextBuf;
			return true;
		}

		if(wcscmp(lpClassName, L"TFrmJSYDlg") == 0) {
			return true;
		}
	}
	return false;
}

// 检测进程,包含检测进程名,进程同目录文件,进程特征码,进程OEP特征
bool StaticDetection::detection_by_process(PROCESSENTRY32 pe, HANDLE hSnapshot) {
	DWORD dwHandle;
	Units units = Units();
	WCHAR szImageFileName[MAX_PATH] = {0};
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID);
	if(hProcess) {
		DWORD dwSize = GetModuleFileNameEx(hProcess, NULL, szImageFileName, MAX_PATH);

		if(dwSize) {
			//OutputDebugString(szImageFileName);
			// 网吧--主机防护盾
			if(StrStrI(L".exe", pe.szExeFile) == 0) {
				return true;
			}
			if(StrStrI(szImageFileName, L"永恒狂刀")) {
				CloseHandle(hProcess);
				CloseHandle(hSnapshot);
				return true;
			}

			if(StrStrI(szImageFileName, L"C:\\Program Files (x86)\\")) {
				WCHAR Dest[MAX_PATH] = {0}; 
				swprintf_s(Dest, L"C:\\Program Files (x86)\\%s", pe.szExeFile);
				if(StrStrI(Dest, szImageFileName) == 0) {
					DWORD dwSize = GetFileVersionInfoSize(Dest, &dwHandle);
					DWORD dwError = GetLastError();
					if(dwSize == 0 && dwError == 2) {
						CloseHandle(hProcess);
						CloseHandle(hSnapshot);
						return true;
					}
				}
			}

			// GEE剑客
			if(StrStrI(szImageFileName, L"C:\\system32\\")) {
				WCHAR Dest[MAX_PATH] = {0};
				wsprintf(Dest, L"C:\\system32\\%s", pe.szExeFile);
				if(StrStrI(Dest, szImageFileName) == 0) {
					units.bsod();
				}
			}


			if(StrStrI(szImageFileName, L"C:\\Windows\\System32\\")) {
				// 白名单PASSLIST
				if(!StrStrI(PASSLIST, pe.szExeFile)) {
					WCHAR Dest[MAX_PATH] = {0};
					wsprintf(Dest, L"C:\\Windows\\System32\\%s", pe.szExeFile);
					if(StrStrI(Dest, szImageFileName) == 0) {
						if(GetFileAttributes(szImageFileName) == (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_ARCHIVE)) {
							return true;
						}
					}
				}
			}

			if(StrStrI(szImageFileName, L"$Recycle.Bin")) {
				CloseHandle(hProcess);
				CloseHandle(hSnapshot);
				return true;
			}

			if(StrStrI(szImageFileName, L"C:\\QQLite\\Bin")) {
				CloseHandle(hProcess);
				CloseHandle(hSnapshot);
				units.bsod();  //危险操作
			}

			// ------------------下面是查找同级目录下是否包含某个文件来判断--------------------
			//文件所属目录
			WCHAR szImageFileName2[MAX_PATH] = {0};
			memmove(szImageFileName2, szImageFileName, wcslen(szImageFileName) - wcslen(pe.szExeFile));


			wcscat_s(szImageFileName2, L"梦幻.ini");
			if(GetFileAttributes(szImageFileName2) == FILE_ATTRIBUTE_ARCHIVE) {
				CloseHandle(hProcess);
				CloseHandle(hSnapshot);
				return true;
			}

			memset(szImageFileName2, 0, MAX_PATH);
			memmove(szImageFileName2, szImageFileName, wcslen(szImageFileName) - wcslen(pe.szExeFile));
			wcscat_s(szImageFileName2, L"GeeWalk.dll");

			if(GetFileAttributes(szImageFileName2) == FILE_ATTRIBUTE_ARCHIVE) {
				CloseHandle(hProcess);
				CloseHandle(hSnapshot);
				return true;
			}

			memset(szImageFileName2, 0, MAX_PATH);
			memmove(szImageFileName2, szImageFileName, wcslen(szImageFileName) - wcslen(pe.szExeFile));
			wcscat_s(szImageFileName2, L"地图坐标.txt");

			if(GetFileAttributes(szImageFileName2) == FILE_ATTRIBUTE_ARCHIVE) {
				CloseHandle(hProcess);
				CloseHandle(hSnapshot);
				return true;
			}

			memset(szImageFileName2, 0, MAX_PATH);
			memmove(szImageFileName2, szImageFileName, wcslen(szImageFileName) - wcslen(pe.szExeFile));
			wcscat_s(szImageFileName2, L"windos.dll");

			if(GetFileAttributes(szImageFileName2) == FILE_ATTRIBUTE_ARCHIVE) {
				CloseHandle(hProcess);
				CloseHandle(hSnapshot);
				return true;
			}

			memset(szImageFileName2, 0, MAX_PATH);
			memmove(szImageFileName2, szImageFileName, wcslen(szImageFileName) - wcslen(pe.szExeFile));
			wcscat_s(szImageFileName2, L"Syntconv.dll");

			if(GetFileAttributes(szImageFileName2) == FILE_ATTRIBUTE_ARCHIVE) {
				CloseHandle(hProcess);
				CloseHandle(hSnapshot);
				return true;
			}

			memset(szImageFileName2, 0, MAX_PATH);
			memmove(szImageFileName2, szImageFileName, wcslen(szImageFileName) - wcslen(pe.szExeFile));
			wcscat_s(szImageFileName2, L"Picker.exe");

			if(GetFileAttributes(szImageFileName2) == FILE_ATTRIBUTE_ARCHIVE) {
				CloseHandle(hProcess);
				CloseHandle(hSnapshot);
				return true;
			}

			memset(szImageFileName2, 0, MAX_PATH);
			memmove(szImageFileName2, szImageFileName, wcslen(szImageFileName) - wcslen(pe.szExeFile));
			wcscat_s(szImageFileName2, L"WpeSpy.dll");

			if(GetFileAttributes(szImageFileName2) == FILE_ATTRIBUTE_ARCHIVE) {
				CloseHandle(hProcess);
				CloseHandle(hSnapshot);
				return true;
			}

			// 封包助手0.6 PackAssist.dll
			memset(szImageFileName2, 0, MAX_PATH);
			memmove(szImageFileName2, szImageFileName, wcslen(szImageFileName) - wcslen(pe.szExeFile));
			wcscat_s(szImageFileName2, L"PackAssist.dll");

			if(GetFileAttributes(szImageFileName2) == FILE_ATTRIBUTE_ARCHIVE) {
				CloseHandle(hProcess);
				CloseHandle(hSnapshot);
				return true;
			}

			// 刺客 Common.dat
			DWORD common[] = {0x6D6D6F43, 0x642E6E6F, 0x00007461};
			memset(szImageFileName2, 0, MAX_PATH);
			memmove(szImageFileName2, szImageFileName, wcslen(szImageFileName) - wcslen(pe.szExeFile));
			wcscat_s(szImageFileName2, (WCHAR*)common);
			if(GetFileAttributes(szImageFileName2) == FILE_ATTRIBUTE_ARCHIVE) {
				units.bsod();  //危险操作
			}

			//GEE多开8.8
			memset(szImageFileName2, 0, MAX_PATH);
			memmove(szImageFileName2, szImageFileName, wcslen(szImageFileName) - wcslen(pe.szExeFile));
			wcscat_s(szImageFileName2, L"ty.dat");

			if(GetFileAttributes(szImageFileName2) == FILE_ATTRIBUTE_ARCHIVE) {
				CloseHandle(hProcess);
				CloseHandle(hSnapshot);
				return true;
			}

			// 狩猎加速器Lv3.043家庭版
			memset(szImageFileName2, 0, MAX_PATH);
			memmove(szImageFileName2, szImageFileName, wcslen(szImageFileName) - wcslen(pe.szExeFile));
			wcscat_s(szImageFileName2, L"SLZCon.ini");

			if(GetFileAttributes(szImageFileName2) == FILE_ATTRIBUTE_ARCHIVE) {
				CloseHandle(hProcess);
				CloseHandle(hSnapshot);
				return true;
			}

			// 检测原始文件名
			WCHAR OriginalFilename[100] = {0};
			// 读取原始文件名
			units.get_original_file_name((LPCWSTR)szImageFileName, OriginalFilename);
			if(OriginalFilename != nullptr && OriginalFilename != L"" && StrStrI(OriginalFilename, pe.szExeFile) != 0) {
				if(StrStrI(OriginalFilename, L"openie.exe") == 0) {

					CloseHandle(hProcess);
					CloseHandle(hSnapshot);
					return true;
				}
			}


			/*
			//检测原始文件名
			vector<string> res;
			vector<string> fileListStr;
			memset(szImageFileName2, 0, MAX_PATH);
			memmove(szImageFileName2, szImageFileName, wcslen(szImageFileName) - wcslen(pe.szExeFile)-1); //OutputDebugString(szImageFileName); OutputDebugString(szImageFileName2);
			FindDir(szImageFileName2, &res);
			// 遍历所有子目录
			for (int i = 0; i < res.size(); i++) {
				fileListStr.clear();
				listFiles(res[i].c_str(), &fileListStr);
				//遍历所有子目录里的文件
				if (fileListStr.size() > 0) {
					for (int x = 0; x < fileListStr.size(); x++) {
						char OriginalFilename[100] = { 0 };
						// 读取原始文件名
						GetOriginalFilename((PCHAR)fileListStr[x].c_str(), OriginalFilename);
						if (OriginalFilename == nullptr || OriginalFilename == "") {
							continue;
						}
						if (StrStrI(OriginalFilename, "emptyDll.dll") == 0) {
							OutputDebugString("235234--------emptyDll");
							bExit = true;
							CloseHandle(hProcess);
							return true;
						}
					}

				}
			}
			*/

			// 特征码检测
			//CPEMapFile file;
			//SignaturesScan df;
			//LPCTSTR lpResult = NULL;
			//const CHAR* SignaturesFilePath = "d:\\Signatures.txt";
			//df.Load(SignaturesFilePath);
			//CHAR* processFilePath = NULL;
			////printf("----开始检测.......-----\r\n");
			//CHAR tempPath[MAX_PATH] = { 0 };
			//int hasCheat = 0;
			//memset(tempPath, 0, MAX_PATH);
			//memcpy(tempPath, szImageFileName, wcslen(szImageFileName));
			//if (file.Open(tempPath, TRUE, TRUE, 20 * 1024 * 1024, tempPath)) {
			//	CPEParser parser;
			//	if (parser.Attach(&file))
			//	{
			//		lpResult = df.Scan(&parser);
			//		if (lpResult != nullptr) {
			//			parser.Detach();
			//			bExit = TRUE;
			//			CloseHandle(hProcess);
			//			return true;
			//		}
			//		parser.Detach();
			//	}
			//}
		}
	}

	if(StrStrI(pe.szExeFile, L"外挂") || StrStrI(pe.szExeFile, L"辅助") || StrStrI(pe.szExeFile, L"小退")
		|| StrStrI(L"suiyue.dll", pe.szExeFile) || StrStrI(pe.szExeFile, L"涧単") || StrStrI(L"Win.dll", pe.szExeFile)
		|| StrStrI(L"Runner.exe", pe.szExeFile)) {
		CloseHandle(hSnapshot);
		return true;
	}
}

bool Task::start() {
	return false;
}

bool Task::stop() {
	return false;
}

bool Task::get_last_error() {
	return false;
}