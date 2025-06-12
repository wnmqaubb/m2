#pragma once
#include "afxdialogex.h"
#include <afxrich.h>
#include <vector>
#include "GateF.h"


// CObServerClient 对话框

class CObServerClientDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CObServerClientDlg)

public:
	CObServerClientDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CObServerClientDlg();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    CSize m_originalSize; // 添加这行
    std::vector<ControlLayoutInfo> m_layoutInfos;

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_LOG_OBSERVER_CLIENT };
#endif

	void AddLog(const CString& log_txt, COLORREF txt_color);
protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV 支持
	BOOL OnInitDialog() override;

	DECLARE_MESSAGE_MAP()
public:
	CRichEditCtrl m_log_observer_client;
};
