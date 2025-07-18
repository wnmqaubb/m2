#include "../pch.h"
#include <../../yk/Lightbone/utils.h>
#include "Service/AntiCheatClient.h"

extern std::shared_ptr<asio::io_service> g_game_io;
#define LLKHF_LOWER_IL_INJECTED 0x00000002
#define LLMHF_LOWER_IL_INJECTED 0x00000002
#pragma optimize("", off )
HHOOK MouseHook = 0;
HHOOK KeyboardHook = 0;

// Low level Mouse filter proc
LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION) {
		if (wParam == WM_RBUTTONDOWN || wParam == WM_LBUTTONDOWN) {
			MSLLHOOKSTRUCT* hookStruct = (MSLLHOOKSTRUCT*)lParam;

			if ((hookStruct->flags & LLMHF_INJECTED) == LLMHF_INJECTED)
				return TRUE;

			if ((hookStruct->flags & LLMHF_LOWER_IL_INJECTED) == LLMHF_LOWER_IL_INJECTED)
				return TRUE;

		}
	}
	return CallNextHookEx(MouseHook, nCode, wParam, lParam);
}

// Low level Keyboard filter proc
LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION) {
		if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
			KBDLLHOOKSTRUCT* hookStruct = (KBDLLHOOKSTRUCT*)lParam;

			if ((hookStruct->flags & LLKHF_INJECTED) == LLKHF_INJECTED)
				return TRUE;

			if ((hookStruct->flags & LLKHF_LOWER_IL_INJECTED) == LLKHF_LOWER_IL_INJECTED)
				return TRUE;

		}
	}

	return CallNextHookEx(KeyboardHook, nCode, wParam, lParam);
}

DWORD WINAPI AntiMacroEx(LPVOID)
{
	HINSTANCE hInstance = BetaModuleTable->hBaseModule;

	MouseHook = BetaFunctionTable->SetWindowsHookExA(WH_MOUSE_LL, MouseHookProc, hInstance, NULL);
	KeyboardHook = BetaFunctionTable->SetWindowsHookExA(WH_KEYBOARD_LL, KeyboardHookProc, hInstance, NULL);

	if (IsWindowsVistaOrGreater()) {
		if (!MouseHook)
			__dieForMacro(1);

		if (!KeyboardHook)
			__dieForMacro(2);
	}

	if (MouseHook || KeyboardHook) {
		MSG message;
		while (BetaFunctionTable->GetMessageA(&message, NULL, 0, 0)) {
			BetaFunctionTable->TranslateMessage(&message);
			BetaFunctionTable->DispatchMessageA(&message);
		}

		__dieForMacro(3);
	}

	BetaFunctionTable->UnhookWindowsHookEx(MouseHook);
	BetaFunctionTable->UnhookWindowsHookEx(KeyboardHook);
	return 0;
}

void report_show_window(CAntiCheatClient* client, bool is_cheat, const std::string& reason)
{
	VMP_VIRTUALIZATION_BEGIN();
	static uint32_t last_tick_count = NULL;
	if (GetTickCount() - last_tick_count >= 200)
	{
		ProtocolC2STaskEcho resp;
		resp.task_id = TASK_PKG_ID_SHOW_WINDOW_HOOK_DETECT;
		resp.is_cheat = true;
		resp.text = xorstr("·Ç·¨´°¿Ú:") + reason;
		client->send(&resp);
		last_tick_count = GetTickCount();
	}
	VMP_VIRTUALIZATION_END();
}

void InitMacroDetect(CAntiCheatClient* client)
{
	auto ShowWindow = IMPORT(L"user32.dll", ShowWindow);
	LightHook::HookMgr::instance().add_context_hook(ShowWindow, [](LightHook::Context& ctx) {
		uintptr_t* param = (uintptr_t*)ctx.esp;
		const uintptr_t return_address = param[0];
		const HWND hwnd = reinterpret_cast<HWND>(param[1]);
		const int nCmdShow = param[2];
		auto GetModuleHandleExW = IMPORT(L"kernel32.dll", GetModuleHandleExW);

		


		auto GetClassNameW = IMPORT(L"user32.dll", GetClassNameW);
		std::wstring class_name;
		class_name.resize(MAX_PATH);
		GetClassNameW(hwnd, (PWCHAR)class_name.data(), MAX_PATH);
		if (class_name[0] == '#')
		{
			return;
		}
		for (auto& ch : class_name)
		{
			if ('0' <= ch && ch <= '9')
			{
				report_show_window((CAntiCheatClient*)ctx.custom_param, true, Utils::String::w2c(class_name));
				break;
			}
		}
	}, client);


	
}