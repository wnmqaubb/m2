#pragma once
#include "MFCUtil.h"


// ProcessModuleDlg 对话框

class CagentDlg;
class ProcessModuleDlg : public CDialogEx
{
	DECLARE_DYNAMIC(ProcessModuleDlg)

public:
	ProcessModuleDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ProcessModuleDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum
	{
		IDD = IDD_MODULE_INFO_DLG
	};
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_process_module_list;
	MFCUtil mfcutil;
	CagentDlg *m_agentDlg;
	int m_current_row_index;
	virtual BOOL OnInitDialog();
	afx_msg void OnLvnColumnclickModuleList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnModuleName();
	afx_msg void OnModulePath();
	afx_msg void OnNMRClickModuleList(NMHDR *pNMHDR, LRESULT *pResult);
};
