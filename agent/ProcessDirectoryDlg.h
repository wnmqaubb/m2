#pragma once
#include "MFCUtil.h"


// ProcessDirectoryDlg 对话框
class CagentDlg;
class ProcessDirectoryDlg : public CDialogEx
{
	DECLARE_DYNAMIC(ProcessDirectoryDlg)

public:
	ProcessDirectoryDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ProcessDirectoryDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum
	{
		IDD = IID_DIRECTORY_INFO_DLG
	};
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_process_directory_list;
	MFCUtil mfcutil;
	CagentDlg *m_agentDlg;
	int m_current_row_index;
	virtual BOOL OnInitDialog();
	afx_msg void OnLvnColumnclickDirectoryList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickDirectoryList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDirectoryName();
};
