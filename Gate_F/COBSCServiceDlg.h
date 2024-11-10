#pragma once
#include "afxdialogex.h"


// COBSCService 对话框

class COBSCServiceDlg : public CDialogEx
{
	DECLARE_DYNAMIC(COBSCServiceDlg)

public:
	COBSCServiceDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~COBSCServiceDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_LOG_OBC_SERVICE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	BOOL OnInitDialog() override;
	void AddLog(const CString& log_txt, COLORREF txt_color);

	DECLARE_MESSAGE_MAP()
public:
	CRichEditCtrl m_log_observer_client;
};
