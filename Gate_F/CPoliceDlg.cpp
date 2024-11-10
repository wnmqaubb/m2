// CPoliceDlg.cpp: 实现文件
//

#include "pch.h"
#include "GateF.h"
#include "afxdialogex.h"
#include "CPoliceDlg.h"


// CPoliceDlg 对话框

IMPLEMENT_DYNAMIC(CPoliceDlg, CDialogEx)

CPoliceDlg::CPoliceDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_POLICES, pParent)
{

}

CPoliceDlg::~CPoliceDlg()
{
}

void CPoliceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPoliceDlg, CDialogEx)
END_MESSAGE_MAP()


// CPoliceDlg 消息处理程序
