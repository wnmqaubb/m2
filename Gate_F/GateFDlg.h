
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
	template<typename T> void SendCurrentSelectedServiceCommand(T* package);
	DECLARE_MESSAGE_MAP()
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GATEF_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	void FillClientView();
	void FillServiceView();


// 实现
protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
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
	afx_msg void OnRefreshServices();
	afx_msg void OnUpdateLogic();
	afx_msg void OnCmdView();
	afx_msg void OnJsQueryDeviceId();
	afx_msg void OnJsExecute();
	afx_msg void OnServiceRemoveCfg();
	afx_msg void OnServiceRemovePlugin();
	afx_msg void OnServiceUploadCfg();
	afx_msg void OnServiceAllUploadCfg();
	afx_msg void OnServiceUploadPlugin();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedLogButton();
	afx_msg void OnBnClickedRefreshLicenseButton();
	afx_msg void OnServiceAddList();
	afx_msg void OnServiceClearList();
	afx_msg void OnServiceS2CPlugin();
	afx_msg void OnTcnSelchangeTabMain(NMHDR* pNMHDR, LRESULT* pResult);
	template<typename T> void BroadCastCurrentSelectedServiceCommand(T* package);
	CTabCtrl m_tab_main;
	CRect m_tab_main_rect;
	std::unique_ptr<CGamesDlg> m_games_dlg;
	std::unique_ptr<CAntiCheatDlg> m_anticheat_dlg;
	std::unique_ptr<CPoliceDlg> m_polices_dlg;
	std::unique_ptr<CLogDlg> m_logs_dlg;

};
