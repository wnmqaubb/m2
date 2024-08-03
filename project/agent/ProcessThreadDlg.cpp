// ProcessThreadDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "agent.h"
#include "ProcessThreadDlg.h"
#include "afxdialogex.h"


// ProcessThreadDlg 对话框

IMPLEMENT_DYNAMIC(ProcessThreadDlg, CDialogEx)

ProcessThreadDlg::ProcessThreadDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_THREAD_INFO_DLG, pParent)
{

}

ProcessThreadDlg::~ProcessThreadDlg()
{
}

void ProcessThreadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_THREAD_LIST, m_process_thread_list);
}


BEGIN_MESSAGE_MAP(ProcessThreadDlg, CDialogEx)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_THREAD_LIST, &ProcessThreadDlg::OnLvnColumnclickThreadList)
END_MESSAGE_MAP()


// ProcessThreadDlg 消息处理程序


BOOL ProcessThreadDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 进程模块列表
	int colIndex = 0;
	m_process_thread_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_GRIDLINES);
	m_process_thread_list.InsertColumn(colIndex++, TEXT("序号"), LVCFMT_LEFT, 38);
	m_process_thread_list.InsertColumn(colIndex++, TEXT("线程ID"), LVCFMT_LEFT, 50);
	m_process_thread_list.InsertColumn(colIndex++, TEXT("线程入口"), LVCFMT_LEFT, 80);
	m_process_thread_list.InsertColumn(colIndex++, TEXT("模块"), LVCFMT_LEFT, 600);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

static int CALLBACK ProcessThreadCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	// columnsByNumberSort 此数组中的列索引按数字排序,其它按字母排序
    std::unique_ptr<CUIntArray> columnsByNumberSort = std::make_unique<CUIntArray>();
	columnsByNumberSort->Add(0);
	columnsByNumberSort->Add(1);

	return MFCUtil::CListCtrlCompareProc(lParam1, lParam2, lParamSort, columnsByNumberSort.get());
}

void ProcessThreadDlg::OnLvnColumnclickThreadList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	mfcutil.LvnColumnclickList(pNMLV, pResult, ProcessThreadCompareProc, &m_process_thread_list);
	*pResult = 0;
}
