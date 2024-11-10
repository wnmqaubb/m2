
// GateFDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "GateF.h"
#include "GateFDlg.h"
#include "DlgProxy.h"
#include "afxdialogex.h"
#include "CGamesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGateFDlg 对话框
#define LOGIC_SERVER_KICK_LIST_FILE ".\\恶性开挂人员名单.txt"

IMPLEMENT_DYNAMIC(CGateFDlg, CDialogEx);

CGateFDlg::CGateFDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GATEF_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pAutoProxy = nullptr;
}

CGateFDlg::~CGateFDlg()
{
	// 如果该对话框有自动化代理，则
	//  此对话框的返回指针为 null，所以它知道
	//  此代理知道该对话框已被删除。
	if (m_pAutoProxy != nullptr)
		m_pAutoProxy->m_pDialog = nullptr;
}

void CGateFDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB_MAIN, m_tab_main);
}

BEGIN_MESSAGE_MAP(CGateFDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CONTEXTMENU()
	ON_WM_TIMER()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_MAIN, &CGateFDlg::OnTcnSelchangeTabMain)
#if defined(GATE_ADMIN)
	ON_COMMAND(ID_SERVICE_REFRESH, &CGateFDlg::OnRefreshServices)
	ON_COMMAND(ID_CMD_VIEW, &CGateFDlg::OnCmdView)
	ON_COMMAND(ID_SERVICE_UPDATE_LOGIC, &CGateFDlg::OnUpdateLogic)
	ON_COMMAND(ID_JS_QUERY_DEVICE_ID, &CGateFDlg::OnJsQueryDeviceId)
	ON_COMMAND(ID_JS_EXECUTE, &CGateFDlg::OnJsExecute)
	ON_BN_CLICKED(IDC_REFRESH_LICENSE_BUTTON, &CGateFDlg::OnBnClickedRefreshLicenseButton)
	ON_COMMAND(ID_SERVICE_S2C_PLUGIN, &CGateFDlg::OnServiceS2CPlugin)
#endif
	ON_COMMAND(ID_SERVICE_REMOVE_CFG, &CGateFDlg::OnServiceRemoveCfg)
	ON_COMMAND(ID_SERVICE_REMOVE_PLUGIN, &CGateFDlg::OnServiceRemovePlugin)
	ON_COMMAND(ID_SERVICE_UPLOAD_CFG, &CGateFDlg::OnServiceUploadCfg)
	ON_COMMAND(ID_SERVICE_ALL_UPLOAD_CFG, &CGateFDlg::OnServiceAllUploadCfg)
	ON_COMMAND(ID_SERVICE_UPLOAD_PLUGIN, &CGateFDlg::OnServiceUploadPlugin)
	ON_BN_CLICKED(IDC_LOG_BUTTON, &CGateFDlg::OnBnClickedLogButton)
	ON_COMMAND(ID_SERVICE_ADD_LIST, &CGateFDlg::OnServiceAddList)
	ON_COMMAND(ID_SERVICE_CLEAR_LIST, &CGateFDlg::OnServiceClearList)
END_MESSAGE_MAP()


// CGateFDlg 消息处理程序

BOOL CGateFDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	// 初始化 CTabCtrl
	m_tab_main.GetClientRect(&m_tab_main_rect);
	m_tab_main.AdjustRect(FALSE, &m_tab_main_rect);
	m_tab_main_rect.DeflateRect(0, 20, 0, 0);
	m_tab_main.InsertItem(0, _T("在线玩家"));
	m_tab_main.InsertItem(1, _T("封挂"));
	m_tab_main.InsertItem(2, _T("策略"));
	m_tab_main.InsertItem(3, _T("日志"));
	ShowAllDlgInTab();

	// 填入一些静态树视图数据(此处只需填入虚拟代码，而不是复杂的数据)
	FillClientView();

	int rowNum = m_games_dlg->m_list_games.GetItemCount();
	int colIndex = 0;

	CString temp = _T("游戏名");
	m_games_dlg->m_list_games.InsertItem(rowNum, temp);
	m_games_dlg->m_list_games.SetItemText(rowNum, colIndex++, temp);
	m_games_dlg->m_list_games.SetItemText(rowNum, colIndex++, temp);
	m_games_dlg->m_list_games.SetItemText(rowNum, colIndex++, temp);
	m_games_dlg->m_list_games.SetItemText(rowNum, colIndex++, temp);
	m_games_dlg->m_list_games.SetItemText(rowNum, colIndex++, temp);
	m_games_dlg->m_list_games.SetItemText(rowNum, colIndex++, temp);
	m_games_dlg->m_list_games.SetItemText(rowNum, colIndex++, temp);
	m_games_dlg->m_list_games.SetItemText(rowNum, colIndex++, temp);
	m_games_dlg->m_list_games.SetItemText(rowNum, colIndex++, temp);
	m_games_dlg->m_list_games.SetItemText(rowNum, colIndex++, temp);
	m_games_dlg->m_list_games.SetItemText(rowNum, colIndex++, temp);
	m_games_dlg->m_list_games.SetItemText(rowNum, colIndex++, temp);
	m_games_dlg->m_list_games.SetItemText(rowNum, colIndex++, temp);
	m_games_dlg->m_list_games.SetItemText(rowNum, colIndex++, temp);
#ifdef GATE_ADMIN
	FillServiceView();
#else
	SetTimer(RELOAD_GAMER_LIST, 1000 * 60 * 10, NULL);
#endif 
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CGateFDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CGateFDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// 当用户关闭 UI 时，如果控制器仍保持着它的某个
//  对象，则自动化服务器不应退出。  这些
//  消息处理程序确保如下情形: 如果代理仍在使用，
//  则将隐藏 UI；但是在关闭对话框时，
//  对话框仍然会保留在那里。

void CGateFDlg::OnClose()
{
	if (CanExit())
		CDialogEx::OnClose();
}

void CGateFDlg::OnOK()
{
	if (CanExit())
		CDialogEx::OnOK();
}

void CGateFDlg::OnCancel()
{
	if (CanExit())
		CDialogEx::OnCancel();
}

BOOL CGateFDlg::CanExit()
{
	// 如果代理对象仍保留在那里，则自动化
	//  控制器仍会保持此应用程序。
	//  使对话框保留在那里，但将其 UI 隐藏起来。
	if (m_pAutoProxy != nullptr)
	{
		ShowWindow(SW_HIDE);
		return FALSE;
	}

	return TRUE;
}

void CGateFDlg::ShowAllDlgInTab()
{
	m_games_dlg = std::make_unique<CGamesDlg>();
	m_games_dlg->Create(IDD_DIALOG_GAMES, &m_tab_main);
	m_games_dlg->MoveWindow(m_tab_main_rect);
	m_games_dlg->ShowWindow(SW_SHOW);

	m_anticheat_dlg = std::make_unique<CAntiCheatDlg>();
	m_anticheat_dlg->Create(IDD_DIALOG_ANTICHEAT, &m_tab_main);
	m_anticheat_dlg->MoveWindow(m_tab_main_rect);
	m_anticheat_dlg->ShowWindow(SW_HIDE);

	m_polices_dlg = std::make_unique<CPoliceDlg>();
	m_polices_dlg->Create(IDD_DIALOG_POLICES, &m_tab_main);
	m_polices_dlg->MoveWindow(m_tab_main_rect);
	m_polices_dlg->ShowWindow(SW_HIDE);

	m_logs_dlg = std::make_unique<CLogDlg>();
	m_logs_dlg->Create(IDD_DIALOG_LOG, &m_tab_main);
	m_logs_dlg->MoveWindow(m_tab_main_rect);
	m_logs_dlg->ShowWindow(SW_HIDE);
}

void CGateFDlg::OnTcnSelchangeTabMain(NMHDR* pNMHDR, LRESULT* pResult)
{
	switch (m_tab_main.GetCurSel())
	{
	case 0:
		m_games_dlg->ShowWindow(SW_SHOW);
		m_anticheat_dlg->ShowWindow(SW_HIDE);
		m_polices_dlg->ShowWindow(SW_HIDE);
		m_logs_dlg->ShowWindow(SW_HIDE);
		break;
	case 1:
		m_games_dlg->ShowWindow(SW_HIDE);
		m_anticheat_dlg->ShowWindow(SW_SHOW);
		m_polices_dlg->ShowWindow(SW_HIDE);
		m_logs_dlg->ShowWindow(SW_HIDE);
		break;
	case 2:
		m_games_dlg->ShowWindow(SW_HIDE);
		m_anticheat_dlg->ShowWindow(SW_HIDE);
		m_polices_dlg->ShowWindow(SW_SHOW);
		m_logs_dlg->ShowWindow(SW_HIDE);
		break;
	case 3:
		m_games_dlg->ShowWindow(SW_HIDE);
		m_anticheat_dlg->ShowWindow(SW_HIDE);
		m_polices_dlg->ShowWindow(SW_HIDE);
		m_logs_dlg->ShowWindow(SW_SHOW);
		break;
	default:
		break;
	}
	*pResult = 0;
}

BOOL CGateFDlg::PreTranslateMessage(MSG* pMsg)
{
	if (WM_KEYFIRST <= pMsg->message && pMsg->message <= WM_KEYLAST)
	{
		if (pMsg->wParam == VK_RETURN)
		{
			m_games_dlg->OnBnClickedOnlineGamerSearch();
		}
}
	return CDialogEx::PreTranslateMessage(pMsg);
}

CString CGateFDlg::GetCurrentSelectedUserName()
{
	auto selectedRow = (int)m_games_dlg->m_list_games.GetFirstSelectedItemPosition() - 1;
	if (selectedRow != -1)
	{
		const std::wstring strUserName = m_games_dlg->m_list_games.GetItemText(selectedRow, 2);
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
void CGateFDlg::SendCurrentSelectedUserServiceCommand(T* package)
{
	auto selectedRow = (int)m_games_dlg->m_list_games.GetFirstSelectedItemPosition() - 1;
	if (selectedRow != -1)
	{
		const std::string ip = CT2A(m_games_dlg->m_list_games.GetItemText(selectedRow, 11));
		const int port = atoi(CT2A(m_games_dlg->m_list_games.GetItemText(selectedRow, 12)));
		theApp.m_ObServerClientGroup(ip, port)->send(package);
	}
	else
	{
		AfxMessageBox(TEXT("请选择一个玩家!"), MB_OK);
	}
}

template<typename T>
void CGateFDlg::SendCurrentSelectedServiceCommand(T* package)
{
	/*auto selectedRow = (int)m_ServiceViewList.GetFirstSelectedItemPosition() - 1;
	if (selectedRow != -1)
	{
		const std::string ip = CT2A(m_ServiceViewList.GetItemText(selectedRow, 2));
		const int port = atoi(CT2A(m_ServiceViewList.GetItemText(selectedRow, 3)));
		theApp.m_ObServerClientGroup(ip, port)->send(package);
	}*/
}

void CGateFDlg::FillClientView()
{
	/*m_games_dlg->m_list_games.SetColumnByIntSort({ 0, 1 });
	m_games_dlg->m_list_games.SetColumnBySearch({ 2, 3 });*/
	m_games_dlg->m_list_games.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_GRIDLINES);
	int colIndex = 0;
	m_games_dlg->m_list_games.InsertColumn(colIndex++, TEXT("序号"), LVCFMT_LEFT, 38);
	m_games_dlg->m_list_games.InsertColumn(colIndex++, TEXT("ID"), LVCFMT_LEFT, 0);
	m_games_dlg->m_list_games.InsertColumn(colIndex++, TEXT("角色名"), LVCFMT_LEFT, 250);
	m_games_dlg->m_list_games.InsertColumn(colIndex++, TEXT("登录IP地址"), LVCFMT_LEFT, 110);
	m_games_dlg->m_list_games.InsertColumn(colIndex++, TEXT("机器码"), LVCFMT_LEFT, 320);
	m_games_dlg->m_list_games.InsertColumn(colIndex++, TEXT("系统版本"), LVCFMT_LEFT, 80);
#ifndef GATE_ADMIN
	m_games_dlg->m_list_games.InsertColumn(colIndex++, TEXT("客户端版本"), LVCFMT_LEFT, 0);
	m_games_dlg->m_list_games.InsertColumn(colIndex++, TEXT("uuid"), LVCFMT_LEFT, 0);
	m_games_dlg->m_list_games.InsertColumn(colIndex++, TEXT("进程ID"), LVCFMT_LEFT, 0);
#else
	m_games_dlg->m_list_games.InsertColumn(colIndex++, TEXT("客户端版本"), LVCFMT_LEFT, 80);
	m_games_dlg->m_list_games.InsertColumn(colIndex++, TEXT("uuid"), LVCFMT_LEFT, 240);
	m_games_dlg->m_list_games.InsertColumn(colIndex++, TEXT("进程ID"), LVCFMT_LEFT, 60);
#endif
	m_games_dlg->m_list_games.InsertColumn(colIndex++, TEXT("登陆时间"), LVCFMT_LEFT, 130);
	m_games_dlg->m_list_games.InsertColumn(colIndex++, TEXT("在线时长"), LVCFMT_LEFT, 80);
#ifndef GATE_ADMIN
	m_games_dlg->m_list_games.InsertColumn(colIndex++, TEXT("服务端IP"), LVCFMT_LEFT, 0);
	m_games_dlg->m_list_games.InsertColumn(colIndex++, TEXT("端口"), LVCFMT_LEFT, 0);
	m_games_dlg->m_list_games.InsertColumn(colIndex++, TEXT("丢包率"), LVCFMT_LEFT, 0);
#else
	m_games_dlg->m_list_games.InsertColumn(colIndex++, TEXT("服务端IP"), LVCFMT_LEFT, 110);
	m_games_dlg->m_list_games.InsertColumn(colIndex++, TEXT("端口"), LVCFMT_LEFT, 60);
	m_games_dlg->m_list_games.InsertColumn(colIndex++, TEXT("丢包率"), LVCFMT_LEFT, 60);
#endif
	m_games_dlg->m_list_games.DeleteAllItems();
}

void CGateFDlg::FillServiceView()
{
	/*m_ServiceViewList.SetColumnByIntSort({ 0, 1 });
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
	m_ServiceViewList.DeleteAllItems();*/
}

void CGateFDlg::OnRefreshServices()
{
	/*m_ServiceViewList.DeleteAllItems();
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
		m_ServiceViewList.SetItemText(rowNum, colIndex++, client->is_auth() && client->is_connected() ? TEXT("是") : TEXT("否"));
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
		});*/
}

void CGateFDlg::OnJsQueryDeviceId()
{
	//ProtocolS2CScript msg;
	//msg.code = "import * as api from 'api'; api.report(0, false, api.get_machine_id().toString(16))";
	//m_games_dlg->SendCurrentSelectedUserCommand(&msg);
}

void CGateFDlg::OnJsExecute()
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


			//if (m_wndTabs.GetActiveTab() == 1) {
			//	//服务列表
			//	BroadCastCurrentSelectedServiceCommand(&msg);
			//}
			//else {
			//	//用户列表
			//	SendCurrentSelectedUserCommand(&msg);
			//}
		}
	}
}
void CGateFDlg::OnCmdView()
{
	/*theApp.GetDocTemplateMgr().Find("Cmd")->CloseAllDocuments(TRUE);
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
	SendCurrentSelectedUserCommand(&msg);*/
}


void CGateFDlg::OnUpdateLogic()
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

void CGateFDlg::OnServiceUploadCfg()
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

void CGateFDlg::OnServiceAllUploadCfg()
{
	//CString gReadFilePathName;
	//CFileDialog fileDlg(true, _T("cfg"), _T("*.cfg"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("cfg Files (*.cfg)|*.cfg"), NULL);
	//if (fileDlg.DoModal() == IDOK)    //弹出对话框  
	//{
	//	std::ifstream file(fileDlg.GetPathName(), std::ios::in | std::ios::binary);
	//	if (file.is_open())
	//	{
	//		std::stringstream ss;
	//		ss << file.rdbuf();
	//		auto str = ss.str();
	//		ProtocolOBC2LSUploadConfig req;
	//		req.file_name = "config.cfg";
	//		req.data.resize(str.size());
	//		std::copy(str.begin(), str.end(), req.data.begin());
	//		std::string ip;
	//		int port = 0;
	//		for (int row_index = 0; row_index < m_ServiceViewList.GetItemCount(); row_index++)
	//		{
	//			ip = CT2A(m_ServiceViewList.GetItemText(row_index, 2));
	//			port = atoi(CT2A(m_ServiceViewList.GetItemText(row_index, 3)));
	//			theApp.m_ObServerClientGroup(ip, port)->send(&req);
	//		}
	//	}
	//}
}

void CGateFDlg::OnServiceRemoveCfg()
{
	ProtocolOBC2LSRemoveConfig req;
	req.file_name = "config.cfg";
	SendCurrentSelectedServiceCommand(&req);
}


void CGateFDlg::OnServiceRemovePlugin()
{
	ProtocolOBC2LSRemovePlugin req;
	req.file_name = "TaskBasic.dll";
	SendCurrentSelectedServiceCommand(&req);
}

void CGateFDlg::OnServiceUploadPlugin()
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

void CGateFDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
		case RELOAD_GAMER_LIST:
		{
			m_games_dlg->OnRefreshUsers();
			break;
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}

void CGateFDlg::OnBnClickedLogButton()
{
	//COutputDlg& outputdlg = theApp.GetMainFrame()->GetOutputWindow();

	//outputdlg.CenterWindow();
	//outputdlg.ShowWindow(SW_SHOW);
}

void CGateFDlg::OnBnClickedRefreshLicenseButton()
{
	theApp.ConnectionLicenses();
	OnRefreshServices();
}


void CGateFDlg::OnServiceAddList()
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


void CGateFDlg::OnServiceClearList()
{
	ProtocolOBC2LSClearList req;
	req.file_name = LOGIC_SERVER_KICK_LIST_FILE;
	SendCurrentSelectedServiceCommand(&req);
}


void CGateFDlg::OnServiceS2CPlugin()
{

	//CString gReadFilePathName;
	//CFileDialog fileDlg(true, _T("dll"), _T("*.dll"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("dll Files (*.dll)|*.dll"), NULL);
	//if (fileDlg.DoModal() != IDOK)    //弹出对话框  
	//{
	//	return;
	//}

	//std::ifstream file(fileDlg.GetPathName(), std::ios::in | std::ios::binary);
	//if (file.is_open() == false)
	//{
	//	return;

	//}

	//std::stringstream ss;
	//ss << file.rdbuf();
	//std::string buffer = ss.str();

	//ProtocolS2CDownloadPlugin package;

	//if (*(uint16_t*)buffer.data() != 0x5A4D)
	//{
	//	RawProtocolImpl raw_package;
	//	if (!raw_package.decode(buffer))
	//	{
	//		AfxMessageBox(L"Decode Error");
	//		return;
	//	}

	//	auto raw_msg = msgpack::unpack((char*)raw_package.body.buffer.data(), raw_package.body.buffer.size());
	//	package = raw_msg.get().as<ProtocolS2CDownloadPlugin>();
	//}
	//else
	//{
	//	std::copy(buffer.begin(), buffer.end(), std::back_inserter(package.data));
	//	xor_buffer(package.data.data(), package.data.size(), kProtocolXorKey);
	//	package.is_crypted = 1;
	//	package.plugin_hash = NetUtils::aphash((unsigned char*)buffer.data(), buffer.size());
	//	package.plugin_name = "TaskBasic.dll";
	//}


	///*if (m_wndTabs.GetActiveTab() == 1) {
	//	//服务列表
	//	BroadCastCurrentSelectedServiceCommand(&package);
	//}
	//else*/ {
	//	//用户列表
	//	m_games_dlg->SendCurrentSelectedUserCommand(&package);
	//}
}

template<typename T>
void CGateFDlg::BroadCastCurrentSelectedServiceCommand(T* package)
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
