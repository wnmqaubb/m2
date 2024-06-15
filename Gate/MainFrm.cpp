
// MainFrm.cpp: CMainFrame 类的实现
//

#include "pch.h"
#include "framework.h"
#include "Gate.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)

const int  iMaxUserToolbars = 10;
const UINT uiFirstUserToolBarId = AFX_IDW_CONTROLBAR_FIRST + 40;
const UINT uiLastUserToolBarId = uiFirstUserToolBarId + iMaxUserToolbars - 1;

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndEx)
    ON_WM_CREATE()
    ON_COMMAND(ID_WINDOW_MANAGER, &CMainFrame::OnWindowManager)
    ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &CMainFrame::OnToolbarCreateNew)
    ON_COMMAND_RANGE(ID_INDICATOR_SERVER_STAUS, ID_INDICATOR_USERS_COUNT, NULL)
    ON_WM_SETTINGCHANGE()
    ON_WM_TIMER()
    ON_WM_CLOSE()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // 状态行指示器
    ID_INDICATOR_SERVER_STAUS,
    ID_INDICATOR_USERS_COUNT,
};

#define TIMER_ID_POLL_WORK_ID 1
// CMainFrame 构造/析构

CMainFrame::CMainFrame() noexcept
{
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	BOOL bNameValid;

	CMDITabInfo mdiTabParams;
	mdiTabParams.m_style = CMFCTabCtrl::STYLE_3D_ONENOTE; // 其他可用样式...
	mdiTabParams.m_bActiveTabCloseButton = TRUE;      // 设置为 FALSE 会将关闭按钮放置在选项卡区域的右侧
	mdiTabParams.m_bTabIcons = FALSE;    // 设置为 TRUE 将在 MDI 选项卡上启用文档图标
	mdiTabParams.m_bAutoColor = TRUE;    // 设置为 FALSE 将禁用 MDI 选项卡的自动着色
	mdiTabParams.m_bDocumentMenu = TRUE; // 在选项卡区域的右边缘启用文档菜单
	EnableMDITabbedGroups(TRUE, mdiTabParams);

	if (!m_wndMenuBar.Create(this))
	{
		TRACE0("未能创建菜单栏\n");
		return -1;      // 未能创建
	}
	
	m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);

	// 防止菜单栏在激活时获得焦点
	CMFCPopupMenu::SetForceMenuFocus(FALSE);

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("未能创建工具栏\n");
		return -1;      // 未能创建
	}
#ifdef GATE_ADMIN
    CString m_cstrTitle;
    GetWindowText(m_cstrTitle);
    SetWindowText(m_cstrTitle + TEXT(" - 管理员版"));
#endif

	CString strToolBarName;
	bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
	ASSERT(bNameValid);
	m_wndToolBar.SetWindowText(strToolBarName);

	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndMenuBar);
	DockPane(&m_wndToolBar);


	// 启用 Visual Studio 2005 样式停靠窗口行为
	CDockingManager::SetDockingMode(DT_SMART);
	// 启用 Visual Studio 2005 样式停靠窗口自动隐藏行为
	EnableAutoHidePanes(CBRS_ALIGN_ANY);


	// 创建停靠窗口
	if (!CreateDockingWindows())
	{
		TRACE0("未能创建停靠窗口\n");
		return -1;
	}

    InitStatusBar();

    /*m_wndOutput.EnableDocking(CBRS_ALIGN_ANY);*/
    //DockPane(&m_wndOutput);

	/*m_wndClientView.EnableDocking(CBRS_ALIGN_ANY);*/
	DockPane(&m_wndClientView);
	
	

	// 基于持久值设置视觉管理器和样式
	OnApplicationLook(theApp.m_nAppLook);

	// 启用增强的窗口管理对话框
	EnableWindowsDialog(ID_WINDOW_MANAGER, ID_WINDOW_MANAGER, TRUE);

	// 将文档名和应用程序名称在窗口标题栏上的顺序进行交换。这
	// 将改进任务栏的可用性，因为显示的文档名带有缩略图。
	ModifyStyle(0, FWS_PREFIXTITLE);

    SetTimer(TIMER_ID_POLL_WORK_ID, 200, NULL);
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	m_strTitle.LoadString(IDS_MAINFRAME_TITLE);
	//标题设置
	cs.style &= ~FWS_ADDTOTITLE;
	cs.style &= ~FWS_PREFIXTITLE;
	if( !CMDIFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	return TRUE;
}

BOOL CMainFrame::CreateDockingWindows()
{
	BOOL bNameValid;
    
	// 创建类视图
	CString strClassView;
	bNameValid = strClassView.LoadString(IDS_CLASS_VIEW);
	ASSERT(bNameValid);
	if (!m_wndClientView.Create(strClassView, this, CRect(0, 0, 200, 700), TRUE, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_TOP | CBRS_FLOAT_MULTI,
		AFX_CBRS_REGULAR_TABS,
		AFX_CBRS_RESIZE))
	{
		TRACE0("未能创建“类视图”窗口\n");
		return FALSE; // 未能创建
	}
    
    m_wndOutput.Create(IDD_OUTPUT_DIALOG);

	SetDockingWindowIcons(theApp.m_bHiColorIcons);
	return TRUE;
}

void CMainFrame::SetDockingWindowIcons(BOOL bHiColorIcons)
{

    HICON hClassViewIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_CLASS_VIEW_HC : IDI_CLASS_VIEW), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
    m_wndClientView.SetIcon(hClassViewIcon, FALSE);

    HICON hOutputBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_OUTPUT_WND_HC : IDI_OUTPUT_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
    m_wndOutput.SetIcon(hOutputBarIcon, FALSE);

	UpdateMDITabbedBarsIcons();
}

// CMainFrame 诊断

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CMainFrame 消息处理程序

void CMainFrame::OnWindowManager()
{
	ShowWindowsDialog();
}


LRESULT CMainFrame::OnToolbarCreateNew(WPARAM wp,LPARAM lp)
{
	return CMDIFrameWndEx::OnToolbarCreateNew(wp, lp);
}

void CMainFrame::OnApplicationLook(UINT id)
{
	CWaitCursor wait;

	theApp.m_nAppLook = id;
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));

	m_wndOutput.UpdateFonts();
	RedrawWindow(nullptr, nullptr, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

	theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}


void CMainFrame::OnServiceCommand(UINT id)
{
    int vecIndex = m_wndStatusBar.CommandToIndex(ID_INDICATOR_SERVER_STAUS);
    switch (id)
    {
    case ID_SERVICE_START:
    {
        SetStatusBar(ID_INDICATOR_SERVER_STAUS, _T("已启动")); 
        SetPaneBackgroundColor(ID_INDICATOR_SERVER_STAUS, RGB(0, 255, 0));
        break;
    }
    case ID_SERVICE_STOP:
    {
        SetStatusBar(ID_INDICATOR_SERVER_STAUS, _T("已停止"));
        SetPaneBackgroundColor(ID_INDICATOR_SERVER_STAUS, RGB(255, 0, 0));
        break;
    }
    default:
        break;
    }
}

void CMainFrame::InitStatusBar()
{
    if (!m_wndStatusBar.Create(this))
    {
        TRACE0("未能创建状态栏\n");
        return;
    }
    m_wndStatusBar.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT));
    int a, b;
    a = m_wndStatusBar.CommandToIndex(ID_INDICATOR_SERVER_STAUS);
    b = m_wndStatusBar.CommandToIndex(ID_INDICATOR_USERS_COUNT);
    m_wndStatusBar.SetPaneWidth(a, 250);
    m_wndStatusBar.SetPaneWidth(b, 180);
    SetStatusBar(ID_INDICATOR_SERVER_STAUS, _T("已停止"));
    SetStatusBar(ID_INDICATOR_USERS_COUNT, _T("0"));
}

BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext)
{
	// 基类将执行真正的工作

	if (!CMDIFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}

	return TRUE;
}

void CMainFrame::SetStatusBar(UINT nIDResource, CString text)
{
    int index = m_wndStatusBar.CommandToIndex(nIDResource);
    CString strPrefix;
    if(index != -1)
    {
        strPrefix.LoadString(nIDResource);
        CString strText;
        strText = strPrefix + _T(":") + text;
        m_wndStatusBar.SetPaneText(index, strText);
    }
}

void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    if (nIDEvent == TIMER_ID_POLL_WORK_ID)
    {
        theApp.m_WorkIo.poll_one();
    }
    CMDIFrameWndEx::OnTimer(nIDEvent);
}


void CMainFrame::OnClose()
{
    theApp.OnServiceStop();
    KillTimer(TIMER_ID_POLL_WORK_ID);
    CMDIFrameWndEx::OnClose();
}

void CMainFrame::SetPaneBackgroundColor(UINT nIDResource, COLORREF color)
{
    int vecIndex = m_wndStatusBar.CommandToIndex(nIDResource);
    m_wndStatusBar.SetPaneBackgroundColor(vecIndex, color);
}