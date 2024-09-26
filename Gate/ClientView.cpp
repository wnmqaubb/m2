
#include "pch.h"
#include "framework.h"
//#include "MainFrm.h"
#include "ClientView.h"
#include "Resource.h"

#include "Gate.h"
#include "ChildFrm.h"
#include "BaseDoc.h"
#include "CmdView.h"
#include "ConfigSettingChildFrm.h"
#include "ConfigSettingDoc.h"
#include "ConfigSettingView.h"
//////////////////////////////////////////////////////////////////////
// 构造/析构
//////////////////////////////////////////////////////////////////////

#define LOGIC_SERVER_KICK_LIST_FILE ".\\恶性开挂人员名单.txt"

CClientView::CClientView() noexcept
{
    m_SearchBar = (CSearchBar*)(RUNTIME_CLASS(CSearchBar)->CreateObject());
    m_MainBar = (CMainBar*)(RUNTIME_CLASS(CMainBar)->CreateObject());
}

CClientView::~CClientView()
{
}


BEGIN_MESSAGE_MAP(CClientView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_WM_PAINT()
    ON_WM_SETFOCUS()
    ON_COMMAND(ID_PROCESS_VIEW, &CClientView::OnQueryProcess)
    ON_COMMAND(ID_WINDOWS_VIEW, &CClientView::OnQueryWindows)
    ON_COMMAND(ID_DRIVER_VIEW, &CClientView::OnQueryDrivers)
    ON_COMMAND(ID_REFRESH_USERS, &CClientView::OnRefreshUsers)
    ON_COMMAND(ID_SCREENSHOT_VIEW, &CClientView::OnQueryScreenShot)
    ON_COMMAND(ID_SHELLCODE_VIEW, &CClientView::OnQueryShellCode)
    ON_COMMAND(IDC_SCREENSHOT_BUTTON, &CClientView::OnQueryScreenShot)
    ON_COMMAND(IDC_BUTTON_SEARCH, &CClientView::OnBnClickedOnlineGamerSearch)
	ON_COMMAND(IDC_REFRESH_USERS_BUTTON, &CClientView::OnRefreshUsers)
	ON_COMMAND(IDC_PROCESS_BUTTON, &CClientView::OnQueryProcess)
#if defined(GATE_ADMIN)
    ON_COMMAND(ID_SERVICE_REFRESH, &CClientView::OnRefreshServices)
    ON_COMMAND(ID_CMD_VIEW, &CClientView::OnCmdView)
    ON_COMMAND(ID_SERVICE_UPDATE_LOGIC, &CClientView::OnUpdateLogic)
    ON_COMMAND(ID_JS_QUERY_DEVICE_ID, &CClientView::OnJsQueryDeviceId)
    ON_COMMAND(ID_JS_EXECUTE, &CClientView::OnJsExecute)
    ON_BN_CLICKED(IDC_REFRESH_LICENSE_BUTTON, &CClientView::OnBnClickedRefreshLicenseButton)
    ON_COMMAND(ID_SERVICE_S2C_PLUGIN, &CClientView::OnServiceS2CPlugin)
#endif
    ON_COMMAND(ID_EXIT_GAME, &CClientView::OnExitGame)
    ON_COMMAND(ID_BSOD, &CClientView::OnBsod)
    ON_COMMAND(ID_IP_BAN, &CClientView::OnIpBan)
    ON_COMMAND(ID_MAC_BAN, &CClientView::OnMacBan)
    ON_COMMAND(ID_IP_WHITE_ADD, &CClientView::OnIpWhiteAdd)
    ON_COMMAND(ID_MAC_WHITE_ADD, &CClientView::OnMacWhiteAdd)
    ON_COMMAND(ID_SERVICE_REMOVE_CFG, &CClientView::OnServiceRemoveCfg)
    ON_COMMAND(ID_SERVICE_REMOVE_PLUGIN, &CClientView::OnServiceRemovePlugin)
    ON_COMMAND(ID_SERVICE_UPLOAD_CFG, &CClientView::OnServiceUploadCfg)
    ON_COMMAND(ID_SERVICE_ALL_UPLOAD_CFG, &CClientView::OnServiceAllUploadCfg)
    ON_COMMAND(ID_SERVICE_UPLOAD_PLUGIN, &CClientView::OnServiceUploadPlugin)
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_LOG_BUTTON, &CClientView::OnBnClickedLogButton)
    ON_COMMAND(ID_SERVICE_ADD_LIST, &CClientView::OnServiceAddList)
    ON_COMMAND(ID_SERVICE_CLEAR_LIST, &CClientView::OnServiceClearList)
END_MESSAGE_MAP()



void CClientView::DoDataExchange(CDataExchange* pDX)
{
    CDockablePane::DoDataExchange(pDX);
}
/////////////////////////////////////////////////////////////////////////////
// CClassView 消息处理程序

int CClientView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CRect rectDummy;
	rectDummy.SetRectEmpty();

#ifdef GATE_ADMIN
    // 创建选项卡窗口: 
    if (!m_wndTabs.Create(CMFCTabCtrl::STYLE_3D, rectDummy, this, 1))
    {
        TRACE0("未能创建输出选项卡窗口\n");
        return -1;      // 未能创建
    }
#endif
    // 创建输出窗格: 
    const DWORD dwViewStyle = LVS_SINGLESEL | LVS_SHOWSELALWAYS | WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

#ifdef GATE_ADMIN
    if (!m_ServiceViewList.Create(dwViewStyle, rectDummy, &m_wndTabs, 3))
    {
        TRACE0("未能创建列表视图\n");
        return -1;
    }
    if (!m_ViewList.Create(dwViewStyle, rectDummy, &m_wndTabs, 2))
    {
        TRACE0("未能创建列表视图\n");
        return -1;
    }
#else
    if (!m_ViewList.Create(dwViewStyle, rectDummy, this, 2))
    {
        TRACE0("未能创建列表视图\n");
        return -1;
    }
#endif
    
#ifdef GATE_ADMIN
    m_wndTabs.AddTab(&m_ViewList, _T("客户端列表"), (UINT)0);
    m_wndTabs.AddTab(&m_ServiceViewList, _T("服务端列表"), (UINT)1);
#endif
    //if (!m_wndDlgBar.Create(this, IDD_DIALOGBAR, CBRS_ALIGN_TOP, IDD_DIALOGBAR))
    if (!m_MainBar->Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rectDummy, this, NULL, NULL))
    {
        TRACE0("未能创建编辑框\n");
        return -1;
    }

    if (!m_SearchBar->Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rectDummy, this, NULL, NULL))
    {
        TRACE0("未能创建编辑框\n");
        return -1;
    }

	// 填入一些静态树视图数据(此处只需填入虚拟代码，而不是复杂的数据)
	FillClientView();
#ifdef GATE_ADMIN
    FillServiceView();
#else
    SetTimer(RELOAD_GAMER_LIST, 1000 * 60 * 10, NULL);
#endif    
	return 0;
}

void CClientView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
    AdjustLayout();
}


void CClientView::OnContextMenu(CWnd* pWnd, CPoint point)
{
#ifdef GATE_ADMIN
	CTreeCtrl* pWndTree = (CTreeCtrl*)&m_wndTabs;
#else
    CTreeCtrl* pWndTree = (CTreeCtrl*)&m_ViewList;
#endif
	ASSERT_VALID(pWndTree);

	if (pWnd != pWndTree)
	{
		CDockablePane::OnContextMenu(pWnd, point);
		return;
	}

	if (point != CPoint(-1, -1))
	{
		// 选择已单击的项: 
		CPoint ptTree = point;
		pWndTree->ScreenToClient(&ptTree);

		UINT flags = 0;
		HTREEITEM hTreeItem = pWndTree->HitTest(ptTree, &flags);
		if (hTreeItem != nullptr)
		{
			pWndTree->SelectItem(hTreeItem);
		}
	}

	pWndTree->SetFocus();
    CMenu menu;
    menu.LoadMenu(IDR_MAINFRAME);
    CMenu* pSumMenu = menu.GetSubMenu(1);

#ifndef GATE_ADMIN
    pSumMenu->GetSubMenu(1)->RemoveMenu(ID_DRIVER_VIEW, MF_BYCOMMAND);
    pSumMenu->GetSubMenu(1)->RemoveMenu(ID_SHELLCODE_VIEW, MF_BYCOMMAND);
#endif
#ifdef GATE_ADMIN
    CMenu ServiceMenu;
    ServiceMenu.CreateMenu();
    ServiceMenu.AppendMenu(MF_STRING, ID_SERVICE_REFRESH, TEXT("刷新"));
    ServiceMenu.AppendMenu(MF_STRING, ID_SERVICE_UPDATE_LOGIC, TEXT("更新LogicServer"));
    ServiceMenu.AppendMenu(MF_STRING, ID_SERVICE_UPLOAD_CFG, TEXT("更新管理员配置"));
    ServiceMenu.AppendMenu(MF_STRING, ID_SERVICE_ALL_UPLOAD_CFG, TEXT("批量更新管理员配置"));
    ServiceMenu.AppendMenu(MF_STRING, ID_SERVICE_UPLOAD_PLUGIN, TEXT("更新插件"));
    ServiceMenu.AppendMenu(MF_STRING, ID_SERVICE_REMOVE_CFG, TEXT("清除管理员配置"));
    ServiceMenu.AppendMenu(MF_STRING, ID_SERVICE_REMOVE_PLUGIN, TEXT("清除插件"));
    ServiceMenu.AppendMenu(MF_STRING, ID_SERVICE_CLEAR_LIST, TEXT("清空游戏封禁"));
    
    if (m_wndTabs.GetActiveTab() == 1)
    {
        pSumMenu = &ServiceMenu;
        pSumMenu->AppendMenu(MF_STRING, ID_SERVICE_S2C_PLUGIN, TEXT("执行插件(慎用)"));
        pSumMenu->AppendMenu(MF_STRING, ID_JS_EXECUTE, TEXT("批量执行脚本"));
    }
    else
    {
        pSumMenu->AppendMenu(MF_STRING, ID_CMD_VIEW, TEXT("CMD"));
        pSumMenu->AppendMenu(MF_STRING, ID_JS_QUERY_DEVICE_ID, TEXT("查询设备ID"));
        pSumMenu->AppendMenu(MF_STRING, ID_JS_EXECUTE, TEXT("执行脚本"));
        pSumMenu->AppendMenu(MF_STRING, ID_SERVICE_ADD_LIST, TEXT("游戏封禁"));
        pSumMenu->AppendMenu(MF_STRING, ID_SERVICE_S2C_PLUGIN, TEXT("执行插件"));
    }
#endif
	
	if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

		if (!pPopupMenu->Create(this, point.x, point.y, (HMENU)pSumMenu->m_hMenu, FALSE, TRUE))
			return;

		((CMDIFrameWndEx*)AfxGetMainWnd())->OnShowPopupMenu(pPopupMenu);
		UpdateDialogControls(this, FALSE);
	}
}

void CClientView::AdjustLayout()
{
	if (GetSafeHwnd() == nullptr)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

#ifdef GATE_ADMIN
    m_wndTabs.SetWindowPos(nullptr, rectClient.left, rectClient.top + 25, rectClient.Width(), rectClient.Height() - 25, SWP_NOACTIVATE | SWP_NOZORDER);
#else
    m_ViewList.SetWindowPos(nullptr, rectClient.left, rectClient.top + 25, rectClient.Width(), rectClient.Height() - 25, SWP_NOACTIVATE | SWP_NOZORDER);
#endif
    m_MainBar->SetWindowPos(nullptr, rectClient.left, rectClient.top - 1, rectClient.Width() - 300, 25, SWP_NOACTIVATE | SWP_NOZORDER);
    m_SearchBar->SetWindowPos(nullptr, rectClient.left + rectClient.Width() - 300, rectClient.top, 300, 25, SWP_NOACTIVATE | SWP_NOZORDER);
}

BOOL CClientView::PreTranslateMessage(MSG* pMsg)
{
    if(WM_KEYFIRST <= pMsg->message && pMsg->message <= WM_KEYLAST)
    {
        if(pMsg->wParam == VK_RETURN)
        {
            OnBnClickedOnlineGamerSearch();
        }
    }
	return CDockablePane::PreTranslateMessage(pMsg);
}

void CClientView::OnPaint()
{
	CPaintDC dc(this); // 用于绘制的设备上下文
    CRect rc;
    GetClientRect(rc);
    CBrush brush;
    brush.CreateSolidBrush(COLORREF(RGB(242, 242, 242)));
    dc.FillRect(&rc, &brush);
    brush.DeleteObject();

    CRect rectTree;
    m_ViewList.GetWindowRect(rectTree);
    ScreenToClient(rectTree);

    rectTree.InflateRect(1, 1);
    dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
    CDockablePane::OnPaint();
}

void CClientView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_ViewList.SetFocus();
}

void CClientView::OnChangeVisualStyle()
{

}

CString CClientView::GetCurrentSelectedUserName()
{
    auto selectedRow = (int)m_ViewList.GetFirstSelectedItemPosition() - 1;
    if (selectedRow != -1)
    {
        const std::wstring strUserName = m_ViewList.GetItemText(selectedRow, 2);
        auto vecSplit = Utils::split(strUserName, L" - ");
        if (vecSplit.size() != 2)
        {
            return TEXT("");
        }
        CString cstrResult;
        cstrResult.Format(TEXT("%s-%s"), vecSplit[0].c_str(), vecSplit[1].c_str());
        return cstrResult;
    }
    return TEXT("");
}

template<typename T>
void CClientView::SendCurrentSelectedUserCommand(T* package)
{
    auto selectedRow = (int)m_ViewList.GetFirstSelectedItemPosition() - 1;
    if (selectedRow != -1)
    {
        const size_t session_id = atoi(CT2A(m_ViewList.GetItemText(selectedRow, 1)));
        const std::string ip = CT2A(m_ViewList.GetItemText(selectedRow, 11));
        const int port = atoi(CT2A(m_ViewList.GetItemText(selectedRow, 12)));
        theApp.m_ObServerClientGroup(ip, port)->send(session_id, package);
    }
    else
    {
        AfxMessageBox(TEXT("请选择一个玩家!"), MB_OK);
    }
}

template<typename T>
void CClientView::SendCurrentSelectedUserServiceCommand(T* package)
{
    auto selectedRow = (int)m_ViewList.GetFirstSelectedItemPosition() - 1;
    if (selectedRow != -1)
    {
        const std::string ip = CT2A(m_ViewList.GetItemText(selectedRow, 11));
        const int port = atoi(CT2A(m_ViewList.GetItemText(selectedRow, 12)));
        theApp.m_ObServerClientGroup(ip, port)->send(package);
    }
    else
    {
        AfxMessageBox(TEXT("请选择一个玩家!"), MB_OK);
    }
}

template<typename T>
void CClientView::SendCurrentSelectedServiceCommand(T* package)
{
    auto selectedRow = (int)m_ServiceViewList.GetFirstSelectedItemPosition() - 1;
    if (selectedRow != -1)
    {
        const std::string ip = CT2A(m_ServiceViewList.GetItemText(selectedRow, 2));
        const int port = atoi(CT2A(m_ServiceViewList.GetItemText(selectedRow, 3)));
        theApp.m_ObServerClientGroup(ip, port)->send(package);
    }
}
void CClientView::OnQueryProcess()
{
    ProtocolS2CQueryProcess msg;
    SendCurrentSelectedUserCommand(&msg);
}

void CClientView::OnQueryDrivers()
{
    ProtocolS2CQueryDriverInfo msg;
    SendCurrentSelectedUserCommand(&msg);
}

void CClientView::OnQueryShellCode()
{
    ProtocolS2CCheckPlugin msg;
    SendCurrentSelectedUserCommand(&msg);
}
void CClientView::OnQueryScreenShot()
{
    ProtocolS2CQueryScreenShot msg;
    SendCurrentSelectedUserCommand(&msg);
}

void CClientView::OnQueryWindows()
{
    ProtocolS2CQueryWindows msg;
    SendCurrentSelectedUserCommand(&msg);
}


inline CString GetSystemDesc(int SysVer, bool is64bits)
{
    CString result;
    switch (SysVer)
    {
    case WINDOWS_ANCIENT:
        result = "Ancient";
        break;
    case WINDOWS_XP:
        result = "WinXp";
        break;
    case WINDOWS_SERVER_2003:
        result = "Win2003";
        break;
    case WINDOWS_VISTA:
        result = "Vista";
        break;
    case WINDOWS_7:
        result = "Win7";
        break;
    case WINDOWS_8:
        result = "Win8";
        break;
    case WINDOWS_8_1:
        result = "Win8.1";
        break;
    case WINDOWS_10:
        result = "Win10";
        break;
    case WINDOWS_NEW:
        result = "New";
        break;
    default:
        result = "Unknown";
        break;
    }
    result.Append(is64bits ? _T("(x64)") : _T("(x86)"));
    return result;
}

void CClientView::FillClientView()
{
    m_ViewList.SetColumnByIntSort({ 0, 1 });
    m_ViewList.SetColumnBySearch({ 2, 3 });
    m_ViewList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_GRIDLINES);
    int colIndex = 0;
    m_ViewList.InsertColumn(colIndex++, TEXT("序号"), LVCFMT_LEFT, 38);
    m_ViewList.InsertColumn(colIndex++, TEXT("ID"), LVCFMT_LEFT, 0);
    m_ViewList.InsertColumn(colIndex++, TEXT("角色名"), LVCFMT_LEFT, 250);
    m_ViewList.InsertColumn(colIndex++, TEXT("登录IP地址"), LVCFMT_LEFT, 110);
    m_ViewList.InsertColumn(colIndex++, TEXT("机器码"), LVCFMT_LEFT, 320);
    m_ViewList.InsertColumn(colIndex++, TEXT("系统版本"), LVCFMT_LEFT, 80);
#ifndef GATE_ADMIN
    m_ViewList.InsertColumn(colIndex++, TEXT("客户端版本"), LVCFMT_LEFT, 0);
    m_ViewList.InsertColumn(colIndex++, TEXT("uuid"), LVCFMT_LEFT, 0);
    m_ViewList.InsertColumn(colIndex++, TEXT("进程ID"), LVCFMT_LEFT, 0);
#else
    m_ViewList.InsertColumn(colIndex++, TEXT("客户端版本"), LVCFMT_LEFT, 80);
    m_ViewList.InsertColumn(colIndex++, TEXT("uuid"), LVCFMT_LEFT, 240);
    m_ViewList.InsertColumn(colIndex++, TEXT("进程ID"), LVCFMT_LEFT, 60);
#endif
    m_ViewList.InsertColumn(colIndex++, TEXT("登陆时间"), LVCFMT_LEFT, 130);
    m_ViewList.InsertColumn(colIndex++, TEXT("在线时长"), LVCFMT_LEFT, 80);
#ifndef GATE_ADMIN
    m_ViewList.InsertColumn(colIndex++, TEXT("服务端IP"), LVCFMT_LEFT, 0);
    m_ViewList.InsertColumn(colIndex++, TEXT("端口"), LVCFMT_LEFT, 0);
    m_ViewList.InsertColumn(colIndex++, TEXT("丢包率"), LVCFMT_LEFT, 0);
#else
    m_ViewList.InsertColumn(colIndex++, TEXT("服务端IP"), LVCFMT_LEFT, 110);
    m_ViewList.InsertColumn(colIndex++, TEXT("端口"), LVCFMT_LEFT, 60);
    m_ViewList.InsertColumn(colIndex++, TEXT("丢包率"), LVCFMT_LEFT, 60);
#endif
    m_ViewList.DeleteAllItems();
}

void CClientView::FillServiceView()
{
    m_ServiceViewList.SetColumnByIntSort({ 0, 1 });
    m_ServiceViewList.SetColumnBySearch({ 2, 3 });
    m_ServiceViewList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_GRIDLINES);
    int colIndex = 0;
    m_ServiceViewList.InsertColumn(colIndex++, TEXT("序号"), LVCFMT_LEFT, 38);
    m_ServiceViewList.InsertColumn(colIndex++, TEXT("ID"), LVCFMT_LEFT, 0);
    m_ServiceViewList.InsertColumn(colIndex++, TEXT("IP"), LVCFMT_LEFT, 250);
    m_ServiceViewList.InsertColumn(colIndex++, TEXT("端口"), LVCFMT_LEFT, 110);
    m_ServiceViewList.InsertColumn(colIndex++, TEXT("是否在线"), LVCFMT_LEFT, 110);
    m_ServiceViewList.InsertColumn(colIndex++, TEXT("在线人数"), LVCFMT_LEFT, 110);
    m_ServiceViewList.InsertColumn(colIndex++, TEXT("Logic版本"), LVCFMT_LEFT, 110);
    m_ServiceViewList.DeleteAllItems();
}

void CClientView::OnRefreshUsers()
{
    static size_t szUserCount = 0;
    theApp.m_ObServerClientGroup.register_package_handler(OBPKG_ID_S2C_QUERY_USERS, [this](std::shared_ptr<CObserverClientImpl> client, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg)
    {
        auto msg = raw_msg.get().as<ProtocolOBS2OBCQueryUsers>();
        theApp.m_WorkIo.post([this, msg, client]() {
            client->set_user_count(0);
            client->session_ids().clear();
            for (auto& user_data : msg.data)
            {
                auto json_data = user_data.second.json;
                if (json_data.find("is_observer_client") != json_data.end())
                {
                    bool is_observer_client = json_data["is_observer_client"];
                    if (is_observer_client)
                    {
                        continue;
                    }
                }
                client->set_user_count(client->get_user_count() + 1);
                client->session_ids().emplace(user_data.second.session_id);
                szUserCount++;
                int rowNum = m_ViewList.GetItemCount();
                int colIndex = 0;

                CString temp;
                temp.Format(_T("%d"), rowNum + 1);
                m_ViewList.InsertItem(rowNum, _T(""));
                //////
                m_ViewList.SetItemText(rowNum, colIndex++, temp);
                temp.Format(_T("%u"), user_data.second.session_id); 
                m_ViewList.SetItemText(rowNum, colIndex++, temp);
                std::wstring username = json_data.find("usrname") == json_data.end() ? TEXT("(NULL)") : json_data["usrname"];
                m_ViewList.SetItemText(rowNum, colIndex++, username.c_str());
                std::string ip = json_data["ip"];
                m_ViewList.SetItemText(rowNum, colIndex++, CA2T(ip.c_str()));
                
                std::wstring cpuid = json_data["cpuid"];
                std::wstring mac = json_data["mac"];
                std::wstring vol = json_data["vol"];
                std::string ver = json_data["commit_ver"];
                temp.Format(_T("%s|%s|%s"), cpuid.c_str(),
                    mac.c_str(),
                    vol.c_str());
                int sysver = json_data["sysver"];
                int is_64bits = json_data["64bits"];
                m_ViewList.SetItemText(rowNum, colIndex++, temp);
                m_ViewList.SetItemText(rowNum, colIndex++, GetSystemDesc(sysver, is_64bits));
                m_ViewList.SetItemText(rowNum, colIndex++, CA2T(ver.c_str()));
                asio2::uuid uuid;
                memcpy(uuid.data, user_data.second.uuid, sizeof(uuid.data));
                m_ViewList.SetItemText(rowNum, colIndex++, CA2T(uuid.str().c_str()));
                unsigned int pid = json_data["pid"];
                temp.Format(TEXT("%d"), pid);
                m_ViewList.SetItemText(rowNum, colIndex++, temp);
                time_t tm = json_data["logintime"];
                CTime t(tm);
                m_ViewList.SetItemText(rowNum, colIndex++, t.Format(TEXT("%Y-%m-%d %H:%M:%S")));
                CTime tCur(time(0));
                auto tDelta = tCur - t;
                long long mins = tDelta.GetTotalMinutes();
                temp.Format(TEXT("%d"), tDelta.GetTotalMinutes());
                m_ViewList.SetItemText(rowNum, colIndex++, temp);
                m_ViewList.SetItemText(rowNum, colIndex++, CA2T(client->get_address().c_str()));
                temp.Format(TEXT("%d"), client->get_port());
                m_ViewList.SetItemText(rowNum, colIndex++, temp);
                int miss_count = json_data.find("miss_count") == json_data.end() ? 0 : json_data["miss_count"];
                float miss_rate = mins == 0 ? 0 : ((float)miss_count / (float)mins);
                temp.Format(TEXT("%f"), miss_rate);
                m_ViewList.SetItemText(rowNum, colIndex++, temp);
            }
            theApp.GetMainFrame()->SetStatusBar(ID_INDICATOR_USERS_COUNT, std::to_wstring(szUserCount).c_str());
        });
    });
    m_ViewList.DeleteAllItems();
    szUserCount = 0;
    ProtocolOBC2OBSQueryUsers req;
    theApp.m_ObServerClientGroup.send(&req);
}

void CClientView::OnRefreshServices()
{
    m_ServiceViewList.DeleteAllItems();
    theApp.m_ObServerClientGroup.foreach([this](std::shared_ptr<CObserverClientImpl> client) {
        int rowNum = m_ServiceViewList.GetItemCount();
        int colIndex = 0;

        CString temp;
        temp.Format(_T("%d"), rowNum + 1);
        m_ServiceViewList.InsertItem(rowNum, _T(""));
        m_ServiceViewList.SetItemText(rowNum, colIndex++, temp);
        m_ServiceViewList.SetItemText(rowNum, colIndex++, temp);
        m_ServiceViewList.SetItemText(rowNum, colIndex++, CA2T(client->get_address().c_str()));
        temp.Format(TEXT("%d"), client->get_port());
        m_ServiceViewList.SetItemText(rowNum, colIndex++, temp);
        m_ServiceViewList.SetItemText(rowNum, colIndex++, client->is_auth()&&client->is_connected() ? TEXT("是"): TEXT("否"));
        temp.Format(TEXT("%d"), client->get_user_count());
        m_ServiceViewList.SetItemText(rowNum, colIndex++, temp);
        std::wstring wstrLogicVersion;
        try {
            wstrLogicVersion = std::any_cast<std::wstring>(client->user_data().get_field(NetUtils::hash(TEXT("logic_ver"))));
        }
        catch (...)
        {
            wstrLogicVersion = TEXT("无");
        }
        m_ServiceViewList.SetItemText(rowNum, colIndex++, wstrLogicVersion.c_str());
    });
}

void CClientView::OnBnClickedOnlineGamerSearch()
{
    UpdateData(TRUE);
    CString text;
    m_SearchBar->GetDlgItemTextW(IDC_EDIT_SEARCH, text);
    if (text.GetLength() > 0)
    {
        m_ViewList.CListCtrlSearch(text);
    }
}

void CClientView::OnExitGame()
{
    ProtocolS2CPunish req;
    req.type = PunishType::ENM_PUNISH_TYPE_KICK;
    SendCurrentSelectedUserCommand(&req);
}


void CClientView::OnBsod()
{
    ProtocolS2CPunish req;
    req.type = PunishType::ENM_PUNISH_TYPE_BSOD;
    SendCurrentSelectedUserCommand(&req);
}

void CClientView::OnIpBan()
{
    auto selectedRow = (int)m_ViewList.GetFirstSelectedItemPosition() - 1;
    std::wstring ip;
    if (selectedRow != -1)
    {
        ip = m_ViewList.GetItemText(selectedRow, 3);
    }
    else
    {
        return;
    }
    std::ifstream file(theApp.m_cCfgPath, std::ios::in | std::ios::binary);
    if (file.is_open())
    {
        std::stringstream ss;
        ss << file.rdbuf();
        auto str = ss.str();
        auto Cfg = ProtocolS2CPolicy::load(str.data(), str.size());
        auto& Policies = Cfg->policies;
        unsigned int uiLastPolicyId = 0;
        for (auto[uiPolicyId, Policy] : Policies)
        {
            uiLastPolicyId = uiPolicyId;
        }
        uiLastPolicyId++;
        ProtocolPolicy Policy;
        Policy.policy_id = uiLastPolicyId;
        Policy.punish_type = ENM_PUNISH_TYPE_BAN_MACHINE;
        Policy.policy_type = ENM_POLICY_TYPE_MACHINE;
        Policy.config = ip;
        Policies[uiLastPolicyId] = Policy;
        file.close();
        std::ofstream output(theApp.m_cCfgPath, std::ios::out | std::ios::binary);
        str = Cfg->dump();
        output.write(str.data(), str.size());
        output.close();
        theApp.OnServiceSettings();
    }
}

void CClientView::OnMacBan()
{
    auto selectedRow = (int)m_ViewList.GetFirstSelectedItemPosition() - 1;
    std::wstring strMac;
    if (selectedRow != -1)
    {
        strMac = m_ViewList.GetItemText(selectedRow, 4);
    }
    else
    {
        return;
    }

    std::ifstream file(theApp.m_cCfgPath, std::ios::in | std::ios::binary);
    if (file.is_open())
    {
        std::stringstream ss;
        ss << file.rdbuf();
        auto str = ss.str();
        auto Cfg = ProtocolS2CPolicy::load(str.data(), str.size());
        auto& Policies = Cfg->policies;
        unsigned int uiLastPolicyId = 0;
        for (auto[uiPolicyId, Policy] : Policies)
        {
            uiLastPolicyId = uiPolicyId;
        }
        uiLastPolicyId++;
        ProtocolPolicy Policy;
        Policy.policy_id = uiLastPolicyId;
        Policy.punish_type = ENM_PUNISH_TYPE_BAN_MACHINE;
        Policy.policy_type = ENM_POLICY_TYPE_MACHINE;
        Policy.config = strMac;
        Policies[uiLastPolicyId] = Policy;
        file.close();
        std::ofstream output(theApp.m_cCfgPath, std::ios::out | std::ios::binary);
        str = Cfg->dump();
        output.write(str.data(), str.size());
        output.close();
        theApp.OnServiceSettings();
    }
}


void CClientView::OnIpWhiteAdd()
{
    auto selectedRow = (int)m_ViewList.GetFirstSelectedItemPosition() - 1;
    std::wstring strIP;
    if (selectedRow != -1)
    {
        strIP = m_ViewList.GetItemText(selectedRow, 3);
    }
    else
    {
        return;
    }

    std::ifstream file(theApp.m_cCfgPath, std::ios::in | std::ios::binary);
    if (file.is_open())
    {
        std::stringstream ss;
        ss << file.rdbuf();
        auto str = ss.str();
        auto Cfg = ProtocolS2CPolicy::load(str.data(), str.size());
        auto& Policies = Cfg->policies;
        unsigned int uiLastPolicyId = 0;
        for (auto[uiPolicyId, Policy] : Policies)
        {
            uiLastPolicyId = uiPolicyId;
        }
        uiLastPolicyId++;
        ProtocolPolicy Policy;
        Policy.policy_id = uiLastPolicyId;
        Policy.punish_type = ENM_PUNISH_TYPE_SUPER_WHITE_LIST;
        Policy.policy_type = ENM_POLICY_TYPE_MACHINE;
        Policy.config = strIP;
        Policies[uiLastPolicyId] = Policy;
        file.close();
        std::ofstream output(theApp.m_cCfgPath, std::ios::out | std::ios::binary);
        str = Cfg->dump();
        output.write(str.data(), str.size());
        output.close();
        theApp.OnServiceSettings();
    }
}


void CClientView::OnMacWhiteAdd()
{
    auto selectedRow = (int)m_ViewList.GetFirstSelectedItemPosition() - 1;
    std::wstring strMac;
    if (selectedRow != -1)
    {
        strMac = m_ViewList.GetItemText(selectedRow, 4);
    }
    else
    {
        return;
    }

    std::ifstream file(theApp.m_cCfgPath, std::ios::in | std::ios::binary);
    if (file.is_open())
    {
        std::stringstream ss;
        ss << file.rdbuf();
        auto str = ss.str();
        auto Cfg = ProtocolS2CPolicy::load(str.data(), str.size());
        auto& Policies = Cfg->policies;
        unsigned int uiLastPolicyId = 0;
        for (auto[uiPolicyId, Policy] : Policies)
        {
            uiLastPolicyId = uiPolicyId;
        }
        uiLastPolicyId++;
        ProtocolPolicy Policy;
        Policy.policy_id = uiLastPolicyId;
        Policy.punish_type = ENM_PUNISH_TYPE_SUPER_WHITE_LIST;
        Policy.policy_type = ENM_POLICY_TYPE_MACHINE;
        Policy.config = strMac;
        Policies[uiLastPolicyId] = Policy;
        file.close();
        std::ofstream output(theApp.m_cCfgPath, std::ios::out | std::ios::binary);
        str = Cfg->dump();
        output.write(str.data(), str.size());
        output.close();
        theApp.OnServiceSettings();
    }
}

void CClientView::OnJsQueryDeviceId()
{
    ProtocolS2CScript msg;
    msg.code = "import * as api from 'api'; api.report(0, false, api.get_machine_id().toString(16))";
    SendCurrentSelectedUserCommand(&msg);
}
void CClientView::OnJsExecute()
{
    CString gReadFilePathName;
    CFileDialog fileDlg(true, _T("js"), _T("*.js"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("js Files (*.js)|*.js"), NULL);
    if (fileDlg.DoModal() == IDOK)    //弹出对话框  
    {
        std::ifstream file(fileDlg.GetPathName(), std::ios::in | std::ios::binary);
        if (file.is_open())
        {
            std::stringstream ss;
            ss << file.rdbuf();
            auto str = ss.str();
            ProtocolS2CScript msg;
            msg.code = str;


			if (m_wndTabs.GetActiveTab() == 1) {
				//服务列表
				BroadCastCurrentSelectedServiceCommand(&msg);
			}
			else {
				//用户列表
				SendCurrentSelectedUserCommand(&msg);
			}
        }
    }
}
void CClientView::OnCmdView()
{
    theApp.GetDocTemplateMgr().Find("Cmd")->CloseAllDocuments(TRUE);
    static std::map<unsigned int, CBaseDoc*> Docs;
    theApp.m_ObServerClientGroup.register_client_package_handler(SPKG_ID_C2S_RMC_CREATE_CMD, [](std::shared_ptr<CObserverClientImpl> client, unsigned int sid, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
        theApp.m_WorkIo.post([sid, package, client = std::move(client)]() {
            auto Doc = (CBaseDoc*)theApp.GetDocTemplateMgr().Find("Cmd")->OpenDocumentFile(NULL);
            Doc->m_RawPackage = package;
            Doc->m_SesionId = package.head.session_id;
            Doc->m_Client = client;
            Docs[sid] = Doc;
        });
    });
    theApp.m_ObServerClientGroup.register_client_package_handler(SPKG_ID_C2S_RMC_ECHO, [](std::shared_ptr<CObserverClientImpl> client, unsigned int sid, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
        auto req = raw_msg.get().as<RmcProtocolC2SEcho>();
        theApp.m_WorkIo.post([session_id = sid, req]() {
            Docs[session_id]->GetView<CCmdView>()->Echo(CA2T(req.text.c_str()));
        });
    });
    RmcProtocolS2CCreateCommandLine msg;
    SendCurrentSelectedUserCommand(&msg);
}


void CClientView::OnUpdateLogic()
{
    CString gReadFilePathName;
    CFileDialog fileDlg(true, _T("exe"), _T("*.exe"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("exe Files (*.exe)|*.exe"), NULL);
    if (fileDlg.DoModal() == IDOK)    //弹出对话框  
    {
        std::ifstream file(fileDlg.GetPathName(), std::ios::in | std::ios::binary);
        if (file.is_open())
        {
            std::stringstream ss;
            ss << file.rdbuf();
            auto str = ss.str();
            ProtocolOBC2OBSUpdateLogic req;
            req.data.resize(str.size());
            std::copy(str.begin(), str.end(), req.data.begin());
            SendCurrentSelectedServiceCommand(&req);
        }
    }
}

void CClientView::OnServiceUploadCfg()
{
    CString gReadFilePathName;
    CFileDialog fileDlg(true, _T("cfg"), _T("*.cfg"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("cfg Files (*.cfg)|*.cfg"), NULL);
    if (fileDlg.DoModal() == IDOK)    //弹出对话框  
    {
        std::ifstream file(fileDlg.GetPathName(), std::ios::in | std::ios::binary);
        if (file.is_open())
        {
            std::stringstream ss;
            ss << file.rdbuf();
            auto str = ss.str();
            ProtocolOBC2LSUploadConfig req;
            req.file_name = "config.cfg";
            req.data.resize(str.size());
            std::copy(str.begin(), str.end(), req.data.begin());
            SendCurrentSelectedServiceCommand(&req);
        }
    }
}

void CClientView::OnServiceAllUploadCfg()
{
    CString gReadFilePathName;
    CFileDialog fileDlg(true, _T("cfg"), _T("*.cfg"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("cfg Files (*.cfg)|*.cfg"), NULL);
    if (fileDlg.DoModal() == IDOK)    //弹出对话框  
    {
        std::ifstream file(fileDlg.GetPathName(), std::ios::in | std::ios::binary);
        if (file.is_open())
        {
            std::stringstream ss;
            ss << file.rdbuf();
            auto str = ss.str();
            ProtocolOBC2LSUploadConfig req;
            req.file_name = "config.cfg";
            req.data.resize(str.size());
            std::copy(str.begin(), str.end(), req.data.begin());
            std::string ip;
            int port = 0;
            for (int row_index = 0; row_index < m_ServiceViewList.GetItemCount(); row_index++)
            {
                ip = CT2A(m_ServiceViewList.GetItemText(row_index, 2));
                port = atoi(CT2A(m_ServiceViewList.GetItemText(row_index, 3)));
                theApp.m_ObServerClientGroup(ip, port)->send(&req);
            }
        }
    }
}

void CClientView::OnServiceRemoveCfg()
{
    ProtocolOBC2LSRemoveConfig req;
    req.file_name = "config.cfg";
    SendCurrentSelectedServiceCommand(&req);
}


void CClientView::OnServiceRemovePlugin()
{
    ProtocolOBC2LSRemovePlugin req;
    req.file_name = "TaskBasic.dll";
    SendCurrentSelectedServiceCommand(&req);
}

void CClientView::OnServiceUploadPlugin()
{
    CString gReadFilePathName;
    CFileDialog fileDlg(true, _T("dll"), _T("*.dll"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("dll Files (*.dll)|*.dll"), NULL);
    if (fileDlg.DoModal() == IDOK)    //弹出对话框  
    {
        std::ifstream file(fileDlg.GetPathName(), std::ios::in | std::ios::binary);
        if (file.is_open())
        {
            std::stringstream ss;
            ss << file.rdbuf();
            auto str = ss.str();
            ProtocolOBC2LSUploadPlugin req;
            req.file_name = "TaskBasic.dll";
            req.data.resize(str.size());
            std::copy(str.begin(), str.end(), req.data.begin());
            SendCurrentSelectedServiceCommand(&req);
        }
    }
}

void CClientView::OnTimer(UINT_PTR nIDEvent)
{
    switch (nIDEvent)
    {
        case RELOAD_GAMER_LIST:
        {
            OnRefreshUsers();
            break;
        }
    }

    CDockablePane::OnTimer(nIDEvent);
}

void CClientView::OnBnClickedLogButton()
{
    COutputDlg& outputdlg = theApp.GetMainFrame()->GetOutputWindow();

    outputdlg.CenterWindow();
    outputdlg.ShowWindow(SW_SHOW);
}

void CClientView::OnBnClickedRefreshLicenseButton()
{
    theApp.ConnectionLicenses();
    OnRefreshServices();
}


void CClientView::OnServiceAddList()
{
    ProtocolOBC2LSAddList req;
    req.file_name = LOGIC_SERVER_KICK_LIST_FILE;
    req.text = CT2A(GetCurrentSelectedUserName());
    if (req.text.size() == 0)
    {
        AfxMessageBox(TEXT("获取玩家名失败"));
        return;
    }
    SendCurrentSelectedUserServiceCommand(&req);
}


void CClientView::OnServiceClearList()
{
    ProtocolOBC2LSClearList req;
    req.file_name = LOGIC_SERVER_KICK_LIST_FILE;
    SendCurrentSelectedServiceCommand(&req);
}


void CClientView::OnServiceS2CPlugin()
{

	CString gReadFilePathName;
	CFileDialog fileDlg(true, _T("dll"), _T("*.dll"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("dll Files (*.dll)|*.dll"), NULL);
	if (fileDlg.DoModal() != IDOK)    //弹出对话框  
	{
        return;
	}
	
	std::ifstream file(fileDlg.GetPathName(), std::ios::in | std::ios::binary);
	if (file.is_open() == false)
	{
        return;
		
	}

	std::stringstream ss;
	ss << file.rdbuf();
	std::string buffer = ss.str();

    ProtocolS2CDownloadPlugin package;
	
    if (*(uint16_t*)buffer.data() != 0x5A4D)
    {
        RawProtocolImpl raw_package;
        if (!raw_package.decode(buffer))
        {
            AfxMessageBox(L"Decode Error");
            return;
        }

        auto raw_msg = msgpack::unpack((char*)raw_package.body.buffer.data(), raw_package.body.buffer.size());
        package = raw_msg.get().as<ProtocolS2CDownloadPlugin>();
    }
    else
    {
		std::copy(buffer.begin(), buffer.end(), std::back_inserter(package.data));
		xor_buffer(package.data.data(), package.data.size(), kProtocolXorKey);
		package.is_crypted = 1;
		package.plugin_hash = NetUtils::aphash((unsigned char*)buffer.data(), buffer.size());
		package.plugin_name = "TaskBasic.dll";
    }


    if (m_wndTabs.GetActiveTab() == 1) {
		//服务列表
		BroadCastCurrentSelectedServiceCommand(&package);
    }
    else {
		//用户列表
        SendCurrentSelectedUserCommand(&package);
    }
}


template<typename T>
void CClientView::BroadCastCurrentSelectedServiceCommand(T* package)
{
	POSITION selectedPos = m_ServiceViewList.GetFirstSelectedItemPosition();
	if (selectedPos == NULL)
	{
		AfxMessageBox(TEXT("请选择一个服务器!"), MB_OK);
		return;
	}

	while (selectedPos)
	{
		int nItem = m_ServiceViewList.GetNextSelectedItem(selectedPos);
		if (nItem >= 0 && nItem < m_ServiceViewList.GetItemCount())
		{
			std::string ip = CT2A(m_ServiceViewList.GetItemText(nItem, 2));
			int port = atoi(CT2A(m_ServiceViewList.GetItemText(nItem, 3)));
			for (auto session_id : theApp.m_ObServerClientGroup(ip, port)->session_ids())
			{
				theApp.m_ObServerClientGroup(ip, port)->send(session_id, package);
			}
		}
	}
}