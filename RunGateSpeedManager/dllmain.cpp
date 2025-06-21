// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <windows.h>
#include <string>
#include <vector>
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")

// 窗口过程
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// 导出函数：创建并显示窗体
extern "C" __declspec(dllexport) void ShowGlobalDelayWindow();

struct ControlInfo {
    int type;       // 1=Static, 2=Edit, 3=Button, 4=CheckBox, 5=GroupBox
    int x, y, w, h;
    std::wstring text;
    DWORD style;
    int id;
    DWORD exStyle;
};

void CreateControls(HWND hWnd) {
    std::vector<ControlInfo> controls = {
        // 分组框
        {5, 15, 15, 250, 200, L"基本设置", WS_VISIBLE | BS_GROUPBOX, 0, 0},
        {5, 275, 15, 250, 200, L"高级设置", WS_VISIBLE | BS_GROUPBOX, 0, 0},
        {5, 15, 225, 250, 170, L"野蛮优化", WS_VISIBLE | BS_GROUPBOX, 0, 0},
        {5, 275, 225, 250, 170, L"锁一键二技能", WS_VISIBLE | BS_GROUPBOX, 0, 0},
        
        // 复选框
        {4, 30, 40, 100, 20, L"开启补偿", WS_VISIBLE | BS_AUTOCHECKBOX, 100, 0},
        {4, 30, 65, 100, 20, L"补充设置", WS_VISIBLE | BS_AUTOCHECKBOX, 101, 0},
        {4, 290, 40, 100, 20, L"野蛮优化", WS_VISIBLE | BS_AUTOCHECKBOX, 102, 0},
        {4, 290, 245, 100, 20, L"开启", WS_VISIBLE | BS_AUTOCHECKBOX, 103, 0},
        
        // 按钮
        {3, 30, 365, 100, 30, L"读取M2配置", WS_VISIBLE, 200, 0},
        {3, 150, 365, 100, 30, L"保存", WS_VISIBLE, 201, 0},
        
        // 标签 - 基本设置
        {1, 40, 100, 80, 20, L"移动速度", WS_VISIBLE, 0, 0},
        {1, 40, 130, 80, 20, L"移动速度", WS_VISIBLE, 0, 0},
        {1, 40, 160, 80, 20, L"移动速度", WS_VISIBLE, 0, 0},
        {1, 170, 100, 80, 20, L"攻击速度", WS_VISIBLE, 0, 0},
        {1, 170, 130, 80, 20, L"魔法速度", WS_VISIBLE, 0, 0},
        {1, 170, 160, 80, 20, L"动作间隔", WS_VISIBLE, 0, 0},
        {1, 170, 190, 80, 20, L"野蛮移动间隔", WS_VISIBLE, 0, 0},
        
        // 编辑框 - 基本设置
        {2, 120, 100, 40, 22, L"470", WS_VISIBLE | WS_BORDER | ES_NUMBER, 300, WS_EX_CLIENTEDGE},
        {2, 120, 130, 40, 22, L"470", WS_VISIBLE | WS_BORDER | ES_NUMBER, 301, WS_EX_CLIENTEDGE},
        {2, 120, 160, 40, 22, L"365", WS_VISIBLE | WS_BORDER | ES_NUMBER, 302, WS_EX_CLIENTEDGE},
        {2, 250, 100, 40, 22, L"220", WS_VISIBLE | WS_BORDER | ES_NUMBER, 303, WS_EX_CLIENTEDGE},
        {2, 250, 130, 40, 22, L"100", WS_VISIBLE | WS_BORDER | ES_NUMBER, 304, WS_EX_CLIENTEDGE},
        {2, 250, 160, 40, 22, L"500", WS_VISIBLE | WS_BORDER | ES_NUMBER, 305, WS_EX_CLIENTEDGE},
        {2, 250, 190, 40, 22, L"200", WS_VISIBLE | WS_BORDER | ES_NUMBER, 306, WS_EX_CLIENTEDGE},
        
        // 标签 - 高级设置
        {1, 290, 100, 80, 20, L"移动间隔", WS_VISIBLE, 0, 0},
        {1, 290, 130, 80, 20, L"移动间隔", WS_VISIBLE, 0, 0},
        {1, 290, 160, 80, 20, L"攻击间隔", WS_VISIBLE, 0, 0},
        {1, 290, 190, 80, 20, L"魔法间隔", WS_VISIBLE, 0, 0},
        
        // 编辑框 - 高级设置
        {2, 370, 100, 40, 22, L"15", WS_VISIBLE | WS_BORDER | ES_NUMBER, 310, WS_EX_CLIENTEDGE},
        {2, 370, 130, 40, 22, L"15", WS_VISIBLE | WS_BORDER | ES_NUMBER, 311, WS_EX_CLIENTEDGE},
        {2, 370, 160, 40, 22, L"3", WS_VISIBLE | WS_BORDER | ES_NUMBER, 312, WS_EX_CLIENTEDGE},
        {2, 370, 190, 40, 22, L"15", WS_VISIBLE | WS_BORDER | ES_NUMBER, 313, WS_EX_CLIENTEDGE},
        
        // 标签 - 野蛮优化
        {1, 30, 260, 80, 20, L"移动间隔", WS_VISIBLE, 0, 0},
        {1, 30, 290, 80, 20, L"移动间隔", WS_VISIBLE, 0, 0},
        {1, 30, 320, 80, 20, L"攻击间隔", WS_VISIBLE, 0, 0},
        {1, 30, 350, 80, 20, L"魔法间隔", WS_VISIBLE, 0, 0},
        {1, 30, 380, 80, 20, L"转身间隔", WS_VISIBLE, 0, 0},
        
        // 编辑框 - 野蛮优化
        {2, 110, 260, 40, 22, L"15", WS_VISIBLE | WS_BORDER | ES_NUMBER, 314, WS_EX_CLIENTEDGE},
        {2, 110, 290, 40, 22, L"15", WS_VISIBLE | WS_BORDER | ES_NUMBER, 315, WS_EX_CLIENTEDGE},
        {2, 110, 320, 40, 22, L"3", WS_VISIBLE | WS_BORDER | ES_NUMBER, 316, WS_EX_CLIENTEDGE},
        {2, 110, 350, 40, 22, L"15", WS_VISIBLE | WS_BORDER | ES_NUMBER, 317, WS_EX_CLIENTEDGE},
        {2, 110, 380, 40, 22, L"100", WS_VISIBLE | WS_BORDER | ES_NUMBER, 318, WS_EX_CLIENTEDGE},
        
        // 标签 - 锁一键二技能
        {1, 290, 260, 80, 20, L"补偿间隔", WS_VISIBLE, 0, 0},
        {1, 290, 290, 80, 20, L"补偿值", WS_VISIBLE, 0, 0},
        {1, 290, 320, 80, 20, L"补偿时间", WS_VISIBLE, 0, 0},
        {1, 290, 350, 80, 20, L"次数", WS_VISIBLE, 0, 0},
        {1, 290, 380, 100, 20, L"最低攻击间隔", WS_VISIBLE, 0, 0},
        {1, 290, 410, 100, 20, L"最低魔法间隔", WS_VISIBLE, 0, 0},
        
        // 编辑框 - 锁一键二技能
        {2, 370, 260, 40, 22, L"30", WS_VISIBLE | WS_BORDER | ES_NUMBER, 319, WS_EX_CLIENTEDGE},
        {2, 370, 290, 40, 22, L"2", WS_VISIBLE | WS_BORDER | ES_NUMBER, 320, WS_EX_CLIENTEDGE},
        {2, 370, 320, 40, 22, L"10", WS_VISIBLE | WS_BORDER | ES_NUMBER, 321, WS_EX_CLIENTEDGE},
        {2, 370, 350, 40, 22, L"3", WS_VISIBLE | WS_BORDER | ES_NUMBER, 322, WS_EX_CLIENTEDGE},
        {2, 400, 380, 40, 22, L"100", WS_VISIBLE | WS_BORDER | ES_NUMBER, 323, WS_EX_CLIENTEDGE},
        {2, 400, 410, 40, 22, L"100", WS_VISIBLE | WS_BORDER | ES_NUMBER, 324, WS_EX_CLIENTEDGE}
    };

    for (const auto& ctrl : controls) {
        HWND hCtrl = NULL;
        DWORD style = WS_CHILD | WS_VISIBLE | ctrl.style;
        
        switch (ctrl.type) {
        case 1: // Static
            hCtrl = CreateWindowEx(ctrl.exStyle, L"STATIC", ctrl.text.c_str(), style,
                ctrl.x, ctrl.y, ctrl.w, ctrl.h, hWnd, NULL, NULL, NULL);
            break;
        case 2: // Edit
            hCtrl = CreateWindowEx(ctrl.exStyle, L"EDIT", ctrl.text.c_str(), style | ES_NUMBER,
                ctrl.x, ctrl.y, ctrl.w, ctrl.h, hWnd, (HMENU)ctrl.id, NULL, NULL);
            break;
        case 3: // Button
            hCtrl = CreateWindowEx(ctrl.exStyle, L"BUTTON", ctrl.text.c_str(), style | BS_PUSHBUTTON,
                ctrl.x, ctrl.y, ctrl.w, ctrl.h, hWnd, (HMENU)ctrl.id, NULL, NULL);
            break;
        case 4: // CheckBox
            hCtrl = CreateWindowEx(ctrl.exStyle, L"BUTTON", ctrl.text.c_str(), style | BS_AUTOCHECKBOX,
                ctrl.x, ctrl.y, ctrl.w, ctrl.h, hWnd, (HMENU)ctrl.id, NULL, NULL);
            break;
        case 5: // GroupBox
            hCtrl = CreateWindowEx(ctrl.exStyle, L"BUTTON", ctrl.text.c_str(), style | BS_GROUPBOX,
                ctrl.x, ctrl.y, ctrl.w, ctrl.h, hWnd, NULL, NULL, NULL);
            break;
        }
        
        // 设置更美观的字体
        HFONT hFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 
                                CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, 
                                DEFAULT_PITCH | FF_DONTCARE, L"微软雅黑");
        if (hCtrl && hFont) {
            SendMessage(hCtrl, WM_SETFONT, (WPARAM)hFont, TRUE);
        }
    }
}

// 导出函数实现
void ShowGlobalDelayWindow() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    
    // 初始化通用控件
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icc);
    
    // 注册窗口类
    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.hInstance      = hInstance;
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_3DFACE+1);
    wcex.lpszClassName  = L"GlobalDelayWindowClass";
    wcex.hIcon          = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hIconSm        = LoadIcon(hInstance, IDI_APPLICATION);
    
    if (!RegisterClassEx(&wcex)) return;
    
    // 计算窗口大小
    int windowWidth = 550;
    int windowHeight = 430;
    
    // 创建窗口
    HWND hWnd = CreateWindow(
        L"GlobalDelayWindowClass", 
        L"全局游戏延迟补偿设置 - 1.0.6.112", 
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 
        windowWidth, windowHeight,
        NULL, NULL, hInstance, NULL
    );
    
    if (!hWnd) return;
    
    // 创建控件
    CreateControls(hWnd);
    
    // 显示窗口
    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);
    
    // 消息循环
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

// 窗口过程
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COMMAND: {
        int id = LOWORD(wParam);
        // 处理按钮点击
        if ((id >= 200 && id <= 201) || (id >= 100 && id <= 103)) {
            // 这里可以添加具体处理逻辑
            MessageBox(hWnd, L"操作已执行", L"提示", MB_OK | MB_ICONINFORMATION);
        }
        break;
    }
    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam;
        SetBkMode(hdc, TRANSPARENT);
        return (LRESULT)GetSysColorBrush(COLOR_3DFACE);
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

