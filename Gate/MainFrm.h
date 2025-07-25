
// MainFrm.h: CMainFrame 类的接口
//

#pragma once
#include "ClientView.h"
#include "OutputDlg.h"
#include "CScrollingText.h"
#include <cstring>

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
        // 转换为ANSI字符串
        std::string str_value = CT2A(v.GetBuffer());
        v.ReleaseBuffer();  // 确保释放CString缓冲区

        // 仅当剪贴板打开成功时执行
        if (!OpenClipboard())
        {
            TRACE("无法打开剪贴板! 错误代码: %d\n", GetLastError());
            return;
        }

        // 准备分配内存
        const size_t len = str_value.length() + 1;
        HGLOBAL hClip = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, len);
        if (!hClip)
        {
            TRACE("内存分配失败! 需要 %zu 字节\n", len);
            CloseClipboard();
            return;
        }

        // 获取可写入指针
        char* buff = static_cast<char*>(GlobalLock(hClip));
        if (!buff)
        {
            TRACE("内存锁定失败! 错误代码: %d\n", GetLastError());
            GlobalFree(hClip);
            CloseClipboard();
            return;
        }

        // 安全拷贝数据
        const errno_t err = strcpy_s(buff, len, str_value.c_str());
        GlobalUnlock(hClip);

        if (err != 0)
        {
            TRACE("字符串拷贝失败! 错误代码: %d\n", err);
            GlobalFree(hClip);
        }
        else
        {
            EmptyClipboard();
            if (!SetClipboardData(CF_TEXT, hClip))
            {
                TRACE("设置剪贴板数据失败! 错误代码: %d\n", GetLastError());
                GlobalFree(hClip);
            }
            // 成功: 系统将接管内存所有权, 无需释放
        }

        CloseClipboard();
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
	CMFCToolBar       m_wndToolBar;
	CMFCStatusBar     m_wndStatusBar;
	CClientView       m_wndClientView;
	COutputDlg        m_wndOutput;
	CScrollingText*   m_scrollingText;
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
	HICON m_hIcon;
public:
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnClose(); 
    void SetPaneBackgroundColor(UINT nIDResource, COLORREF color);
	bool isProcessRunning(const std::string& processName);
	HANDLE find_process(const std::string& processName);
	void SetScrollText(CString scrollingText) {
		m_scrollingText->SetText(scrollingText);
	}

};


