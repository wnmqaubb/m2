#pragma once


// ScreenShotDlg 对话框

class ScreenShotDlg : public CDialogEx
{
	DECLARE_DYNAMIC(ScreenShotDlg)

public:
	ScreenShotDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ScreenShotDlg();
	void ShowPicture();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum
	{
		IDD = IDD_SCREENSHOT_DLG
	};
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_screenshot_path;
	afx_msg void OnPaint();
	afx_msg void OnBnClickedBrowseFile();
	BOOL OpenFolderAndSelectFile(CString szPath);
	//	virtual BOOL OnInitDialog();
};
