// CLog.cpp: 实现文件
//

#include "pch.h"
#include "afxdialogex.h"
#include "CLogDlg.h"
#include "Resource.h"
#include <memory>
#include "GateF.h"
#include "GateFDlg.h"
#include "CObServerClientDlg.h"
#include "CObSCServiceDlg.h"
#include "CObSCLogicDlg.h"

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
    ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CLogDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

    // 获取初始窗口大小
    CRect rect;
    GetClientRect(&rect);
    m_originalSize = rect.Size();

    // 初始化布局信息
    CWnd* pChild = GetWindow(GW_CHILD);
    while (pChild)
    {
        ControlLayoutInfo info;
        pChild->GetWindowRect(&info.originalRect);
        ScreenToClient(&info.originalRect);
        info.nID = pChild->GetDlgCtrlID();

        // 设置锚定方式
        if (info.nID == IDC_TAB_LOG || info.nID == IDD_DIALOG_LOG_OBSERVER_CLIENT || info.nID == IDD_DIALOG_LOG_OBC_SERVICE || info.nID == IDD_DIALOG_LOG_OBC_LOGIC) {
            info.anchor = AnchorStyle::LEFT | AnchorStyle::TOP | AnchorStyle::RIGHT | AnchorStyle::BOTTOM;
        }
        else if (info.nID == IDC_BUTTON_SEARCH || info.nID == IDC_EDIT_SEARCH) {
            info.anchor = AnchorStyle::TOP | AnchorStyle::RIGHT;
        }
        else {
            info.anchor = AnchorStyle::LEFT | AnchorStyle::TOP;
        }

        m_layoutInfos.push_back(info);
        pChild = pChild->GetNextWindow();
    }

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

void CLogDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    if (nType == SIZE_MINIMIZED || cx <= 0 || cy <= 0)
        return;

    // 计算大小变化量
    int deltaX = cx - m_originalSize.cx;
    int deltaY = cy - m_originalSize.cy;

    // 使用延迟窗口定位以获得更好的性能
    HDWP hdwp = BeginDeferWindowPos((int)m_layoutInfos.size());

    for (auto& info : m_layoutInfos)  // 修改为auto&以便更新原始位置
    {
        CWnd* pWnd = GetDlgItem(info.nID);
        if (!pWnd || !pWnd->GetSafeHwnd())
            continue;

        CRect newRect = info.originalRect;

        // 根据锚定方式调整位置和大小
        if ((info.anchor & AnchorStyle::RIGHT) != 0)
        {
            if ((info.anchor & AnchorStyle::LEFT) != 0)
            {
                // 左右都锚定，调整宽度
                newRect.right += deltaX;
            }
            else
            {
                // 只锚定右边，移动位置
                newRect.left += deltaX;
                newRect.right += deltaX;
            }
        }

        if ((info.anchor & AnchorStyle::BOTTOM) != 0)
        {
            if ((info.anchor & AnchorStyle::TOP) != 0)
            {
                // 上下都锚定，调整高度
                newRect.bottom += deltaY;
            }
            else
            {
                // 只锚定下边，移动位置
                newRect.top += deltaY;
                newRect.bottom += deltaY;
            }
        }

        hdwp = DeferWindowPos(hdwp, pWnd->m_hWnd, NULL,
                              newRect.left, newRect.top,
                              newRect.Width(), newRect.Height(),
                              SWP_NOZORDER);

        // 更新控件的原始位置信息
        info.originalRect = newRect;  // 添加这行以更新控件位置
    }

    if (hdwp)
        EndDeferWindowPos(hdwp);

    // 更新原始大小为当前大小
    m_originalSize = CSize(cx, cy);

    // 调整标签控件大小
    if (m_tab_log.GetSafeHwnd())
    {
        CRect tabRect;
        GetClientRect(tabRect);
        m_tab_log.MoveWindow(tabRect);

        // 重新计算客户区域
        m_tab_log_rect = tabRect;
        m_tab_log.AdjustRect(FALSE, &m_tab_log_rect);
        m_tab_log_rect.DeflateRect(0, 0, 0, 0); // 调整客户区

        // 调整所有子对话框大小
        if (m_observer_client_dlg && m_observer_client_dlg->GetSafeHwnd())
        {
            m_observer_client_dlg->MoveWindow(m_tab_log_rect);
            m_observer_client_dlg->Invalidate();  // 强制重绘
        }
        if (m_obsc_service_dlg && m_obsc_service_dlg->GetSafeHwnd())
            m_obsc_service_dlg->MoveWindow(m_tab_log_rect);
        if (m_obsc_logic_dlg && m_obsc_logic_dlg->GetSafeHwnd())
            m_obsc_logic_dlg->MoveWindow(m_tab_log_rect);
    }
}