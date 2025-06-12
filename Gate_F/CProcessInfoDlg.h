#pragma once
#include "afxdialogex.h"
#include <vector>
#include "GateF.h"


// CProcessInfoDlg 对话框
class CGateFDlg;

class CProcessInfoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CProcessInfoDlg)

public:
	CProcessInfoDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CProcessInfoDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PROCESSINFO };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
    CSize m_originalSize; // 添加这行
    std::vector<ControlLayoutInfo> m_layoutInfos;

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_list_process_info;
	CListCtrl m_list_module_info;
	CListCtrl m_list_thread_info;
	CListCtrl m_list_directory_info;
	CTabCtrl m_tab_process_info_main;
	CRect m_tab_main_rect;
	virtual BOOL OnInitDialog();
	void OnTcnSelchangeTabMain(NMHDR* pNMHDR, LRESULT* pResult);
	void ShowAllDlgInTab();
	void LoadFile(const std::wstring& file_path);
	void OnContextMenu(CWnd* /* pWnd */, CPoint point);
	void FillViewList();
	void FillViewListData();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void OnListItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	void OnProcessNameBan();
	void OnProcessPathBan();
	void OnGetGameUserFile();
	void InitModuleWindowView();
	void InitThreadWindowView();
	void InitDirectoryWindowView();
	void FillModuleWindow(const std::vector<ProtocolModuleInfo>& module);
	void FillThreadWindow(const std::map<uint32_t, ProtocolThreadInfo>& threads);
	void FillDirectoryWindow(const std::vector<ProtocolDirectoryInfo>& directories);
	void OnCopyValue();
	void OnCopyModulePath();
	RawProtocolImpl package;
	ProtocolC2SQueryProcess processes;
};
