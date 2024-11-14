#pragma once
#include "afxdialogex.h"


// CAntiCheatDlg 对话框

class CAntiCheatDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAntiCheatDlg)

public:
	CAntiCheatDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CAntiCheatDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_ANTICHEAT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	DECLARE_MESSAGE_MAP()
public:
	CListBox* m_current_list;
	// 策略超时
	BOOL m_policy_timeout;
	// 检测代理
	BOOL m_detect_proxy;
	// 检测脱机
	BOOL m_detect_offline;
	// 检测加速
	BOOL m_detect_speed;
	// 检测虚拟机
	BOOL m_detect_virtual;
	// 检测心跳超时
	BOOL m_heartbeat_timeout;
	// 防封包攻击
	BOOL m_anti_cc;
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/); 
	afx_msg void OnAddToList(const CString& list_name);
	afx_msg void OnRemoveFromWhiteList();
	std::string GetFilePath();
	void SaveListToFile();
	void ReadFileAndShowList(const std::string& filePath);
	CListBox m_list_white_machine;
	virtual BOOL OnInitDialog();
	CListBox m_list_white_ip;
	CListBox m_list_white_rolename;
	CListBox m_list_black_machine;
	CListBox m_list_black_ip;
	CListBox m_list_black_rolename;
};
