
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
#include <Lightbone/xorstr.hpp>
#include <asio2/base/error.hpp>
#include <asio2/http/request.hpp>
#include <asio2/bho/beast/http/field.hpp>
#include <asio2/bho/beast/http/verb.hpp>
#include <asio2/http/detail/http_util.hpp>
#include <asio2/http/http_client.hpp>
//////////////////////////////////////////////////////////////////////
// 构造/析构
//////////////////////////////////////////////////////////////////////

#define LOGIC_SERVER_KICK_LIST_FILE ".\\恶性开挂人员名单.txt"
// 在视图类cpp文件顶部添加
std::mutex CClientView::g_dataMutex;
std::vector<std::unique_ptr<UserData>> CClientView::g_allUserData;

CClientView::CClientView() noexcept
{
    m_SearchBar = (CSearchBar*)(RUNTIME_CLASS(CSearchBar)->CreateObject());
    m_MainBar = (CMainBar*)(RUNTIME_CLASS(CMainBar)->CreateObject());
}

CClientView::~CClientView()
{
#ifdef GATE_ADMIN
    KillTimer(theApp.TIMER_ID_SYNC_LICENSE);
#else
    KillTimer(theApp.TIMER_ID_RELOAD_GAMER_LIST);
#endif
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
    ON_BN_CLICKED(IDC_BUTTON_SYNC_LICENSE, &CClientView::OnBnClickedSyncLicenseButton)
    ON_COMMAND(ID_SERVICE_S2C_PLUGIN, &CClientView::OnServiceS2CPlugin)
#endif
    ON_COMMAND(ID_EXIT_GAME, &CClientView::OnExitGame)
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
    ON_MESSAGE(WM_UPDATE_USER_LIST, OnUpdateUserList)
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
    if (!m_ViewList.Create(dwViewStyle | LVS_OWNERDATA, rectDummy, &m_wndTabs, IDC_LIST_USERS))
    {
        TRACE0("未能创建列表视图\n");
        return -1;
    }
#else
    if (!m_ViewList.Create(dwViewStyle | LVS_OWNERDATA, rectDummy, this, 2))
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
    m_ViewList.OnDoubleClick = [this]() {
        OnQueryProcess();
    };
    SetTimer(theApp.TIMER_ID_SYNC_LICENSE, 1000 * 60 * 60 * 1, NULL);
#else
    SetTimer(theApp.TIMER_ID_RELOAD_GAMER_LIST, 1000 * 60 * 10, NULL);
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
	menu.LoadMenu(IDR_MENU_USERS_RIGHT);
	CMenu* pSumMenu = menu.GetSubMenu(0);

#ifndef GATE_ADMIN
    pSumMenu->DeleteMenu(1, MF_BYPOSITION);
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
        theApp.m_ObServerClientGroup.get_observer_client(ip, port)->send(session_id, package);
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
        theApp.m_ObServerClientGroup.get_observer_client(ip, port)->send(package);
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
        theApp.m_ObServerClientGroup.get_observer_client(ip, port)->send(package);
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

void CClientView::FillClientView()
{
    m_ViewList.SetColumnByIntSort({ 0, 1, 8, 10 });
    m_ViewList.SetColumnBySearch({ 2, 3 });
    m_ViewList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
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
    m_ViewList.InsertColumn(colIndex++, TEXT("登陆时间"), LVCFMT_LEFT, 120);
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
    m_ServiceViewList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
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

//void CClientView::OnRefreshUsers()
//{
//    static size_t szUserCount = 0;
//    theApp.m_ObServerClientGroup.register_package_handler(OBPKG_ID_S2C_QUERY_USERS, [this](std::shared_ptr<CObserverClientImpl> client, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg)
//    {
//        auto msg = raw_msg.get().as<ProtocolOBS2OBCQueryUsers>();
//        theApp.m_WorkIo.post([this, msg, client]() {
//            client->set_user_count(0);
//            client->session_ids().clear();
//            for (auto& user_data : msg.data)
//            {
//                auto json_data = user_data.second.json;
//                if (json_data.find("is_observer_client") != json_data.end())
//                {
//                    bool is_observer_client = json_data["is_observer_client"];
//                    if (is_observer_client)
//                    {
//                        continue;
//                    }
//                }
//                client->set_user_count(client->get_user_count() + 1);
//                client->session_ids().emplace(user_data.second.session_id);
//                szUserCount++;
//                int rowNum = m_ViewList.GetItemCount();
//                int colIndex = 0;
//
//                CString temp;
//                temp.Format(_T("%d"), rowNum + 1);
//                m_ViewList.InsertItem(rowNum, _T(""));
//                //////
//                m_ViewList.SetItemText(rowNum, colIndex++, temp);
//                temp.Format(_T("%u"), user_data.second.session_id); 
//                m_ViewList.SetItemText(rowNum, colIndex++, temp);
//                std::wstring username = json_data.find("usrname") == json_data.end() ? TEXT("(NULL)") : json_data["usrname"];
//                m_ViewList.SetItemText(rowNum, colIndex++, username.c_str());
//                std::string ip = json_data["ip"];
//                m_ViewList.SetItemText(rowNum, colIndex++, CA2T(ip.c_str()));
//                
//                std::wstring cpuid = json_data["cpuid"];
//                std::wstring mac = json_data["mac"];
//                std::wstring vol = json_data["vol"];
//                std::string ver = json_data["commit_ver"];
//                temp.Format(_T("%s|%s|%s"), cpuid.c_str(), mac.c_str(), vol.c_str());
//                int sysver = json_data["sysver"];
//                int is_64bits = json_data["64bits"];
//                m_ViewList.SetItemText(rowNum, colIndex++, temp);
//                m_ViewList.SetItemText(rowNum, colIndex++, GetSystemDesc(sysver, is_64bits));
//                m_ViewList.SetItemText(rowNum, colIndex++, CA2T(ver.c_str()));
//                asio2::uuid uuid;
//                memcpy(uuid.data, user_data.second.uuid, sizeof(uuid.data));
//                m_ViewList.SetItemText(rowNum, colIndex++, CA2T(uuid.str().c_str()));
//                unsigned int pid = json_data["pid"];
//                temp.Format(TEXT("%d"), pid);
//                m_ViewList.SetItemText(rowNum, colIndex++, temp);
//                time_t tm = json_data["logintime"];
//                CTime t(tm);
//                m_ViewList.SetItemText(rowNum, colIndex++, t.Format(TEXT("%Y-%m-%d %H:%M:%S")));
//                CTime tCur(time(0));
//                auto tDelta = tCur - t;
//                long long mins = tDelta.GetTotalMinutes();
//                temp.Format(TEXT("%d"), tDelta.GetTotalMinutes());
//                m_ViewList.SetItemText(rowNum, colIndex++, temp);
//                m_ViewList.SetItemText(rowNum, colIndex++, CA2T(client->get_address().c_str()));
//                m_ViewList.SetItemText(rowNum, colIndex++, CA2T(client->get_port().c_str()));
//                int miss_count = json_data.find("miss_count") == json_data.end() ? 0 : json_data["miss_count"];
//                float miss_rate = mins == 0 ? 0 : ((float)miss_count / (float)mins);
//                temp.Format(TEXT("%f"), miss_rate);
//                m_ViewList.SetItemText(rowNum, colIndex++, temp);
//            }
//            theApp.GetMainFrame()->SetStatusBar(ID_INDICATOR_USERS_COUNT, std::to_wstring(szUserCount).c_str());
//        });
//    });
//    m_ViewList.DeleteAllItems();
//    szUserCount = 0;
//    ProtocolOBC2OBSQueryUsers req;
//    theApp.m_ObServerClientGroup.send(&req);
//}

void CClientView::OnRefreshUsers()
{
    // 显示加载状态
    theApp.GetMainFrame()->SetStatusBar(ID_INDICATOR_USERS_COUNT, L"加载中...");

    // 清空旧数据
    {
        std::lock_guard<std::mutex> lock(g_dataMutex);
        g_allUserData.clear();
    }

    // 检查控件是否有效
    if (::IsWindow(m_ViewList.GetSafeHwnd()))
    {
        m_ViewList.SetItemCountEx(0, LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
    }

    // 注册数据处理器
    theApp.m_ObServerClientGroup.register_package_handler(OBPKG_ID_S2C_QUERY_USERS,
        [this](std::shared_ptr<CObserverClientImpl> client, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
        auto msg = raw_msg.get().as<ProtocolOBS2OBCQueryUsers>();

        // 在后台线程处理数据
        theApp.m_WorkIo.post([this, msg, client]() {
            // 1. 收集所有数据
            std::vector<std::unique_ptr<UserData>> tempData;
            tempData.reserve(msg.data.size()); // 预分配空间

            client->set_user_count(0);
            for (auto& user_data : msg.data)
            {
                auto json_data = user_data.second.json;
                if (json_data.find("is_observer_client") != json_data.end() &&
                    json_data["is_observer_client"])
                {
                    continue;
                }

                client->set_user_count(client->get_user_count() + 1);
                client->session_ids().emplace(user_data.second.session_id);
                auto ud = std::make_unique<UserData>();
                ud->session_id = user_data.second.session_id;

                if (json_data.find("usrname") != json_data.end()) {
                    ud->username = json_data["usrname"].get<std::wstring>();
                }
                else {
                    ud->username = L"(NULL)";
                }

                // 其他字段也做类似处理
                ud->ip = json_data["ip"].get<std::string>();
                ud->cpuid = json_data["cpuid"].get<std::wstring>();
                ud->mac = json_data["mac"].get<std::wstring>();
                ud->vol = json_data["vol"].get<std::wstring>();
                ud->sysver = json_data["sysver"].get<int>();
                ud->is_64bits = json_data["64bits"].get<int>();
                ud->commit_ver = json_data["commit_ver"].get<std::string>();

                memcpy(ud->uuid.data, user_data.second.uuid, sizeof(ud->uuid.data));

                ud->pid = json_data["pid"].get<unsigned int>();
                ud->logintime = json_data["logintime"].get<time_t>();

                if (json_data.find("miss_count") != json_data.end()) {
                    ud->miss_count = json_data["miss_count"].get<int>();
                }
                else {
                    ud->miss_count = 0;
                }

                ud->client_address = client->get_address();
                ud->client_port = client->get_port();
                tempData.push_back(std::move(ud));
            }

            // 2. 追加到总数据
            {
                std::lock_guard<std::mutex> lock(g_dataMutex);
                for (auto& ud : tempData) {
                    g_allUserData.push_back(std::move(ud));
                }
            }
            // 3. 通知UI线程更新
            PostMessage(WM_UPDATE_USER_LIST, 0, 0);
        });
    });
    // 发送查询请求
    ProtocolOBC2OBSQueryUsers req;
    theApp.m_ObServerClientGroup.send(&req);
}

// 处理WM_UPDATE_USER_LIST消息
LRESULT CClientView::OnUpdateUserList(WPARAM wParam, LPARAM lParam)
{
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastUpdateTime).count();

    // 限制更新频率（最多每秒60次）
    if (elapsed < 16) {  // ~60 FPS
        return 0;
    }

    m_lastUpdateTime = now;
    // 获取当前数据总数
    size_t itemCount = 0;
    {
        std::lock_guard<std::mutex> lock(g_dataMutex);
        itemCount = g_allUserData.size();
    }
    // 虚拟列表模式
    m_ViewList.SetItemCountEx(itemCount, LVSICF_NOSCROLL);

    theApp.GetMainFrame()->SetStatusBar(ID_INDICATOR_USERS_COUNT, std::to_wstring(itemCount).c_str());
    // 如果有数据但当前没有选中项，默认选中第一项
    if (!g_allUserData.empty() && m_ViewList.GetSelectedCount() == 0)
    {
        m_ViewList.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    }

    // 重置垂直滚动条位置到顶部
    m_ViewList.Scroll(CSize(0, 0));

    return 0;
}

// 虚拟列表获取数据
void CClientView::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
    LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

    if (pDispInfo->item.iItem >= static_cast<int>(g_allUserData.size()))
    {
        *pResult = 0;
        return;
    }

    const UserData& ud = *g_allUserData[pDispInfo->item.iItem];

    if (pDispInfo->item.mask & LVIF_TEXT)
    {
        CString temp;
        switch (pDispInfo->item.iSubItem)
        {
            case 0: temp.Format(_T("%d"), pDispInfo->item.iItem + 1); break;
            case 1: temp.Format(_T("%u"), ud.session_id); break;
            case 2: _tcscpy_s(pDispInfo->item.pszText, pDispInfo->item.cchTextMax, ud.username.c_str()); return;
            case 3: _tcscpy_s(pDispInfo->item.pszText, pDispInfo->item.cchTextMax, CA2T(ud.ip.c_str())); return;
            case 4: temp.Format(_T("%s|%s|%s"), ud.cpuid.c_str(), ud.mac.c_str(), ud.vol.c_str()); break;
            case 5: _tcscpy_s(pDispInfo->item.pszText, pDispInfo->item.cchTextMax, GetSystemDesc(ud.sysver, ud.is_64bits)); return;
            case 6: _tcscpy_s(pDispInfo->item.pszText, pDispInfo->item.cchTextMax, CA2T(ud.commit_ver.c_str())); return;
            case 7: _tcscpy_s(pDispInfo->item.pszText, pDispInfo->item.cchTextMax, CA2T(ud.get_uuid_str().c_str())); return;
            case 8: temp.Format(TEXT("%d"), ud.pid); break;
            case 9:
            {
                CTime t(ud.logintime);
                _tcscpy_s(pDispInfo->item.pszText, pDispInfo->item.cchTextMax, t.Format(TEXT("%Y-%m-%d %H:%M:%S")));
                return;
            }
            case 10: temp.Format(TEXT("%d"), (CTime(time(0)) - CTime(ud.logintime)).GetTotalMinutes()); break;
            case 11: _tcscpy_s(pDispInfo->item.pszText, pDispInfo->item.cchTextMax, CA2T(ud.client_address.c_str())); return;
            case 12: _tcscpy_s(pDispInfo->item.pszText, pDispInfo->item.cchTextMax, CA2T(ud.client_port.c_str())); return;
            case 13:
            {
                long long mins = (CTime(time(0)) - CTime(ud.logintime)).GetTotalMinutes();
                float miss_rate = mins == 0 ? 0 : ((float)ud.miss_count / (float)mins);
                temp.Format(TEXT("%f"), miss_rate);
                break;
            }
        }
        _tcscpy_s(pDispInfo->item.pszText, pDispInfo->item.cchTextMax, temp);
    }
    *pResult = 0;
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
        m_ServiceViewList.SetItemText(rowNum, colIndex++, CA2T(client->get_port().c_str()));
        m_ServiceViewList.SetItemText(rowNum, colIndex++, client->is_auth()&&client->is_started() ? TEXT("是"): TEXT("否"));
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

uint32_t CClientView::next_gm_policy_id(std::map<uint32_t, ProtocolPolicy>& policies) {
	uint32_t policy_id = GATE_POLICY_ID + 1;

	while (policies.find(policy_id) != policies.end()) {
		policy_id++;
		if (policy_id >= GATE_ADMIN_POLICY_ID) {
			break;
		}
	}
	return policy_id;
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
		unsigned int uiLastPolicyId = next_gm_policy_id(Policies);
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
		unsigned int uiLastPolicyId = next_gm_policy_id(Policies);
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
		unsigned int uiLastPolicyId = next_gm_policy_id(Policies);
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
		unsigned int uiLastPolicyId = next_gm_policy_id(Policies);
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

void CClientView::OnTimer(UINT_PTR nIDEvent)
{
    switch (nIDEvent)
    {
    #ifdef GATE_ADMIN
        case theApp.TIMER_ID_SYNC_LICENSE:
        {
            OnBnClickedSyncLicenseButton();
            break;
        }
    #else
        case theApp.TIMER_ID_RELOAD_GAMER_LIST:
        {
            OnRefreshUsers();
            break;
        }
    #endif
    }

    CDockablePane::OnTimer(nIDEvent);
}

void CClientView::OnBnClickedLogButton()
{
    COutputDlg& outputdlg = theApp.GetMainFrame()->GetOutputWindow();

    outputdlg.CenterWindow();
    outputdlg.ShowWindow(SW_SHOW);
}

#ifdef GATE_ADMIN
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
            req.file_name = CT2A(fileDlg.GetFileName());//"config.cfg";
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
            req.file_name = CT2A(fileDlg.GetFileName());//"config.cfg";
            req.data.resize(str.size());
            std::copy(str.begin(), str.end(), req.data.begin());
            std::string ip;
            int port = 0;
            for (int row_index = 0; row_index < m_ServiceViewList.GetItemCount(); row_index++)
            {
                ip = CT2A(m_ServiceViewList.GetItemText(row_index, 2));
                port = atoi(CT2A(m_ServiceViewList.GetItemText(row_index, 3)));
                theApp.m_ObServerClientGroup.get_observer_client(ip, port)->send(&req);
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

void CClientView::OnBnClickedRefreshLicenseButton()
{
    theApp.ConnectionLicenses();
    OnRefreshServices();
}

void CClientView::OnBnClickedSyncLicenseButton()
{
    auto license_info = http_get_license_info();
    if (license_info.empty() || !license_info._Starts_with("[{")) {
        AfxMessageBox(TEXT("获取license信息失败"));
        return;
    }
    std::filesystem::path license_path = theApp.m_ExeDir;
    license_path = license_path / "license.txt";
    // 将license信息写入文件
    std::ofstream output(license_path, std::ios::out | std::ios::binary);
    if  (!output.is_open())
    {
        AfxMessageBox(TEXT("license.txt文件打开失败"));
        return;
    }
    output.write(license_info.c_str(), license_info.size());
    output.close();

    OnBnClickedRefreshLicenseButton();
}

std::string CClientView::http_get_license_info() {
    VMP_VIRTUALIZATION_BEGIN(__FUNCTION__)
    // http://121.43.101.216:13568/fetch_all_serials_ip_packer.php
    try
    {
        asio2::http_client client;
        const std::string host = xorstr("121.43.101.216");
        const int port = 13568;
        // 登录请求
        http::web_request login_req;
        login_req.method(http::verb::post);
        login_req.target(xorstr("/login.php"));
        login_req.set(http::field::user_agent, "Chrome");
        login_req.set(http::field::content_type, "application/x-www-form-urlencoded");
        login_req.set(http::field::cookie, "lang=zh; PHPSESSID=ti0mvftdvlcfhv4bmc23i0mu83");
        login_req.body() = xorstr("login=4451151&password=4451152");

        // 发送登录请求
        auto login_rep = client.execute(host, port, login_req, std::chrono::seconds(5));

        if (auto error = asio2::get_last_error()) {
            return error.message().c_str();
        }

        // 查询
        http::web_request req;
        req.method(http::verb::get);
        req.target(xorstr("/fetch_all_serials_ip_packer.php"));
        req.set(http::field::user_agent, "Chrome");
        req.set(http::field::content_type, "application/x-www-form-urlencoded");
        req.set(http::field::cookie, "lang=zh; PHPSESSID=ti0mvftdvlcfhv4bmc23i0mu83");
        req.prepare_payload();
        auto rep = client.execute(host, port, req, std::chrono::seconds(5));

        if (auto error = asio2::get_last_error()) {
            return error.message().c_str();
        }
        else {
            return rep.body().c_str();
        }
    }
    catch (...)
    {
        return "";
    }
    return "";
    VMP_VIRTUALIZATION_END();
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
            auto observer_client = theApp.m_ObServerClientGroup.get_observer_client(ip, port);
			for (auto session_id : observer_client->session_ids())
			{
                observer_client->send(session_id, package);
			}
		}
	}
}
#endif