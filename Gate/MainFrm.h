
// MainFrm.h: CMainFrame 类的接口
//

#pragma once
#include "ClientView.h"
#include "OutputDlg.h"

class CMainFrame : public CMDIFrameWndEx
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame() noexcept;

// 特性
public:

// 操作
public:

// 重写
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = nullptr, CCreateContext* pContext = nullptr);
    virtual void SetStatusBar(UINT nIDResource, CString text);
    virtual void OnServiceCommand(UINT id);
	inline void CopyToClipboard(CString v)
	{
		std::string str_value = CT2A(v.GetBuffer());
		HGLOBAL hClip;
		if (OpenClipboard())
		{
			int len = str_value.length() + 1;
			EmptyClipboard();
			hClip = GlobalAlloc(GMEM_MOVEABLE, len);
			char* buff = (char*)GlobalLock(hClip);
			strcpy_s(buff, len, str_value.c_str());
			GlobalUnlock(hClip);
			SetClipboardData(CF_TEXT, hClip);
			CloseClipboard();
			GlobalFree(hClip);
		}
	}
// 实现
public:
	virtual ~CMainFrame();
    COutputDlg& GetOutputWindow() { return m_wndOutput; }
    CClientView& GetClientView() { return m_wndClientView; }
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
protected:  // 控件条嵌入成员
	CMFCMenuBar       m_wndMenuBar;
	CMFCToolBar      m_wndToolBar;
	CMFCStatusBar     m_wndStatusBar;
	CClientView       m_wndClientView;
	COutputDlg        m_wndOutput;
// 生成的消息映射函数
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowManager();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void OnApplicationLook(UINT id);
    
	DECLARE_MESSAGE_MAP()
    void InitStatusBar();
	BOOL CreateDockingWindows();
	void SetDockingWindowIcons(BOOL bHiColorIcons); 
public:
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnClose(); 
    void SetPaneBackgroundColor(UINT nIDResource, COLORREF color);
};


