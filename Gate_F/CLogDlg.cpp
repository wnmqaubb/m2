// CLog.cpp: 实现文件
//

#include "pch.h"
#include "afxdialogex.h"
#include "CLogDlg.h"
#include "Resource.h"
#include <memory>


// CLog 对话框

IMPLEMENT_DYNAMIC(CLogDlg, CDialogEx)

CLogDlg::CLogDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_LOG, pParent)
{

}

CLogDlg::~CLogDlg()
{
}

void CLogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB_LOG, m_tab_log);
}

BEGIN_MESSAGE_MAP(CLogDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_LOG, &CLogDlg::OnTcnSelchangeTabMain)
END_MESSAGE_MAP()

BOOL CLogDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: 在此添加额外的初始化代码
	// 初始化 CTabCtrl
	m_tab_log.GetClientRect(&m_tab_log_rect);
	m_tab_log.AdjustRect(FALSE, &m_tab_log_rect);
	m_tab_log_rect.DeflateRect(0, 20, 0, 0);
	m_tab_log.InsertItem(0, _T("系统日志"));
	m_tab_log.InsertItem(1, _T("连接日志"));
	m_tab_log.InsertItem(2, _T("外挂拦截日志"));
	ShowAllDlgInTab();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}
// CLog 消息处理程序

void CLogDlg::ShowAllDlgInTab()
{
	m_observer_client_dlg = std::make_unique<CObServerClientDlg>();
	m_observer_client_dlg->Create(IDD_DIALOG_LOG_OBSERVER_CLIENT, &m_tab_log);
	m_observer_client_dlg->MoveWindow(m_tab_log_rect);
	m_observer_client_dlg->ShowWindow(SW_SHOW);

	m_obsc_service_dlg = std::make_unique<COBSCServiceDlg>();
	m_obsc_service_dlg->Create(IDD_DIALOG_LOG_OBC_SERVICE, &m_tab_log);
	m_obsc_service_dlg->MoveWindow(m_tab_log_rect);
	m_obsc_service_dlg->ShowWindow(SW_HIDE);

	m_obsc_logic_dlg = std::make_unique<COBSCLogicDlg>();
	m_obsc_logic_dlg->Create(IDD_DIALOG_LOG_OBC_LOGIC, &m_tab_log);
	m_obsc_logic_dlg->MoveWindow(m_tab_log_rect);
	m_obsc_logic_dlg->ShowWindow(SW_HIDE);
}

void CLogDlg::OnTcnSelchangeTabMain(NMHDR* pNMHDR, LRESULT* pResult)
{
	switch (m_tab_log.GetCurSel())
	{
		case 0:
			m_observer_client_dlg->ShowWindow(SW_SHOW);
			m_obsc_service_dlg->ShowWindow(SW_HIDE);
			m_obsc_logic_dlg->ShowWindow(SW_HIDE);
			break;
		case 1:
			m_observer_client_dlg->ShowWindow(SW_HIDE);
			m_obsc_service_dlg->ShowWindow(SW_SHOW);
			m_obsc_logic_dlg->ShowWindow(SW_HIDE);
			break;
		case 2:
			m_observer_client_dlg->ShowWindow(SW_HIDE);
			m_obsc_service_dlg->ShowWindow(SW_HIDE);
			m_obsc_logic_dlg->ShowWindow(SW_SHOW);
			break;
		default:
			break;
	}
	*pResult = 0;
}