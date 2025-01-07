// CLog.cpp: 实现文件
//

#include "pch.h"
#include "afxdialogex.h"
#include "CLogDlg.h"
#include "Resource.h"
#include <memory>
#include "GateF.h"


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

	m_tab_log.GetClientRect(&m_tab_log_rect);
	m_tab_log.AdjustRect(FALSE, &m_tab_log_rect);
	m_tab_log_rect.DeflateRect(0, 20, 0, 0);
	m_tab_log.InsertItem(0, _T("系统日志"));
	m_tab_log.InsertItem(1, _T("连接日志"));
	m_tab_log.InsertItem(2, _T("外挂拦截日志"));
	ShowAllDlgInTab();
	m_tab_log.SetCurSel(2);
	OnTcnSelchangeTabMain(nullptr, nullptr);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}
// CLog 消息处理程序

void CLogDlg::ShowAllDlgInTab()
{
	m_observer_client_dlg = std::make_shared<CObServerClientDlg>();
	m_observer_client_dlg->Create(IDD_DIALOG_LOG_OBSERVER_CLIENT, &m_tab_log);
	m_observer_client_dlg->MoveWindow(m_tab_log_rect);
	m_observer_client_dlg->ShowWindow(SW_HIDE);

	m_obsc_service_dlg = std::make_shared<COBSCServiceDlg>();
	m_obsc_service_dlg->Create(IDD_DIALOG_LOG_OBC_SERVICE, &m_tab_log);
	m_obsc_service_dlg->MoveWindow(m_tab_log_rect);
	m_obsc_service_dlg->ShowWindow(SW_HIDE);

	m_obsc_logic_dlg = std::make_shared<COBSCLogicDlg>();
	m_obsc_logic_dlg->Create(IDD_DIALOG_LOG_OBC_LOGIC, &m_tab_log);
	m_obsc_logic_dlg->MoveWindow(m_tab_log_rect);
	m_obsc_logic_dlg->ShowWindow(SW_SHOW);
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
}

void CLogDlg::LogPrint(int type, LPCTSTR format, ...)
{
	CString buf;
	va_list ap;
	va_start(ap, format);
	buf.FormatV(format, ap);
	va_end(ap);
	CTime tm = CTime::GetCurrentTime();
	buf.Format(_T("[%s]%s"), tm.Format(_T("%H:%M:%S")), buf);
	theApp.m_WorkIo.post([this, type, buf]() {
		switch (type)
		{
			case ObserverClientLog:
				theApp.GetMainFrame()->m_logs_dlg->m_observer_client_dlg->AddLog(buf, RGB(0, 200, 0));
				//AdjustHorzScroll(m_wndObserverClientLog);
				break;
			case ServiceLog:
				theApp.GetMainFrame()->m_logs_dlg->m_obsc_service_dlg->AddLog(buf, RGB(25, 25, 205));
				//AdjustHorzScroll(m_wndServiceLog);
				break;
			case LogicServerLog:
				theApp.GetMainFrame()->m_logs_dlg->m_obsc_logic_dlg->AddLog(buf, RGB(255, 0, 0));
				//AdjustHorzScroll(m_wndLogicServerLog);
				break;
			default:
				break;
		}
		});
}