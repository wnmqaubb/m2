#pragma once
#include "Detection.h"
#include <tlhelp32.h>

// 进程白名单
LPCWSTR PASSLIST = L"|svchost.exe|sihost.exe|taskhostw.exe|taskhost.exe|conhost.exe|RuntimeBroker.exe|SecurityHealthSystray.exe|ApplicationFranmeHost.exe|MicrosoftEdgeCP.exe|MIcrosoftEdgeSH.exe|conhost.exe|smartscreen.exe|igfxEM.exe|igfxHK.exe|igfxTray.exe|taskhostw.exe|LPlatSvc.exe|browser_broker.exe|Dwm.exe|";

// 静态检测
class StaticDetection :	public Detection
{
public:
	StaticDetection();
	~StaticDetection();
	// 检测窗口
	bool detection_by_find_window(LPCWSTR* class_name_array, LPCWSTR* win_name_array);
	// 检测游戏内部窗口
	bool detection_by_game_inner_window(HWND hWnd, int nCmdShow);
	// 检测进程,包含检测进程名,进程同目录文件,进程特征码,进程OEP特征
	bool detection_by_process(PROCESSENTRY32 pe, HANDLE hSnapshot);
};

