
// test_client_pluginDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "test_client_plugin.h"
#include "test_client_pluginDlg.h"
#include "afxdialogex.h"
#include "../../Lightbone/api_resolver.h"
#include <../../14.16.27023/include/string>
#include "../../../VC-LTL/VC/14.16.27023/include/thread"
#include <winbase.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CtestclientpluginDlg 对话框

extern void __stdcall client_entry(std::string guard_gate_ip) noexcept;
extern void __stdcall DoUnInit();
using client_entry_t = decltype(&client_entry);
using uninit_t = decltype(&DoUnInit);

CtestclientpluginDlg::CtestclientpluginDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TEST_CLIENT_PLUGIN_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CtestclientpluginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CtestclientpluginDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON2, &CtestclientpluginDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON1, &CtestclientpluginDlg::OnBnClickedButton1)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON3, &CtestclientpluginDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CtestclientpluginDlg::OnBnClickedButton4)
END_MESSAGE_MAP()


// CtestclientpluginDlg 消息处理程序

BOOL CtestclientpluginDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

#ifndef _DEBUG
	m_hmodule = LoadLibraryA("D:\\work\\Repos\\M2\\build\\bin\\Release\\Win32\\NewClient_f.dll");
#else
	m_hmodule = LoadLibraryA("D:\\work\\Repos\\M2\\build\\bin\\Debug\\Win32\\NewClient_f.dll");
#endif

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CtestclientpluginDlg::OnPaint()
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
HCURSOR CtestclientpluginDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CtestclientpluginDlg::load_dll() {
	while (!m_hmodule) 
	{
#ifndef _DEBUG
		m_hmodule = LoadLibraryA("D:\\work\\Repos\\M2\\build\\bin\\Release\\Win32\\NewClient_f.dll");
#else
		m_hmodule = LoadLibraryA("D:\\work\\Repos\\M2\\build\\bin\\Debug\\Win32\\NewClient_f.dll");
#endif
	}
}

void CtestclientpluginDlg::OnBnClickedButton3()
{
	load_dll();
	if (m_hmodule) {
		client_entry_t entry = (client_entry_t)ApiResolver::get_proc_address(m_hmodule, CT_HASH("client_entry"));
		std::string ip = "140.210.20.215";
		entry(ip);
	}
}


void CtestclientpluginDlg::OnBnClickedButton4()
{
	load_dll();
	if (m_hmodule) {
		uninit_t uninit = (uninit_t)ApiResolver::get_proc_address(m_hmodule, CT_HASH("DoUnInit"));
		uninit();
		try
		{

			if (FreeLibrary(m_hmodule)) {
				m_hmodule = nullptr;
				OutputDebugString(L"free dll ok");
			}
			else {
				OutputDebugString(L"free dll failed");
			}
		}
		catch (CMemoryException* e)
		{
			AfxMessageBox(L"free dll failed1");
		}
		catch (CFileException* e)
		{
			AfxMessageBox(L"free dll failed2");
		}
		catch (CException* e)
		{
			AfxMessageBox(L"free dll failed3");
		}
	}
}

void CtestclientpluginDlg::OnBnClickedButton1()
{
	SetTimer(1, 1000, nullptr);
}

void CtestclientpluginDlg::OnBnClickedButton2()
{
	KillTimer(1);
}

void CtestclientpluginDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
		case 1:
			// 自动测试小退开始
			OnBnClickedButton3();
			std::this_thread::sleep_for(std::chrono::seconds(1));
			// 自动测试小退结束
			OnBnClickedButton4();
			break;
		default:
			break;
	}

	CDialogEx::OnTimer(nIDEvent);
}

