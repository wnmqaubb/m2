
// ChildFrm.cpp: CChildFrame 类的实现
//

#include "pch.h"
#include "framework.h"
#include "Gate.h"

#include "ProcessChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CChildFrame

IMPLEMENT_DYNCREATE(CProcessChildFrame, CMDIChildWndEx)

BEGIN_MESSAGE_MAP(CProcessChildFrame, CMDIChildWndEx)
	ON_WM_CREATE()
END_MESSAGE_MAP()

// CChildFrame 构造/析构

CProcessChildFrame::CProcessChildFrame() noexcept
{
	// TODO: 在此添加成员初始化代码
}

CProcessChildFrame::~CProcessChildFrame()
{
}


BOOL CProcessChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改 CREATESTRUCT cs 来修改窗口类或样式
	if( !CMDIChildWndEx::PreCreateWindow(cs) )
		return FALSE;
	return TRUE;
}

// CChildFrame 诊断

#ifdef _DEBUG
void CProcessChildFrame::AssertValid() const
{
	CMDIChildWndEx::AssertValid();
}

void CProcessChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWndEx::Dump(dc);
}
#endif //_DEBUG

// CChildFrame 消息处理程序

int CProcessChildFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
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

    DockPane(&m_wndProcessSubViewWnd);
	return 0;
} 



BOOL CProcessChildFrame::CreateDockingWindows()
{
    if (!m_wndProcessSubViewWnd.Create(_T("属性"), this, CRect(0, 0, 400, 200), TRUE, ID_VIEW_OUTPUTWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI,
        AFX_CBRS_REGULAR_TABS,
        AFX_CBRS_RESIZE))
    {
        TRACE0("未能属性窗口\n");
        return FALSE;
    }
    m_wndProcessSubViewWnd.SetMinSize(CSize(400, 200));
    return TRUE;
}