#pragma once
#include <Windows.h>
#include "client\task_mgr.h"

__declspec(selectany) WCHAR g_name[MAX_PATH] = { 0 };
__declspec(selectany) bool bfirst = false;
__declspec(selectany) struct Send_Data {
	int nId;
	DWORD client_time;												//��¼�ͻ��˵�ʱ��
	WCHAR first_name[MAX_PATH];										//ֻ����һ�ε��û���
	WCHAR update_name[MAX_PATH];										//ʵʱ���µ��û���
	WCHAR ImageBuffer[1024];											//����ͼƬ���ݵĻ�����
}sd_name = { 0 };
// ������,����һЩС����
class Units : public Task {
public:
	Units ();
	~Units ();

	bool start();
	bool stop();
	bool get_last_error();
	// ִ������
	void bsod ();
	// ��ȡ�ļ���ԭʼ�ļ���
	void get_original_file_name (LPCWSTR szVersionFile, LPWSTR OriginalFilename);
	// ����Ȩ��
	void power ();
	// ������
	bool detection_speed ();
	unsigned __stdcall get_game_window_username ();
};
