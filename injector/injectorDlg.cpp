
// injectorDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "injector.h"
#include "injectorDlg.h"
#include "afxdialogex.h"
#include <core.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CinjectorDlg 对话框

CinjectorDlg::CinjectorDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_INJECTOR_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CinjectorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CinjectorDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON1, &CinjectorDlg::OnBnClickedButton1)
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_HOTKEY()
END_MESSAGE_MAP()


// CinjectorDlg 消息处理程序

BOOL CinjectorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CinjectorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CinjectorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CinjectorDlg::OnBnClickedButton1()
{
    
    // TODO: 在此添加控件通知处理程序代码
}


int CinjectorDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDialogEx::OnCreate(lpCreateStruct) == -1)
        return -1;

    // TODO:  在此添加您专用的创建代码
    RegisterHotKey(GetSafeHwnd(), 1001, NULL, VK_HOME);
    return 0;
}


void CinjectorDlg::OnDestroy()
{
    CDialogEx::OnDestroy();
    UnregisterHotKey(GetSafeHwnd(), 1001);
    // TODO: 在此处添加消息处理程序代码
}


void CinjectorDlg::OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    CDialogEx::OnHotKey(nHotKeyId, nKey1, nKey2);

    switch (nHotKeyId)
    {
    case 1001:
    {
        CString text;
        HWND hwnd = ::GetForegroundWindow();
        text.Format(TEXT("%d"), hwnd);
        GetDlgItem(IDC_STATIC_HWND)->SetWindowText(text);
        DWORD tid = GetWindowThreadProcessId(hwnd, NULL);
        CoreImpl().setup(tid);
        PostMessageA(hwnd, 0x801, NULL, NULL);
        break;
    }
    default:
        break;
    }

}
