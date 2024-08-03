// shell_manager.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "agent.h"
#include "shellManagerDlg.h"
#include "afxdialogex.h"
#include <fstream>
#include <filesystem>
#include <algorithm>


// shell_manager 对话框

IMPLEMENT_DYNAMIC(ShellManagerDlg, CDialogEx)

ShellManagerDlg::ShellManagerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SHELL_MANAGER_DLG, pParent)
	, m_search_text(_T(""))
	, m_shell_auto(FALSE)
{
	auto shell_auto_txt = std::filesystem::current_path() / "shell\\shellauto.txt";
	if(std::filesystem::exists(shell_auto_txt))
	{
		std::ifstream shell_auto(shell_auto_txt);
		std::string checked;
		shell_auto >> std::skipws >> checked;
		m_shell_auto = (bool)atoi(checked.data());		
	}
}

ShellManagerDlg::~ShellManagerDlg()
{
}

void ShellManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SHELL_FILE_LIST, m_shell_file_list);
	DDX_Text(pDX, IDC_EDIT1, m_search_text);
	DDX_Check(pDX, IDC_CHECK3, m_shell_auto);
}


BEGIN_MESSAGE_MAP(ShellManagerDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &ShellManagerDlg::OnBnClickedFlush)
	ON_BN_CLICKED(IDC_CHECK1, &ShellManagerDlg::OnBnClickedCheckedAll)
	ON_BN_CLICKED(IDC_CHECK2, &ShellManagerDlg::OnBnClickedInvert)
	ON_BN_CLICKED(IDC_BUTTON2, &ShellManagerDlg::OnBnClickedSearch)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_SHELL_FILE_LIST, &ShellManagerDlg::OnLvnColumnclickShellFileList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_SHELL_FILE_LIST, &ShellManagerDlg::OnLvnItemchangedShellFileList)
	ON_NOTIFY(NM_CLICK, IDC_SHELL_FILE_LIST, &ShellManagerDlg::OnNMClickShellFileList)
	ON_BN_CLICKED(IDC_BUTTON3, &ShellManagerDlg::OnBnClickedSave)
	ON_BN_CLICKED(IDC_BUTTON_SHELL_ACTION, &ShellManagerDlg::OnBnClickedShellAction)
	ON_BN_CLICKED(IDC_CHECK3, &ShellManagerDlg::OnBnClickedShellAuto)
END_MESSAGE_MAP()


// shell_manager 消息处理程序

BOOL ShellManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_shell_file_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES);
	int colIndex = 0;
	m_shell_file_list.InsertColumn(colIndex++, TEXT("启用"), LVCFMT_LEFT, 38);
	m_shell_file_list.InsertColumn(colIndex++, TEXT("序号"), LVCFMT_LEFT, 38);
	m_shell_file_list.InsertColumn(colIndex++, TEXT("文件名"), LVCFMT_LEFT, 150);
	m_shell_file_list.InsertColumn(colIndex++, TEXT("外挂名"), LVCFMT_LEFT, 150);
	m_shell_file_list.InsertColumn(colIndex++, TEXT("说明"), LVCFMT_LEFT, 600);
	m_shell_file_list.InsertColumn(colIndex++, TEXT("readme文件路径"), LVCFMT_LEFT, 0);
	m_shell_file_list.InsertColumn(colIndex++, TEXT("shell文件路径"), LVCFMT_LEFT, 0);
	UpdateData();
	ScanShellDirectory();
	return TRUE;
}


void ShellManagerDlg::OnBnClickedFlush()
{
    GetDlgItem(IDC_BUTTON1)->EnableWindow(false);
	ScanShellDirectory();

    GetDlgItem(IDC_BUTTON1)->EnableWindow(true);
}

void ShellManagerDlg::OnBnClickedCheckedAll()
{
	for(int i = 0; i < m_shell_file_list.GetItemCount(); i++)
	{
		m_shell_file_list.SetCheck(i, ((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck());
	}
}


void ShellManagerDlg::OnBnClickedInvert()
{

	for(int i = 0; i < m_shell_file_list.GetItemCount(); i++)
	{
		m_shell_file_list.SetCheck(i, !m_shell_file_list.GetCheck(i));
	}
}


void ShellManagerDlg::OnBnClickedSearch()
{
	CUIntArray columnIndexs;
	columnIndexs.Add(2);
	columnIndexs.Add(3);
	columnIndexs.Add(4);
	UpdateData(TRUE);
	mfcutil.CListCtrlSearch(m_search_text, m_search_text_old, m_shell_file_list, columnIndexs);
}

void ShellManagerDlg::EnumShellDirectory(std::function<void(std::filesystem::path,bool, std::string)> callback)
{
	namespace fs = std::filesystem;
	auto shellboot = fs::current_path() / GlobalString::ANTICHEAT::SHELL_DIRECTORIES;
	if(!fs::exists(shellboot) || !fs::is_directory(shellboot))
	{
		fs::create_directories(shellboot);
		return;
	}
	fs::path txtFile;
	for(auto& shellDir : fs::directory_iterator(shellboot))
	{
		if(!shellDir.is_directory())
		{
			continue;
		}

		for(auto& shell : fs::directory_iterator(shellDir.path()))
		{
			if(shell.is_directory() || shell.file_size() == 0 || shell.path().extension().empty())
			{
				continue;
			}
			auto extension = shell.path().extension().generic_string();
			transform(extension.begin(), extension.end(), extension.begin(), ::toupper);
			if(GlobalString::ANTICHEAT::FILE_EXTENSION_BIN != extension)
			{
				continue;
			}
			txtFile = shellDir.path() / GlobalString::ANTICHEAT::SHELL_README_TXT;
			if(!fs::exists(txtFile))
			{
				continue;
			}

			std::ifstream readme(txtFile);
			std::string checked,description;;
			readme >> std::skipws >> checked;
			readme >> std::skipws >> description;
			readme.close();
			bool checked_ = false;
			try
			{
				checked_ = (bool)atoi(checked.data());				
			}
			catch(...)
			{
			}

			callback(shell, checked_, description);
			
		}
	}

}

void ShellManagerDlg::ScanShellDirectory()
{
	m_shell_file_list.DeleteAllItems();
	UINT i = 0, colIndex = 0;
	CString seq;
	EnumShellDirectory([&](std::filesystem::path file,bool checked,std::string description) {
		std::filesystem::path txtFile = file.parent_path() / GlobalString::ANTICHEAT::SHELL_README_TXT;
		seq.Format(TEXT("%02d"), i + 1);
		colIndex = 0;
		m_shell_file_list.InsertItem(i, TEXT(""));
		m_shell_file_list.SetItemText(i, colIndex++, TEXT(""));
		m_shell_file_list.SetItemText(i, colIndex++, seq);
		m_shell_file_list.SetItemText(i, colIndex++, file.filename().c_str());
		m_shell_file_list.SetItemText(i, colIndex++, file.parent_path().filename().c_str());
		m_shell_file_list.SetItemText(i, colIndex++, Utils::string2wstring(description, CP_UTF8).c_str());
		m_shell_file_list.SetItemText(i, colIndex++, txtFile.c_str());
		m_shell_file_list.SetItemText(i, colIndex++, file.c_str());
		m_shell_file_list.SetCheck(i, checked ? true : false);
		i++;
	});

}

static int CALLBACK ShellCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	// columnsByNumberSort 此数组中的列索引按数字排序,其它按字母排序
    std::unique_ptr<CUIntArray> columnsByNumberSort = std::make_unique<CUIntArray>();
	columnsByNumberSort->Add(0);
	columnsByNumberSort->Add(1);

	return MFCUtil::CListCtrlCompareProc(lParam1, lParam2, lParamSort, columnsByNumberSort.get());
}

void ShellManagerDlg::OnLvnColumnclickShellFileList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	mfcutil.LvnColumnclickList(pNMLV, pResult, ShellCompareProc, &m_shell_file_list);
	*pResult = 0;
}


void ShellManagerDlg::OnLvnItemchangedShellFileList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	//判断m_bHit，即是否点击了CheckBox
	if(m_bHit)
	{
		m_bHit = FALSE;		//复位
		if(m_shell_file_list.GetCheck(m_itemSel))
		{       //CheckBox被选中
			//do your own processing 
		} else
		{    
			((CButton*)GetDlgItem(IDC_CHECK1))->SetCheck(false);
		}
	}
	*pResult = 0;
}


void ShellManagerDlg::OnNMClickShellFileList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	//获取单击所在的行号
	//找出鼠标位置
	DWORD dwPos = GetMessagePos();
	CPoint point(LOWORD(dwPos), HIWORD(dwPos));
	m_shell_file_list.ScreenToClient(&point);
	//定义结构体
	LVHITTESTINFO lvinfo;
	lvinfo.pt = point;
	//获取行号信息
	int nItem = m_shell_file_list.HitTest(&lvinfo);
	if(nItem != -1)
	{
		m_itemSel = lvinfo.iItem;
	}

		 //判断是否点击在CheckBox上
	if(lvinfo.flags == LVHT_ONITEMSTATEICON)
	{
		m_bHit = TRUE;
	}
	*pResult = 0;
}


void ShellManagerDlg::OnBnClickedSave()
{
    GetDlgItem(IDC_BUTTON3)->EnableWindow(false);

	for(int i = 0; i < m_shell_file_list.GetItemCount(); i++)
	{
		CString txtFile = m_shell_file_list.GetItemText(i, 5);
		std::fstream readme(txtFile, std::ios::binary | std::ios::out | std::ios::in);
		readme.seekp(0, std::ios::beg);
		readme.write(m_shell_file_list.GetCheck(i) ? "1" : "0", 1);
		readme.close();

	}

    Singleton<TaskShellcode>::getInstance().shellcode_init();
    GetDlgItem(IDC_BUTTON3)->EnableWindow(true);
}

// 下发所有选中的shell
void ShellManagerDlg::OnBnClickedShellAction()
{
	Singleton<TaskPolicy>::getInstance().async_.delay_execute_shellcode([&]() {
		for(int i = 0; i < m_shell_file_list.GetItemCount(); i++)
		{
			if(!m_shell_file_list.GetCheck(i))
			{
				continue;
			}
			std::string file = CT2A(m_shell_file_list.GetItemText(i, 6));
            std::string shellcode;
            Singleton<TaskShellcode>::getInstance().read_shell_file(file, shellcode);
            Singleton<TaskPolicy>::getInstance().async_.delay_execute_shellcode([shellcode]() {
                Singleton<TaskShellcode>::getInstance().send_to_all(shellcode);
            }, 300);
		}
	}, 10);	
}


void ShellManagerDlg::OnOK()
{
	return;
}


BOOL ShellManagerDlg::PreTranslateMessage(MSG* pMsg)
{
	if(WM_KEYFIRST <= pMsg->message && pMsg->message <= WM_KEYLAST)
	{
		if(pMsg->wParam == VK_RETURN)
		{
			OnBnClickedSearch();
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void ShellManagerDlg::OnBnClickedShellAuto()
{
	UpdateData(TRUE);
	auto shell_auto_txt = std::filesystem::current_path() / "shell\\shellauto.txt";
	if(std::filesystem::exists(shell_auto_txt))
	{
		std::fstream readme(shell_auto_txt, std::ios::binary | std::ios::out | std::ios::in);
		readme.seekp(0, std::ios::beg);
		readme.write(m_shell_auto ? "1" : "0",1);
		readme.close();
	}
}
