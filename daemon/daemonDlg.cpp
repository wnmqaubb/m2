
// daemonDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "daemon.h"
#include "daemonDlg.h"
#include "afxdialogex.h"
#include <corecrt_io.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CdaemonDlg 对话框
#define WM_TRAYICON_MSG (WM_USER+100)



CdaemonDlg::CdaemonDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DAEMON_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CdaemonDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CdaemonDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDCANCEL, &CdaemonDlg::OnBnClickedCancel)
    ON_MESSAGE(WM_TRAYICON_MSG, OnTrayCallBackMsg)
    ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()


// CdaemonDlg 消息处理程序

UINT Daemon(LPVOID lpParam)
{

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    GetStartupInfoA(&si);
    si.wShowWindow = SW_HIDE;
    ZeroMemory(&pi, sizeof(pi));

    char pPath[MAX_PATH] = {0};
    GetCurrentDirectoryA(MAX_PATH, pPath);

    strcat_s(pPath, "\\网关VIP版.exe");

    char pCmd[MAX_PATH] = {0};
    strcat_s(pCmd, "cmd /c ");
    strcat_s(pCmd, pPath);

    do
    {
        if(_access(pPath, 0) != -1)
        {
            if(!CreateProcessA(NULL, pCmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
            {
                return -1;
            }

            WaitForSingleObject(pi.hProcess, INFINITE);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }

        Sleep(3000);
    } while(true);
    return 0;
}

BOOL CdaemonDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	ShowWindow(SW_SHOW);

	// TODO: 在此添加额外的初始化代码
    AfxBeginThread(Daemon,NULL);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CdaemonDlg::OnPaint()
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
HCURSOR CdaemonDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CdaemonDlg::OnBnClickedCancel()
{
    CDialogEx::OnCancel();
}

BOOL CdaemonDlg::TrayMyIcon(BOOL bAdd)//bAdd为TRUE就添加，bAdd为FALSE就不添加。
{
    BOOL bRet = FALSE;
    NOTIFYICONDATA tnd;
    tnd.cbSize = sizeof(NOTIFYICONDATA);
    tnd.hWnd = GetSafeHwnd();//就是m_hWnd
    tnd.uID = IDR_MAINFRAME;
    if(bAdd == TRUE)
    {
        tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;//uFlags这个参数为了标识下面的哪个参数可用
        tnd.uCallbackMessage = WM_TRAYICON_MSG;//NIF_MESSAGE,任务栏托盘图标在被鼠标点击时触发的消息
        tnd.hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));//NIF_ICON
        memcpy(tnd.szTip, _T("网关守护"), sizeof(tnd.szTip));
        ShowWindow(SW_MINIMIZE);//把窗口最小化。
        ShowWindow(SW_HIDE);//只是把程序窗口隐藏了。（界面的可视化设计其实就是一层窗户纸，捅破了就好了）
        bRet = Shell_NotifyIcon(NIM_ADD, &tnd);//添加任务栏托盘图标
    }
    else
    {
        ShowWindow(SW_SHOWNA);//显示窗口
        SetForegroundWindow();//把程序窗口设置成前景图标
        bRet = Shell_NotifyIcon(NIM_DELETE, &tnd);//删除任务栏托盘图标
    }
    return bRet;
}

LRESULT CdaemonDlg::OnTrayCallBackMsg(WPARAM wparam, LPARAM lparam)
{
    switch(lparam)
    {
    //case WM_RBUTTONUP://鼠标右键单击的消息
    //{
    //    CMenu mMenu, *pMenu = NULL;
    //    CPoint pt;
    //    mMenu.LoadMenu(IDR_MENU2);
    //    pMenu = mMenu.GetSubMenu(0);
    //    GetCursorPos(&pt);
    //    SetForegroundWindow();//不加这一段代码可能出现右键菜单不消失的现象
    //    pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
    //    break;
    //}
    case WM_LBUTTONUP:
    case WM_LBUTTONDBLCLK:
    {
        ShowWindow(SW_RESTORE);
    }
    SetForegroundWindow();
    TrayMyIcon(FALSE);
    break;
    default:
        break;
    }
    return NULL;
}

void CdaemonDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if(nID == SC_MINIMIZE)
    {
        ShowWindow(SW_HIDE);
        TrayMyIcon(true);
    }

    CDialogEx::OnSysCommand(nID, lParam);
}
