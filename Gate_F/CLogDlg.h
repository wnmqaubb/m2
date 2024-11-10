#pragma once
#include "afxdialogex.h"
#include "CObServerClientDlg.h"
#include "COBSCServiceDlg.h"
#include "COBSCLogicDlg.h"


// CLog 对话框

class CLogDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CLogDlg)

public:
	CLogDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CLogDlg();

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
	std::unique_ptr<CObServerClientDlg> m_observer_client_dlg;
	std::unique_ptr<COBSCServiceDlg> m_obsc_service_dlg;
	std::unique_ptr<COBSCLogicDlg> m_obsc_logic_dlg;
public:
	CTabCtrl m_tab_log;
	CRect m_tab_log_rect;
};
