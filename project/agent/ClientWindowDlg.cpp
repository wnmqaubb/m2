// ProcessWindowDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "agent.h"
#include "clientWindowDlg.h"
#include "afxdialogex.h"
#include "protocol.h"


// ProcessWindowDlg 对话框

IMPLEMENT_DYNAMIC(ClientWindowDlg, CDialogEx)

ClientWindowDlg::ClientWindowDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CLIENT_WINDOW_DLG, pParent)
{

}

ClientWindowDlg::~ClientWindowDlg()
{
}

void ClientWindowDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CLIENT_WINDOW_LIST, m_client_window_list);
}


BEGIN_MESSAGE_MAP(ClientWindowDlg, CDialogEx)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_CLIENT_WINDOW_LIST, &ClientWindowDlg::OnLvnColumnclickClientWindowList)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_CLIENT_WINDOW_LIST, &ClientWindowDlg::OnNMCustomdrawClientWindowList)
	ON_COMMAND(ID_WINDOW_TITLE, &ClientWindowDlg::OnWindowTitle)
	ON_COMMAND(ID_WINDOW_CLASS_NAME, &ClientWindowDlg::OnWindowClassName)
	ON_COMMAND(ID_WINDOW_BOTH, &ClientWindowDlg::OnWindowBoth)
	ON_NOTIFY(NM_RCLICK, IDC_CLIENT_WINDOW_LIST, &ClientWindowDlg::OnNMRClickClientWindowList)
END_MESSAGE_MAP()


// ProcessWindowDlg 消息处理程序

BOOL ClientWindowDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_agentDlg = (CagentDlg*)theApp.m_pMainWnd;

	// 进程模块列表
	int colIndex = 0;
	m_client_window_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_GRIDLINES);
	m_client_window_list.InsertColumn(colIndex++, TEXT("序号"), LVCFMT_LEFT, 38);
	m_client_window_list.InsertColumn(colIndex++, TEXT("窗口标题"), LVCFMT_LEFT, 200);
	m_client_window_list.InsertColumn(colIndex++, TEXT("窗口类名"), LVCFMT_LEFT, 380);
	m_client_window_list.InsertColumn(colIndex++, TEXT("窗口进程"), LVCFMT_LEFT, 60);
	m_client_window_list.InsertColumn(colIndex++, TEXT("进程状态"), LVCFMT_LEFT, 60);
	m_client_window_list.InsertColumn(colIndex++, TEXT("窗口进程名"), LVCFMT_LEFT, 100);
	m_client_window_list.InsertColumn(colIndex++, TEXT("窗口线程"), LVCFMT_LEFT, 60);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void ClientWindowDlg::ShowClientWindowDlg(ProtocolProcessInfo& data)
{
	UINT i = 0, colIndex = 0;;
	CString seq, window_process_id, window_thread_id;
	m_client_window_list.DeleteAllItems();
	for(auto& window : data.windows)
	{
        m_client_window_list.InsertItem(i, TEXT(""));
        colIndex = 0;
        m_client_window_list.SetItemText(i, colIndex++, std::to_wstring(i + 1).c_str());
        m_client_window_list.SetItemText(i, colIndex++, window.caption.c_str());
        m_client_window_list.SetItemText(i, colIndex++, window.class_name.c_str());
        m_client_window_list.SetItemText(i, colIndex++, std::to_wstring(window.pid).c_str());
        m_client_window_list.SetItemText(i, colIndex++, window.pid != -1 ? TEXT("正常") : TEXT("进程被隐藏"));
        m_client_window_list.SetItemText(i, colIndex++, window.process_name.c_str());
        m_client_window_list.SetItemText(i, colIndex++, std::to_wstring(window.tid).c_str());
        i++;
	}
}

static int CALLBACK ClientWindowCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	// columnsByNumberSort 此数组中的列索引按数字排序,其它按字母排序
    std::unique_ptr<CUIntArray> columnsByNumberSort = std::make_unique<CUIntArray>();
	columnsByNumberSort->Add(0);
	columnsByNumberSort->Add(3);
	columnsByNumberSort->Add(6);

	return MFCUtil::CListCtrlCompareProc(lParam1, lParam2, lParamSort, columnsByNumberSort.get());
}

void ClientWindowDlg::OnLvnColumnclickClientWindowList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	mfcutil.LvnColumnclickList(pNMLV, pResult, ClientWindowCompareProc, &m_client_window_list);
	*pResult = 0;
}


void ClientWindowDlg::OnNMCustomdrawClientWindowList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVCUSTOMDRAW pLVCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);
	*pResult = CDRF_DODEFAULT;

	if(CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	} else if(CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		//处理，将进程隐藏的标记为橙色  
		int isHideProcess = _ttoi(m_client_window_list.GetItemText(pLVCD->nmcd.dwItemSpec, 3));
		if(!isHideProcess)
		{   //当前选中的item   
			pLVCD->clrTextBk = RGB(0xFF, 0xA5, 0);
		}
	}
}


void ClientWindowDlg::OnNMRClickClientWindowList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	if((m_current_row_index = pNMItemActivate->iItem) != -1)
	{
		DWORD dwPos = GetMessagePos();
		CPoint point(LOWORD(dwPos), HIWORD(dwPos));
		CMenu menu;
		VERIFY(menu.LoadMenu(IDR_MENU_WINDOW));
		CMenu* popup = menu.GetSubMenu(0);
		ASSERT(popup != NULL);
		popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	}

	*pResult = 0;
}



void ClientWindowDlg::OnWindowTitle()
{
    using namespace SubProto;
	CString ruleText = m_client_window_list.GetItemText(m_current_row_index, 1);
    if (!Global::AddPolicy(ENM_POLICY_TYPE_WINDOW_NAME, ENM_PUNISH_TYPE_NO_OPEARATION, ruleText.GetString()))
    {
        MessageBox(TEXT("此窗口标题已在黑名单中,请不要重复添加"));
    }
}


void ClientWindowDlg::OnWindowClassName()
{
    using namespace SubProto;
	CString ruleText = m_client_window_list.GetItemText(m_current_row_index, 2);
    if (!Global::AddPolicy(ENM_POLICY_TYPE_WINDOW_NAME, ENM_PUNISH_TYPE_NO_OPEARATION, ruleText.GetString()))
    {
        MessageBox(TEXT("此窗口类名已在黑名单中,请不要重复添加"));
    }
}


void ClientWindowDlg::OnWindowBoth()
{
    using namespace SubProto;
	CString ruleText = m_client_window_list.GetItemText(m_current_row_index, 1) + "|" + m_client_window_list.GetItemText(m_current_row_index, 2);
    if (!Global::AddPolicy(ENM_POLICY_TYPE_WINDOW_NAME, ENM_PUNISH_TYPE_NO_OPEARATION, ruleText.GetString()))
    {
        MessageBox(TEXT("此窗口标题|类名已在黑名单中,请不要重复添加"));
    }
}