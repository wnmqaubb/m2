<<<<<<< HEAD
﻿
// ChildFrm.h: CChildFrame 类的接口
//

#pragma once
#include "ConfigSettingSubView.h"

class CConfigSettingChildFrame : public CMDIChildWndEx
{
	DECLARE_DYNCREATE(CConfigSettingChildFrame)
public:
	CConfigSettingChildFrame() noexcept;

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
	virtual ~CConfigSettingChildFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
    CConfigSettingSubViewWnd m_wndSubViewWnd;
// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    BOOL CreateDockingWindows();
    afx_msg void OnClose();
};
=======
﻿
// ChildFrm.h: CChildFrame 类的接口
//

#pragma once
#include "ConfigSettingSubView.h"

class CConfigSettingChildFrame : public CMDIChildWndEx
{
	DECLARE_DYNCREATE(CConfigSettingChildFrame)
public:
	CConfigSettingChildFrame() noexcept;

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
	virtual ~CConfigSettingChildFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
    CConfigSettingSubViewWnd m_wndSubViewWnd;
// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    BOOL CreateDockingWindows();
    afx_msg void OnClose();
};
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9
