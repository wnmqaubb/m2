// CSpeedDialog.cpp: 实现文件
//

#include "pch.h"
#include "RunGateSpeedManage1.h"
#include "afxdialogex.h"
#include "CSpeedDialog.h"


// CSpeedDialog 对话框

IMPLEMENT_DYNAMIC(CSpeedDialog, CDialogEx)

CSpeedDialog::CSpeedDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DELAY_COMPENSATION, pParent)
{

}

CSpeedDialog::~CSpeedDialog()
{
}

void CSpeedDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSpeedDialog, CDialogEx)
END_MESSAGE_MAP()


// CSpeedDialog 消息处理程序
