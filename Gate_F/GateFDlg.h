
// GateFDlg.h: 头文件
//

#pragma once
#include "CGamesDlg.h"
#include <memory>
#include "CPoliceDlg.h"
#include "CAntiCheatDlg.h"
#include "CLogDlg.h"

class CGateFDlgAutoProxy;


// CGateFDlg 对话框
class CGateFDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CGateFDlg);
	friend class CGateFDlgAutoProxy;

// 构造
public:
	CGateFDlg(CWnd* pParent = nullptr);	// 标准构造函数
	virtual ~CGateFDlg();

	CString GetCurrentSelectedUserName();
	typedef enum _SETTIMEOUT_ID
	{
		RELOAD_GAMER_LIST = 1,
	}SETTIMEOUT_ID;
	template<typename T> void SendCurrentSelectedUserServiceCommand(T* package);
	DECLARE_MESSAGE_MAP()
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GATEF_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	void FillClientView();


	bool isProcessRunning(const std::string& processName);
	// 实现
protected:
	CGateFDlgAutoProxy* m_pAutoProxy;
	HICON m_hIcon;

	BOOL CanExit();

	void ShowAllDlgInTab();
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	virtual void OnOK();
	virtual void OnCancel();
public:
	void SwitchToTab(int index);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void SetPaneBackgroundColor(UINT nIDResource, COLORREF color);
	void OnServiceCommand(UINT id);
	HANDLE find_process(const std::string& processName);
	afx_msg void OnTcnSelchangeTabMain(NMHDR* pNMHDR, LRESULT* pResult);
	void InitStatusBar();
	void SetStatusBar(UINT nIDResource, CString text);
	CTabCtrl m_tab_main;
	CRect m_tab_main_rect;
	std::unique_ptr<CGamesDlg> m_games_dlg;
	std::unique_ptr<CAntiCheatDlg> m_anticheat_dlg;
	std::unique_ptr<CPoliceDlg> m_polices_dlg;
	std::unique_ptr<CLogDlg> m_logs_dlg; 
	CStatusBar m_wndStatusBar;

};
