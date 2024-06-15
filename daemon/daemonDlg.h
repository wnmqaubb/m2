<<<<<<< HEAD
﻿
// daemonDlg.h: 头文件
//

#pragma once


// CdaemonDlg 对话框
class CdaemonDlg : public CDialogEx
{
// 构造
public:
	CdaemonDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DAEMON_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
    int daemon();
    afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedCancel();
    BOOL TrayMyIcon(BOOL bAdd);//bAdd为TRUE就添加，bAdd为FALSE就不添加。
    LRESULT OnTrayCallBackMsg(WPARAM wparam, LPARAM lparam);
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
};
=======
﻿
// daemonDlg.h: 头文件
//

#pragma once


// CdaemonDlg 对话框
class CdaemonDlg : public CDialogEx
{
// 构造
public:
	CdaemonDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DAEMON_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
    int daemon();
    afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedCancel();
    BOOL TrayMyIcon(BOOL bAdd);//bAdd为TRUE就添加，bAdd为FALSE就不添加。
    LRESULT OnTrayCallBackMsg(WPARAM wparam, LPARAM lparam);
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
};
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9
