
// MainFrm.cpp: CMainFrame 类的实现
//

#include "pch.h"
#include "framework.h"
#include "Gate.h"

#include "MainFrm.h"
#include <afxmenubar.h>
#include "CScrollingText.h"
#include <Psapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)
#define PSAPI_VERSION 1
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

// CMainFrame 构造/析构

CMainFrame::CMainFrame() noexcept
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
#ifdef VERSION_RED
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_RED);
#endif
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
#ifdef VERSION_BLUE
	SetWindowText(TEXT("及时雨定制版"));
#endif
#ifdef VERSION_RED
	SetWindowText(TEXT("及时雨鸿蒙版"));
#endif
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	CString strToolBarName;
	bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
	ASSERT(bNameValid);
	m_wndToolBar.SetWindowText(strToolBarName);

	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndMenuBar);
	DockPane(&m_wndToolBar);

#ifndef GATE_ADMIN  // 管理员版不显示滚动文本 
	{
		// 创建静态文本控件
		m_scrollingText = new CScrollingText();
		m_scrollingText->Create(_T(""), WS_CHILD | WS_VISIBLE | SS_LEFT, CRect(0, 0, 100, 30), this);

		//m_scrollingText->SetText(_T("动态创建的大字体静态文本，这是一段较长的文本用于测试滚动效果"));
		m_scrollingText->StartScrolling();

		//m_textToShow = TEXT("动态创建的大字体静态文本，这是一段较长的文本用于测试滚动效果");
		// 获取菜单栏（CMFCMenuBar）高度
		const CMFCMenuBar* pMenuBar = GetMenuBar();
		int menuBarHeight = 0;
		if (pMenuBar)
		{
			CClientDC dc(this);
			CRect menuBarRect;
			pMenuBar->GetClientRect(menuBarRect);
			menuBarHeight = menuBarRect.Height();
		}

		// 调整静态文本控件位置使其显示在菜单栏上方
		CRect clientRect;
		GetClientRect(&clientRect);
		m_scrollingText->MoveWindow(200, 0, clientRect.Width()-200, menuBarHeight+35);
		// 将静态文本控件置于菜单栏之上的 Z 顺序
		m_scrollingText->BringWindowToTop();
	}

	if (theApp.is_parent_gate) {
		SetTimer(TIMER_ID_CHILD_SERIVCE_ID, 1000 * 30, NULL);
	}
#endif

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
#ifndef GATE_ADMIN
	else if (nIDEvent == TIMER_ID_CHILD_SERIVCE_ID)
	{
		bool service_stoped = false;
		bool logic_server_stoped = false;

		// 已停止
		HANDLE existingMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, L"mtx_service");
		if (existingMutex != NULL) {
			if (!isProcessRunning("g_Service.exe")) {
				// 互斥体已存在，但是进程已经退出,尝试关闭和释放它			
				ReleaseMutex(existingMutex);
			}
			CloseHandle(existingMutex);
			
		}
		else {
			service_stoped = true;
		}

		HANDLE existingMutex1 = OpenMutex(MUTEX_ALL_ACCESS, FALSE, L"mtx_logic_server");
		if (existingMutex1 != NULL) {
			if (!isProcessRunning("g_LogicServer.exe")) {
				// 互斥体已存在，但是进程已经退出,尝试关闭和释放它			
				ReleaseMutex(existingMutex1);
			}
			CloseHandle(existingMutex1);
		}
		else {
			logic_server_stoped = true;
		}

		if (service_stoped || logic_server_stoped)
		{
			KillTimer(TIMER_ID_CHILD_SERIVCE_ID);
			// 进程已停止，尝试重启		
			if (service_stoped)
			{
				// 尝试关闭所有子进程
				HANDLE hProcess = find_process("g_LogicServer.exe");
				if (hProcess) {
					TerminateProcess(hProcess, 0);
					CloseHandle(hProcess);
				}
				while (existingMutex) {
					ReleaseMutex(existingMutex);
					CloseHandle(existingMutex);
				}
			}
			// 进程已停止，尝试重启		
			if (logic_server_stoped)
			{
				HANDLE hProcess = find_process("g_Service.exe");
				if (hProcess) {
					TerminateProcess(hProcess, 0);
					CloseHandle(hProcess);
				}
				while (existingMutex1) {
					ReleaseMutex(existingMutex1);
					CloseHandle(existingMutex1);
				}
			}
			theApp.OnServiceStop1();
			theApp.OnServiceStart();
			SetTimer(TIMER_ID_CHILD_SERIVCE_ID, 1000 * 30, NULL);
		}
	}
#endif
    CMDIFrameWndEx::OnTimer(nIDEvent);
}


void CMainFrame::OnClose()
{
    theApp.OnServiceStop();
    KillTimer(TIMER_ID_POLL_WORK_ID);
	if (theApp.is_parent_gate) {
		KillTimer(TIMER_ID_CHILD_SERIVCE_ID);
	}
    CMDIFrameWndEx::OnClose();
}

void CMainFrame::SetPaneBackgroundColor(UINT nIDResource, COLORREF color)
{
    int vecIndex = m_wndStatusBar.CommandToIndex(nIDResource);
    m_wndStatusBar.SetPaneBackgroundColor(vecIndex, color);
}

bool CMainFrame::isProcessRunning(const std::string& processName)
{
	DWORD processes[1024], needed;
	if (!EnumProcesses(processes, sizeof(processes), &needed))
	{
		return false;
	}

	int processCount = needed / sizeof(DWORD);
	for (int i = 0; i < processCount; i++)
	{
		if (processes[i] != 0)
		{
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);
			if (hProcess)
			{
				char processNameBuffer[MAX_PATH];
				DWORD size = sizeof(processNameBuffer);
				if (QueryFullProcessImageNameA(hProcess, 0, processNameBuffer, &size))
				{
					std::string fullProcessName(processNameBuffer);
					size_t lastSlash = fullProcessName.find_last_of("\\");
					if (lastSlash != std::string::npos)
					{
						std::string exeName = fullProcessName.substr(lastSlash + 1);
						if (exeName == processName)
						{
							CloseHandle(hProcess);
							return true;
						}
					}
				}
				CloseHandle(hProcess);
			}
		}
	}
	return false;
}


HANDLE CMainFrame::find_process(const std::string& processName)
{
	DWORD processes[1024], needed;
	if (!EnumProcesses(processes, sizeof(processes), &needed))
	{
		return nullptr;
	}

	int processCount = needed / sizeof(DWORD);
	for (int i = 0; i < processCount; i++)
	{
		if (processes[i] != 0)
		{
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);
			if (hProcess)
			{
				char processNameBuffer[MAX_PATH];
				DWORD size = sizeof(processNameBuffer);
				if (QueryFullProcessImageNameA(hProcess, 0, processNameBuffer, &size))
				{
					std::string fullProcessName(processNameBuffer);
					size_t lastSlash = fullProcessName.find_last_of("\\");
					if (lastSlash != std::string::npos)
					{
						std::string exeName = fullProcessName.substr(lastSlash + 1);
						if (exeName == processName)
						{
							CloseHandle(hProcess);
							return OpenProcess(PROCESS_TERMINATE, FALSE, processes[i]);
						}
					}
				}
				CloseHandle(hProcess);
			}
		}
	}
	return nullptr;
}
