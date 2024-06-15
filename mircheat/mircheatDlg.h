<<<<<<< HEAD
﻿
// mircheatDlg.h: 头文件
//

#pragma once
#include "generaldlg.h"
#include <afxdialogex.h>


// CmircheatDlg 对话框
class CmircheatDlg : public CDialogEx
{
// 构造
public:
	CmircheatDlg(CWnd* pParent = nullptr);	// 标准构造函数
    ~CmircheatDlg();
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MIRCHEAT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnTcnSelchangeMainTab(NMHDR *pNMHDR, LRESULT *pResult);
    CTabCtrl m_main_base_tab; 
    CStatusBarCtrl m_StatusBar;
    GeneralDlg m_generalDlg;
    CDialogEx* m_pDialog[3];
    int m_current_select_tab;
    afx_msg void OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);
};
=======
﻿
// mircheatDlg.h: 头文件
//

#pragma once
#include "generaldlg.h"
#include <afxdialogex.h>


// CmircheatDlg 对话框
class CmircheatDlg : public CDialogEx
{
// 构造
public:
	CmircheatDlg(CWnd* pParent = nullptr);	// 标准构造函数
    ~CmircheatDlg();
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MIRCHEAT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnTcnSelchangeMainTab(NMHDR *pNMHDR, LRESULT *pResult);
    CTabCtrl m_main_base_tab; 
    CStatusBarCtrl m_StatusBar;
    GeneralDlg m_generalDlg;
    CDialogEx* m_pDialog[3];
    int m_current_select_tab;
    afx_msg void OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);
};
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9
