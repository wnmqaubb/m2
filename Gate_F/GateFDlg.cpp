// GateFDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "GateF.h"
#include "GateFDlg.h"
#include "DlgProxy.h"
#include "afxdialogex.h"
#include "CGamesDlg.h"
#include <Psapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CGateFDlg 对话框
#define LOGIC_SERVER_KICK_LIST_FILE ".\\恶性开挂人员名单.txt"

IMPLEMENT_DYNAMIC(CGateFDlg, CDialogEx);

CGateFDlg::CGateFDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_GATEF_DIALOG, pParent)
{
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
    ON_COMMAND_RANGE(ID_INDICATOR_SERVER_STAUS, ID_INDICATOR_USERS_COUNT, NULL)
END_MESSAGE_MAP()

static UINT indicators[] =
{
    ID_SEPARATOR,           // 状态行指示器
    ID_INDICATOR_SERVER_STAUS,
    ID_INDICATOR_USERS_COUNT,
};

// CGateFDlg 消息处理程序

BOOL CGateFDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
    //  执行此操作
    m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_BLUE);
    SetWindowText(TEXT("及时雨定制版"));
#ifdef VERSION_RED
    SetWindowText(TEXT("及时雨鸿蒙版"));
    m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_RED);
#endif
#ifdef VERSION_VIP
    SetWindowText(TEXT("及时雨内部版"));
    m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_RED);
#endif
    SetIcon(m_hIcon, TRUE);			// 设置大图标
    SetIcon(m_hIcon, FALSE);		// 设置小图标

    InitStatusBar();
    // TODO: 在此添加额外的初始化代码
    // 初始化 CTabCtrl
    m_tab_main.GetClientRect(&m_tab_main_rect);
    m_tab_main.AdjustRect(FALSE, &m_tab_main_rect);
    m_tab_main_rect.DeflateRect(0, 20, 0, 0);
    m_tab_main.InsertItem(0, _T("在线玩家"));
    m_tab_main.InsertItem(1, _T("封挂"));
    m_tab_main.InsertItem(2, _T("策略"));
    m_tab_main.InsertItem(3, _T("进程"));
    m_tab_main.InsertItem(4, _T("日志"));
    ShowAllDlgInTab();

    // 填入一些静态树视图数据(此处只需填入虚拟代码，而不是复杂的数据)
    FillClientView();

    SetTimer(TIMER_ID_POLL_WORK_ID, 200, NULL);
    SetTimer(RELOAD_GAMER_LIST, 1000 * 60 * 10, NULL);
    if (theApp.is_parent_gate) {
        SetTimer(TIMER_ID_CHILD_SERIVCE_ID, 1000 * 30, NULL);
    }
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
    theApp.OnServiceStop();
    KillTimer(TIMER_ID_POLL_WORK_ID);
    KillTimer(RELOAD_GAMER_LIST);
    if (theApp.is_parent_gate) {
        KillTimer(TIMER_ID_CHILD_SERIVCE_ID);
    }
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

void CGateFDlg::SwitchToTab(int index)
{
    m_tab_main.SetCurSel(index);
    switch (index)
    {
        case 0:
            m_games_dlg->ShowWindow(SW_SHOW);
            m_anticheat_dlg->ShowWindow(SW_HIDE);
            m_polices_dlg->ShowWindow(SW_HIDE);
            m_process_info_dlg->ShowWindow(SW_HIDE);
            m_logs_dlg->ShowWindow(SW_HIDE);
            break;
        case 1:
            m_games_dlg->ShowWindow(SW_HIDE);
            m_anticheat_dlg->ShowWindow(SW_SHOW);
            m_polices_dlg->ShowWindow(SW_HIDE);
            m_process_info_dlg->ShowWindow(SW_HIDE);
            m_logs_dlg->ShowWindow(SW_HIDE);
            break;
        case 2:
            m_games_dlg->ShowWindow(SW_HIDE);
            m_anticheat_dlg->ShowWindow(SW_HIDE);
            m_polices_dlg->ShowWindow(SW_SHOW);
            m_process_info_dlg->ShowWindow(SW_HIDE);
            m_logs_dlg->ShowWindow(SW_HIDE);
            break;
        case 3:
            m_games_dlg->ShowWindow(SW_HIDE);
            m_anticheat_dlg->ShowWindow(SW_HIDE);
            m_polices_dlg->ShowWindow(SW_HIDE);
            m_process_info_dlg->ShowWindow(SW_SHOW);
            m_logs_dlg->ShowWindow(SW_HIDE);
            break;
        case 4:
            m_games_dlg->ShowWindow(SW_HIDE);
            m_anticheat_dlg->ShowWindow(SW_HIDE);
            m_polices_dlg->ShowWindow(SW_HIDE);
            m_process_info_dlg->ShowWindow(SW_HIDE);
            m_logs_dlg->ShowWindow(SW_SHOW);
            break;
        default:
            break;
    }
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

    m_process_info_dlg = std::make_unique<CProcessInfoDlg>();
    m_process_info_dlg->Create(IDD_DIALOG_PROCESSINFO, &m_tab_main);
    m_process_info_dlg->MoveWindow(m_tab_main_rect);
    m_process_info_dlg->ShowWindow(SW_HIDE);

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
            m_process_info_dlg->ShowWindow(SW_HIDE);
            m_logs_dlg->ShowWindow(SW_HIDE);
            break;
        case 1:
            m_games_dlg->ShowWindow(SW_HIDE);
            m_anticheat_dlg->ShowWindow(SW_SHOW);
            m_polices_dlg->ShowWindow(SW_HIDE);
            m_process_info_dlg->ShowWindow(SW_HIDE);
            m_logs_dlg->ShowWindow(SW_HIDE);
            break;
        case 2:
            m_games_dlg->ShowWindow(SW_HIDE);
            m_anticheat_dlg->ShowWindow(SW_HIDE);
            m_polices_dlg->ShowWindow(SW_SHOW);
            m_process_info_dlg->ShowWindow(SW_HIDE);
            m_logs_dlg->ShowWindow(SW_HIDE);
            break;
        case 3:
            m_games_dlg->ShowWindow(SW_HIDE);
            m_anticheat_dlg->ShowWindow(SW_HIDE);
            m_polices_dlg->ShowWindow(SW_HIDE);
            m_process_info_dlg->ShowWindow(SW_SHOW);
            m_logs_dlg->ShowWindow(SW_HIDE);
            break;
        case 4:
            m_games_dlg->ShowWindow(SW_HIDE);
            m_anticheat_dlg->ShowWindow(SW_HIDE);
            m_polices_dlg->ShowWindow(SW_HIDE);
            m_process_info_dlg->ShowWindow(SW_HIDE);
            m_logs_dlg->ShowWindow(SW_SHOW);
            break;
        default:
            break;
    }
    *pResult = 0;
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
        const int port = _ttoi(m_games_dlg->m_list_games.GetItemText(selectedRow, 12));
        theApp.m_ObServerClientGroup(ip, port)->async_send(package);
    }
    else
    {
        AfxMessageBox(TEXT("请选择一个玩家!"), MB_OK);
    }
}

void CGateFDlg::FillClientView()
{
    m_games_dlg->m_list_games.SetColumnByIntSort({ 0, 1 });
    m_games_dlg->m_list_games.SetColumnBySearch({ 2, 3 });
    m_games_dlg->m_list_games.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_GRIDLINES);
    int colIndex = 0;
    m_games_dlg->m_list_games.InsertColumn(colIndex++, TEXT("序号"), LVCFMT_LEFT, 38);
    m_games_dlg->m_list_games.InsertColumn(colIndex++, TEXT("ID"), LVCFMT_LEFT, 0);
    m_games_dlg->m_list_games.InsertColumn(colIndex++, TEXT("角色名"), LVCFMT_LEFT, 250);
    m_games_dlg->m_list_games.InsertColumn(colIndex++, TEXT("登录IP地址"), LVCFMT_LEFT, 110);
    m_games_dlg->m_list_games.InsertColumn(colIndex++, TEXT("机器码"), LVCFMT_LEFT, 270);
    m_games_dlg->m_list_games.InsertColumn(colIndex++, TEXT("系统版本"), LVCFMT_LEFT, 80);
    m_games_dlg->m_list_games.InsertColumn(colIndex++, TEXT("登陆时间"), LVCFMT_LEFT, 115);
    m_games_dlg->m_list_games.InsertColumn(colIndex++, TEXT("在线时长"), LVCFMT_LEFT, 80);
    m_games_dlg->m_list_games.InsertColumn(colIndex++, TEXT("服务端IP"), LVCFMT_LEFT, 0);
    m_games_dlg->m_list_games.InsertColumn(colIndex++, TEXT("端口"), LVCFMT_LEFT, 0);
    m_games_dlg->m_list_games.DeleteAllItems();
}

bool CGateFDlg::isProcessRunning(const std::string& processName)
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

void CGateFDlg::OnTimer(UINT_PTR nIDEvent)
{
    switch (nIDEvent)
    {
        case RELOAD_GAMER_LIST:
        {
            m_games_dlg->OnRefreshUsers();
            break;
        }
        case TIMER_ID_POLL_WORK_ID:
        {
            theApp.m_WorkIo.poll_one();
            break;
        }
        case TIMER_ID_CHILD_SERIVCE_ID:
        {
            // 检查服务是否停止响应,如果停止响应则尝试重启
            // 定时检测g_service通讯是否正常,用发包并开始计时,如果在30秒内还没有回包就认为是不响应了,就结束g_service和g_logicserver再重启
            // 无响应
            if (!theApp.is_service_stauts)
            {
                ReStartChildProcess();
                break;
            }
            else {
                theApp.is_service_stauts = false;
            }

            bool service_stoped = false;
            bool logic_server_stoped = false;

            // 已停止
            HANDLE existingMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, L"mtx_service");
            if (existingMutex != NULL) {
                if (!isProcessRunning("g_Service.exe")) {
                    // 互斥体已存在，但是进程已经退出,尝试关闭和释放它
                    ReleaseMutex(existingMutex);
                    CloseHandle(existingMutex);
                    service_stoped = true;
                }
                else {
                    CloseHandle(existingMutex);
                }
            }
            else {
                service_stoped = true;
            }

            HANDLE existingMutex1 = OpenMutex(MUTEX_ALL_ACCESS, FALSE, L"mtx_logic_server");
            if (existingMutex1 != NULL) {
                if (!isProcessRunning("g_LogicServer.exe")) {
                    // 互斥体已存在，但是进程已经退出,尝试关闭和释放它
                    ReleaseMutex(existingMutex1);
                    CloseHandle(existingMutex1);
                    logic_server_stoped = true;
                }
                else {
                    CloseHandle(existingMutex1);
                }
            }
            else {
                logic_server_stoped = true;
            }

            if (service_stoped || logic_server_stoped)
            {
                ReStartChildProcess();
            }
            break;
        }
    }
    CDialogEx::OnTimer(nIDEvent);
}

// 进程停止，尝试重启
void CGateFDlg::ReStartChildProcess() {
    KillTimer(TIMER_ID_CHILD_SERIVCE_ID);
    // 尝试关闭所有子进程
    HANDLE hProcess = find_process("g_LogicServer.exe");
    if (hProcess) {
        TerminateProcess(hProcess, 0);
        CloseHandle(hProcess);
    }
    // 进程已停止，尝试重启
    hProcess = find_process("g_Service.exe");
    if (hProcess) {
        TerminateProcess(hProcess, 0);
        CloseHandle(hProcess);
    }
    theApp.OnServiceStop1();
    theApp.OnServiceStart();
    SetTimer(TIMER_ID_CHILD_SERIVCE_ID, 1000 * 30, NULL);
}

void CGateFDlg::InitStatusBar()
{
    if (!m_wndStatusBar.Create(this))
    {
        TRACE0("未能创建状态栏\n");
        return;
    }
    m_wndStatusBar.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT));
    // 设置状态栏的位置和大小
    CRect rect;
    GetClientRect(rect);
    m_wndStatusBar.SetWindowPos(NULL, 0, rect.bottom - 20, rect.right, 20, SWP_NOZORDER);
    int a, b;
    a = m_wndStatusBar.CommandToIndex(ID_INDICATOR_SERVER_STAUS);
    b = m_wndStatusBar.CommandToIndex(ID_INDICATOR_USERS_COUNT);
    m_wndStatusBar.SetPaneInfo(a, ID_INDICATOR_SERVER_STAUS, SBPS_NORMAL, 250);
    m_wndStatusBar.SetPaneInfo(b, ID_INDICATOR_USERS_COUNT, SBPS_NORMAL, 180);
    SetStatusBar(ID_INDICATOR_SERVER_STAUS, _T("已停止"));
    SetStatusBar(ID_INDICATOR_USERS_COUNT, _T("0"));
}

void CGateFDlg::SetStatusBar(UINT nIDResource, CString text)
{
    int index = m_wndStatusBar.CommandToIndex(nIDResource);
    CString strPrefix;
    if (index != -1)
    {
        strPrefix.LoadString(nIDResource);
        CString strText;
        strText = strPrefix + _T(":") + text;
        m_wndStatusBar.SetPaneText(index, strText);
    }
}

void CGateFDlg::SetPaneBackgroundColor(UINT nIDResource, COLORREF color)
{
    int vecIndex = m_wndStatusBar.CommandToIndex(nIDResource);
    //m_wndStatusBar.SetPaneBackgroundColor(vecIndex, color);
}

void CGateFDlg::OnServiceCommand(UINT id)
{
    int vecIndex = m_wndStatusBar.CommandToIndex(ID_INDICATOR_SERVER_STAUS);
    switch (id)
    {
        case ID_SERVICE_START:
        {
            SetStatusBar(ID_INDICATOR_SERVER_STAUS, _T("已启动"));
            //SetPaneBackgroundColor(ID_INDICATOR_SERVER_STAUS, RGB(0, 255, 0));
            break;
        }
        case ID_SERVICE_STOP:
        {
            SetStatusBar(ID_INDICATOR_SERVER_STAUS, _T("已停止"));
            //SetPaneBackgroundColor(ID_INDICATOR_SERVER_STAUS, RGB(255, 0, 0));
            break;
        }
        default:
            break;
    }
}

HANDLE CGateFDlg::find_process(const std::string& processName)
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