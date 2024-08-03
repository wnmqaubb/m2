#pragma once
#include "protocol.h"
#include "MFCUtil.h"


// ProcessWindowDlg 对话框
class CagentDlg;
class ClientWindowDlg : public CDialogEx
{
	DECLARE_DYNAMIC(ClientWindowDlg)

public:
	ClientWindowDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ClientWindowDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum
	{
		IDD = IDD_CLIENT_WINDOW_DLG
	};
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_client_window_list;
	MFCUtil mfcutil;
	int m_current_row_index;
	CagentDlg *m_agentDlg;
	void ShowClientWindowDlg(ProtocolProcessInfo& data);
	afx_msg void OnLvnColumnclickClientWindowList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawClientWindowList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnWindowTitle();
	afx_msg void OnWindowClassName();
	afx_msg void OnWindowBoth();
	afx_msg void OnNMRClickClientWindowList(NMHDR *pNMHDR, LRESULT *pResult);
};
