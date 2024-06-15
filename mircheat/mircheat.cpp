<<<<<<< HEAD
﻿// mircheat1.cpp: 定义 DLL 的初始化例程。
//

#include "pch.h"
#include "framework.h"
#include "mircheat.h"
#include "mircheatDlg.h"
#include "utils\utils.h"
#include "cheat.h"
#include <core.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO:  如果此 DLL 相对于 MFC DLL 是动态链接的，
//		则从此 DLL 导出的任何调入
//		MFC 的函数必须将 AFX_MANAGE_STATE 宏添加到
//		该函数的最前面。
//
//		例如: 
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// 此处为普通函数体
//		}
//
//		此宏先于任何 MFC 调用
//		出现在每个函数中十分重要。  这意味着
//		它必须作为以下项中的第一个语句:
//		出现，甚至先于所有对象变量声明，
//		这是因为它们的构造函数可能生成 MFC
//		DLL 调用。
//
//		有关其他详细信息，
//		请参阅 MFC 技术说明 33 和 58。
//

// Cmircheat1App

BEGIN_MESSAGE_MAP(CmircheatApp, CWinApp)
END_MESSAGE_MAP()


// Cmircheat1App 构造

CmircheatApp::CmircheatApp()
{
	// TODO:  在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的 CmircheatApp 对象

CmircheatApp theApp;


// Cmircheat1App 初始化

void show_main_dlg(void* param)
{
    //Cheat::instance().hook_init();
    //HWND hMainWnd = FindWindowA("TFrmMain", " - ");
    //HWND hMainWnd = GetHwndByProcessId(GetCurren0tProcessId());
    //CWnd *pMainWnd = CWnd::FromHandle(hMainWnd);
    if (!theApp.dlg)
    {
        Cheat::instance().hook_init();
        theApp.dlg = new CmircheatDlg();
        theApp.dlg->Create(IDD_MIRCHEAT_DIALOG, CWnd::FromHandle((HWND)param));
        theApp.m_pMainWnd = CWnd::FromHandle((HWND)param);
    }
    

    //theApp.dlg.Create(IDD_MIRCHEAT_DIALOG, NULL);
    if (theApp.dlg)
    {
        theApp.dlg->ShowWindow(SW_SHOW);
    }
    
    //MSG msg;//消息循环  如果dll宿主程序为自己调用的程序 消息循环可以不需要 如果宿主程序为第三方进程 如果没有消息循环 调用后窗口会闪退.
    //while(GetMessage(&msg, NULL, 0, 0))
    //{
    //    TranslateMessage(&msg);
    //    DispatchMessage(&msg);
    //}
}

BOOL CmircheatApp::InitInstance()
{
    CWinApp::InitInstance();

    AfxEnableControlContainer();
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    

    /*
    CWinApp::InitInstance();
    CmircheatDlg dlg;
    m_pMainWnd = &dlg;
    _beginthread(&show_main_dlg, NULL, m_pMainWnd);
    */
	return TRUE;
}
bool Core::entrypoint(HMODULE hinstance)
{
    
    return true;
}
bool Core::exitpoint()
{
    return true;
}

LRESULT Core::on_recv_msg(__in int code, __in WPARAM wparam, __in LPARAM lparam)
{
    LPMSG msg = (LPMSG)lparam;
    switch (msg->message)
    {
    case 0x801:
    {
        show_main_dlg(msg->hwnd);
        break;
    }
    default:
        break;
    }
    return CallNextHookEx(NULL, code, wparam, lparam);
}

DLL_API Core& CoreImpl()
{
    static Core instance;
    return instance;
=======
﻿// mircheat1.cpp: 定义 DLL 的初始化例程。
//

#include "pch.h"
#include "framework.h"
#include "mircheat.h"
#include "mircheatDlg.h"
#include "utils\utils.h"
#include "cheat.h"
#include <core.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO:  如果此 DLL 相对于 MFC DLL 是动态链接的，
//		则从此 DLL 导出的任何调入
//		MFC 的函数必须将 AFX_MANAGE_STATE 宏添加到
//		该函数的最前面。
//
//		例如: 
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// 此处为普通函数体
//		}
//
//		此宏先于任何 MFC 调用
//		出现在每个函数中十分重要。  这意味着
//		它必须作为以下项中的第一个语句:
//		出现，甚至先于所有对象变量声明，
//		这是因为它们的构造函数可能生成 MFC
//		DLL 调用。
//
//		有关其他详细信息，
//		请参阅 MFC 技术说明 33 和 58。
//

// Cmircheat1App

BEGIN_MESSAGE_MAP(CmircheatApp, CWinApp)
END_MESSAGE_MAP()


// Cmircheat1App 构造

CmircheatApp::CmircheatApp()
{
	// TODO:  在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的 CmircheatApp 对象

CmircheatApp theApp;


// Cmircheat1App 初始化

void show_main_dlg(void* param)
{
    //Cheat::instance().hook_init();
    //HWND hMainWnd = FindWindowA("TFrmMain", " - ");
    //HWND hMainWnd = GetHwndByProcessId(GetCurren0tProcessId());
    //CWnd *pMainWnd = CWnd::FromHandle(hMainWnd);
    if (!theApp.dlg)
    {
        Cheat::instance().hook_init();
        theApp.dlg = new CmircheatDlg();
        theApp.dlg->Create(IDD_MIRCHEAT_DIALOG, CWnd::FromHandle((HWND)param));
        theApp.m_pMainWnd = CWnd::FromHandle((HWND)param);
    }
    

    //theApp.dlg.Create(IDD_MIRCHEAT_DIALOG, NULL);
    if (theApp.dlg)
    {
        theApp.dlg->ShowWindow(SW_SHOW);
    }
    
    //MSG msg;//消息循环  如果dll宿主程序为自己调用的程序 消息循环可以不需要 如果宿主程序为第三方进程 如果没有消息循环 调用后窗口会闪退.
    //while(GetMessage(&msg, NULL, 0, 0))
    //{
    //    TranslateMessage(&msg);
    //    DispatchMessage(&msg);
    //}
}

BOOL CmircheatApp::InitInstance()
{
    CWinApp::InitInstance();

    AfxEnableControlContainer();
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    

    /*
    CWinApp::InitInstance();
    CmircheatDlg dlg;
    m_pMainWnd = &dlg;
    _beginthread(&show_main_dlg, NULL, m_pMainWnd);
    */
	return TRUE;
}
bool Core::entrypoint(HMODULE hinstance)
{
    
    return true;
}
bool Core::exitpoint()
{
    return true;
}

LRESULT Core::on_recv_msg(__in int code, __in WPARAM wparam, __in LPARAM lparam)
{
    LPMSG msg = (LPMSG)lparam;
    switch (msg->message)
    {
    case 0x801:
    {
        show_main_dlg(msg->hwnd);
        break;
    }
    default:
        break;
    }
    return CallNextHookEx(NULL, code, wparam, lparam);
}

DLL_API Core& CoreImpl()
{
    static Core instance;
    return instance;
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9
}