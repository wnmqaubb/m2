
// mircheatDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "mircheat.h"
#include "mircheatDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CmircheatDlg 对话框
#define HotKeyID_INSERT (WM_USER+1000)
#define HotKeyID_ALT_F12 (WM_USER+1001)


CmircheatDlg::CmircheatDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MIRCHEAT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CmircheatDlg::~CmircheatDlg()
{
    DestroyWindow();
}

void CmircheatDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_MAIN_TAB, m_main_base_tab);
}

BEGIN_MESSAGE_MAP(CmircheatDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_NOTIFY(TCN_SELCHANGE, IDC_MAIN_TAB, &CmircheatDlg::OnTcnSelchangeMainTab)
    ON_WM_HOTKEY()
END_MESSAGE_MAP()


// CmircheatDlg 消息处理程序

BOOL CmircheatDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

    ::RegisterHotKey(m_hWnd, HotKeyID_INSERT, NULL, VK_INSERT);
    ::RegisterHotKey(m_hWnd, HotKeyID_ALT_F12, MOD_ALT, VK_F12);

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标


    // 进程模块,线程tab
    m_main_base_tab.InsertItem(0, TEXT("基本"));
    m_main_base_tab.InsertItem(1, TEXT("战斗"));
    m_main_base_tab.InsertItem(2, TEXT("保护"));
    //创建两个对话框
    m_generalDlg.Create(IDD_MAIN_GENERAL_DLG, &m_main_base_tab);
    //m_processThreadDlg.Create(IDD_THREAD_INFO_DLG, &m_main_base_tab);
    //m_processDirectoryDlg.Create(IIDD_DIRECTORY_INFO_DLG, &m_main_base_tab);
    //设定在Tab内显示的范围
    CRect rc;
    m_main_base_tab.GetClientRect(rc);
    rc.top += 23;
    rc.bottom -= 0;
    rc.left += 0;
    rc.right -= 0;
    m_generalDlg.MoveWindow(&rc);
    //m_processThreadDlg.MoveWindow(&rc);
    //m_processDirectoryDlg.MoveWindow(&rc);
    //把对话框对象指针保存起来
    m_pDialog[0] = &m_generalDlg;
    /*m_pDialog[1] = &m_processThreadDlg;
    m_pDialog[2] = &m_processDirectoryDlg;*/

    //显示初始页面
    m_pDialog[0]->ShowWindow(SW_SHOW);
    /*m_pDialog[1]->ShowWindow(SW_HIDE);
    m_pDialog[2]->ShowWindow(SW_HIDE);*/

    //保存当前选择
    m_current_select_tab = 0;

    m_StatusBar.Create(WS_CHILD | WS_VISIBLE | SBT_OWNERDRAW, CRect(0, 0, 0, 0), this, 0);

    int strPartDim[3] = {70, 250, -1}; //分割数量
    m_StatusBar.SetParts(3, strPartDim);

    //设置状态栏文本
    m_StatusBar.SetText(L"玄武辅助", 0, 0);
    m_StatusBar.SetText(L"Insert/Alt+F12/呼/隐", 1, 0);
    m_StatusBar.SetText(L"VIP版", 2, 0);
    //下面是在状态栏中加入图标
    //m_StatusBar.SetIcon(1,
    //    SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME),
    //        FALSE));//为第二个分栏中加的图标

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CmircheatDlg::OnPaint()
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
HCURSOR CmircheatDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CmircheatDlg::OnTcnSelchangeMainTab(NMHDR *pNMHDR, LRESULT *pResult)
{
    //把当前的页面隐藏起来
    //m_pDialog[m_current_select_tab]->ShowWindow(SW_HIDE);
    //得到新的页面索引
    //m_current_select_tab = m_main_base_tab.GetCurSel();
    //把新的页面显示出来
    //m_pDialog[m_current_select_tab]->ShowWindow(SW_SHOW);
    *pResult = 0;
}


void CmircheatDlg::OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2)
{
    if(nHotKeyId == HotKeyID_INSERT)
    {
        this->ShowWindow(SW_SHOW);
    }
    else if(nHotKeyId == HotKeyID_ALT_F12)
    {
        this->ShowWindow(SW_HIDE);
    }

    CDialogEx::OnHotKey(nHotKeyId, nKey1, nKey2);
}
