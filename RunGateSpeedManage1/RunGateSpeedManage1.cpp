// RunGateSpeedManage1.cpp: 定义 DLL 的初始化例程。
//

#include "pch.h"
#include "framework.h"
#include "RunGateSpeedManage1.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include "CSpeedDialog.h"

#pragma comment(lib, "comctl32.lib")
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

// CRunGateSpeedManage1App

BEGIN_MESSAGE_MAP(CRunGateSpeedManage1App, CWinApp)
END_MESSAGE_MAP()


// CRunGateSpeedManage1App 构造

CRunGateSpeedManage1App::CRunGateSpeedManage1App()
{
	// TODO:  在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的 CRunGateSpeedManage1App 对象

CRunGateSpeedManage1App theApp;


// CRunGateSpeedManage1App 初始化

BOOL CRunGateSpeedManage1App::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}

// 导出函数声明
extern "C" __declspec(dllexport) void ShowSettingsDialog();

// 实现
void ShowSettingsDialog() {
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CSpeedDialog dlg;
    dlg.DoModal();
}