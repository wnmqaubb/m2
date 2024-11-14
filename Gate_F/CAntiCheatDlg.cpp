// CAntiCheatDlg.cpp: 实现文件
//

#include "pch.h"
#include "GateF.h"
#include "afxdialogex.h"
#include "CAntiCheatDlg.h"


// CAntiCheatDlg 对话框

IMPLEMENT_DYNAMIC(CAntiCheatDlg, CDialogEx)

CAntiCheatDlg::CAntiCheatDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_ANTICHEAT, pParent)
	, m_policy_timeout(TRUE)
	, m_detect_proxy(TRUE)
	, m_detect_offline(TRUE)
	, m_detect_speed(TRUE)
	, m_detect_virtual(TRUE)
	, m_heartbeat_timeout(TRUE)
	, m_anti_cc(TRUE)
{

}

BEGIN_MESSAGE_MAP(CAntiCheatDlg, CDialogEx)
	ON_WM_CONTEXTMENU()
	//ON_COMMAND(ID_ADD_TO_WHITELIST, OnAddToWhiteList)
	ON_COMMAND(ID_REMOVE_FROM_WHITELIST, OnRemoveFromWhiteList)
END_MESSAGE_MAP()

CAntiCheatDlg::~CAntiCheatDlg()
{
}

BOOL CAntiCheatDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_current_list = &m_list_white_machine;
	ReadFileAndShowList("机器码白名单.txt");
	m_current_list = &m_list_white_ip;
	ReadFileAndShowList("IP白名单.txt");
	m_current_list = &m_list_white_rolename;
	ReadFileAndShowList("角色名白名单.txt");
	m_current_list = &m_list_black_machine;
	ReadFileAndShowList("机器码黑名单.txt");
	m_current_list = &m_list_black_ip;
	ReadFileAndShowList("IP黑名单.txt");
	m_current_list = &m_list_black_rolename;
	ReadFileAndShowList("角色名黑名单.txt");
	

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CAntiCheatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK6, m_policy_timeout);
	DDX_Check(pDX, IDC_CHECK1, m_detect_proxy);
	DDX_Check(pDX, IDC_CHECK2, m_detect_offline);
	DDX_Check(pDX, IDC_CHECK3, m_detect_speed);
	DDX_Check(pDX, IDC_CHECK4, m_detect_virtual);
	DDX_Check(pDX, IDC_CHECK5, m_heartbeat_timeout);
	DDX_Check(pDX, IDC_CHECK7, m_anti_cc);
	DDX_Control(pDX, IDC_LIST_WHITE_MACHINE, m_list_white_machine);
	DDX_Control(pDX, IDC_LIST_WHITE_IP, m_list_white_ip);
	DDX_Control(pDX, IDC_LIST_WHITE_ROLENAME, m_list_white_rolename);
	DDX_Control(pDX, IDC_LIST_BLACK_MACHINE, m_list_black_machine);
	DDX_Control(pDX, IDC_LIST_BLACK_IP, m_list_black_ip);
	DDX_Control(pDX, IDC_LIST_BLACK_ROLENAME, m_list_black_rolename);
}

// CAntiCheatDlg 消息处理程序


void CAntiCheatDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	m_current_list = (CListBox*)pWnd;
	CMenu menu;
	menu.LoadMenu(IDR_MENU_WHITE_LIST); // IDR_MENU1是右键菜单资源的ID
	CMenu* pPopupMenu = menu.GetSubMenu(0);
	pPopupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN, point.x, point.y, this);
}

// 添加到白名单的操作
void CAntiCheatDlg::OnAddToList(const CString& list_name)
{
	m_current_list->InsertString(0, list_name);
	SaveListToFile();
}

// 从白名单删除的操作
void CAntiCheatDlg::OnRemoveFromWhiteList()
{
	int selectedIndex = m_current_list->GetCurSel();
	if (selectedIndex != LB_ERR)
	{
		m_current_list->DeleteString(selectedIndex);
		SaveListToFile();
	}
}

std::string CAntiCheatDlg::GetFilePath() {
	std::string path = "";
	switch (m_current_list->GetDlgCtrlID()) {
		case IDC_LIST_WHITE_MACHINE:
			path = "机器码白名单.txt";
			break;
		case IDC_LIST_BLACK_MACHINE:
			path = "机器码黑名单.txt";
			break;
		case IDC_LIST_WHITE_IP:
			path = "IP白名单.txt";
			break;
		case IDC_LIST_BLACK_IP:
			path = "IP黑名单.txt";
			break;
		case IDC_LIST_WHITE_ROLENAME:
			path = "角色名白名单.txt";
			break;
		case IDC_LIST_BLACK_ROLENAME:
			path = "角色名黑名单.txt";
			break;
		default:
			break;
	}
	return path;
}
// 在对话框关闭时保存白名单到文件
void CAntiCheatDlg::SaveListToFile()
{
	const std::string& filePath = GetFilePath(); 
	if (filePath.empty())
		return;
	std::ofstream outFile(filePath, std::ios::out);
	if (outFile.is_open())
	{
		int count = m_current_list->GetCount();
		for (int i = 0; i < count; i++)
		{
			CString list_value;
			m_current_list->GetText(i, list_value);
			outFile << CT2A(list_value.GetBuffer()) << std::endl;
		}
		outFile.close();
	}
}

// 初始化对话框时读取文件并显示在列表框
void CAntiCheatDlg::ReadFileAndShowList(const std::string& filePath)
{
	if (filePath.empty())
		return;
	std::ifstream inFile(filePath);
	if (inFile.is_open())
	{
		std::string machineCode;
		while (std::getline(inFile, machineCode))
		{
			m_current_list->AddString((CA2T)machineCode.c_str());
		}
		inFile.close();
	}
}
