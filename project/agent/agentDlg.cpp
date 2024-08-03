
// agentDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "agent.h"
#include "agentDlg.h"
#include "afxdialogex.h"
#include "ProcessInfoDlg.h"
#include <sstream>
#include "protocol.h"
#include <xlocale>
#include <codecvt>
#include "utils\utils.h"
#include <WinNls.h>
#include <string>
#include <atlenc.h>
#include "global_string.h"
#include <WinBase.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
using namespace SubProto;
#define WM_TRAYICON_MSG (WM_USER+100)

// 判断网关是否对应多个序列号,如果对应多个序列号,其中一个序列号过期将只提示,而不关闭网关
BOOL is_multi_serial = FALSE;
static std::set<std::wstring> serials_white_list;
std::vector<std::unique_ptr<Protocol>> protocol_shell_code_list; 
std::unordered_map<std::wstring, uint32_t> ip_handshake_count_table;
int shellcode_action_count;
PVOID GamerInfoHeapHandle = NULL;
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum
	{
		IDD = IDD_ABOUTBOX
	};
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
    
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CagentDlg 对话框



CagentDlg::CagentDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_AGENT_DIALOG, pParent),
	m_server(this), m_server_started(false)
    , m_back_game_lazy_time(3)
    , m_exit_game_lazy_time(3)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CagentDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST2, m_client_list);
    DDX_Text(pDX, IDC_EDIT_ONLINE_GAMER_SEARCH, m_search_text);
    DDX_Text(pDX, IDC_EDIT1, m_back_game_lazy_time);
    DDX_Text(pDX, IDC_EDIT3, m_exit_game_lazy_time);
}

BEGIN_MESSAGE_MAP(CagentDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_SCREENSHOT, &CagentDlg::OnBnClickedScreenshot)
	ON_BN_CLICKED(IDC_BUTTON3, &CagentDlg::OnBnClickedShowRuleDlg)
	ON_BN_CLICKED(IDC_BUTTON6, &CagentDlg::OnBnClickedBSOD)
	ON_BN_CLICKED(IDC_BUTTON7, &CagentDlg::OnBnClickedGetProcesses)
	ON_BN_CLICKED(IDC_BUTTON8, &CagentDlg::OnBnClickedGetWindows)
	ON_BN_CLICKED(IDC_BUTTON10, &CagentDlg::OnBnClickedSendShellCode)
	ON_WM_TIMER()
	ON_NOTIFY(NM_RCLICK, IDC_LIST2, &CagentDlg::OnNMRClickListRightMenu)
	ON_COMMAND(ID_32772, &CagentDlg::OnBnClickedGetProcesses)
	ON_COMMAND(ID_32773, &CagentDlg::OnBnClickedScreenshot)
	ON_COMMAND(ID_MENU_EXIT_GAME, &CagentDlg::OnMenuOffline)
	ON_MESSAGE(ASYNC_DO_MODAL, &CagentDlg::OnAsyncDoModal)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST2, &CagentDlg::OnNMCustomdrawList2)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_CLICK, IDC_LIST2, &CagentDlg::OnNMClickListRow)
	ON_BN_CLICKED(IDC_BUTTON11, &CagentDlg::OnBnClickedDetectionCheat)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST2, &CagentDlg::OnLvnColumnclickList2)
	ON_BN_CLICKED(IDC_BUTTON12, &CagentDlg::OnMenuOffline)
	ON_BN_CLICKED(IDC_BUTTON4, &CagentDlg::OnBnClickedShellManager)
	ON_COMMAND(ID_COPY_MACHINEID, &CagentDlg::OnCopyMachineid)
	ON_COMMAND(ID_ADD_BLACKLIST, &CagentDlg::OnAddBlacklist)
	ON_COMMAND(ID_RMENU_SHELLCODE_ACTION, &CagentDlg::OnBnClickedSendShellCode)
    ON_BN_CLICKED(IDC_BUTTON5, &CagentDlg::OnBnClickedSwitchLogDlg)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON2, &CagentDlg::ReloadGamerList)
	ON_BN_CLICKED(IDC_BUTTON_ONLINE_GAMER_SEARCH, &CagentDlg::OnBnClickedButtonOnlineGamerSearch)
    ON_BN_CLICKED(IDC_CHECK1, &CagentDlg::OnBnClickedBackGameLazyEnable)
    ON_MESSAGE(WM_TRAYICON_MSG, OnTrayCallBackMsg)
    ON_EN_CHANGE(IDC_EDIT3, &CagentDlg::OnEnChangeEdit3)
    ON_EN_CHANGE(IDC_EDIT1, &CagentDlg::OnEnChangeEdit1)
    ON_BN_CLICKED(IDC_CHECK2, &CagentDlg::OnBnClickedExitGameLazyEnable)
END_MESSAGE_MAP()

// CagentDlg 消息处理程序
extern "C" BOOL APIENTRY AntiCheatEntry2();

BOOL CagentDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。
    m_start_tickcount = GetTickCount();
	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if(pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if(!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

    GamerInfoHeapHandle = /*GetProcessHeap();*/HeapCreate(NULL, 1024 * 1024 * 5, 0);
    if(GamerInfoHeapHandle == NULL)
    {
        wchar_t heap_errormsg[100];
        swprintf_s(heap_errormsg, L"网关初始化失败!请联系客服.错误代码:%d", GetLastError());
        MessageBox(heap_errormsg, L"错误提示", MB_OK);
        return 1;
    }

	m_server->SetMaxPackSize(0x3FFFFF);
    m_server->SetMaxConnectionCount(20000);

    m_LogDlg.Create(IDD_DIALOG_LOG);
	m_processInfoDlg.Create(IDD_PROCESS_INFO);
	m_screenshotdlg.Create(IDD_SCREENSHOT_DLG);
	m_clientWindowDlg.Create(IDD_CLIENT_WINDOW_DLG);
	m_shellManagerDlg.Create(IDD_SHELL_MANAGER_DLG);
	m_ruleDlg.Create(IDD_RULE_LIST_DLG);
	// TODO: 在此添加额外的初始化代码


	m_client_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_GRIDLINES);
	int colIndex = 0;
	m_client_list.InsertColumn(colIndex++, TEXT("序号"), LVCFMT_LEFT, 38);
	m_client_list.InsertColumn(colIndex++, TEXT("ID"), LVCFMT_LEFT, 0);
	m_client_list.InsertColumn(colIndex++, TEXT("角色名"), LVCFMT_LEFT, 250);
	m_client_list.InsertColumn(colIndex++, TEXT("登录IP地址"), LVCFMT_LEFT, 110);
	m_client_list.InsertColumn(colIndex++, TEXT("机器码"), LVCFMT_LEFT, 320);
	GetCurrentHWID();
    Singleton<TaskShellcode>::getInstance().shellcode_init();
	// ******************** 定时执行 ********************
	// 定时刷新玩家列表
#ifndef BUILD_ADMIN
	SetTimer(RELOAD_GAMER_LIST, 1000 * 60 * 5, NULL);
#endif
	SetTimer(MACHINE_BLACK_LIST, 1000 * 60 * 1, NULL);
	SetTimer(SHELL_CODE_ACTION_COUNT, 1000, NULL); 
#ifndef _DEBUG
    SetTimer(IP_TABLES_CLEAR, 24 * 60 * 60 * 1000, NULL);
    SetTimer(VMP_SERIAL_VALIDATE, 1000 * 60 * 60, NULL);
    if(!VmpSerialValidate())
    {
        exit(0);
    }
#endif
	// ******************** 定时执行 ********************
    OnBnClickedListen();
#ifdef _DEBUG
	/*decltype(&AntiCheatEntry2) anti_cheat_entry = (decltype(&AntiCheatEntry2))GetProcAddress(LoadLibrary(TEXT("client.dll")), "AntiCheatEntry2");
	if (anti_cheat_entry)
	{
		anti_cheat_entry();
	}*/
#endif
    ((CStatic*)GetDlgItem(IDC_STATIC_SHELLCODE_ACTION_COUNT))->SetWindowText(L"无任务");
#ifdef BUILD_ADMIN
    #define APPEND_TEXT "(管理员)"
    GetDlgItem(IDC_BUTTON10)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_CHECK_SHELLCODE_HEARTBEAT)->ShowWindow(SW_SHOW);
#else
    SetTimer(ONLINE_CHECK, 1000 * 20 * 1, NULL);
    #define APPEND_TEXT 
#endif
    SetWindowText(TEXT(CONFIG_APP_NAME "网关VIP版" APPEND_TEXT));

    GetDlgItem(IDC_MFCLINK_WEBSITE)->SetWindowText(TEXT("官方网址:" CONFIG_WEBSITE));
    ((CMFCLinkCtrl*)GetDlgItem(IDC_MFCLINK_WEBSITE))->SetURL(TEXT(CONFIG_WEBSITE));
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CagentDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	} 
    else if(nID == SC_MINIMIZE)
    {
        ShowWindow(SW_HIDE);
        TrayMyIcon(true);
    }else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CagentDlg::OnPaint()
{
	if(IsIconic())
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
	} else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CagentDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CagentDlg::OnBnClickedListen()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_server_started)
	{
		if(m_server->Stop())
		{
			GLOG(TEXT("停止监听端口成功"));
			m_server_started = false;
		} else
		{
            GLOG(TEXT("停止监听端口失败,失败原因:%d"), m_server->GetLastError());
			m_server_started = true;
		}
		return;
	}

#ifdef BUILD_ADMIN
    CONST USHORT port = 23228;
#else
    CONST USHORT port = 23227;
#endif

	if(m_server->Start(TEXT("0.0.0.0"), port))
	{
		GLOG(TEXT("监听端口23227 成功"));
		m_server_started = true;
		ReloadGamerList();
	} else
	{
		GLOG(TEXT("监听端口23227 失败，失败原因:%d"), m_server->GetLastError());
		m_server_started = false;
	}
	
	log_server_register_trans_send([this](unsigned long id, const unsigned char* buffer, int size)->bool {
		return this->m_server->Send(id, buffer, size);
	});
}

inline std::tuple<std::wstring,int> CagentDlg::GetClientAddress(ITcpServer* pSender, CONNID dwConnID)
{
    TCHAR ip[50];
    int ip_size = sizeof(ip) / sizeof(TCHAR);
    USHORT port;
    pSender->GetRemoteAddress(dwConnID, ip, ip_size, port);
    return std::make_tuple(ip, port);
}

EnHandleResult CagentDlg::OnAccept(ITcpServer* pSender, CONNID dwConnID, UINT_PTR soClient)
{
    auto[ip, port] = GetClientAddress(pSender, dwConnID);
    Utils::log_to_file(dwConnID, Utils::CHANNEL_EVENT, GlobalString::LOG::CLIENT_IP_CONN.c_str(), ip, port);
    return HR_OK;
}

EnHandleResult CagentDlg::OnClose(ITcpServer* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode)
{
    if(m_gamer_list.find(dwConnID) != m_gamer_list.end())
    {
        HeapFree(GamerInfoHeapHandle, 0, m_gamer_list.at(dwConnID));
        m_gamer_list.erase(dwConnID);
        Sleep(10);
    }
	return HR_OK;
}

EnHandleResult CagentDlg::OnHandShake(ITcpServer* pSender, CONNID dwConnID)
{
    auto[ip, port] = GetClientAddress(pSender, dwConnID);
    std::lock_guard<std::mutex> lck(ip_tables_mtx_);
    if(ip_handshake_count_table.find(ip) == ip_handshake_count_table.end())
    {
        ip_handshake_count_table.emplace(std::make_pair(ip, 1));
    }
    else if(++ip_handshake_count_table[ip] > kMaxIPConnectCount)
    {
        Utils::log_to_file(dwConnID, Utils::CHANNEL_EVENT, "ip:%ls 超出连接次数，不下发云代码", ip.c_str());
        return HR_ERROR;
    }

    if(is_multi_serial && serials_white_list.find(ip) == serials_white_list.end())
    {
        return HR_ERROR;
    }
    //Utils::log_to_file(dwConnID, Utils::CHANNEL_EVENT, GlobalString::LOG::CLIENT_HANDSHAKE.c_str(), ip, port);
#ifndef _DEBUG
    if(m_shellManagerDlg.m_shell_auto && GetTickCount() - m_start_tickcount > 60000)
#endif
    {
        Singleton<TaskPolicy>::getInstance().async_.delay_execute_shellcode([dwConnID]() {
            Singleton<TaskShellcode>::getInstance().send_all_shell(dwConnID);
            }, 300);
    }
#ifndef BUILD_ADMIN
    bool back_game_lazy_enable = ((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck();
    Singleton<TaskJudgment>::getInstance().send_back_game_lazy(dwConnID, back_game_lazy_enable ? L"true" : L"false", m_back_game_lazy_time);

    bool exit_game_lazy_enable = ((CButton*)GetDlgItem(IDC_CHECK2))->GetCheck();
    Singleton<TaskJudgment>::getInstance().send_exit_game_lazy(dwConnID, exit_game_lazy_enable ? L"true" : L"false", m_exit_game_lazy_time);
#endif

	return HR_OK;
}

EnHandleResult CagentDlg::OnPrepareListen(ITcpServer* pSender, SOCKET soListen)
{
	GLOG(TEXT("准备开始监听..."));
	return HR_OK;
}
EnHandleResult CagentDlg::OnReceive(ITcpServer* pSender, CONNID dwConnID, int iLength)
{
	return HR_OK;
}

CString CagentDlg::GetConnectInfo(CONNID dwConnID)
{
    auto[ip, port] = GetClientAddress(m_server, dwConnID);
	CString result;
	result.Format(TEXT("%s:%u"), ip, port);
	return result;
}

EnHandleResult CagentDlg::OnReceive(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength)
{
    if(is_multi_serial)
    {
        if(auto[ip, port] = GetClientAddress(pSender, dwConnID); serials_white_list.find(ip) == serials_white_list.end())
        {
            return HR_IGNORE;
        }
    }

	Protocol proto;
	if(proto.unpak(pData, iLength))
    {
		json package;
		proto.get_json(package);
		if(package.find(GlobalString::JsonProperty::ID) == package.end())
		{
			Utils::log_to_file(dwConnID, Utils::CHANNEL_ERROR, GlobalString::LOG::FORMAT_S.c_str(),GlobalString::ErrorMsg::MISSID_EMSG.c_str());
			return HR_ERROR;
		}
		Protocol::PackageId package_id = package[GlobalString::JsonProperty::ID];
		log_server_trans_recv(dwConnID, package_id, pData, iLength);
		if(task_map_.find(package_id) != task_map_.end())
		{
			Task& task = *task_map_[package_id];
			try
			{
				task.on_recv_package(pSender, dwConnID, package, proto.m_type);
			} catch(...)
			{
				Utils::log_to_file(dwConnID, Utils::CHANNEL_ERROR, GlobalString::LOG::EXCEPTION_PACKAGE_ID.c_str(), package_id);
			}
		}
        else if (Protocol::PackageId::PACKAGE_ID_SHELLCODE_START <= package_id &&
            package_id <= Protocol::PackageId::PACKAGE_ID_UNDEFINED)
        {
            try
            {
                Singleton<TaskShellcode>::getInstance().on_recv_package(pSender, dwConnID, package, proto.m_type);
            }
            catch (...)
            {
                Utils::log_to_file(dwConnID, Utils::CHANNEL_ERROR, GlobalString::LOG::SHELLCODE_EXCEPTION_PACKAGE_ID.c_str(), package_id);
            }
        }
        else
        {
            Utils::log_to_file(dwConnID, Utils::CHANNEL_ERROR, GlobalString::LOG::UNKNOWN_PACKAGE_PACKAGE_ID.c_str(), package_id);
        }
	} else
	{
		Utils::log_to_file(dwConnID, Utils::CHANNEL_ERROR,GlobalString::LOG::FORMAT_S.c_str(), GlobalString::ErrorMsg::UNPACK_EMSG.c_str());
	}

	return HR_OK;
}


EnHandleResult CagentDlg::OnSend(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength)
{
	return HR_OK;
}

EnHandleResult CagentDlg::OnShutdown(ITcpServer* pSender)
{
	return HR_OK;
}


void CagentDlg::EnumConnectID(std::function<void(CONNID)> callback)
{
	DWORD count = 0;
	m_server->GetAllConnectionIDs(nullptr, count);
    
	std::unique_ptr<CONNID[]> connect_ids = std::make_unique<CONNID[]>(count);
	m_server->GetAllConnectionIDs(connect_ids.get(), count);
	for(size_t i = 0; i < count; i++)
	{
		callback(connect_ids[i]);
	}
}

void CagentDlg::SendToAll(Protocol& proto)
{
    json temp;
    proto.to_json(temp);
    Protocol package(temp);
    EnumConnectID([&](CONNID conn_id) {
        m_server->Send(conn_id, package.get_raw_buffer(), package.get_raw_size());
    });
}

void CagentDlg::SendToAll(unsigned char* buffer, size_t buffer_size)
{
	EnumConnectID([&](CONNID conn_id) {
		m_server->Send(conn_id, buffer, buffer_size);
	});
}

void CagentDlg::SendShellCode(uintptr_t conn_id, Protocol& package)
{
    if(!m_server->IsConnected(conn_id))
    {
        return;
    }

    if(!m_server->Send(conn_id, package.get_raw_buffer(), package.get_raw_size()))
    {
        Utils::log_to_file(conn_id, Utils::CHANNEL_EVENT, "%d|下发失败", conn_id);
    }
}

void CagentDlg::Send(uintptr_t conn_id, Protocol& proto)
{
    json temp;
    proto.to_json(temp);
    Protocol package(temp);
    if(!m_server->IsConnected(conn_id))
    {
        return;
    }

    if(!m_server->Send(conn_id, package.get_raw_buffer(), package.get_raw_size()))
    {
        Utils::log_to_file(conn_id, Utils::CHANNEL_EVENT, "%d|下发失败", conn_id);
    }
}

void CagentDlg::OnBnClickedScreenshot()
{
	CONNID m_current_connect_id = GetSelectedConnID();
	if(m_current_connect_id != -1)
	{
        Singleton<TaskScreenShot>::getInstance().send(m_current_connect_id, false);
	}
}

// 配置规则
void CagentDlg::OnBnClickedShowRuleDlg()
{
	m_ruleDlg.CenterWindow();
	m_ruleDlg.SetFocus();
	m_ruleDlg.ShowWindow(SW_SHOW);
}

// 执行蓝屏
void CagentDlg::OnBnClickedBSOD()
{
	CONNID m_current_connect_id = GetSelectedConnID();
	if(m_current_connect_id != -1)
	{
		if(MessageBox(L"此操作危险,确定要执行蓝屏操作吗?", L"警告", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
		{
            Singleton<TaskJudgment>::getInstance().send_bsod(m_current_connect_id);
		}
	}
}

void CagentDlg::ShowProcessInfo(ProtocolProcessInfo& data)
{
	m_processInfoDlg.ShowProcessInfo(data);
	m_processInfoDlg.CenterWindow();
	m_processInfoDlg.SetFocus();
	m_processInfoDlg.ShowWindow(SW_SHOW);
}

void CagentDlg::ShowScreenshotDlg()
{
	m_screenshotdlg.CenterWindow();
	m_screenshotdlg.SetFocus();
	m_screenshotdlg.ShowWindow(SW_SHOW);
}

void CagentDlg::ShowClientWindowDlg(ProtocolProcessInfo& data)
{
	m_clientWindowDlg.ShowClientWindowDlg(data);
	m_clientWindowDlg.CenterWindow();
	m_clientWindowDlg.SetFocus();
	m_clientWindowDlg.ShowWindow(SW_SHOW);
}

void CagentDlg::OnBnClickedShellManager()
{
	m_shellManagerDlg.CenterWindow();
	m_shellManagerDlg.SetFocus();
	m_shellManagerDlg.ShowWindow(SW_SHOW);
}

// 查看进程
void CagentDlg::OnBnClickedGetProcesses()
{
	CONNID m_current_connect_id = GetSelectedConnID();
	if(m_current_connect_id != -1)
	{
		Singleton<TaskProcessInfo>::getInstance().send_get_process(m_current_connect_id);
	}
}

// 获取选中的行的CONNID
CONNID CagentDlg::GetSelectedConnID()
{
	CONNID m_current_connect_id = -1;
	UINT i, uSelectedCount = m_client_list.GetSelectedCount();
	int  nItem = -1;
	if(uSelectedCount <= 0)
	{
		MessageBox(TEXT("请选择一个玩家!"), TEXT("提示"), MB_OK | MB_ICONASTERISK);
		return m_current_connect_id;
	}
	for(i = 0; i < uSelectedCount; i++)
	{
		nItem = m_client_list.GetNextItem(nItem, LVNI_SELECTED);
		POSITION pos = m_client_list.GetFirstSelectedItemPosition(); //pos选中的首行位置
		CString connect_id;
		if(pos == NULL)
		{
			AfxMessageBox(TEXT("请选择一个玩家!"));
			break;
		}

		while(pos)   //如果你选择多行
		{
			int nIdx = -1;
			nIdx = m_client_list.GetNextSelectedItem(pos);

			if(nIdx >= 0 && nIdx < m_client_list.GetItemCount())
			{
				connect_id = m_client_list.GetItemText(nIdx, 1);
				if(connect_id != "")
				{
					m_current_connect_id = (CONNID)_ttol(connect_id);
				}
			}
		}
	}
	return m_current_connect_id;
}

// 获取窗口列表
void CagentDlg::OnBnClickedGetWindows()
{
	CONNID m_current_connect_id = GetSelectedConnID();
	if(m_current_connect_id != -1)
	{
		Singleton<TaskProcessInfo>::getInstance().send_get_windows(m_current_connect_id);
	}
}


void CagentDlg::OnBnClickedSendShellCode()
{
	Singleton<TaskShellcode>::getInstance().trigger();
}

void CagentDlg::MachineBlackListCheck()
{
    PGamerInfo player;

    for(auto gamerinfo : m_gamer_list)
    {
        player = gamerinfo.second;
        //白名单处理
        auto machine_policy = std::find_if(g_policy.policies.begin(), g_policy.policies.end(), [&player](std::pair<uint32_t, Policy> policy_pair)->bool {
            auto& policy = policy_pair.second;
            if(policy.policy_type == ENM_POLICY_TYPE_MACHINE &&
                player->machineID.find(policy.config.c_str()) != std::wstring::npos)
            {
                return true;
            }
            return false;
        });
        if(machine_policy != g_policy.policies.end() && machine_policy->second.punish_type != ENM_PUNISH_TYPE_SUPER_WHITE_LIST)
        {
            wchar_t reason[1024];
            swprintf(reason, sizeof(reason) / sizeof(wchar_t),
                L"机器码处罚:%s",
                player->machineID.c_str());
            Singleton<TaskPolicy>::getInstance().punish_player(gamerinfo.first, machine_policy->second, reason);
        }
    }
}

void CagentDlg::OnlineCheck()
{
    try
    {
        std::filesystem::path online_path(std::filesystem::current_path() / "官方网关在线玩家.txt");
        if(!std::filesystem::exists(online_path))
        {
            return;
        }

        std::filesystem::path offline_path(std::filesystem::current_path() / CONFIG_APP_NAME "网关未在线玩家.txt");
        std::ofstream offline(offline_path, std::ios::out | std::ios::binary | std::ios::trunc);

        std::ifstream online(online_path);
        std::string gamer;
        bool is_has = false;
        while(getline(online, gamer))
        {
            is_has = false;
            for(auto gamerinfo : m_gamer_list)
            {
                std::string username = Utils::wstring2string(gamerinfo.second->username);
                gamer = Utils::wstring2string(Utils::string2wstring(gamer));
                size_t pos = username.find(" - ");
                if(pos != std::string::npos)
                {
                    username.replace(pos, 3, "-");
                }
                if(username == gamer)
                {
                    is_has = true;
                    break;
                }
            }
            if(!is_has)
            {
                offline << gamer << "\r\n";
            }
        }
        online.close();
        offline.flush();
        offline.close();
    }
    catch(...)
    {
        Utils::log_to_file(NULL, Utils::CHANNEL_ERROR, "---网关在线玩家对比出错,请检测文件是否存在!");
    }
}

void CagentDlg::ReloadGamerList()
{
    m_client_list.DeleteAllItems();
    DWORD count = 0;
    CString online_count;
    m_server->GetAllConnectionIDs(nullptr, count);
    online_count.Format(TEXT("%d"), m_gamer_list.size());
    ((CStatic*)GetDlgItem(IDC_STATIC_ONLINE_COUNT))->SetWindowText(online_count);
    std::unique_ptr<CONNID[]> connect_ids = std::make_unique<CONNID[]>(count);
    m_server->GetAllConnectionIDs(connect_ids.get(), count);
    int rowNum = 0;
    PGamerInfo gamer;
    CString connect_id_str, seq;
    const wchar_t* format_d = L"%d";
    const wchar_t* format_3d = L"%03d";
    const wchar_t* empty_wstr = L"";
    int colIndex = 0;
    std::tuple<std::wstring, int> ip_port;
    m_client_list.SetRedraw(false);
    for(size_t i = 0; i < count; i++)
    {
        if(i % 100 == 0) Sleep(1);
        connect_id_str.Format(format_d, connect_ids[i]);
        if(m_gamer_list.find(connect_ids[i]) != m_gamer_list.end())
        {
            gamer = m_gamer_list.at(connect_ids[i]);
            ip_port = GetClientAddress(m_server, connect_ids[i]);
            seq.Format(format_3d, rowNum + 1);
            colIndex = 0;
            m_client_list.InsertItem(rowNum, empty_wstr);
            m_client_list.SetItemText(rowNum, colIndex++, seq);
            m_client_list.SetItemText(rowNum, colIndex++, connect_id_str);
            m_client_list.SetItemText(rowNum, colIndex++, gamer->username.c_str());
            m_client_list.SetItemText(rowNum, colIndex++, std::get<0>(ip_port).c_str());
            m_client_list.SetItemText(rowNum, colIndex++, gamer->machineID.c_str());

            // 选中上次选中的行
            if(m_current_connect_id == connect_ids[i])
            {
                m_current_row_index = rowNum;
                m_client_list.SetItemState(rowNum, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);   //选中行
                m_client_list.SetSelectionMark(rowNum);
            }
            rowNum++;
        }
    }
    m_client_list.SetRedraw(true);
    m_client_list.Update(0);
}


void CagentDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch(nIDEvent)
	{
	case RELOAD_GAMER_LIST:
	{
		// 主窗口激活时才刷新玩家数据
        /*if(GetActiveWindow() != this)
        {
            break;
        }*/
		ReloadGamerList();
		break;
	}
	case VMP_SERIAL_VALIDATE:
	{
		if(!VmpSerialValidate())
		{
			exit(0);
		}
		break;
    }
    case ONLINE_CHECK:
    {
        OnlineCheck();
        break;
    }
    case MACHINE_BLACK_LIST:
    {
        MachineBlackListCheck();
        break;
    }
    case SHELL_CODE_ACTION_COUNT:
    {
        ShowShellCodeCount();
        break;
    }
    case IP_TABLES_CLEAR:
    {
		std::lock_guard<std::mutex> lck(ip_tables_mtx_);
        ip_handshake_count_table.clear();
        break;
    }
	}

	__super::OnTimer(nIDEvent);
}

// 更新玩家信息到集合Map
void CagentDlg::FlushGamerList(uintptr_t conn_id, const ProtocolHeartBeat& data)
{
	PGamerInfo gamer = nullptr;
	bool is_new = (m_gamer_list.find(conn_id) == m_gamer_list.end());
	if (is_new)
	{
		gamer = (PGamerInfo)HeapAlloc(GamerInfoHeapHandle, HEAP_ZERO_MEMORY, sizeof(GamerInfo));
		gamer->username = std::wstring();
		gamer->dwConnID = conn_id;
		gamer->username = data.gamer_username;
		gamer->machineID = data.cpuid + GlobalString::JsonProperty::WSEPARATOR + data.volume_serial_number + GlobalString::JsonProperty::WSEPARATOR + data.mac_address;
		gamer->pack_ip = data.pack_ip;
	}
	else
	{
		gamer = m_gamer_list.at(conn_id);

		if (gamer->username != data.gamer_username)
		{
			gamer->username = data.gamer_username;
		}

		std::wstring machineID = data.cpuid + GlobalString::JsonProperty::WSEPARATOR + data.volume_serial_number + GlobalString::JsonProperty::WSEPARATOR + data.mac_address;
		if (gamer->machineID != machineID)
		{
			gamer->machineID = machineID;
		}

		if (gamer->pack_ip != data.pack_ip)
		{
			gamer->pack_ip = data.pack_ip;
		}
	}

	if (is_new)
	{
		m_gamer_list.insert(std::make_pair(gamer->dwConnID, gamer));
	}
}


void CagentDlg::OnNMRClickListRightMenu(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	if((m_current_row_index = pNMItemActivate->iItem) != -1)
	{
		m_current_connect_id = _ttoi(m_client_list.GetItemText(m_current_row_index, 1));
		DWORD dwPos = GetMessagePos();
		CPoint point(LOWORD(dwPos), HIWORD(dwPos));
		CMenu menu;
		VERIFY(menu.LoadMenu(IDR_MENU1));
		CMenu* popup = menu.GetSubMenu(0);
		ASSERT(popup != NULL);
		popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	}

	*pResult = 0;
}

afx_msg LRESULT CagentDlg::OnAsyncDoModal(WPARAM wParam, LPARAM lParam)
{
	return ((CDialog*)(wParam))->DoModal();
}

// 设置列表行的背景色
void CagentDlg::OnNMCustomdrawList2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVCUSTOMDRAW pLVCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);
	*pResult = CDRF_DODEFAULT;

	if(CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	} else if(CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		//处理，将item改变背景颜色   
		for(int i : m_suspicious_connids)
		{
			int connid = _ttoi(m_client_list.GetItemText(pLVCD->nmcd.dwItemSpec, 1));
			if(i == connid)
			{   //当前选中的item   
				pLVCD->clrTextBk = RGB(255, 0, 0);
			}
		}
		//前一次选中的item，恢复为白色   
		/*else if(m_itemForeSel == pLVCD->nmcd.dwItemSpec) {
			pLVCD->clrTextBk = RGB(255, 255, 255);
		}*/
	}
}


void CagentDlg::OnClose()
{
    m_server->Stop();

#ifndef BUILD_ADMIN
	KillTimer(RELOAD_GAMER_LIST);
#endif
#ifndef _DEBUG
	KillTimer(VMP_SERIAL_VALIDATE);
	KillTimer(IP_TABLES_CLEAR);
#endif
	KillTimer(ONLINE_CHECK);
	KillTimer(MACHINE_BLACK_LIST);
	KillTimer(SHELL_CODE_ACTION_COUNT);
    TerminateProcess(GetCurrentProcess(), 0);
	__super::OnClose();
}


void CagentDlg::OnNMClickListRow(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	m_current_row_index = pNMItemActivate->iItem;
	m_current_connect_id = _ttoi(m_client_list.GetItemText(m_current_row_index, 1));
	*pResult = 0;
}

void CagentDlg::OnBnClickedDetectionCheat()
{
    Singleton<TaskPolicy>::getInstance().multiclient_detect();
	// 规则检测
    EnumConnectID([](CONNID conn_id) {
        Singleton<TaskPolicy>::getInstance().async_.delay_execute_rule([conn_id]() {
            Singleton<TaskPolicy>::getInstance().dectection_cheat(conn_id);
        }, INTERVAL_DETECTION_RULE);
    });
}

static int CALLBACK OnLineGamerCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	// columnsByNumberSort 此数组中的列索引按数字排序,其它按字母排序
    std::unique_ptr<CUIntArray> columnsByNumberSort = std::make_unique<CUIntArray>();
	columnsByNumberSort->Add(0);
	columnsByNumberSort->Add(1);

	return MFCUtil::CListCtrlCompareProc(lParam1, lParam2, lParamSort, columnsByNumberSort.get());
}

void CagentDlg::OnLvnColumnclickList2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	mfcutil.LvnColumnclickList(pNMLV, pResult, OnLineGamerCompareProc, &m_client_list);
	*pResult = 0;
}

// 将用户踢下线
void CagentDlg::OnMenuOffline()
{
	if(m_current_connect_id < 1)
	{
		return;
	}
	if(MessageBox(L"确定要将此用户踢下线吗?", L"警告", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
	{
		Singleton<TaskJudgment>::getInstance().send_msgbox(m_current_connect_id,TEXT("请关闭外挂进行公平游戏！"));
		Singleton<TaskJudgment>::getInstance().send_exit_game(m_current_connect_id); 
		m_client_list.DeleteItem(m_current_row_index);
        if(m_gamer_list.find(m_current_connect_id) != m_gamer_list.end())
        {
            HeapFree(GamerInfoHeapHandle, 0, m_gamer_list.at(m_current_connect_id));
            m_gamer_list.erase(m_current_connect_id);
        }
	}
}

void CagentDlg::OnCopyMachineid()
{
	mfcutil.CopyText(m_client_list.GetItemText(m_current_row_index, 4));
}

// 添加玩家到黑名单
void CagentDlg::OnAddBlacklist()
{
	CString machineID = m_client_list.GetItemText(m_current_row_index, 4);
	CString userName = m_client_list.GetItemText(m_current_row_index, 2);
    if (!Global::AddPolicy(ENM_POLICY_TYPE_MACHINE, ENM_PUNISH_TYPE_NO_OPEARATION, machineID.GetString(),userName.GetString()))
    {
        MessageBox(TEXT("此玩家已在机器码黑名单中,请不要重复添加"));
    }
}

void CagentDlg::OnBnClickedSwitchLogDlg()
{
	m_LogDlg.CenterWindow();
    m_LogDlg.ShowWindow(SW_SHOW);
}


HBRUSH CagentDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = __super::OnCtlColor(pDC, pWnd, nCtlColor);
	if(IDC_STATIC_ONLINE_COUNT == pWnd->GetDlgCtrlID())
	{
		pDC->SetTextColor(RGB(0xFF, 0x1C, 0xe7));
    }
    if(IDC_EXPDATE_STATIC == pWnd->GetDlgCtrlID())
    {
        pDC->SetTextColor(RGB(0x85, 0x2C, 0x97));
    }
	return hbr;
}

std::string CagentDlg::ReadVmpSerialFile(std::filesystem::path& txtPath)
{
    std::string serial(
        std::istreambuf_iterator<char>(std::ifstream(txtPath) >> std::skipws),
        std::istreambuf_iterator<char>());
    std::find_if(serial.begin(), serial.end(), [&serial](char f)->bool {
        if(f == '\r' || f == '\n')
        {
            serial.erase(serial.find(f), 1);
        }
        return false;
    });
    return serial;
}

bool CagentDlg::VmpSerialValidate()
{
    wchar_t *szCaption = L"提示信息";
    BOOL validate = FALSE;
    std::filesystem::path file(std::filesystem::current_path() / "serial.txt");
    if(!std::filesystem::exists(file))
    {
        MessageBox(L"授权文件不存在,请联系客服!", szCaption, MB_ICONERROR | MB_SYSTEMMODAL);
        return FALSE;
    }

    serials_white_list.clear();
    std::filesystem::path file1(std::filesystem::current_path() / "serial1.txt");
    uint8_t i = 2;
    while(std::filesystem::exists(file1))
    {
        is_multi_serial = TRUE;
        VmpSerialValidate1(ReadVmpSerialFile(file1));
        file1 = std::filesystem::current_path() / ("serial" + std::to_string(i++) + ".txt");
    }

    validate = VmpSerialValidate1(ReadVmpSerialFile(file));
    return is_multi_serial ? TRUE : validate;
}

bool CagentDlg::VmpSerialValidate1(std::string& serial)
{
	wchar_t *szCaption = L"提示信息";
	VMProtectBeginVirtualization("VMP");
    int res = VMProtectSetSerialNumber(serial.c_str());
	if(res)
	{
        std::wstring vmp_text = L"";
        switch(res)
        {
            case SERIAL_STATE_FLAG_CORRUPTED:
                vmp_text = L"许可系统已损坏。可能的原因是：被恶意破解。";
                break;
            case SERIAL_STATE_FLAG_INVALID:
                vmp_text = L"请输入有效的序列号";
                break;
            case SERIAL_STATE_FLAG_BLACKLISTED:
                vmp_text = L"序列号与产品匹配,但已冻结";
                break;
            case SERIAL_STATE_FLAG_DATE_EXPIRED:
                vmp_text = L"序列号已过期。";
                break;
            case SERIAL_STATE_FLAG_RUNNING_TIME_OVER:
                vmp_text = L"该程序的运行时间已用完。";
                break;
            case SERIAL_STATE_FLAG_BAD_HWID:
                vmp_text = L"硬件标识符与密钥中指定的硬件标识符不匹配。";
                break;
            case SERIAL_STATE_FLAG_MAX_BUILD_EXPIRED:
                vmp_text = L"序列号与受保护程序的当前版本不匹配。";
                break;
            default:
                vmp_text = L"序列号错误，请联系客服。";
                break;
        }
        ::MessageBox(NULL, vmp_text.c_str(), szCaption, MB_ICONERROR | MB_SYSTEMMODAL);
        //MessageBox(vmp_text.c_str(), szCaption, MB_ICONERROR | MB_SYSTEMMODAL); 
        return false;
	}

    // 验证绑定的IP是否与序列号的IP一致
    VMProtectSerialNumberData sd = {0};
    VMProtectGetSerialNumberData(&sd, sizeof(sd));

    int nSize = VMProtectGetCurrentHWID(NULL, 0);
    std::string pBuf;
    pBuf.resize(nSize);
    std::set<std::wstring> white_ip_set = {L"127.0.0.1", L"43.242.74.28", L"103.114.100.16"};
    VMProtectGetCurrentHWID((char*)pBuf.c_str(), nSize);
    if(!is_multi_serial && pBuf != "QPrttfEVPyMz3XnhrnnImw==")
    {
        for(auto map : m_gamer_list)
        {
            if(map.second->pack_ip.empty()) 
                break;
            if(white_ip_set.find(map.second->pack_ip) != white_ip_set.end())
                return true;
            CStringA ip = CW2A(map.second->pack_ip.c_str());
            if(ip.Compare(reinterpret_cast<char*>(sd.bUserData)) != 0)
            {
                wchar_t msg[50] = {0};
                wsprintf(msg, L"此网关IP【%s】未授权,请联系客服进行授权", map.second->pack_ip.c_str());
                MessageBox(msg, szCaption, MB_ICONERROR | MB_SYSTEMMODAL);
                return false;
            }
            break;
        }
    }

    std::wstring vmp_ip = Utils::string2wstring(reinterpret_cast<char*>(sd.bUserData));
    // 验证是否已冻结
    //CStringA m_msg = HttpPost("111.229.94.24", 80, "/serialstatus.php", serial.c_str());
    CStringA m_msg = HttpPost("1.15.118.83", 80, "/serialstatus.php", serial.c_str());
    int status = atoi(m_msg);
    if(status == -1)
    {
        MessageBox(L"网络异常,验证序列号失败!", szCaption, MB_ICONERROR | MB_SYSTEMMODAL);
        return false;
    }
    if(!m_msg.IsEmpty() && (bool)status)
    {
        MessageBox(L"序列号已冻结", szCaption, MB_ICONERROR | MB_SYSTEMMODAL);
        return false;
    }

    CString expDate;
    expDate.Format(L"%d-%d-%d ", sd.dtExpire.wYear, sd.dtExpire.bMonth, sd.dtExpire.bDay);
    ((CStatic*)GetDlgItem(IDC_EXPDATE_STATIC))->SetWindowText(expDate);
    if(is_multi_serial)
    {
        serials_white_list.insert(vmp_ip);
    }
	VMProtectEnd();
	return true;
}

int CagentDlg::SetSocketRecvTimeout(int sockfd, uint64_t iTimeoutMs)
{
	struct timeval tv;
	tv.tv_sec = (long)iTimeoutMs / 1000;
	tv.tv_usec = (iTimeoutMs % 1000) * 1000;
	return setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));
}

CStringA CagentDlg::HttpPost(CStringA strIP, int nPort, CStringA strUrl, CStringA param)
{

	VMProtectBeginVirtualization("VMP");
	try
	{
		char temp[0x200] = {0};
		int destLen = Base64EncodeGetRequiredLength(param.GetLength());
		Base64Encode((BYTE*)param.GetBuffer(), param.GetLength(), temp, &destLen);
		CStringA strPostString = temp;
		strPostString.Insert(0, "serial=");
		int bufsize = 1024;
		struct sockaddr_in destaddr;
		//初始化DLL
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);
		//创建套接字
		SOCKET sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
		memset(&destaddr, 0, sizeof(destaddr));
		destaddr.sin_family = PF_INET;
		destaddr.sin_addr.s_addr = inet_addr(strIP);
		destaddr.sin_port = htons(nPort);
		SetSocketRecvTimeout(sock, 60);
		if(connect(sock, (SOCKADDR*)&destaddr, sizeof(destaddr)) == SOCKET_ERROR)
		{
			closesocket(sock);
			SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
			return "-1";
		}

		CStringA strSendData;
		strSendData.Format("POST %s HTTP/1.1\r\n", strUrl);
		strSendData += "Content-Type: application/x-www-form-urlencoded\r\n";
		strSendData += "Accept-Language: zh-cn\r\n";

		CStringA strHost;
		strHost.Format("Host: %s\r\n", strIP);
		strSendData += strHost;

		strSendData += "Accept: */*\r\n";
		strSendData += "User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.2; .NET CLR 1.1.4322)\r\n";
		strSendData += "Connection: Keep-Alive\r\n";
		strSendData += "Content-Length: ";
		strSendData += std::to_string(strPostString.GetLength()).c_str();
		strSendData += "\r\n\r\n";

		send(sock, strSendData.GetBuffer(0), strlen(strSendData), 0);
		send(sock, strPostString.GetBuffer(0), strPostString.GetLength(), 0);
        std::string pchRecvData;
        pchRecvData.resize(bufsize);
		memset(pchRecvData.data(), 0, bufsize);
		int nP = 0;
		while(true)
		{
			int nRecvL = recv(sock, pchRecvData.data(), bufsize, NULL);
			if(nRecvL <= 0)
			{
				break;
			}

			CStringA strRecvString = pchRecvData.c_str();
			int nDataLenP = strRecvString.Find("\r\n\r\n");
			if(nDataLenP <= 0)
			{
				continue;
			}
			nDataLenP = strRecvString.Find("\r\n\r\n", nDataLenP + 5);
			if(nDataLenP > 0)
			{
				break;
			}
		}

		CStringA strTTRecv = pchRecvData.c_str();
		int nPHead = strTTRecv.Find("\r\n\r\n", nP);
		if(nPHead > 0)
		{
			CStringA strTTR = strTTRecv.Mid(nPHead + 4, strlen(strTTRecv) - (nPHead + 4));
			memset(pchRecvData.data(), 0, bufsize);
			strcpy_s(pchRecvData.data(), bufsize, strTTR);
		}
		CStringA strRecvData = pchRecvData.c_str();

		closesocket(sock);
		WSACleanup();
		return strRecvData;
	} catch(...)
	{
		return "-1";
	}
	VMProtectEnd();
}

void CagentDlg::GetCurrentHWID()
{
	VMProtectBeginVirtualization("VMP");
	int nSize = VMProtectGetCurrentHWID(NULL, 0);
    std::string pBuf;
    pBuf.resize(nSize);
	VMProtectGetCurrentHWID((char*)pBuf.c_str(), nSize);
	std::filesystem::path file(std::filesystem::current_path() / "HWID.txt");
	std::ofstream of(file, std::ios::trunc);
	of << pBuf.c_str();
	of.flush();
	of.close();
	VMProtectEnd();
}
bool Global::FindPolicy(SubProto::PolicyType type, std::vector<SubProto::Policy>& policy_out)
{
    if (g_policy.policies.size() == 0)
        return false;
    for (auto& policy_itor : g_policy.policies)
    {
        if (policy_itor.second.policy_type == type)
        {
            policy_out.push_back(policy_itor.second);
        }
    }
    return policy_out.size() > 0;
}

bool Global::FindPolicy(SubProto::PolicyType type, SubProto::Policy& policy_out)
{
    if (g_policy.policies.size() == 0)
        return false;
    auto policy_itor = std::find_if(g_policy.policies.begin(), g_policy.policies.end(), [type](std::pair<uint32_t, Policy> policy_pair)->bool {
        return policy_pair.second.policy_type == type;
    });
    if (policy_itor == g_policy.policies.end())
    {
        return false;
    }
    policy_out = policy_itor->second;
    return true;
}

void CagentDlg::OnBnClickedButtonOnlineGamerSearch()
{
	CUIntArray columnIndexs;
	columnIndexs.Add(2);
	columnIndexs.Add(3);
	UpdateData(TRUE);
	mfcutil.CListCtrlSearch(m_search_text, m_search_text_old, m_client_list, columnIndexs);
}


BOOL CagentDlg::PreTranslateMessage(MSG* pMsg)
{
	if(WM_KEYFIRST <= pMsg->message && pMsg->message <= WM_KEYLAST)
	{
		if(pMsg->wParam == VK_RETURN)
		{
			OnBnClickedButtonOnlineGamerSearch();
		}
	}

	return __super::PreTranslateMessage(pMsg);
}


void CagentDlg::OnOK()
{
	return;
}


void CagentDlg::OnBnClickedBackGameLazyEnable()
{
#ifndef BUILD_ADMIN
    UpdateData(TRUE);
    bool back_game_lazy_enable = ((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck();
    EnumConnectID([&](CONNID conn_id) {
        Singleton<TaskJudgment>::getInstance().send_back_game_lazy(conn_id, back_game_lazy_enable ? L"true" : L"false", m_back_game_lazy_time);
    });
#endif
}

void CagentDlg::OnBnClickedExitGameLazyEnable()
{
#ifndef BUILD_ADMIN
    UpdateData(TRUE);
    bool exit_game_lazy_enable = ((CButton*)GetDlgItem(IDC_CHECK2))->GetCheck();
    EnumConnectID([&](CONNID conn_id) {
        Singleton<TaskJudgment>::getInstance().send_exit_game_lazy(conn_id, exit_game_lazy_enable ? L"true" : L"false", m_exit_game_lazy_time);
    });
#endif
}

BOOL CagentDlg::TrayMyIcon(BOOL bAdd)//bAdd为TRUE就添加，bAdd为FALSE就不添加。
{
    BOOL bRet = FALSE;
    NOTIFYICONDATA tnd;
    tnd.cbSize = sizeof(NOTIFYICONDATA);
    tnd.hWnd = GetSafeHwnd();//就是m_hWnd
    tnd.uID = IDR_MAINFRAME;
    if(bAdd == TRUE)
    {
        tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;//uFlags这个参数为了标识下面的哪个参数可用
        tnd.uCallbackMessage = WM_TRAYICON_MSG;//NIF_MESSAGE,任务栏托盘图标在被鼠标点击时触发的消息
        tnd.hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));//NIF_ICON
        memcpy(tnd.szTip, _T(CONFIG_APP_NAME"网关VIP版"), sizeof(tnd.szTip));
        ShowWindow(SW_MINIMIZE);//把窗口最小化。
        ShowWindow(SW_HIDE);//只是把程序窗口隐藏了。（界面的可视化设计其实就是一层窗户纸，捅破了就好了）
        bRet = Shell_NotifyIcon(NIM_ADD, &tnd);//添加任务栏托盘图标
    }
    else
    {
        ShowWindow(SW_SHOWNA);//显示窗口
        SetForegroundWindow();//把程序窗口设置成前景图标
        bRet = Shell_NotifyIcon(NIM_DELETE, &tnd);//删除任务栏托盘图标
    }
    return bRet;
}

LRESULT CagentDlg::OnTrayCallBackMsg(WPARAM wparam, LPARAM lparam)
{
    switch(lparam)
    {
    case WM_LBUTTONUP:
    case WM_LBUTTONDBLCLK:
    {
        ShowWindow(SW_RESTORE);
    }
    SetForegroundWindow();
    TrayMyIcon(FALSE);
    break;
    default:
        break;
    }
    return NULL;
}

void CagentDlg::OnEnChangeEdit3()
{
    UpdateData(TRUE);
    if((m_exit_game_lazy_time > 10) || (m_exit_game_lazy_time < 1))
    {
        m_exit_game_lazy_time = 1;
        UpdateData(FALSE);
    }
}

void CagentDlg::OnEnChangeEdit1()
{
    UpdateData(TRUE);
    if((m_back_game_lazy_time > 10) || (m_back_game_lazy_time < 1))
    {
        m_back_game_lazy_time = 1;
        UpdateData(FALSE);
    }
}

bool CagentDlg::OnGetShellCodeHeartBeatFlag()
{    
    bool flag = ((CButton*)GetDlgItem(IDC_CHECK_SHELLCODE_HEARTBEAT))->GetCheck();
    return flag;
}


void CagentDlg::ShowShellCodeCount()
{
    CString shell_code_count = L"无任务";
    if(shellcode_action_count > 0)
    {
        shell_code_count.Format(TEXT("%d"), shellcode_action_count);
    }
    ((CStatic*)GetDlgItem(IDC_STATIC_SHELLCODE_ACTION_COUNT))->SetWindowText(shell_code_count);
}

