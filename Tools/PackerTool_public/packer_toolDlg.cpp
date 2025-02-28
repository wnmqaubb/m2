// packer_toolDlg.cpp: 实现文件

#include "pch.h"
#include <asio2/util/base64.hpp>
#include <asio2/http/http_client.hpp>
#include "afxdialogex.h"
#include "packer_toolDlg.h"
#include "Service/Ini_tool.h"
#include <filesystem>
#include <fstream>
#include <mutex>
#include <thread>
#include <vector>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include <resource.h>
#include <Lightbone/xorstr.hpp>
#include <cstdlib>
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
#define WM_UPDATE_WINDOW (WM_USER + 1)
#define UPDATE_PROGRESS() InterlockedIncrement(&m_completedTasks)
class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDM_ABOUTBOX };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDM_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CpackertoolDlg 对话框



CpackertoolDlg::CpackertoolDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_PACKER_TOOL_DIALOG, pParent)
    , m_snhash(_T(""))
    , m_ip(_T(""))
    , m_login_stauts(_T(""))
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CpackertoolDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_FILE_PATH, m_pack_file_edit);
    DDX_Control(pDX, IDC_EDIT_RESULT, m_result_edit);
    DDX_Control(pDX, IDC_PROGRESS_PACK, m_progress_ctrl);
    DDX_Text(pDX, IDC_EDIT1, m_snhash);
    DDX_Text(pDX, IDC_EDIT3, m_ip);
    DDX_Text(pDX, IDC_EDIT4, m_login_stauts);
}

BEGIN_MESSAGE_MAP(CpackertoolDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON_PACK, &CpackertoolDlg::OnBnClickedButtonPack)
    ON_BN_CLICKED(IDC_BUTTON2, &CpackertoolDlg::OnOpenMultipleFiles)
    ON_BN_CLICKED(IDC_BUTTON1, &CpackertoolDlg::OnBnClickedValidateSnhash)
    ON_WM_DROPFILES()
    ON_MESSAGE(WM_UPDATE_WINDOW, OnUpdateWindow)
    ON_WM_TIMER()
END_MESSAGE_MAP()


// CpackertoolDlg 消息处理程序

BOOL CpackertoolDlg::OnInitDialog()
{
    VMP_VIRTUALIZATION_BEGIN(__FUNCTION__)
    CDialogEx::OnInitDialog();

    // 将“关于...”菜单项添加到系统菜单中。

    // IDM_ABOUTBOX 必须在系统命令范围内。
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != nullptr)
    {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
    //  执行此操作
    SetIcon(m_hIcon, TRUE);            // 设置大图标
    SetIcon(m_hIcon, FALSE);        // 设置小图标
    // 启用拖放
    //DragAcceptFiles(TRUE);
    // 
    std::filesystem::path pack_exe_path = ".\\config.ini";
    if (!std::filesystem::exists(pack_exe_path)) {
        std::ofstream output(pack_exe_path, std::ios::out | std::ios::binary);
        std::string str = "[LF]\r\n";
        output.write(str.data(), str.size());
        output.close();
    }
    std::string snhash = IniTool::read_ini<std::string>(".\\config.ini", "LF", "sn", "");
    if (!snhash.empty()) {
        m_snhash = CA2T(snhash.c_str());
        UpdateData(FALSE);
    }
    m_progress_ctrl.SetRange(0, 100);
    m_progress_ctrl.SetPos(0);
    m_progress_ctrl.ShowWindow(SW_SHOW);
    VMP_VIRTUALIZATION_END()
    return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CpackertoolDlg::MyUpdateWindow()
{
    CWnd* pMainWnd = AfxGetMainWnd();
    if (pMainWnd && ::IsWindow(pMainWnd->GetSafeHwnd()))
    {
        pMainWnd->PostMessage(WM_UPDATE_WINDOW);
    }
}

LRESULT CpackertoolDlg::OnUpdateWindow(WPARAM wParam, LPARAM lParam)
{
    // 在这里调用 UpdateWindow() 或其他 UI 更新操作
    UpdateWindow();
    return 0;
}

void CpackertoolDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialogEx::OnSysCommand(nID, lParam);
    }
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CpackertoolDlg::OnPaint()
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

bool CpackertoolDlg::exec_cmd(CString cmd,bool show_log) {
    VMP_VIRTUALIZATION_BEGIN(__FUNCTION__)
    if (cmd.IsEmpty()) {
        return false;
    }
    InterlockedIncrement(&m_completedTasks);
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    // 设置窗口隐藏
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    // 拼接命令行
    //cmd.Insert(0, TEXT("cmd /c "));
    if (show_log) {
        cmd.Append(TEXT(" > result.txt"));
    }
    // 解析环境变量
    TCHAR expandedCmd[1024];
    DWORD result = ExpandEnvironmentStrings(cmd, expandedCmd, 1024);
    if (result == 0 || result > 1024)
    {
        AfxMessageBox(TEXT("系统错误!"));
        return 1;
    }
    InterlockedIncrement(&m_completedTasks);
    // 创建进程
    if (!CreateProcess(NULL, expandedCmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        //std::cerr << "CreateProcess failed." << std::endl;
        return false;
    }
    // 等待进程结束，并定期更新进度
    while (WaitForSingleObject(pi.hProcess, 100) == WAIT_TIMEOUT)
    {
        // 处理消息队列防止卡死
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    // 确保每个命令完成时进度+1
    InterlockedIncrement(&m_completedTasks);
    // 关闭进程和线程句柄
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    VMP_VIRTUALIZATION_END()
    return true; 
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CpackertoolDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CpackertoolDlg::OnDropFiles(HDROP hDropInfo) {
    m_draggedFiles.clear();  // 清空之前的文件列表
    
    // 获取拖放文件数量
    UINT fileCount = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
    
    for (UINT i = 0; i < fileCount; i++) {
        CString filePath;
        TCHAR szFile[MAX_PATH];
        
        // 获取文件路径
        DragQueryFile(hDropInfo, i, szFile, MAX_PATH);
        filePath = szFile;
        
        // 检查是否为.exe文件
        if (filePath.Right(4).MakeLower() == _T(".exe")) {
            m_draggedFiles.push_back(filePath);
        }
    }
    
    DragFinish(hDropInfo);
    
    // 更新UI显示拖放文件
    CString fileList;
    for (const auto& file : m_draggedFiles) {
        fileList += file + _T("\r\n");
    }
    GetDlgItem(IDC_EDIT_FILE_PATH)->SetWindowText(fileList);
}

void CpackertoolDlg::PackFileThread(const std::filesystem::path& pack_exe_path) {    
    VMP_VIRTUALIZATION_BEGIN(__FUNCTION__)
    UPDATE_PROGRESS(); // 步骤1: 开始打包
    CString cmd, cmd2, cmd3;
    MyUpdateWindow();
    std::wstring new_path = pack_exe_path.wstring();
    new_path.insert(new_path.rfind(TEXT(".")), TEXT("[及时雨]"));
    cmd.Format(TEXT("\"%%Temp%%\\packer.exe\" --pack_exe --exe \"%s\" --dll \"%%Temp%%\\%s\" --output \"%s\" --ip_address %s"),
               pack_exe_path.wstring().c_str(),
               TEXT("NewClient.dll"),
               new_path.c_str(),
               m_ip
    );

    UPDATE_PROGRESS();
    USES_CONVERSION;
    //std::system(T2A(cmd));
    exec_cmd(cmd);
    // vmp
    std::wstring new_path_vmp = new_path;
    new_path_vmp.insert(new_path.rfind(TEXT(".")), TEXT("[V]"));
    cmd2.Format(TEXT("\"%%Temp%%\\VMProtect_Con.exe\" \"%s\" \"%s\" -pf \"%%Temp%%\\1.vmp\""),
                new_path.c_str(),
                new_path_vmp.c_str()
    );
    //std::system(T2A(cmd2));
    UPDATE_PROGRESS();
    exec_cmd(cmd2,false);

    // 复制附加数据 2.13.8 会自动复制
    //cmd3.Format(TEXT("packer.exe --copy_append_data --src %s --dst %s"),
    //	pack_exe_path.wstring().c_str(),
    //	new_path_vmp.c_str()
    //);

    ////std::system(T2A(cmd3));
    //exec_cmd(cmd3);

    UPDATE_PROGRESS();
    
    VMP_VIRTUALIZATION_END()
}

void CpackertoolDlg::OnBnClickedButtonPack() {
    VMP_VIRTUALIZATION_BEGIN(__FUNCTION__)
        // 在耗时操作中处理消息队列
        MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    if (m_ip.IsEmpty()) {
        AfxMessageBox(TEXT("请先登录"));
        return;
    }
    if (m_draggedFiles.empty()) {
        AfxMessageBox(_T("请先拖入要封装的登录器"));
        return;
    }

    m_result_edit.SetWindowText(TEXT(""));
    // 获取 %Temp% 环境变量的值 
    char* temp = nullptr;
    size_t len = 0;

    // 使用 _dupenv_s 获取 %Temp% 环境变量的值
    errno_t err = _dupenv_s(&temp, &len, "Temp");
    if (err || temp == nullptr)
    {
        AfxMessageBox(_T("系统错误"));
        return;
    }
    std::string tempDir = temp;
    if (!std::filesystem::exists(tempDir + "\\packer.exe"))
    {
        AfxMessageBox(_T("封装器主程序未找到"));
        SHowLog(TEXT("封装器主程序未找到"));
        return;
    }
    else if (!std::filesystem::exists(tempDir + "\\NewClient.dll"))
    {
        AfxMessageBox(_T("主模块未找到"));
        SHowLog(TEXT("主模块未找到"));
        return;
    }

    // 重置进度
    InterlockedExchange(&m_completedTasks, 0);
    InterlockedExchange(&m_totalTasks, m_draggedFiles.size() * 10);
    // 进度条初始化和显示
    SetTimer(1, 100, NULL);  // 每 100 毫秒触发一次
    // 创建3线程并发打包
    std::vector<std::thread> threads;
    //std::vector<std::wstring> loginer_names;
    SHowLog(L"开始封装...");
    UpdateWindow();

    // 使用工作线程执行任务
    AfxBeginThread([](LPVOID pParam) -> UINT {
        CpackertoolDlg* pThis = (CpackertoolDlg*)pParam;
        // 执行打包操作
        for (auto& file : pThis->m_draggedFiles)
        {
            std::filesystem::path path(file.GetString());
            pThis->PackFileThread(path);
        }

        // 验证最终进度
        LONG final = InterlockedCompareExchange(&pThis->m_completedTasks, 0, 0);
        LONG total = InterlockedCompareExchange(&pThis->m_totalTasks, 0, 0);
        //char buffer[1024];
        //sprintf_s(buffer, 1024, "打包进度: %d/%d", final, total);
        //OutputDebugStringA(buffer);
        // 强制最终进度为100%
        if (final != total) {
            pThis->m_progress_ctrl.SetPos(100); 
        }
        pThis->SHowLog(L"封装完成");
        std::this_thread::sleep_for(std::chrono::seconds(2));
        pThis->KillTimer(1);
        return 0;
    }, this);
    // 使用 std::async 异步执行任务
    //std::async(std::launch::async, [this]() {
    //    for (int i = 0; i < min(3, m_draggedFiles.size()); i++) {
    //        std::filesystem::path pack_exe_path(m_draggedFiles[i].GetString());
    //        if (!std::filesystem::exists(pack_exe_path)) {
    //            SHowLog(L"登录器不存在");
    //            continue;
    //        }
    //        PackFileThread(pack_exe_path);
    //    }
    //});

    //for (int i = 0; i < min(3, m_draggedFiles.size()); i++) {

    //    std::filesystem::path pack_exe_path(m_draggedFiles[i].GetString());
    //    if (!std::filesystem::exists(pack_exe_path)) {
    //        SHowLog(L"登录器不存在");
    //        continue;
    //    }
    //    //loginer_names.push_back(pack_exe_path.filename().wstring());
    //    threads.emplace_back(&CpackertoolDlg::PackFileThread, this, pack_exe_path);
    //}
    //
    //// 等待线程结束
    //for (auto& thread : threads) {
    //    thread.join();
    //}
    //KillTimer(1);
    /*m_progress_ctrl.SetPos(100);
    SHowLog(L"封装完成");*/
    UpdateWindow();
    VMP_VIRTUALIZATION_END()
}

CString CpackertoolDlg::packer_tool_validate(const std::string& snhash){
    VMP_VIRTUALIZATION_BEGIN(__FUNCTION__)
    try
    {
        const std::string host = xorstr("121.43.101.216");
        const std::string port = xorstr("13568");
        asio2::base64 base64;
        asio2::http_client http_client;
        auto base64_sn = base64.encode((const unsigned char*)snhash.data(), snhash.size());
        http::web_request req;
        req.method(http::verb::post);
        req.target(xorstr("/packer_tool_validate.php"));
        req.set(http::field::user_agent, "Chrome");
        req.set(http::field::content_type, "application/x-www-form-urlencoded");
        req.body() = http::url_encode(xorstr("sn=") + base64_sn);
        req.prepare_payload();
        auto rep = asio2::http_client::execute(host, port, req);

        if (auto error = asio2::get_last_error()) {
            return CA2W(error.message().c_str());
        }
        else {
            return CA2W(rep.body().c_str());
        }
    }
    catch (...)
    {
        return L"";
    }
    VMP_VIRTUALIZATION_END();
}

void CpackertoolDlg::SHowLog(CString log_text) {
    try
    {
        USES_CONVERSION;
        CString str;
        m_result_edit.GetWindowText(str);
        if (log_text.IsEmpty()) {
            std::ifstream result("result.txt", std::ios::in | std::ios::binary);
            result.seekg(0, result.end);
            size_t size = result.tellg();
            result.seekg(0);
            std::string text;
            text.resize(size);
            result.read(text.data(), size);
            result.close();
            if (str.IsEmpty()) {
                m_result_edit.SetWindowText(A2T(text.c_str()));
            }
            else {
                m_result_edit.SetWindowText(str + "\r\n" + A2T(text.c_str()));
            }
        }
        else {
            if (str.IsEmpty()) {
                m_result_edit.SetWindowText(log_text);
            }
            else {
                m_result_edit.SetWindowText(str + "\r\n" + log_text);
            }
        }
        MyUpdateWindow();
    }
    catch (CException* e)
    {
    }
}

void CpackertoolDlg::OnBnClickedVMP() {
    //CString temp;
    //auto pa = std::filesystem::current_path();
    //if (!std::filesystem::exists("packer.exe")) {
    //    m_result_edit.SetWindowText(TEXT("封装器主程序未找到"));
    //    return;
    //}
    //else if (!std::filesystem::exists("NewClient.dll")) {
    //    m_result_edit.SetWindowText(TEXT("反作弊模块未找到"));
    //    return;
    //}
    //VMP_VIRTUALIZATION_BEGIN(__FUNCTION__)
    //m_pack_file_edit.GetWindowText(temp);
    //std::filesystem::path pack_exe_path(temp.GetString());
    //std::wstring new_path = pack_exe_path.wstring();
    //new_path.insert(new_path.rfind(TEXT(".")), TEXT("[及时雨]"));
    //CString cmd;
    //cmd.Format(TEXT("packer.exe --pack_exe --exe %s --dll %s --output %s --ip_address %s > result.txt"),
    //    pack_exe_path.wstring().c_str(),
    //    TEXT("NewClient.dll"),
    //    new_path.c_str(),
    //    //TEXT("config.txt"),
    //    m_ip
    //);
    //USES_CONVERSION;
    //system(CT2A(cmd));
    //std::ifstream result("result.txt", std::ios::in | std::ios::binary);
    //result.seekg(0, result.end);
    //size_t size = result.tellg();
    //result.seekg(0);
    //std::string text;
    //text.resize(size);
    //VMP_VIRTUALIZATION_END()
    //result.read(text.data(), size);
    //m_result_edit.SetWindowText(CA2T(text.c_str()));
}

void CpackertoolDlg::OnOpenMultipleFiles()
{
    VMP_VIRTUALIZATION_BEGIN(__FUNCTION__)
    // 设置文件过滤器（隐藏 .exe 和 .dll）
    const TCHAR szFilter[] = _T("登录器文件(*.exe)|*.exe||");

    // 创建 Vista 风格的文件对话框（支持多选）
    CFileDialog dlgFile(TRUE, NULL, NULL,
                        OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER,
                        szFilter, this);

    // 设置缓冲区大小（确保足够大）
    const DWORD dwBufferSize = 65535;
    dlgFile.GetOFN().lpstrFile = new TCHAR[dwBufferSize];
    dlgFile.GetOFN().nMaxFile = dwBufferSize;
    memset(dlgFile.GetOFN().lpstrFile, 0, dwBufferSize * sizeof(TCHAR));

    // 显示对话框
    if (dlgFile.DoModal() == IDOK)
    {
        POSITION pos = dlgFile.GetStartPosition();
        while (pos != NULL)
        {
            CString strFilePath = dlgFile.GetNextPathName(pos);
            m_draggedFiles.push_back(strFilePath);
        }      

        // 更新UI显示拖放文件
        CString fileList;
        for (const auto& file : m_draggedFiles) {
            fileList += file + _T("\r\n");
        }

        GetDlgItem(IDC_EDIT_FILE_PATH)->SetWindowText(fileList);
    }

    delete[] dlgFile.GetOFN().lpstrFile; // 释放缓冲区
    VMP_VIRTUALIZATION_END()
}


void CpackertoolDlg::OnBnClickedValidateSnhash()
{
    VMP_VIRTUALIZATION_BEGIN(__FUNCTION__)
    UpdateData(TRUE);
    if (m_snhash.IsEmpty()) {
        AfxMessageBox(TEXT("请输入序列号"));
        return;
    }
    std::string snhash = CT2A(m_snhash.GetString());
    auto ip = packer_tool_validate(snhash);
    if (ip.IsEmpty()) {
        AfxMessageBox(TEXT("验证失败,请确认序列号"));
        return;
    }
    IniTool::write_ini<std::string>(".\\config.ini", "LF", "sn", snhash);
    m_ip = ip;
    m_login_stauts = TEXT("已登录");
    UpdateData(false);
    VMP_VIRTUALIZATION_END()
    return;
}

void CpackertoolDlg::OnTimer(UINT_PTR nIDEvent)
{
    int progress;
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    if (nIDEvent == 1) {
        LONG completed = InterlockedCompareExchange(&m_completedTasks, 0, 0);
        LONG total = InterlockedCompareExchange(&m_totalTasks, 0, 0);

        /*char buffer[1024];
        sprintf_s(buffer, 1024, "==打包进度: %d/%d", completed, total);
        OutputDebugStringA(buffer);*/
        int progress = 0;
        if (total > 0 && completed <= total)
            progress = (completed * 100) / total;

        m_progress_ctrl.SetPos(progress);
        m_progress_ctrl.RedrawWindow();
    }
    CDialogEx::OnTimer(nIDEvent);
}
