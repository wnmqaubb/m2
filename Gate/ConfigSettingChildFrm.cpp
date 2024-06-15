<<<<<<< HEAD
﻿
// ChildFrm.cpp: CChildFrame 类的实现
//

#include "pch.h"
#include "framework.h"
#include "Gate.h"

#include "ConfigSettingChildFrm.h"
#include "ConfigSettingDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CChildFrame

IMPLEMENT_DYNCREATE(CConfigSettingChildFrame, CMDIChildWndEx)

BEGIN_MESSAGE_MAP(CConfigSettingChildFrame, CMDIChildWndEx)
	ON_WM_CREATE()
    ON_WM_CLOSE()
END_MESSAGE_MAP()

// CChildFrame 构造/析构

CConfigSettingChildFrame::CConfigSettingChildFrame() noexcept
{
	// TODO: 在此添加成员初始化代码
}

CConfigSettingChildFrame::~CConfigSettingChildFrame()
{
}


BOOL CConfigSettingChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改 CREATESTRUCT cs 来修改窗口类或样式
	if( !CMDIChildWndEx::PreCreateWindow(cs) )
		return FALSE;
	return TRUE;
}

// CChildFrame 诊断

#ifdef _DEBUG
void CConfigSettingChildFrame::AssertValid() const
{
	CMDIChildWndEx::AssertValid();
}

void CConfigSettingChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWndEx::Dump(dc);
}
#endif //_DEBUG


// CChildFrame 消息处理程序

int CConfigSettingChildFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIChildWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

    EnableDocking(CBRS_ALIGN_ANY);

    // 创建停靠窗口
    if (!CreateDockingWindows())
    {
        TRACE0("未能创建停靠窗口\n");
        return -1;
    }

    DockPane(&m_wndSubViewWnd);
	return 0;
} 



BOOL CConfigSettingChildFrame::CreateDockingWindows()
{
    if (!m_wndSubViewWnd.Create(_T("属性"), this, CRect(0, 0, 260, 200), TRUE, ID_VIEW_OUTPUTWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI,
        AFX_CBRS_REGULAR_TABS,
        AFX_CBRS_RESIZE))
    {
        TRACE0("未能属性窗口\n");
        return FALSE;
    }
    m_wndSubViewWnd.SetMinSize(CSize(260, 200));
    return TRUE;
}

void CConfigSettingChildFrame::OnClose()
{
    ((CConfigSettingDoc*)theApp.m_ConfigDoc)->DoFileSave();
    CMDIChildWndEx::OnClose();
}
=======
﻿
// ChildFrm.cpp: CChildFrame 类的实现
//

#include "pch.h"
#include "framework.h"
#include "Gate.h"

#include "ConfigSettingChildFrm.h"
#include "ConfigSettingDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CChildFrame

IMPLEMENT_DYNCREATE(CConfigSettingChildFrame, CMDIChildWndEx)

BEGIN_MESSAGE_MAP(CConfigSettingChildFrame, CMDIChildWndEx)
	ON_WM_CREATE()
    ON_WM_CLOSE()
END_MESSAGE_MAP()

// CChildFrame 构造/析构

CConfigSettingChildFrame::CConfigSettingChildFrame() noexcept
{
	// TODO: 在此添加成员初始化代码
}

CConfigSettingChildFrame::~CConfigSettingChildFrame()
{
}


BOOL CConfigSettingChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改 CREATESTRUCT cs 来修改窗口类或样式
	if( !CMDIChildWndEx::PreCreateWindow(cs) )
		return FALSE;
	return TRUE;
}

// CChildFrame 诊断

#ifdef _DEBUG
void CConfigSettingChildFrame::AssertValid() const
{
	CMDIChildWndEx::AssertValid();
}

void CConfigSettingChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWndEx::Dump(dc);
}
#endif //_DEBUG


// CChildFrame 消息处理程序

int CConfigSettingChildFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIChildWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

    EnableDocking(CBRS_ALIGN_ANY);

    // 创建停靠窗口
    if (!CreateDockingWindows())
    {
        TRACE0("未能创建停靠窗口\n");
        return -1;
    }

    DockPane(&m_wndSubViewWnd);
	return 0;
} 



BOOL CConfigSettingChildFrame::CreateDockingWindows()
{
    if (!m_wndSubViewWnd.Create(_T("属性"), this, CRect(0, 0, 260, 200), TRUE, ID_VIEW_OUTPUTWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI,
        AFX_CBRS_REGULAR_TABS,
        AFX_CBRS_RESIZE))
    {
        TRACE0("未能属性窗口\n");
        return FALSE;
    }
    m_wndSubViewWnd.SetMinSize(CSize(260, 200));
    return TRUE;
}

void CConfigSettingChildFrame::OnClose()
{
    ((CConfigSettingDoc*)theApp.m_ConfigDoc)->DoFileSave();
    CMDIChildWndEx::OnClose();
}
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9
