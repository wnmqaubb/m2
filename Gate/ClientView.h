<<<<<<< HEAD
﻿
#pragma once

#include "ClientViewList.h"
#include "SearchBar.h"
#include "MainBar.h"

class CClientView : public CDockablePane
{
public:
	CClientView() noexcept;
	virtual ~CClientView();

	void AdjustLayout();
	void OnChangeVisualStyle();
    CString GetCurrentSelectedUserName();
    typedef enum _SETTIMEOUT_ID
    {
        RELOAD_GAMER_LIST = 1,
    }SETTIMEOUT_ID;

    template<typename T> void SendCurrentSelectedUserCommand(T* package);
    template<typename T> void SendCurrentSelectedUserServiceCommand(T* package);
    template<typename T> void SendCurrentSelectedServiceCommand(T* package);
    CMainBar* GetMainBar() { return m_MainBar; }
protected:
    CMFCTabCtrl	m_wndTabs;
	CClientViewList m_ViewList;
    CMainBar* m_MainBar;
    CViewList m_ServiceViewList;
    CSearchBar* m_SearchBar;
	void FillClientView();
    void FillServiceView();
// 重写
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnQueryProcess();
    afx_msg void OnQueryWindows();
    afx_msg void OnRefreshUsers();
    afx_msg void OnRefreshServices();
    afx_msg void OnQueryDrivers();
    afx_msg void OnQueryShellCode();
    afx_msg void OnQueryScreenShot();
    afx_msg void OnBnClickedOnlineGamerSearch();
    afx_msg void OnUpdateLogic();
    afx_msg void OnCmdView();
    afx_msg void OnExitGame();
    afx_msg void OnBsod();
    afx_msg void OnIpBan();
    afx_msg void OnMacBan();
    afx_msg void OnIpWhiteAdd();
    afx_msg void OnMacWhiteAdd();
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
    template<typename T> void BroadCastCurrentSelectedServiceCommand(T* package);
};

=======
﻿
#pragma once

#include "ClientViewList.h"
#include "SearchBar.h"
#include "MainBar.h"

class CClientView : public CDockablePane
{
public:
	CClientView() noexcept;
	virtual ~CClientView();

	void AdjustLayout();
	void OnChangeVisualStyle();
    CString GetCurrentSelectedUserName();
    typedef enum _SETTIMEOUT_ID
    {
        RELOAD_GAMER_LIST = 1,
    }SETTIMEOUT_ID;

    template<typename T> void SendCurrentSelectedUserCommand(T* package);
    template<typename T> void SendCurrentSelectedUserServiceCommand(T* package);
    template<typename T> void SendCurrentSelectedServiceCommand(T* package);
    CMainBar* GetMainBar() { return m_MainBar; }
protected:
    CMFCTabCtrl	m_wndTabs;
	CClientViewList m_ViewList;
    CMainBar* m_MainBar;
    CViewList m_ServiceViewList;
    CSearchBar* m_SearchBar;
	void FillClientView();
    void FillServiceView();
// 重写
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnQueryProcess();
    afx_msg void OnQueryWindows();
    afx_msg void OnRefreshUsers();
    afx_msg void OnRefreshServices();
    afx_msg void OnQueryDrivers();
    afx_msg void OnQueryShellCode();
    afx_msg void OnQueryScreenShot();
    afx_msg void OnBnClickedOnlineGamerSearch();
    afx_msg void OnUpdateLogic();
    afx_msg void OnCmdView();
    afx_msg void OnExitGame();
    afx_msg void OnBsod();
    afx_msg void OnIpBan();
    afx_msg void OnMacBan();
    afx_msg void OnIpWhiteAdd();
    afx_msg void OnMacWhiteAdd();
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
    template<typename T> void BroadCastCurrentSelectedServiceCommand(T* package);
};

>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9
