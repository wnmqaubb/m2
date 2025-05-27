
// GateF.cpp: 定义应用程序的类行为。
//

#include "pch.h"
#include "GateF.h"
#include "GateFDlg.h"
#include "asio2/util/base64.hpp"
#include "asio2/util/sha1.hpp"
#include "vmprotect/VMProtectSDK.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma data_seg("sing")
unsigned int giInstancePid = 0;
#pragma data_seg()
#pragma comment(linker,"/section:sing,RWS")

// CGateFApp

BEGIN_MESSAGE_MAP(CGateFApp, CWinAppEx)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CGateFApp 构造
std::shared_ptr<spdlog::logger> slog;
CGateFApp::CGateFApp()
{
    slog = spdlog::basic_logger_mt("GateF_logger", "gatef_log.txt");
    // 设置日志级别为调试
#ifdef _DEBUG
    spdlog::set_level(spdlog::level::debug);
#else
    spdlog::set_level(spdlog::level::warn);
#endif
    // 设置日志消息的格式模式
    spdlog::set_pattern("%^[%m-%d %H:%M:%S][%l]%$ %v");
    slog->flush_on(spdlog::level::warn);
    spdlog::flush_every(std::chrono::seconds(3));

	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	GetModuleFileNameA(NULL, m_ExeDir, sizeof(m_ExeDir));
	auto cParentDir = std::filesystem::path(m_ExeDir).parent_path().string();
	strcpy_s(m_ExeDir, cParentDir.c_str());
	m_cCfgPath = m_ExeDir;
	m_cCfgPath = m_cCfgPath + TEXT("\\config.cfg");
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的 CGateFApp 对象

CGateFApp theApp;

const GUID CDECL BASED_CODE _tlid =
		{0x5053d9b2,0x3353,0x4680,{0x97,0x9d,0xa6,0x5d,0xa3,0xcf,0x8d,0x1b}};
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;

BOOL CGateFApp::InitInstance()
{
    CString strCmdLine = AfxGetApp()->m_lpCmdLine;
    if (strCmdLine == TEXT("/StartService"))
    {
        is_parent_gate = false;
        std::filesystem::path path(m_ExeDir);
        giInstancePid = GetCurrentProcessId();
        STARTUPINFOA si = {};
        HANDLE pHandles[2] = {};
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_SHOW;
        PROCESS_INFORMATION pi = {};
        const std::string strCmdLine = " " + std::to_string(giInstancePid) + " 6";

        path = path / "g_Service.exe";
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
            if (si.wShowWindow == SW_SHOW)
            {
                DWORD error = GetLastError();
                CString errorMessage;
                errorMessage.Format(TEXT("启动Service失败，错误码: %d"), error);
                AfxMessageBox(errorMessage);
            }
        }
        pHandles[0] = pi.hProcess;

        //si.wShowWindow = SW_HIDE;
        path = path.parent_path() / "g_LogicServer.exe";
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
            if (si.wShowWindow == SW_SHOW)
            {
                DWORD error = GetLastError();
                CString errorMessage;
                errorMessage.Format(TEXT("启动LogicServer失败，错误码: %d"), error);
                AfxMessageBox(errorMessage);
            }
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

    // 初始化 richedit2 库
    if (!AfxInitRichEdit2()) {
        AfxMessageBox(L"无法初始化 richedit 库。可能是由于系统不支持该 DLL 版本。");
        return FALSE;
    }

    // 初始化 OLE 库
    if (!AfxOleInit())
    {
        AfxMessageBox(IDP_OLE_INIT_FAILED);
        return FALSE;
    }

    // 加载 richedit2 库
    HINSTANCE hriched = LoadLibrary(L"RICHED20.DLL");
    if (hriched == NULL) {
        AfxMessageBox(L"无法加载 richedit2 库。可能是由于系统不支持该 DLL 版本。");
        return FALSE;
    }

    m_bHiColorIcons = TRUE;
    m_childpHandle = NULL;
    // 加载策略配置文件
    InitConfig();

    // 创建 shell 管理器，以防对话框包含
    // 任何 shell 树视图控件或 shell 列表视图控件。
    //CShellManager *pShellManager = new CShellManager;

    // 激活“Windows Native”视觉管理器，以便在 MFC 控件中启用主题
    CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

    // 标准初始化
    // 如果未使用这些功能并希望减小
    // 最终可执行文件的大小，则应移除下列
    // 不需要的特定初始化例程
    // 更改用于存储设置的注册表项
    // TODO: 应适当修改该字符串，
    // 例如修改为公司或组织名
    SetRegistryKey(_T("应用程序向导生成的本地应用程序"));
    // 分析自动化开关或注册/注销开关的命令行。
    CCommandLineInfo cmdInfo;
    ParseCommandLine(cmdInfo);

    // 应用程序是用 /Embedding 或 /Automation 开关启动的。
    //使应用程序作为自动化服务器运行。
    if (cmdInfo.m_bRunEmbedded || cmdInfo.m_bRunAutomated)
    {
        // 通过 CoRegisterClassObject() 注册类工厂。
        COleTemplateServer::RegisterAll();
    }
    // 应用程序是用 /Unregserver 或 /Unregister 开关启动的。  移除
    // 注册表中的项。
    else if (cmdInfo.m_nShellCommand == CCommandLineInfo::AppUnregister)
    {
        COleObjectFactory::UpdateRegistryAll(FALSE);
        AfxOleUnregisterTypeLib(_tlid, _wVerMajor, _wVerMinor);
        return FALSE;
    }
    // 应用程序是以独立方式或用其他开关(如 /Register
    // 或 /Regserver)启动的。  更新注册表项，包括类型库。
    else
    {
        COleObjectFactory::UpdateRegistryAll();
        AfxOleRegisterTypeLib(AfxGetInstanceHandle(), _tlid);
        if (cmdInfo.m_nShellCommand == CCommandLineInfo::AppRegister)
            return FALSE;
    }

    CGateFDlg dlg;
    m_pMainWnd = &dlg;

    OnServiceStart();
    INT_PTR nResponse = dlg.DoModal();
    if (nResponse == IDOK)
    {
        // TODO: 在此放置处理何时用
        //  “确定”来关闭对话框的代码
    }
    else if (nResponse == IDCANCEL)
    {
        // TODO: 在此放置处理何时用
        //  “取消”来关闭对话框的代码
    }
    else if (nResponse == -1)
    {
        TRACE(traceAppMsg, 0, "警告: 对话框创建失败，应用程序将意外终止。\n");
        TRACE(traceAppMsg, 0, "警告: 如果您在对话框上使用 MFC 控件，则无法 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS。\n");
    }

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
    ControlBarCleanUp();
#endif

    // 释放加载的库
    if (hriched != NULL)
    {
        FreeLibrary(hriched);
    }

    // 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
    //  而不是启动应用程序的消息泵。
    return TRUE;
}

CGateFDlg* CGateFApp::GetMainFrame()
{
	ASSERT(m_pMainWnd->IsKindOf(RUNTIME_CLASS(CGateFDlg)));
	return (CGateFDlg*)m_pMainWnd;
}

int CGateFApp::ExitInstance()
{
	AfxOleTerm(FALSE);
	return CWinApp::ExitInstance();
}

void CGateFApp::OnServiceStart()
{
	if (m_childpHandle == NULL && giInstancePid == 0)
	{
		STARTUPINFOA si = {};
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
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
		//m_childpHandle = pi.hProcess;
		giInstancePid = pi.dwProcessId;
	}
	else
	{
		LogPrint(ObserverClientLog, TEXT("检测到服务已启动"));
	}
}


void CGateFApp::OnServiceStop()
{
	if (m_pMainWnd->MessageBox(TEXT("确认要停止服务吗?"), TEXT("提示"), MB_YESNO) == IDYES)
	{
		OnServiceStop1();
		LogPrint(ObserverClientLog, TEXT("服务已停止"));
		theApp.GetMainFrame()->SetStatusBar(ID_INDICATOR_SERVER_STAUS, _T("已停止"));
		//theApp.GetMainFrame()->SetPaneBackgroundColor(ID_INDICATOR_SERVER_STAUS, RGB(255, 0, 0));
	}
}

void CGateFApp::OnServiceStop1()
{
	if (m_childpHandle == NULL && giInstancePid != 0)
	{
		m_childpHandle = OpenProcess(PROCESS_TERMINATE, FALSE, giInstancePid);
		if (m_childpHandle)
		{
			TerminateProcess(m_childpHandle, 0);
			CloseHandle(m_childpHandle);
		}
	}

	// 关闭g_Service.exe
	HANDLE hProcess = theApp.GetMainFrame()->find_process("g_Service.exe");
	if (hProcess) {
		TerminateProcess(hProcess, 0);
		CloseHandle(hProcess);
	}
	// 关闭g_LogicServer.exe
	hProcess = theApp.GetMainFrame()->find_process("g_LogicServer.exe");
	if (hProcess) {
		TerminateProcess(hProcess, 0);
		CloseHandle(hProcess);
	}
	m_childpHandle = NULL;
	giInstancePid = 0;
}

void CGateFApp::OnServiceSettings()
{
	OpenConfig();
	GetMainFrame()->SwitchToTab(2);
	GetMainFrame()->m_polices_dlg->RefreshViewList();
	GetMainFrame()->m_polices_dlg->ShowWindow(SW_SHOW);
}

void CGateFApp::InitConfig()
{
	OpenConfig();
}

void CGateFApp::OpenConfig()
{
	std::ifstream file(theApp.m_cCfgPath, std::ios::in | std::ios::binary);
	if (file.is_open())
	{
		std::stringstream ss;
		ss << file.rdbuf();
		auto str = ss.str();
		theApp.m_cfg = ProtocolS2CPolicy::load(str.data(), str.size());
		file.close();
	}
	else
	{
		ProtocolS2CPolicy policy;
		std::ofstream cfg(m_cCfgPath.GetBuffer(), std::ios::out | std::ios::binary);
		auto buffer = policy.dump();
		cfg.write(buffer.data(), buffer.size());
		cfg.close();
		OpenConfig();
	}
}

void CGateFApp::SaveConfig()
{
	std::ofstream output(theApp.m_cCfgPath, std::ios::out | std::ios::binary);
	auto& str = theApp.m_cfg->dump();
	output.write(str.data(), str.size());
	output.close();
}


// 打开并定位文件
BOOL CGateFApp::OpenFolderAndSelectFile(CString szPath)
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

std::string CGateFApp::ReadLicense()
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

std::string CGateFApp::ReadAuthKey()
{
	std::string sn = ReadLicense();
	VMProtectBeginVirtualization(__FUNCTION__);
	auto sn_sha1 = asio2::sha1(asio2::base64().decode(sn));
	auto auth_key = asio2::base64().encode((unsigned char*)&sn_sha1, sizeof(sn_sha1));
	VMProtectEnd();
	return auth_key;
}

CString CGateFApp::ReadExpire()
{
    std::string sn = ReadLicense(); 
    VMProtectBeginVirtualization(__FUNCTION__); 
    int res = VMProtectSetSerialNumber(sn.c_str());
    if (res)
    {
        std::wstring vmp_text = L"";
        switch (res)
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
        AfxMessageBox(vmp_text.c_str());
        return L"";
    }
    VMProtectSerialNumberData sd = { 0 };
    VMProtectGetSerialNumberData(&sd, sizeof(sd));
    CString vmp_expire_t;
    vmp_expire_t.Format(TEXT("%d-%d-%d"), sd.dtExpire.wYear, sd.dtExpire.bMonth, sd.dtExpire.bDay);
    if (!vmp_expire_t.IsEmpty()) {
        ((CStatic*)theApp.GetMainFrame()->m_games_dlg->GetDlgItem(IDC_EXPDATE_STATIC))->SetWindowText(vmp_expire_t);
    }
    VMProtectEnd();
    return vmp_expire_t;
}

