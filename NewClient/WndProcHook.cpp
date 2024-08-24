#include "pch.h"
#include "WndProcHook.h"
#include <algorithm>

extern asio::io_service g_game_io;
namespace WndProcHook
{
    //asio::steady_timer restore_hook_timer(g_game_io);
    LightHook::ContextHook* set_window_long_hook = nullptr;
    WNDPROC old_wnd_proc = nullptr;
	std::set<uint32_t> main_hwnd;
    LRESULT WINAPI wnd_proc_wrapper(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        g_game_io.poll_one();
        return old_wnd_proc(hwnd, msg, wparam, lparam);
    }

    void install_hook()
    {
        if (set_window_long_hook == nullptr)
        {
            set_window_long_hook = new LightHook::ContextHook();
            set_window_long_hook->install(IMPORT(L"user32.dll", SetWindowLongA), [](LightHook::Context& ctx) {
                uintptr_t* param = (uintptr_t*)ctx.esp;
                if (param[2] == GWLP_WNDPROC)
                {
                    HWND hwnd = (HWND)param[1];
                    std::wstring class_name;
                    class_name.resize(200);
                    GetClassName(hwnd, class_name.data(), class_name.capacity());
                    transform(class_name.begin(), class_name.end(), class_name.begin(), ::towlower);
                    if (Utils::String::to_utf8(class_name) == L"tfrmmain")
                    {
						if ((old_wnd_proc && (WNDPROC)param[3] == old_wnd_proc) || main_hwnd.find((uint32_t)hwnd) != main_hwnd.end()) {
							return;
						}

                        old_wnd_proc = (WNDPROC)param[3];
						param[3] = (uintptr_t)&wnd_proc_wrapper;
						main_hwnd.emplace((uint32_t)hwnd);
					}
                }
            }, NULL);
            asio2::timer restore_hook_timer;
			restore_hook_timer.start_timer("restore_hook_timer", std::chrono::seconds(10), []() {
				if (set_window_long_hook != nullptr) {
					set_window_long_hook->restore();
					delete set_window_long_hook;
				}
				});
			/*restore_hook_timer.expires_after(std::chrono::seconds(10));
			restore_hook_timer.async_wait([](std::error_code ec) {
				if (set_window_long_hook != nullptr) {
					set_window_long_hook->restore();
					delete set_window_long_hook;
				}
				});*/
        }
    }
}