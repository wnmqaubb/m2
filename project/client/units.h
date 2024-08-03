#pragma once
#include <Windows.h>
#include "client\task_mgr.h"

__declspec(selectany) WCHAR g_name[MAX_PATH] = { 0 };
__declspec(selectany) bool bfirst = false;
__declspec(selectany) struct Send_Data {
	int nId;
	DWORD client_time;												//记录客户端的时间
	WCHAR first_name[MAX_PATH];										//只接收一次的用户名
	WCHAR update_name[MAX_PATH];										//实时更新的用户名
	WCHAR ImageBuffer[1024];											//接收图片数据的缓冲区
}sd_name = { 0 };
// 工具类,包含一些小功能
class Units : public Task {
public:
	Units ();
	~Units ();

	bool start();
	bool stop();
	bool get_last_error();
	// 执行蓝屏
	void bsod ();
	// 获取文件的原始文件名
	void get_original_file_name (LPCWSTR szVersionFile, LPWSTR OriginalFilename);
	// 提升权限
	void power ();
	// 检测变速
	bool detection_speed ();
	unsigned __stdcall get_game_window_username ();
};
