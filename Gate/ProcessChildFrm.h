
// ChildFrm.h: CChildFrame 类的接口
//

#pragma once
#include "ProcessSubView.h"

class CProcessChildFrame : public CMDIChildWndEx
{
	DECLARE_DYNCREATE(CProcessChildFrame)
public:
	CProcessChildFrame() noexcept;

// 特性
protected:
	CSplitterWndEx m_wndSplitter;
public:

// 操作
public:

// 重写
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// 实现
public:
	virtual ~CProcessChildFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
    CProcessSubViewWnd m_wndProcessSubViewWnd;
// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    BOOL CreateDockingWindows();
};
