// ProcessModuleDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "agent.h"
#include "ProcessModuleDlg.h"
#include "afxdialogex.h"


// ProcessModuleDlg 对话框

IMPLEMENT_DYNAMIC(ProcessModuleDlg, CDialogEx)

ProcessModuleDlg::ProcessModuleDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MODULE_INFO_DLG, pParent)
{

}

ProcessModuleDlg::~ProcessModuleDlg()
{
}

void ProcessModuleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MODULE_LIST, m_process_module_list);
}


BEGIN_MESSAGE_MAP(ProcessModuleDlg, CDialogEx)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_MODULE_LIST, &ProcessModuleDlg::OnLvnColumnclickModuleList)
	ON_COMMAND(ID_MODULE_NAME, &ProcessModuleDlg::OnModuleName)
	ON_COMMAND(ID_MODULE_PATH, &ProcessModuleDlg::OnModulePath)
	ON_NOTIFY(NM_RCLICK, IDC_MODULE_LIST, &ProcessModuleDlg::OnNMRClickModuleList)
END_MESSAGE_MAP()


// ProcessModuleDlg 消息处理程序


BOOL ProcessModuleDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_agentDlg = (CagentDlg*)theApp.m_pMainWnd;

	// 进程模块列表
	int colIndex = 0;
	m_process_module_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_GRIDLINES);
	m_process_module_list.InsertColumn(colIndex++, TEXT("序号"), LVCFMT_LEFT, 38);
	m_process_module_list.InsertColumn(colIndex++, TEXT("模块名称"), LVCFMT_LEFT, 150);
	m_process_module_list.InsertColumn(colIndex++, TEXT("模块路径"), LVCFMT_LEFT, 550);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

static int CALLBACK ProcessModuleCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	// columnsByNumberSort 此数组中的列索引按数字排序,其它按字母排序
    std::unique_ptr<CUIntArray> columnsByNumberSort = std::make_unique<CUIntArray>();
	columnsByNumberSort->Add(0);

	return MFCUtil::CListCtrlCompareProc(lParam1, lParam2, lParamSort, columnsByNumberSort.get());
}

void ProcessModuleDlg::OnLvnColumnclickModuleList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	mfcutil.LvnColumnclickList(pNMLV, pResult, ProcessModuleCompareProc, &m_process_module_list);
	*pResult = 0;
}


void ProcessModuleDlg::OnNMRClickModuleList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	if((m_current_row_index = pNMItemActivate->iItem) != -1)
	{
		DWORD dwPos = GetMessagePos();
		CPoint point(LOWORD(dwPos), HIWORD(dwPos));
		CMenu menu;
		VERIFY(menu.LoadMenu(IDR_MENU_MODULE));
		CMenu* popup = menu.GetSubMenu(0);
		ASSERT(popup != NULL);
		popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	}

	*pResult = 0;
}


void ProcessModuleDlg::OnModuleName()
{
    using namespace SubProto;
	CString ruleText = m_process_module_list.GetItemText(m_current_row_index, 1);
    if (!Global::AddPolicy(ENM_POLICY_TYPE_MODULE_NAME, ENM_PUNISH_TYPE_NO_OPEARATION, ruleText.GetString()))
    {
        MessageBox(TEXT("此模块名称已在黑名单中,请不要重复添加"));
    }
}


void ProcessModuleDlg::OnModulePath()
{
    using namespace SubProto;
	CString ruleText = m_process_module_list.GetItemText(m_current_row_index, 2);
    if (!Global::AddPolicy(ENM_POLICY_TYPE_MODULE_NAME, ENM_PUNISH_TYPE_NO_OPEARATION, ruleText.GetString()))
    {
        MessageBox(TEXT("此模块路径已在黑名单中,请不要重复添加"));
    }
}
