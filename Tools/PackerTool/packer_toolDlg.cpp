
// packer_toolDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "packer_tool.h"
#include "packer_toolDlg.h"
#include "afxdialogex.h"
#include <filesystem>
#include <fstream>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
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


// CpackertoolDlg 对话框



CpackertoolDlg::CpackertoolDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PACKER_TOOL_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CpackertoolDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_FILE_PATH, m_pack_file_edit);
    DDX_Control(pDX, IDC_EDIT_RESULT, m_result_edit);
}

BEGIN_MESSAGE_MAP(CpackertoolDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON_PACK, &CpackertoolDlg::OnBnClickedButtonPack)
    ON_BN_CLICKED(IDC_BUTTON2, &CpackertoolDlg::OnBnClickedButton2)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()



// CpackertoolDlg 消息处理程序

BOOL CpackertoolDlg::OnInitDialog()
{
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
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

bool CpackertoolDlg::exec_cmd(CString cmd) {
	if (cmd.IsEmpty()) {
		return false;
	}
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	// 设置窗口隐藏
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	// 拼接命令行
	cmd.Insert(0, TEXT("cmd /c "));
	cmd.Append(TEXT(" > result.txt"));
	// 创建进程
	if (!CreateProcess(NULL, cmd.GetBuffer(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		//std::cerr << "CreateProcess failed." << std::endl;
		return false;
	}
	// 等待进程结束
	WaitForSingleObject(pi.hProcess, INFINITE);
	// 关闭进程和线程句柄
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	OnBnClickedLog();
	return true;
}
//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CpackertoolDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}



void CpackertoolDlg::OnBnClickedButtonPack()
{
    // TODO: 在此添加控件通知处理程序代码
    CString cmd,cmd2,cmd3;
	CString temp;
	m_result_edit.SetWindowText(TEXT(""));
	auto pa = std::filesystem::current_path();
    if (!std::filesystem::exists("packer.exe"))
    {
        m_result_edit.SetWindowText(TEXT("封装器主程序未找到"));
        return;
    }
    else if (!std::filesystem::exists("NewClient.dll"))
    {
        m_result_edit.SetWindowText(TEXT("反作弊模块未找到"));
        return;
	}
	UpdateWindow();
	m_pack_file_edit.GetWindowText(temp);
    std::filesystem::path pack_exe_path(temp.GetString());
	std::wstring new_path = pack_exe_path.wstring();
	new_path.insert(new_path.rfind(TEXT(".")), TEXT("[及时雨]"));
    cmd.Format(TEXT("packer.exe --pack_exe --exe %s --dll %s --output %s --config %s"),
        pack_exe_path.wstring().c_str(),
        TEXT("NewClient.dll"),
		new_path.c_str(),
        TEXT("config.txt")
        );

	USES_CONVERSION;
	//std::system(T2A(cmd));
	exec_cmd(cmd);
	// vmp
	std::wstring new_path_vmp = new_path;
	new_path_vmp.insert(new_path.rfind(TEXT(".")), TEXT("[V]"));
	cmd2.Format(TEXT("VMProtect_Con.exe %s %s -pf 1.vmp"),
		new_path.c_str(),
		new_path_vmp.c_str()
	);
	//std::system(T2A(cmd2));
	exec_cmd(cmd2);

	// 复制附加数据
	cmd3.Format(TEXT("packer.exe --copy_append_data --src %s --dst %s"),
		pack_exe_path.wstring().c_str(),
		new_path_vmp.c_str()
	);

	//std::system(T2A(cmd3));
	exec_cmd(cmd3);
}
//
//void CpackertoolDlg::OnBnClickedButtonPack()
//{
//    // TODO: 在此添加控件通知处理程序代码
//    CString cmd,cmd2,cmd3;
//    CString temp;
//	auto pa = std::filesystem::current_path();
//    if (!std::filesystem::exists("packer.exe"))
//    {
//        m_result_edit.SetWindowText(TEXT("封装器主程序未找到"));
//        return;
//    }
//    else if (!std::filesystem::exists("NewClient.dll"))
//    {
//        m_result_edit.SetWindowText(TEXT("反作弊模块未找到"));
//        return;
//    }
//    m_pack_file_edit.GetWindowText(temp);
//    std::filesystem::path pack_exe_path(temp.GetString());
//	std::wstring new_path = pack_exe_path.wstring();
//	new_path.insert(new_path.rfind(TEXT(".")), TEXT("[及时雨]"));
//    cmd.Format(TEXT("packer.exe --pack_exe --exe %s --dll %s --output %s --config %s > result.txt"),
//        pack_exe_path.wstring().c_str(),
//        TEXT("NewClient.dll"),
//		new_path.c_str(),
//        TEXT("config.txt")
//        );
//
//	// vmp
//	std::wstring new_path_vmp = new_path;
//	new_path_vmp.insert(new_path.rfind(TEXT(".")), TEXT("[V]"));
//	cmd2.Format(TEXT(" & \"D:\\tool\\52pojie\\Tools\\Packers\\VMProtect Ultimate3.4\\VMProtect_Con.exe\" %s %s -pf 1.vmp > result.txt"),
//		new_path.c_str(),
//		new_path_vmp.c_str()
//	);
//
//	// 复制附加数据
//	cmd3.Format(TEXT(" & packer.exe --copy_append_data --src %s --dst %s > result.txt & pause "),
//		pack_exe_path.wstring().c_str(),
//		new_path_vmp.c_str()
//	);
//
//	USES_CONVERSION;
//	auto all_cmd = cmd + cmd2 + cmd3;
//	system(T2A(all_cmd));
//	OnBnClickedLog();
//}

void CpackertoolDlg::OnBnClickedLog() {
	try
	{
		USES_CONVERSION;
		std::ifstream result("result.txt", std::ios::in | std::ios::binary);
		result.seekg(0, result.end);
		size_t size = result.tellg();
		result.seekg(0);
		std::string text;
		text.resize(size);
		result.read(text.data(), size);
		result.close();
		CString str;
		m_result_edit.GetWindowText(str);
		m_result_edit.SetWindowText(str + "\n" + A2T(text.c_str()));
		UpdateWindow();
	}
	catch (CException* e)
	{
	}
}

void CpackertoolDlg::OnBnClickedVMP()
{
	// TODO: 在此添加控件通知处理程序代码
	CString cmd;
	CString temp;
	auto pa = std::filesystem::current_path();
	if (!std::filesystem::exists("packer.exe"))
	{
		m_result_edit.SetWindowText(TEXT("封装器主程序未找到"));
		return;
	}
	else if (!std::filesystem::exists("NewClient.dll"))
	{
		m_result_edit.SetWindowText(TEXT("反作弊模块未找到"));
		return;
	}
	m_pack_file_edit.GetWindowText(temp);
	std::filesystem::path pack_exe_path(temp.GetString());
	std::wstring new_path = pack_exe_path.wstring();
	new_path.insert(new_path.rfind(TEXT(".")), TEXT("[及时雨]"));
	cmd.Format(TEXT("packer.exe --pack_exe --exe %s --dll %s --output %s --config %s > result.txt"),
		pack_exe_path.wstring().c_str(),
		TEXT("NewClient.dll"),
		new_path.c_str(),
		TEXT("config.txt")
	);
	USES_CONVERSION;
	system(T2A(cmd));
	std::ifstream result("result.txt", std::ios::in | std::ios::binary);
	result.seekg(0, result.end);
	size_t size = result.tellg();
	result.seekg(0);
	std::string text;
	text.resize(size);
	result.read(text.data(), size);
	m_result_edit.SetWindowText(A2T(text.c_str()));
}

void CpackertoolDlg::OnBnClickedButton2()
{
    // TODO: 在此添加控件通知处理程序代码
    TCHAR path[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, path);
    CFileDialog file_dialog(TRUE, _T("bin"), path, 0, _T("登录器文件(*.exe)|*.exe|所有文件(*.*)|*.*||"), this);
    if (file_dialog.DoModal() != IDOK)
    {
        return;
    }
   
    m_pack_file_edit.SetWindowText(file_dialog.GetPathName().GetString());
   
}

void CpackertoolDlg::OnDropFiles(HDROP hDropInfo)
{
	UINT uBufLength = DragQueryFile(hDropInfo, 0, NULL, 0);
	if(uBufLength)
	{
		TCHAR *szPath = new TCHAR[uBufLength * 4];  //保存文件路径

		memset(szPath, 0, uBufLength * 4);

		DragQueryFile(hDropInfo, 0, szPath, uBufLength + 1);  //获取文件路径
		SetDlgItemText(IDC_EDIT_FILE_PATH, szPath);     //更新EDIT框
		DragFinish(hDropInfo);   //拖放结束后,释放内存
		delete[] szPath;
	}

	CDialogEx::OnDropFiles(hDropInfo);
}
