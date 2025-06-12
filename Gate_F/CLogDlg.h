#pragma once
#include "afxdialogex.h"
#include "CObServerClientDlg.h"
#include "COBSCServiceDlg.h"
#include "COBSCLogicDlg.h"
#include <vector>
#include "GateF.h"


enum OutputWndLogType
{
	ObserverClientLog,
	ServiceLog,
	LogicServerLog
};
// CLog 对话框
class CGateFDlg;

class CLogDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CLogDlg)

public:
	CLogDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CLogDlg();
	void LogPrint(int type, LPCTSTR format, ...);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    CSize m_originalSize; // 添加这行
    std::vector<ControlLayoutInfo> m_layoutInfos;

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_LOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
	BOOL OnInitDialog();
	void ShowAllDlgInTab();
	void OnTcnSelchangeTabMain(NMHDR* pNMHDR, LRESULT* pResult);
public:
	std::shared_ptr<CObServerClientDlg> m_observer_client_dlg;
	std::shared_ptr<COBSCServiceDlg> m_obsc_service_dlg;
	std::shared_ptr<COBSCLogicDlg> m_obsc_logic_dlg;
	CTabCtrl m_tab_log;
	CRect m_tab_log_rect;
};
