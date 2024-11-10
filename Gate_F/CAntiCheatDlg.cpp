// CAntiCheatDlg.cpp: 实现文件
//

#include "pch.h"
#include "GateF.h"
#include "afxdialogex.h"
#include "CAntiCheatDlg.h"


// CAntiCheatDlg 对话框

IMPLEMENT_DYNAMIC(CAntiCheatDlg, CDialogEx)

CAntiCheatDlg::CAntiCheatDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_ANTICHEAT, pParent)
{

}

CAntiCheatDlg::~CAntiCheatDlg()
{
}

void CAntiCheatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAntiCheatDlg, CDialogEx)
END_MESSAGE_MAP()


// CAntiCheatDlg 消息处理程序
