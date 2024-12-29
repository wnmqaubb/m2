// CProcessInfoDlg.cpp: 实现文件
//

#include "pch.h"
#include "GateF.h"
#include "afxdialogex.h"
#include "CProcessInfoDlg.h"


// CProcessInfoDlg 对话框

IMPLEMENT_DYNAMIC(CProcessInfoDlg, CDialogEx)

CProcessInfoDlg::CProcessInfoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_PROCESSINFO, pParent)
{

}

CProcessInfoDlg::~CProcessInfoDlg()
{
}

void CProcessInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PROCESSINFO, m_list_process_info);
	DDX_Control(pDX, IDC_LIST_MODULEINFO, m_list_module_info);
	DDX_Control(pDX, IDC_LIST_THREADINFO, m_list_thread_info);
	DDX_Control(pDX, IDC_LIST_DIRECTORYINFO, m_list_directory_info);
	DDX_Control(pDX, IDC_TAB1, m_tab_process_info_main);
}


BEGIN_MESSAGE_MAP(CProcessInfoDlg, CDialogEx)
	ON_WM_CONTEXTMENU()
	ON_WM_SIZE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CProcessInfoDlg::OnTcnSelchangeTabMain)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_PROCESSINFO, &CProcessInfoDlg::OnListItemChanged)
	ON_COMMAND(ID_PROCESS_NAME, &CProcessInfoDlg::OnProcessNameBan)
	ON_COMMAND(ID_PROCESS_PATH, &CProcessInfoDlg::OnProcessPathBan)
	ON_COMMAND(ID_GET_GAMEUSER_FILE, &CProcessInfoDlg::OnGetGameUserFile)
	ON_COMMAND(ID_COPY_MODULE_NAME, &CProcessInfoDlg::OnCopyValue)
	ON_COMMAND(ID_COPY_THREAD_ENTRY, &CProcessInfoDlg::OnCopyValue)
	ON_COMMAND(ID_COPY_FILE_NAME, &CProcessInfoDlg::OnCopyValue)
	ON_COMMAND(ID_COPY_MODULE_PATH, &CProcessInfoDlg::OnCopyModulePath)
END_MESSAGE_MAP()


// CProcessInfoDlg 消息处理程序


BOOL CProcessInfoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	FillViewList();

	// 初始化 CTabCtrl
	m_tab_process_info_main.GetClientRect(&m_tab_main_rect);
	m_tab_process_info_main.AdjustRect(FALSE, &m_tab_main_rect);
	m_tab_main_rect.DeflateRect(0, 20, 0, 0);
	m_tab_process_info_main.InsertItem(0, _T("模块"));
	m_tab_process_info_main.InsertItem(1, _T("线程"));
	m_tab_process_info_main.InsertItem(2, _T("进程目录"));
	InitModuleWindowView();
	InitThreadWindowView();
	InitDirectoryWindowView();
	m_tab_process_info_main.SetCurSel(0);
	OnTcnSelchangeTabMain(nullptr, nullptr);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CProcessInfoDlg::OnTcnSelchangeTabMain(NMHDR* pNMHDR, LRESULT* pResult)
{
	switch (m_tab_process_info_main.GetCurSel())
	{
		case 0:
			m_list_module_info.ShowWindow(SW_SHOW);
			m_list_thread_info.ShowWindow(SW_HIDE);
			m_list_directory_info.ShowWindow(SW_HIDE);
			break;
		case 1:
			m_list_module_info.ShowWindow(SW_HIDE);
			m_list_thread_info.ShowWindow(SW_SHOW);
			m_list_directory_info.ShowWindow(SW_HIDE);
			break;
		case 2:
			m_list_module_info.ShowWindow(SW_HIDE);
			m_list_thread_info.ShowWindow(SW_HIDE);
			m_list_directory_info.ShowWindow(SW_SHOW);
			break;
		default:
			break;
	}
}

void CProcessInfoDlg::ShowAllDlgInTab()
{
	m_list_module_info.MoveWindow(m_tab_main_rect);
	m_list_module_info.ShowWindow(SW_SHOW);

	m_list_thread_info.MoveWindow(m_tab_main_rect);
	m_list_thread_info.ShowWindow(SW_HIDE);

	m_list_directory_info.MoveWindow(m_tab_main_rect);
	m_list_directory_info.ShowWindow(SW_HIDE);
}

void CProcessInfoDlg::LoadFile(const std::wstring& file_path)
{
	std::ifstream file(file_path, std::ios::binary);
	if (file.is_open())
	{
		std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		std::string_view sv(str);
		package.decode(sv);
		auto raw_msg = msgpack::unpack((char*)package.body.buffer.data(), package.body.buffer.size());
		const auto package_id = raw_msg.get().via.array.ptr[0].as<unsigned int>();
		processes = raw_msg.get().as<ProtocolC2SQueryProcess>();
		FillViewListData();
		file.close();
	}
}

void CProcessInfoDlg::OnContextMenu(CWnd*  pWnd , CPoint point)
{
	CPoint clientPoint = point;
	UINT flags;
	if (pWnd == &m_list_process_info)
	{
		m_list_process_info.ScreenToClient(&clientPoint);
		int item = m_list_process_info.HitTest(clientPoint, &flags);
		if (item != -1)
		{
			// 显示右键菜单
			CMenu menu;
			menu.LoadMenu(IDR_PROCESS_RIGHT_MENU);
			menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
		}
	}
	else {
		CMenu menu;
		menu.CreatePopupMenu();
		if (m_tab_process_info_main.GetCurSel() == 0)
		{
			m_list_module_info.ScreenToClient(&clientPoint);
			int item = m_list_module_info.HitTest(clientPoint, &flags);
			menu.AppendMenu(MF_STRING, ID_COPY_MODULE_NAME, TEXT("复制模块名"));
			menu.AppendMenu(MF_STRING, ID_COPY_MODULE_PATH, TEXT("复制模块路径"));
		}
		else if (m_tab_process_info_main.GetCurSel() == 1)
		{
			m_list_thread_info.ScreenToClient(&clientPoint);
			int item = m_list_thread_info.HitTest(clientPoint, &flags);
			menu.AppendMenu(MF_STRING, ID_COPY_THREAD_ENTRY, TEXT("复制线程入口"));
			menu.AppendMenu(MF_STRING, ID_COPY_MODULE_PATH, TEXT("复制模块路径"));
		}
		else if (m_tab_process_info_main.GetCurSel() == 2)
		{
			m_list_directory_info.ScreenToClient(&clientPoint);
			int item = m_list_directory_info.HitTest(clientPoint, &flags);
			menu.AppendMenu(MF_STRING, ID_COPY_FILE_NAME, TEXT("复制文件名"));
		}

		menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	}
}

void CProcessInfoDlg::FillViewList()
{
	m_list_process_info.SetRedraw(FALSE);
	m_list_process_info.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_GRIDLINES);
	int colIndex = 0;
	m_list_process_info.InsertColumn(colIndex++, TEXT("序号"), LVCFMT_LEFT, 38);
	m_list_process_info.InsertColumn(colIndex++, TEXT("PID"), LVCFMT_LEFT, 50);
	m_list_process_info.InsertColumn(colIndex++, TEXT("父PID"), LVCFMT_LEFT, 45);
	m_list_process_info.InsertColumn(colIndex++, TEXT("进程名称"), LVCFMT_LEFT, 150);
	m_list_process_info.InsertColumn(colIndex++, TEXT("进程路径"), LVCFMT_LEFT, 530);
	m_list_process_info.InsertColumn(colIndex++, TEXT("进程位数"), LVCFMT_LEFT, 70);
	m_list_process_info.SetRedraw(TRUE);
}

void CProcessInfoDlg::FillViewListData() {
	m_list_process_info.SetRedraw(FALSE);
	m_list_process_info.DeleteAllItems();
	CString connect_id_str, seq;
	const wchar_t* format_d = L"%d";
	const wchar_t* format_3d = L"%03d";
	const wchar_t* empty_wstr = L"";
	const wchar_t* no_access_wstr = L"无权限";
	const wchar_t* empty_list_wstr = L"模块列表为空";
	const wchar_t* x32_wstr = L"x32";
	const wchar_t* x64_wstr = L"x64";
	int rowNum = 0;	int colIndex = 0;
	for (auto& process : processes.data)
	{
		if (rowNum % 100 == 0) Sleep(1);
		colIndex = 0;
		CString id;
		CString temp;
		id.Format(format_d, rowNum + 1);
		m_list_process_info.InsertItem(rowNum, empty_wstr);
		m_list_process_info.SetItemText(rowNum, colIndex++, id);
		temp.Format(format_d, process.second.pid);
		m_list_process_info.SetItemText(rowNum, colIndex++, temp);
		temp.Format(format_d, process.second.parent_pid);
		m_list_process_info.SetItemText(rowNum, colIndex++, temp);
		m_list_process_info.SetItemText(rowNum, colIndex++, process.second.name.c_str());
		m_list_process_info.SetItemText(rowNum, colIndex++, process.second.no_access ? no_access_wstr :
			process.second.modules.empty() ? empty_list_wstr : process.second.modules.front().path.c_str());
		m_list_process_info.SetItemText(rowNum, colIndex++, process.second.is_64bits ? x64_wstr : x32_wstr);
		rowNum++;
	}
	m_list_process_info.SetRedraw(TRUE);
}

void CProcessInfoDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	if (GetSafeHwnd() == nullptr)
	{
		return;
	}

	/*CRect rectClient;
	GetClientRect(rectClient);

	m_list_process_info.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);*/
}

void CProcessInfoDlg::OnListItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if (pNMListView->uChanged == LVIF_STATE)
	{
		if (pNMListView->uNewState & LVIS_SELECTED)
		{
			int nItem = pNMListView->iItem;
			CString csPid = m_list_process_info.GetItemText(nItem, 1);
			uint32_t pid = _ttoi(csPid);
			auto& process = processes.data.find(pid)->second;
			FillModuleWindow(process.modules);
			FillThreadWindow(process.threads);
			FillDirectoryWindow(process.directories);
		}
	}
}

void CProcessInfoDlg::InitModuleWindowView()
{
	//m_list_module_info.SetColumnByIntSort({ 0, 2 });
	m_list_module_info.SetRedraw(FALSE);
	int colIndex = 0;
	m_list_module_info.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_GRIDLINES);
	m_list_module_info.InsertColumn(colIndex++, TEXT("序号"), LVCFMT_LEFT, 38);
	m_list_module_info.InsertColumn(colIndex++, TEXT("模块基址"), LVCFMT_LEFT, 120);
	m_list_module_info.InsertColumn(colIndex++, TEXT("模块大小"), LVCFMT_LEFT, 80);
	m_list_module_info.InsertColumn(colIndex++, TEXT("模块名"), LVCFMT_LEFT, 150);
	m_list_module_info.InsertColumn(colIndex++, TEXT("模块路径"), LVCFMT_LEFT, 530);

	m_list_module_info.SetRedraw(TRUE);
}

void CProcessInfoDlg::InitThreadWindowView()
{
	//m_list_thread_info.SetColumnByIntSort({ 0, 1 });
	m_list_thread_info.SetRedraw(FALSE);
	int colIndex = 0;
	m_list_thread_info.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_GRIDLINES);
	colIndex = 0;
	m_list_thread_info.InsertColumn(colIndex++, TEXT("序号"), LVCFMT_LEFT, 38);
	m_list_thread_info.InsertColumn(colIndex++, TEXT("线程ID"), LVCFMT_LEFT, 50);
	m_list_thread_info.InsertColumn(colIndex++, TEXT("线程入口"), LVCFMT_LEFT, 120);
	m_list_thread_info.InsertColumn(colIndex++, TEXT("主线程"), LVCFMT_LEFT, 50);
	m_list_thread_info.InsertColumn(colIndex++, TEXT("模块路径"), LVCFMT_LEFT, 530);
	m_list_thread_info.SetRedraw(TRUE);
}

void CProcessInfoDlg::InitDirectoryWindowView()
{
	//m_list_directory_info.SetColumnByIntSort({ 0 });
	m_list_directory_info.SetRedraw(FALSE);
	int colIndex = 0;
	m_list_directory_info.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_GRIDLINES);
	colIndex = 0;
	m_list_directory_info.InsertColumn(colIndex++, TEXT("序号"), LVCFMT_LEFT, 38);
	m_list_directory_info.InsertColumn(colIndex++, TEXT("类型"), LVCFMT_LEFT, 50);
	m_list_directory_info.InsertColumn(colIndex++, TEXT("文件名"), LVCFMT_LEFT, 530);
	m_list_directory_info.SetRedraw(TRUE);
}

void CProcessInfoDlg::FillModuleWindow(const std::vector<ProtocolModuleInfo>& modules)
{
	m_list_module_info.SetRedraw(FALSE);
	int colIndex = 0;
	m_list_module_info.DeleteAllItems();
	const wchar_t* empty_wstr = _T("");
	const wchar_t* format_d = _T("%d");
	const wchar_t* format_0x08X = _T("0x%08X");
	const wchar_t* format_0xllX = _T("0x%llX");
	int rowNum = 0;
	for (auto& module : modules)
	{
		if (rowNum % 100 == 0) Sleep(1);
		colIndex = 0;
		CString id;
		CString temp;
		id.Format(format_d, rowNum + 1);
		m_list_module_info.InsertItem(rowNum, empty_wstr);
		m_list_module_info.SetItemText(rowNum, colIndex++, id);
		temp.Format(format_0xllX, module.base);
		m_list_module_info.SetItemText(rowNum, colIndex++, temp);
		temp.Format(format_0x08X, module.size_of_image);
		m_list_module_info.SetItemText(rowNum, colIndex++, temp);
		m_list_module_info.SetItemText(rowNum, colIndex++, module.module_name.c_str());
		m_list_module_info.SetItemText(rowNum, colIndex++, module.path.c_str());
		rowNum++;
	}
	m_list_module_info.SetRedraw(TRUE);
}

void CProcessInfoDlg::FillThreadWindow(const std::map<uint32_t, ProtocolThreadInfo>& threads)
{
	m_list_thread_info.SetRedraw(FALSE);
	int colIndex = 0;
	m_list_thread_info.DeleteAllItems();
	const wchar_t* empty_wstr = _T("");
	const wchar_t* format_d = _T("%d");
	const wchar_t* format_0xllX = _T("0x%llX");
	const wchar_t* main_thread_wstr = _T("1");
	const wchar_t* child_thread_wstr = _T("0");
	int rowNum = 0;
	for (auto& thread : threads)
	{
		if (rowNum % 100 == 0) Sleep(1);
		colIndex = 0;
		CString id;
		CString temp;
		id.Format(format_d, rowNum + 1);
		m_list_thread_info.InsertItem(rowNum, empty_wstr);
		m_list_thread_info.SetItemText(rowNum, colIndex++, id);
		temp.Format(format_d, thread.second.tid);
		m_list_thread_info.SetItemText(rowNum, colIndex++, temp);
		temp.Format(format_0xllX, thread.second.start_address);
		m_list_thread_info.SetItemText(rowNum, colIndex++, temp);
		m_list_thread_info.SetItemText(rowNum, colIndex++, (thread.second.is_main_thread ? main_thread_wstr : child_thread_wstr));
		m_list_thread_info.SetItemText(rowNum, colIndex++, thread.second.owner_module.c_str());
		rowNum++;
	}
	m_list_thread_info.SetRedraw(TRUE);
}

void CProcessInfoDlg::FillDirectoryWindow(const std::vector<ProtocolDirectoryInfo>& directories)
{
	m_list_directory_info.SetRedraw(FALSE);
	int colIndex = 0;
	m_list_directory_info.DeleteAllItems();
	const wchar_t* empty_wstr = _T("");
	const wchar_t* format_d = _T("%d");
	int rowNum = 0;
	for (auto& dir : directories)
	{
		if (rowNum % 100 == 0) Sleep(1);
		colIndex = 0;
		CString id;
		CString temp;
		id.Format(format_d, rowNum + 1);
		m_list_directory_info.InsertItem(rowNum, empty_wstr);
		m_list_directory_info.SetItemText(rowNum, colIndex++, id);
		m_list_directory_info.SetItemText(rowNum, colIndex++, dir.is_directory ? TEXT("[目录]") : TEXT("[文件]"));
		m_list_directory_info.SetItemText(rowNum, colIndex++, dir.path.c_str());
		rowNum++;
	}
	m_list_directory_info.SetRedraw(TRUE);
}

void CProcessInfoDlg::OnProcessNameBan()
{
	auto selectedRow = (int)m_list_process_info.GetFirstSelectedItemPosition() - 1;
	std::wstring process_name;
	if (selectedRow != -1)
	{
		process_name = m_list_process_info.GetItemText(selectedRow, 3);
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
		unsigned int uiLastPolicyId = theApp.GetMainFrame ()->m_polices_dlg->next_gm_policy_id(Policies);
		ProtocolPolicy Policy;
		Policy.policy_id = uiLastPolicyId;
		Policy.punish_type = ENM_PUNISH_TYPE_NO_OPEARATION;
		Policy.policy_type = ENM_POLICY_TYPE_PROCESS_NAME;
		Policy.config = process_name;
		Policies[uiLastPolicyId] = Policy;
		file.close();
		std::ofstream output(theApp.m_cCfgPath, std::ios::out | std::ios::binary);
		str = Cfg->dump();
		output.write(str.data(), str.size());
		output.close();
		theApp.OnServiceSettings();
		theApp.GetMainFrame()->m_polices_dlg->OnSelectItem(uiLastPolicyId);
	}
}

void CProcessInfoDlg::OnProcessPathBan()
{
	auto selectedRow = (int)m_list_process_info.GetFirstSelectedItemPosition() - 1;
	std::wstring process_path;
	if (selectedRow != -1)
	{
		process_path = m_list_process_info.GetItemText(selectedRow, 4);
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
		unsigned int uiLastPolicyId = theApp.GetMainFrame()->m_polices_dlg->next_gm_policy_id(Policies);
		ProtocolPolicy Policy;
		Policy.policy_id = uiLastPolicyId;
		Policy.punish_type = ENM_PUNISH_TYPE_NO_OPEARATION;
		Policy.policy_type = ENM_POLICY_TYPE_PROCESS_NAME;
		Policy.config = process_path;
		Policies[uiLastPolicyId] = Policy;
		file.close();
		std::ofstream output(theApp.m_cCfgPath, std::ios::out | std::ios::binary);
		str = Cfg->dump();
		output.write(str.data(), str.size());
		output.close();
		theApp.OnServiceSettings();
	}

}

void CProcessInfoDlg::OnGetGameUserFile()
{
	auto selectedRow = (int)m_list_process_info.GetFirstSelectedItemPosition() - 1;
	CString process_path;
	if (selectedRow != -1)
	{
		process_path = m_list_process_info.GetItemText(selectedRow, 4);
		std::filesystem::path output(theApp.m_ExeDir);
		output /= "gamer_files";
		if (!std::filesystem::exists(output))
		{
			std::filesystem::create_directory(output);
		}
		std::string process_path_str = CT2A(process_path);
		std::filesystem::path file_name(process_path_str);
		output /= file_name.filename();

		CString cmd;
		cmd.Format(TEXT("download -p \"%s\" -o \"%s\""), process_path, CA2T(output.string().c_str()));
		//theApp.GetMainFrame()->GetClientView().OnCmdView();
		cmd.Replace(_T("\\"), _T("\\\\"));
		theApp.GetMainFrame()->CopyToClipboard(cmd);
	}
	else
	{
		return;
	}
}

void CProcessInfoDlg::OnCopyValue()
{
	CString cell_value;
	int selectedRow;
	if (m_tab_process_info_main.GetCurSel() == 0)
	{
		selectedRow = (int)m_list_module_info.GetFirstSelectedItemPosition() - 1;
		if (selectedRow != -1)
		{
			cell_value = m_list_module_info.GetItemText(selectedRow, 3);
		}
	}
	else if (m_tab_process_info_main.GetCurSel() == 1)
	{
		selectedRow = (int)m_list_thread_info.GetFirstSelectedItemPosition() - 1;
		if (selectedRow != -1)
		{
			cell_value = m_list_thread_info.GetItemText(selectedRow, 2);
		}
	}
	else if (m_tab_process_info_main.GetCurSel() == 2)
	{
		selectedRow = (int)m_list_directory_info.GetFirstSelectedItemPosition() - 1;
		if (selectedRow != -1)
		{
			cell_value = m_list_directory_info.GetItemText(selectedRow, 2);
		}
	}
	theApp.GetMainFrame()->CopyToClipboard(cell_value);
}

void CProcessInfoDlg::OnCopyModulePath()
{
	CString cell_value;
	int selectedRow;
	if (m_tab_process_info_main.GetCurSel() == 0)
	{
		selectedRow = (int)m_list_module_info.GetFirstSelectedItemPosition() - 1;
		if (selectedRow != -1)
		{
			cell_value = m_list_module_info.GetItemText(selectedRow, 4);
		}
	}
	else if (m_tab_process_info_main.GetCurSel() == 1)
	{
		selectedRow = (int)m_list_thread_info.GetFirstSelectedItemPosition() - 1;
		if (selectedRow != -1)
		{
			cell_value = m_list_thread_info.GetItemText(selectedRow, 4);
		}
	}
	cell_value.Replace(_T("\\"), _T("\\\\"));
	theApp.GetMainFrame()->CopyToClipboard(cell_value);
}