
// Gate.cpp: 定义应用程序的类行为。
//

#include "pch.h"
#include "framework.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "Gate.h"
#include "MainFrm.h"

#include "ProcessChildFrm.h"
#include "ProcessDoc.h"
#include "ProcessView.h"
#include "ChildFrm.h"
#include "DriverDoc.h"
#include "DriverView.h"
#include "WindowDoc.h"
#include "WindowView.h"
#include "CmdView.h"
#include "ConfigSettingChildFrm.h"
#include "ConfigSettingDoc.h"
#include "ConfigSettingView.h"

#include <3rdparty/vmprotect/VMProtectSDK.h>
#include <asio2/util/sha1.hpp>
#include <asio2/util/base64.hpp>
#include "../version.build"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#pragma data_seg("sing")
unsigned int giInstancePid = 0;
#pragma data_seg()
#pragma comment(linker,"/section:sing,RWS")


// CGateApp

BEGIN_MESSAGE_MAP(CGateApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CGateApp::OnAppAbout)
	ON_COMMAND(ID_SERVICE_START, &CGateApp::OnServiceStart)
	ON_COMMAND(ID_SERVICE_STOP, &CGateApp::OnServiceStop)
    ON_COMMAND(ID_APP_EXIT, &CGateApp::OnAppExit)
    ON_COMMAND(ID_SERVICE_SETTINGS, &CGateApp::OnServiceSettings)
    ON_COMMAND(ID_TOOLBAR_CONFIG, &CGateApp::OnConfig)
END_MESSAGE_MAP()

// CGateApp 构造

CGateApp::CGateApp() noexcept
{
    GetModuleFileNameA(NULL, m_ExeDir, sizeof(m_ExeDir));
    auto cParentDir = std::filesystem::path(m_ExeDir).parent_path().string();
    strcpy_s(m_ExeDir, cParentDir.c_str());
    m_cCfgPath = m_ExeDir;
    m_cCfgPath = m_cCfgPath + TEXT("\\config.cfg");

	m_bHiColorIcons = TRUE;
    m_childpHandle = NULL;

	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// 如果应用程序是利用公共语言运行时支持(/clr)构建的，则: 
	//     1) 必须有此附加设置，“重新启动管理器”支持才能正常工作。
	//     2) 在您的项目中，您必须按照生成顺序向 System.Windows.Forms 添加引用。
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: 将以下应用程序 ID 字符串替换为唯一的 ID 字符串；建议的字符串格式
	//为 CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("Gate.AppID.NoVersion"));

	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}

GATE_EXPORT CGateApp* GetGateAppInstance()
{
    return &theApp;
}

// 唯一的 CGateApp 对象

CGateApp theApp;


// CGateApp 初始化

BOOL CGateApp::InitInstance()
{
#ifndef GATE_ADMIN
    CString strCmdLine = AfxGetApp()->m_lpCmdLine;
    if (strCmdLine == TEXT("/StartService"))
    {
        giInstancePid = GetCurrentProcessId();
        HANDLE pHandles[2] = {};
        std::filesystem::path path(m_ExeDir);
        path = path / "Service.exe";
        STARTUPINFOA si = {};
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        PROCESS_INFORMATION pi = {};
        const std::string strCmdLine = " " + std::to_string(giInstancePid) + " 6";
        std::string strServiceCmdline = path.string() + strCmdLine;
        BOOL res = CreateProcessA(NULL,
            (char*)strServiceCmdline.c_str(),
            0,
            0,
            FALSE,
            NORMAL_PRIORITY_CLASS,
            NULL,
            m_ExeDir,
            &si,
            &pi);
        if (res == FALSE)
        {
            AfxMessageBox(TEXT("启动Service失败"));
        }
        pHandles[0] = pi.hProcess;
        path = path.parent_path() / "LogicServer.exe";
        std::string strLogicServerCmdline = path.string() + strCmdLine;
        res = CreateProcessA(NULL,
            (char*)strLogicServerCmdline.c_str(),
            0,
            0,
            FALSE,
            NORMAL_PRIORITY_CLASS,
            NULL,
            m_ExeDir,
            &si,
            &pi);
        if (res == FALSE)
        {
            AfxMessageBox(TEXT("启动LogicServer失败"));
        }
        pHandles[1] = pi.hProcess;
        WaitForMultipleObjects(2, pHandles, TRUE, INFINITE);
        ExitProcess(0);
        return FALSE;
    }
    auto observer_client_ = m_ObServerClientGroup.get_observer_client(kDefaultLocalhost, kDefaultServicePort);
    observer_client_->start(kDefaultLocalhost, kDefaultServicePort);
    observer_client_->set_auth_key(ReadAuthKey());
    observer_client_->get_gate_notify_mgr().register_handler(CLIENT_CONNECT_SUCCESS_NOTIFY_ID, [this]() {
        m_WorkIo.post([this]() {
            GetMainFrame()->OnServiceCommand(ID_SERVICE_START);
        });
});
    observer_client_->get_gate_notify_mgr().register_handler(CLIENT_DISCONNECT_NOTIFY_ID, [this]() {
        m_WorkIo.post([this]() {
            GetMainFrame()->OnServiceCommand(ID_SERVICE_STOP);
        });
    });
    m_ObServerClientGroup.create_threads();
#else
    ConnectionLicenses();
    
    m_ObServerClientGroup.create_threads(std::thread::hardware_concurrency() * 2);
#endif;
    
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。  否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	m_bSaveState = FALSE;

	// 初始化 OLE 库
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	// 使用 RichEdit 控件需要 AfxInitRichEdit2()
	// AfxInitRichEdit2();

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
#if 1
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));
	LoadStdProfileSettings(4);  // 加载标准 INI 文件选项(包括 MRU)
#endif

	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// 注册应用程序的文档模板。  文档模板
	// 将用作文档、框架窗口和视图之间的连接
    AddDocTemplate(GetDocTemplateMgr().Add("Process", IDS_STRING_PROCESS_EXT,
        RUNTIME_CLASS(CProcessDoc),
        RUNTIME_CLASS(CProcessChildFrame), // 自定义 MDI 子框架
        RUNTIME_CLASS(CProcessView)));
    AddDocTemplate(GetDocTemplateMgr().Add("Driver", IDS_STRING_DRIVER_EXT,
        RUNTIME_CLASS(CDriverDoc),
        RUNTIME_CLASS(CChildFrame), // 自定义 MDI 子框架
        RUNTIME_CLASS(CDriverView)));
    AddDocTemplate(GetDocTemplateMgr().Add("Window", IDS_STRING_WINDOW_EXT,
        RUNTIME_CLASS(CWindowDoc),
        RUNTIME_CLASS(CChildFrame), // 自定义 MDI 子框架
        RUNTIME_CLASS(CWindowView)));
    AddDocTemplate(GetDocTemplateMgr().Add("Config", IDS_STRING_CONFIG_EXT,
        RUNTIME_CLASS(CConfigSettingDoc),
        RUNTIME_CLASS(CConfigSettingChildFrame), // 自定义 MDI 子框架
        RUNTIME_CLASS(CConfigSettingView)));
#if defined(GATE_ADMIN)
    AddDocTemplate(GetDocTemplateMgr().Add("Cmd", IDS_STRING_CMD_EXT,
        RUNTIME_CLASS(CBaseDoc),
        RUNTIME_CLASS(CChildFrame), // 自定义 MDI 子框架
        RUNTIME_CLASS(CCmdView)));
#endif
	// 创建主 MDI 框架窗口
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;
#if 0
	// 分析标准 shell 命令、DDE、打开文件操作的命令行
	CCommandLineInfo cmdInfo;
    ParseCommandLine(cmdInfo);
    cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;

	// 调度在命令行中指定的命令。  如果
	// 用 /RegServer、/Register、/Unregserver 或 /Unregister 启动应用程序，则返回 FALSE。
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
#endif

	// 主窗口已初始化，因此显示它并对其进行更新
	pMainFrame->ShowWindow(m_nCmdShow);
    pMainFrame->UpdateWindow();

    m_wndConfig.Create(IDD_CONFIG_DIALOG);
    OnServiceStart();
	return TRUE;
}

int CGateApp::ExitInstance()
{
	//TODO: 处理可能已添加的附加资源
    m_ObServerClientGroup.stop();

	AfxOleTerm(FALSE);
	CleanState();
	return CWinAppEx::ExitInstance();
}

// CGateApp 消息处理程序


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg() noexcept;

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnToolbarStart();
    CStatic m_VersionStatic;
};

CAboutDlg::CAboutDlg() noexcept : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_ABOUT_VERSION, m_VersionStatic);
    m_VersionStatic.SetWindowText(TEXT(FILE_VERSION_STR));
}



BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// 用于运行对话框的应用程序命令
void CGateApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CGateApp 自定义加载/保存方法

void CGateApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDR_MAINFRAME);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_MAINFRAME);
}

void CGateApp::LoadCustomState()
{
}

void CGateApp::SaveCustomState()
{
}

CMainFrame* CGateApp::GetMainFrame()
{
    ASSERT(m_pMainWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
    return (CMainFrame*)m_pMainWnd;
}

// CGateApp 消息处理程序

void CGateApp::OnServiceStart()
{
#ifndef GATE_ADMIN
    if (m_childpHandle == NULL && giInstancePid == 0 )
    {
        STARTUPINFOA si = {};
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_SHOW;
        CHAR chrFullPath[MAX_PATH] = { 0 };
        GetModuleFileNameA(NULL, chrFullPath, sizeof(chrFullPath));

        PROCESS_INFORMATION pi = {};
        BOOL res = CreateProcessA(NULL,
            (char*)(std::string(chrFullPath) + " " + "/StartService").c_str(),
            0,
            0,
            FALSE,
            NORMAL_PRIORITY_CLASS,
            NULL,
            m_ExeDir,
            &si,
            &pi);
        if (res == FALSE)
        {
            LogPrint(ObserverClientLog, TEXT("启动服务失败"));
            return;
        }
        else
        {
            LogPrint(ObserverClientLog, TEXT("启动服务成功"));
            theApp.GetMainFrame()->SetStatusBar(ID_INDICATOR_SERVER_STAUS, _T("已启动"));
            theApp.GetMainFrame()->SetPaneBackgroundColor(ID_INDICATOR_SERVER_STAUS, RGB(0, 255, 0));
        }
        m_childpHandle = pi.hProcess;
    }
    else
    {
        LogPrint(ObserverClientLog, TEXT("检测到服务已启动"));
    }
#endif
}


void CGateApp::OnServiceStop()
{
#ifndef GATE_ADMIN
    if(m_pMainWnd->MessageBox(TEXT("确认要停止服务吗?"), TEXT("提示"), MB_YESNO) == IDYES)
    {
        if(m_childpHandle == NULL && giInstancePid != 0)
        {
            m_childpHandle = OpenProcess(PROCESS_TERMINATE, FALSE, giInstancePid);
        }
        if (m_childpHandle)
        {
            TerminateProcess(m_childpHandle, 0);
            CloseHandle(m_childpHandle);
            m_childpHandle = NULL;
            giInstancePid = 0;
        }
        LogPrint(ObserverClientLog, TEXT("服务已停止"));
        theApp.GetMainFrame()->SetStatusBar(ID_INDICATOR_SERVER_STAUS, _T("已停止"));
        theApp.GetMainFrame()->SetPaneBackgroundColor(ID_INDICATOR_SERVER_STAUS, RGB(255, 0, 0));
    }
#endif
}

void CGateApp::OnAppExit()
{
    ASSERT(m_pMainWnd != NULL);
    m_pMainWnd->SendMessage(WM_CLOSE);
}


void CGateApp::OnServiceSettings()
{
    auto tConfig = GetDocTemplateMgr().Find("Config");
    tConfig->CloseAllDocuments(TRUE);
    m_ConfigDoc = tConfig->OpenDocumentFile(m_cCfgPath);
    if (!m_ConfigDoc)
    {
        ProtocolS2CPolicy policy;
        std::ofstream cfg(m_cCfgPath.GetBuffer(), std::ios::out | std::ios::binary);
        auto buffer = policy.dump();
        cfg.write(buffer.data(), buffer.size());
        cfg.close();
        m_ConfigDoc = tConfig->OpenDocumentFile(m_cCfgPath);
    }
}

void CGateApp::OnConfig()
{
    OnServiceSettings();
    m_wndConfig.ShowConfigDlg();
}


// 打开并定位文件
BOOL CGateApp::OpenFolderAndSelectFile(CString szPath)
{
    LPSHELLFOLDER pDesktopFolder;
    TCHAR szFullPath[MAX_PATH] = { 0 };
    GetFullPathName(szPath, MAX_PATH, szFullPath, NULL);
    if (SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder)))
    {
        LPITEMIDLIST pidl;
        ULONG chEaten;
        ULONG dwAttributes;
        HRESULT hr;
        hr = pDesktopFolder->ParseDisplayName(
            NULL, NULL, szFullPath, &chEaten, &pidl, &dwAttributes);
        if (FAILED(hr))
        {
            pDesktopFolder->Release();
            return FALSE;
        }
        LPCITEMIDLIST pidlFolder = pidl;
        CoInitialize(NULL);
        hr = SHOpenFolderAndSelectItems(pidl, 0, NULL, 0);
        pDesktopFolder->Release();
        if (hr == S_OK)
        {
            return TRUE;
        }
    }
    return FALSE;
}


std::string CGateApp::ReadLicense()
{
    std::filesystem::path path(m_ExeDir);
    path = path / "serial.txt";
    if (std::filesystem::exists(path) == false)
        return "";
    std::string serial(
        std::istreambuf_iterator<char>(std::ifstream(path) >> std::skipws),
        std::istreambuf_iterator<char>());
    std::find_if(serial.begin(), serial.end(), [&serial](char f)->bool {
        if (f == '\r' || f == '\n')
        {
            serial.erase(serial.find(f), 1);
        }
        return false;
    });
    return serial;
}

std::string CGateApp::ReadAuthKey()
{
    std::string sn = ReadLicense();
    VMProtectBeginVirtualization(__FUNCTION__);
    auto sn_sha1 = asio2::sha1(asio2::base64().decode(sn));
    auto auth_key = asio2::base64().encode((unsigned char*)&sn_sha1, sizeof(sn_sha1));
    VMProtectEnd();
    return auth_key;
}

void CGateApp::ConnectionLicenses()
{
    std::filesystem::path license_path = m_ExeDir;
    license_path = license_path / "license.txt";
    std::ifstream file(license_path, std::ios::in | std::ios::binary);
    if (file.is_open() == false)
    {
        AfxMessageBox(TEXT("找不到license.txt文件"));
        ExitProcess(0);
        return;
    }
    std::stringstream ss;
    ss << file.rdbuf();
    file.close();

    try
    {
        json licenses(json::parse(ss.str()));
        for (int i = 0; i < licenses.size(); i++)
        {
            const std::string ip = licenses[i]["ip"];
            const std::string snhash = licenses[i]["snhash"];
            const int port = licenses[i].find("port") != licenses[i].end() ? licenses[i]["port"] : kDefaultServicePort;
            auto observer_client = m_ObServerClientGroup.get_observer_client(ip, port);
            observer_client->async_start(ip, port);
            observer_client->set_auth_key(snhash);
        }
    }
    catch (...)
    {
        AfxMessageBox(TEXT("解析license.txt失败"));
    }
}