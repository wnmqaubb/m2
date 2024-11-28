#include "pch.h"
#include "WndProcHook.h"
#include <algorithm>

extern asio::io_service g_game_io;

namespace WndProcHook
{    
	LightHook::InlineHook read_file_hook;
	decltype(&::ReadFile) old_read_file_ptr = nullptr;

	BOOL WINAPI read_file_ptr(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
	{
		g_game_io.poll_one();
		return old_read_file_ptr(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
	}

	void install_hook()
	{
		read_file_hook.install(&::ReadFile, &read_file_ptr, &old_read_file_ptr);
	}

	void restore_hook()
	{
		read_file_hook.restore();
	}
}