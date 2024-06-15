#pragma once

/////////////////////////////////////////////////////////////////////////////
// COutputList 窗口

enum OutputWndLogType
{
    ObserverClientLog,
    ServiceLog,
    LogicServerLog
};
class COutputList : public CListBox
{
    // 构造
public:
    COutputList() noexcept;
    void AddString(LPCTSTR lpszItem)
    {
        int nCount = GetCount();
        if (nCount > 10000)
            ResetContent();
        CListBox::AddString(lpszItem);
        SetCurSel(nCount);
    }
    // 实现
public:
    virtual ~COutputList();

protected:
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnEditCopy();
    afx_msg void OnEditClear();

    DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////

// OutputDlg 对话框

class COutputDlg : public CDialogEx
{
	DECLARE_DYNAMIC(COutputDlg)

public:
	COutputDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~COutputDlg();
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_OUTPUT_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
    void AdjustHorzScroll(CListBox& wndListBox);

    CMFCTabCtrl	m_wndTabs;
    COutputList m_wndObserverClientLog;
    COutputList m_wndServiceLog;
    COutputList m_wndLogicServerLog;

    // 实现
public:
    void LogPrint(int type, LPCTSTR format, ...);
    void UpdateFonts();
protected:
	DECLARE_MESSAGE_MAP()
    virtual BOOL OnInitDialog();
public:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
};

