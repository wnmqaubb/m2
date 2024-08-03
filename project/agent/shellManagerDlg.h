#pragma once
#include "MFCUtil.h"


// shell_manager 对话框

class ShellManagerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(ShellManagerDlg)

public:
	ShellManagerDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ShellManagerDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SHELL_MANAGER_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedFlush();
	afx_msg void OnBnClickedCheckedAll();
	afx_msg void OnBnClickedInvert();
	afx_msg void OnBnClickedSearch();
	void EnumShellDirectory(std::function<void(std::filesystem::path, bool, std::string)> callback);
	void ScanShellDirectory();
	MFCUtil mfcutil;
	bool m_bHit = false;
	int m_itemSel;
	CListCtrl m_shell_file_list;
	CString m_search_text;
	CString m_search_text_old;
	virtual BOOL OnInitDialog();
	afx_msg void OnLvnColumnclickShellFileList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchangedShellFileList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickShellFileList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedSave();
	afx_msg void OnBnClickedShellAction();
	virtual void OnOK();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	BOOL m_shell_auto;
	afx_msg void OnBnClickedShellAuto();
};
