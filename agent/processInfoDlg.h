#pragma once
#include "protocol.h"
#include "ProcessThreadDlg.h"
#include "ProcessModuleDlg.h"
#include "ProcessDirectoryDlg.h"
#include "MFCUtil.h"


// DlgProcessInfo 对话框
class CagentDlg;
class ProcessInfoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(ProcessInfoDlg)

public:
	ProcessInfoDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ProcessInfoDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum
	{
		IDD = IDD_PROCESS_INFO
	};
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_process_info_list;
	json m_json_data;
	int m_current_row_index;
	CagentDlg *m_agentDlg;
	CString m_search_text;
	CString m_search_text_old;
	CTabCtrl m_module_thread_info_tab;
	int m_current_select_tab;
	ProcessModuleDlg m_processModuleDlg;
	ProcessThreadDlg m_processThreadDlg;
	ProcessDirectoryDlg m_processDirectoryDlg;
	MFCUtil mfcutil;
	CDialogEx* m_pDialog[3];
	//	afx_msg void OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
	virtual BOOL OnInitDialog();
	afx_msg void OnNMCustomdrawList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnColumnclickList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedFlash();
	afx_msg void OnBnClickedSearch();
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
	void ShowProcessInfo(ProtocolProcessInfo& data);
	void ShowProcessModuleInfo(ProtocolProcessInfo& data);
	void ShowProcessThreadInfo(ProtocolProcessInfo& data);
	void ShowProcessDirectoryInfo(ProtocolProcessInfo& data);
	afx_msg void OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnProcessName();
	afx_msg void OnProcessPath();
	virtual void OnOK();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
