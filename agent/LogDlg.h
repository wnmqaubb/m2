#pragma once


// LogDlg 对话框

class LogDlg : public CDialogEx
{
	DECLARE_DYNAMIC(LogDlg)

public:
	LogDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~LogDlg();
    void log(LPCTSTR format, ...);
	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_LOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
    
public:
    CRichEditCtrl m_RichEditLog;
	SCROLLINFO Info = {0};
    CButton m_VerboseCheck;
    afx_msg void OnBnClickedButtonClearLog();
};
