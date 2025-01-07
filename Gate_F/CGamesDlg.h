#pragma once
#include "afxdialogex.h"
#include "ClientViewList.h"


// GamesDlg 对话框

class CGamesDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CGamesDlg)

public:
	CGamesDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CGamesDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_GAMES };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	afx_msg void OnCommand(UINT nID);
	afx_msg void OnQueryWindows();
	afx_msg void OnQueryDrivers();
	afx_msg void OnQueryShellCode();
	afx_msg void OnQueryScreenShot();
	afx_msg void OnExitGame();
	afx_msg void OnIpBan();
	afx_msg void OnMacBan();
	void OnRoleNameBan();
	afx_msg void OnIpWhiteAdd();
	afx_msg void OnMacWhiteAdd();
	DECLARE_MESSAGE_MAP()
	void OnRoleNameWhiteAdd();
	void OnWhiteOrBlackAdd(const CString& list_name, CListBox* m_current_list);
public:
	afx_msg void OnQueryProcess();
	template<typename T> void SendCurrentSelectedUserCommand(T* package);
	afx_msg void OnBnClickedOnlineGamerSearch();
	// 在线玩家列表
	CGateFDlgList m_list_games;
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	BOOL PreTranslateMessage(MSG* pMsg);
	void OnRefreshUsers();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	void AddUserToList(std::shared_ptr<CObserverClientImpl> client, const unsigned int session_id,
		const nlohmann::json& json_data, const CTime& tCur);
};
