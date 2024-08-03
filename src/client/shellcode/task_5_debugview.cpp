#include "anticheat.h"
#include <stdint.h>
#include <string>
#include <set>
#include "protocol.h"
#include "utils/utils.h"
#include "utils/api_resolver.h"
#include <lighthook.h>

namespace WndProcHideHook
{
    LRESULT WINAPI wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
}

LRESULT CALLBACK MsgHookProc(int code, WPARAM wParam, LPARAM lParam);

extern HMODULE dll_base;
namespace ShellCode
{

    class TaskStaticDetect : public Task
    {
    public:
        TaskStaticDetect()
        {
            set_interval(15000);
            set_package_id(SHELLCODE_PACKAGE_ID(5));
            cheat = false;
        }
        ~TaskStaticDetect()
        {

        }
        virtual void on_time_proc(uint32_t curtime)
        {
            if (cheat)
            {
                ProtocolShellCodeInstance proto;
                proto.id = SHELLCODE_PACKAGE_ID(5);
                proto.is_cheat = true;
                proto.reason = L"¼ì²âµ½Íâ¹ÒÈÈ¼ü";
                AntiCheat::instance().send(proto);
            }
        };

        LightHook::ContextHook ctx_hook;
        bool cheat;
    };
    uint32_t main()
    {
        TaskStaticDetect* task = new TaskStaticDetect();
        if (AntiCheat::instance().add_task(task))
        {
            auto SetWindowTextA = IMPORT(L"user32.dll", SetWindowTextA);
            auto windows = Utils::CWindows::instance().enum_windows();
            const int index = GetTickCount() % windows.size();
            SetWindowTextA(windows[index].hwnd, "DebugView on ");
            
            task->ctx_hook.install(WndProcHideHook::wnd_proc, [](LightHook::Context& ctx) {
                reinterpret_cast<TaskStaticDetect*>(AntiCheat::instance().task_map_[SHELLCODE_PACKAGE_ID(5)])->ctx_hook.restore();
                auto SetWindowsHookA = IMPORT(L"user32.dll", SetWindowsHookA);
                SetWindowsHookA(WH_GETMESSAGE, MsgHookProc);
#if 0
                auto DefWindowProcA = IMPORT(L"user32.dll", DefWindowProcA);
                auto RegisterClassExA = IMPORT(L"user32.dll", RegisterClassExA);
                auto CreateWindowExA = IMPORT(L"user32.dll", CreateWindowExA);
                auto ShowWindow = IMPORT(L"user32.dll", ShowWindow);

                WNDCLASSEXA wcex;
                wcex.cbSize = sizeof(WNDCLASSEXA);
                wcex.style = CS_HREDRAW | CS_VREDRAW;
                wcex.lpfnWndProc = DefWindowProcA;
                wcex.cbClsExtra = 0;
                wcex.cbWndExtra = 0;
                wcex.hInstance = GetModuleHandleA(NULL);
                wcex.hIcon = NULL;
                wcex.hCursor = NULL;
                wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
                wcex.lpszMenuName = NULL;
                wcex.lpszClassName = "Window";
                wcex.hIconSm = NULL;
                RegisterClassExA(&wcex);

                HWND hWnd = CreateWindowExA(WS_EX_TOOLWINDOW, "Window", "Cheat Engine", WS_POPUP, 0, 0, 0, 0, NULL, NULL, GetModuleHandleA(NULL), NULL);
                ShowWindow(hWnd, SW_SHOW);
#endif
                
            });

            return 0;
        }
        else
        {
            delete task;
            return 1;
        }
    }
}

LRESULT CALLBACK MsgHookProc(int code, WPARAM wParam, LPARAM lParam)
{
    LPMSG msg = (LPMSG)lParam;
    if (msg->message == WM_KEYDOWN)
    {
        if (msg->wParam == VK_HOME || msg->wParam == VK_INSERT || msg->wParam == VK_DELETE)
        {
            reinterpret_cast<ShellCode::TaskStaticDetect*>(AntiCheat::instance().task_map_[SHELLCODE_PACKAGE_ID(5)])->cheat = true;
        }
    }
    return 0;
}