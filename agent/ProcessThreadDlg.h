#pragma once
#include "MFCUtil.h"


// ProcessThreadDlg 对话框

class ProcessThreadDlg : public CDialogEx
{
	DECLARE_DYNAMIC(ProcessThreadDlg)

public:
	ProcessThreadDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ProcessThreadDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum
	{
		IDD = IDD_THREAD_INFO_DLG
	};
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_process_thread_list;
	MFCUtil mfcutil;
	virtual BOOL OnInitDialog();
	afx_msg void OnLvnColumnclickThreadList(NMHDR *pNMHDR, LRESULT *pResult);
};
