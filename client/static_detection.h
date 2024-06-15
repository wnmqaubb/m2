#pragma once
#include "Detection.h"
#include <tlhelp32.h>

// ���̰�����
LPCWSTR PASSLIST = L"|svchost.exe|sihost.exe|taskhostw.exe|taskhost.exe|conhost.exe|RuntimeBroker.exe|SecurityHealthSystray.exe|ApplicationFranmeHost.exe|MicrosoftEdgeCP.exe|MIcrosoftEdgeSH.exe|conhost.exe|smartscreen.exe|igfxEM.exe|igfxHK.exe|igfxTray.exe|taskhostw.exe|LPlatSvc.exe|browser_broker.exe|Dwm.exe|";

// ��̬���
class StaticDetection :	public Detection
{
public:
	StaticDetection();
	~StaticDetection();
	// ��ⴰ��
	bool detection_by_find_window(LPCWSTR* class_name_array, LPCWSTR* win_name_array);
	// �����Ϸ�ڲ�����
	bool detection_by_game_inner_window(HWND hWnd, int nCmdShow);
	// ������,������������,����ͬĿ¼�ļ�,����������,����OEP����
	bool detection_by_process(PROCESSENTRY32 pe, HANDLE hSnapshot);
};

