// ProcessDirectoryDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "agent.h"
#include "ProcessDirectoryDlg.h"
#include "afxdialogex.h"

using namespace SubProto;
// ProcessDirectoryDlg 对话框

IMPLEMENT_DYNAMIC(ProcessDirectoryDlg, CDialogEx)

ProcessDirectoryDlg::ProcessDirectoryDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IID_DIRECTORY_INFO_DLG, pParent)
{

}

ProcessDirectoryDlg::~ProcessDirectoryDlg()
{
}

void ProcessDirectoryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DIRECTORY_LIST, m_process_directory_list);
}


BEGIN_MESSAGE_MAP(ProcessDirectoryDlg, CDialogEx)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_DIRECTORY_LIST, &ProcessDirectoryDlg::OnLvnColumnclickDirectoryList)
	ON_NOTIFY(NM_RCLICK, IDC_DIRECTORY_LIST, &ProcessDirectoryDlg::OnNMRClickDirectoryList)
	ON_COMMAND(ID_DIRECTORY_NAME, &ProcessDirectoryDlg::OnDirectoryName)
END_MESSAGE_MAP()


// ProcessDirectoryDlg 消息处理程序


BOOL ProcessDirectoryDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_agentDlg = (CagentDlg*)theApp.m_pMainWnd;

	// 进程目录列表
	int colIndex = 0;
	m_process_directory_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_GRIDLINES);
	m_process_directory_list.InsertColumn(colIndex++, TEXT("序号"), LVCFMT_LEFT, 38);
	m_process_directory_list.InsertColumn(colIndex++, TEXT("类型"), LVCFMT_LEFT, 45);
	m_process_directory_list.InsertColumn(colIndex++, TEXT("文件名"), LVCFMT_LEFT, 300);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

static int CALLBACK ProcessDirectoryCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	// columnsByNumberSort 此数组中的列索引按数字排序,其它按字母排序
    std::unique_ptr<CUIntArray> columnsByNumberSort = std::make_unique<CUIntArray>();
	columnsByNumberSort->Add(0);

	return MFCUtil::CListCtrlCompareProc(lParam1, lParam2, lParamSort, columnsByNumberSort.get());
}


void ProcessDirectoryDlg::OnLvnColumnclickDirectoryList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	mfcutil.LvnColumnclickList(pNMLV, pResult, ProcessDirectoryCompareProc, &m_process_directory_list);
	*pResult = 0;
}


void ProcessDirectoryDlg::OnNMRClickDirectoryList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	if((m_current_row_index = pNMItemActivate->iItem) != -1)
	{
		DWORD dwPos = GetMessagePos();
		CPoint point(LOWORD(dwPos), HIWORD(dwPos));
		CMenu menu;
		VERIFY(menu.LoadMenu(IDR_MENU_DIRECTORY));
		CMenu* popup = menu.GetSubMenu(0);
		ASSERT(popup != NULL);
		popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	}

	*pResult = 0;
}


void ProcessDirectoryDlg::OnDirectoryName()
{
	CString ruleText = m_process_directory_list.GetItemText(m_current_row_index, 2);
    if (!Global::AddPolicy(ENM_POLICY_TYPE_FILE_NAME, ENM_PUNISH_TYPE_NO_OPEARATION, ruleText.GetString()))
    {
        MessageBox(TEXT("此文件名称已在黑名单中,请不要重复添加"));
    }
}
