// DlgProcessInfo.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "agent.h"
#include "ProcessInfoDlg.h"
#include "afxdialogex.h"
#include "agentDlg.h"
#include <afxstr.h>
#include <atlconv.h>
#include <tchar.h>

// DlgProcessInfo 对话框

IMPLEMENT_DYNAMIC(ProcessInfoDlg, CDialogEx)

ProcessInfoDlg::ProcessInfoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PROCESS_INFO, pParent)
	, m_search_text(_T(""))
{

}

ProcessInfoDlg::~ProcessInfoDlg()
{
}


void ProcessInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_process_info_list);
	DDX_Text(pDX, IDC_EDIT1, m_search_text);
	DDX_Control(pDX, IDC_TAB1, m_module_thread_info_tab);
}


BEGIN_MESSAGE_MAP(ProcessInfoDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_NOTIFY(NM_CLICK, IDC_LIST1, &ProcessInfoDlg::OnNMClickList1)
	ON_BN_CLICKED(IDC_BUTTON1, &ProcessInfoDlg::OnBnClickedFlash)
	ON_BN_CLICKED(IDC_BUTTON2, &ProcessInfoDlg::OnBnClickedSearch)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &ProcessInfoDlg::OnTcnSelchangeTab1)
	//	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, &ProcessInfoDlg::OnLvnItemchangedList1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST1, &ProcessInfoDlg::OnNMCustomdrawList1)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST1, &ProcessInfoDlg::OnLvnColumnclickList1)
	ON_NOTIFY(NM_RCLICK, IDC_LIST1, &ProcessInfoDlg::OnNMRClickList1)
	ON_COMMAND(ID_POCESS_NAME, &ProcessInfoDlg::OnProcessName)
	ON_COMMAND(ID_PROCESS_PATH, &ProcessInfoDlg::OnProcessPath)
END_MESSAGE_MAP()


// DlgProcessInfo 消息处理程序


BOOL ProcessInfoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_agentDlg = (CagentDlg*)theApp.m_pMainWnd;

	int colIndex = 0;
	m_process_info_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_GRIDLINES);
	m_process_info_list.InsertColumn(colIndex++, TEXT("序号"), LVCFMT_LEFT, 38);
	m_process_info_list.InsertColumn(colIndex++, TEXT("PID"), LVCFMT_LEFT, 40);
	m_process_info_list.InsertColumn(colIndex++, TEXT("进程名称"), LVCFMT_LEFT, 150);
	m_process_info_list.InsertColumn(colIndex++, TEXT("进程路径"), LVCFMT_LEFT, 530);


	// 进程模块,线程tab
	m_module_thread_info_tab.InsertItem(0, TEXT("模块信息"));
	m_module_thread_info_tab.InsertItem(1, TEXT("线程信息"));
	m_module_thread_info_tab.InsertItem(2, TEXT("进程目录"));
	//创建两个对话框
	m_processModuleDlg.Create(IDD_MODULE_INFO_DLG, &m_module_thread_info_tab);
	m_processThreadDlg.Create(IDD_THREAD_INFO_DLG, &m_module_thread_info_tab);
	m_processDirectoryDlg.Create(IIDD_DIRECTORY_INFO_DLG, &m_module_thread_info_tab);
	//设定在Tab内显示的范围
	CRect rc;
	m_module_thread_info_tab.GetClientRect(rc);
	rc.top += 20;
	rc.bottom -= 0;
	rc.left += 0;
	rc.right -= 0;
	m_processModuleDlg.MoveWindow(&rc);
	m_processThreadDlg.MoveWindow(&rc);
	m_processDirectoryDlg.MoveWindow(&rc);

	//把对话框对象指针保存起来
	m_pDialog[0] = &m_processModuleDlg;
	m_pDialog[1] = &m_processThreadDlg;
	m_pDialog[2] = &m_processDirectoryDlg;

	//显示初始页面
	m_pDialog[0]->ShowWindow(SW_SHOW);
	m_pDialog[1]->ShowWindow(SW_HIDE);
	m_pDialog[2]->ShowWindow(SW_HIDE);
	//保存当前选择
	m_current_select_tab = 0;
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void ProcessInfoDlg::OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	UINT i, uSelectedCount = m_process_info_list.GetSelectedCount();
	int nItem = -1;

	//CagentDlg* parent = (CagentDlg*)m_pParentWnd;
	//AfxMessageBox(TEXT("ProcessInfoDlg::OnNMClickList1"));
	//parent->OnBnClickedButton3();
	//AfxMessageBox(TEXT("ProcessInfoDlg::OnNMClickList2"));
	if(uSelectedCount > 0)
	{
		for(i = 0; i < uSelectedCount; i++)
		{
			nItem = m_process_info_list.GetNextItem(nItem, LVNI_SELECTED);
			POSITION pos = m_process_info_list.GetFirstSelectedItemPosition(); //pos选中的首行位置
			if(pos == NULL)
				AfxMessageBox(TEXT("请选择一个玩家!"));
			else
			{
				while(pos)   //如果你选择多行
				{
					int nIdx = -1;
					nIdx = m_process_info_list.GetNextSelectedItem(pos);

					if(nIdx >= 0 && nIdx < m_process_info_list.GetItemCount())
					{
						CONNID connect_id = MainWnd->m_current_connect_id;
						CString PID = m_process_info_list.GetItemText(nIdx, 1);
						//PID.GetString();
						//声明标识符
						//USES_CONVERSION;
						//char* pid = T2A(PID);
						int pid = _ttoi(PID);
						if(connect_id != -1)
						{
							Singleton<TaskProcessInfo>::getInstance().send_get_modules(connect_id, pid);
							Singleton<TaskProcessInfo>::getInstance().send_get_threads(connect_id, pid);
							Singleton<TaskProcessInfo>::getInstance().send_get_directories(connect_id, pid);
						}
					}
				}
			}
			break;
		}
	}
	*pResult = 0;
}


void ProcessInfoDlg::ShowProcessInfo(ProtocolProcessInfo& data)
{
	UINT i = 0, colIndex = 0;;
	CString seq, pid;
	m_process_info_list.DeleteAllItems();
	for(auto& process : data.processes)
	{
        m_process_info_list.InsertItem(i, TEXT(""));
		colIndex = 0;
        std::filesystem::path path(process.path);
		m_process_info_list.SetItemText(i, colIndex++, std::to_wstring(i+1).c_str());
		m_process_info_list.SetItemText(i, colIndex++, std::to_wstring(process.pid).c_str());
		m_process_info_list.SetItemText(i, colIndex++, path.filename().wstring().c_str());
		m_process_info_list.SetItemText(i, colIndex++, path.wstring().c_str());
		i++;
	}
}

void ProcessInfoDlg::ShowProcessModuleInfo(ProtocolProcessInfo& data)
{
	UINT i = 0;
	CString seq;
	m_processModuleDlg.m_process_module_list.DeleteAllItems();
	for(auto& module : data.modules)
	{
        std::filesystem::path path(module.path);
		m_processModuleDlg.m_process_module_list.InsertItem(i, TEXT(""));
		int colIndex = 0;
		m_processModuleDlg.m_process_module_list.SetItemText(i, colIndex++, std::to_wstring(i+1).c_str());
		m_processModuleDlg.m_process_module_list.SetItemText(i, colIndex++, path.filename().wstring().c_str());
		m_processModuleDlg.m_process_module_list.SetItemText(i, colIndex++, path.wstring().c_str());
		i++;
	}
}

void ProcessInfoDlg::ShowProcessThreadInfo(ProtocolProcessInfo& data)
{
	UINT i = 0;
	CString startaddr;
	m_processThreadDlg.m_process_thread_list.DeleteAllItems();
	for(auto& thread : data.threads)
	{
		m_processThreadDlg.m_process_thread_list.InsertItem(i, TEXT(""));
		startaddr.Format(TEXT("0x%llX"), thread.startaddr);
		int colIndex = 0;
		m_processThreadDlg.m_process_thread_list.SetItemText(i, colIndex++, std::to_wstring(i + 1).c_str());
		m_processThreadDlg.m_process_thread_list.SetItemText(i, colIndex++, std::to_wstring(thread.tid).c_str());
		m_processThreadDlg.m_process_thread_list.SetItemText(i, colIndex++, startaddr);
		m_processThreadDlg.m_process_thread_list.SetItemText(i, colIndex++, thread.owner_module.c_str());
		i++;
	}
}

void ProcessInfoDlg::ShowProcessDirectoryInfo(ProtocolProcessInfo& data)
{
	UINT i = 0, colIndex = 0;
	CString seq;
	m_processDirectoryDlg.m_process_directory_list.DeleteAllItems();
    for (auto& dir : data.dirs)
    {
        colIndex = 0;
        m_processDirectoryDlg.m_process_directory_list.InsertItem(i, TEXT(""));
        m_processDirectoryDlg.m_process_directory_list.SetItemText(i, colIndex++, std::to_wstring(i+1).c_str());
        m_processDirectoryDlg.m_process_directory_list.SetItemText(i, colIndex++, dir.is_directory ? TEXT("[目录]") : TEXT("[文件]"));
        m_processDirectoryDlg.m_process_directory_list.SetItemText(i, colIndex++, dir.path.c_str());
        i++;
    }
}


void ProcessInfoDlg::OnBnClickedFlash()
{
	m_processModuleDlg.m_process_module_list.DeleteAllItems();
	m_processThreadDlg.m_process_thread_list.DeleteAllItems();
}


void ProcessInfoDlg::OnBnClickedSearch()
{
	CUIntArray columnIndexs;
	columnIndexs.Add(3); 
	UpdateData(TRUE);
	mfcutil.CListCtrlSearch(m_search_text.Trim(), m_search_text_old, m_process_info_list, columnIndexs);
}


void ProcessInfoDlg::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	//把当前的页面隐藏起来
	m_pDialog[m_current_select_tab]->ShowWindow(SW_HIDE);
	//得到新的页面索引
	m_current_select_tab = m_module_thread_info_tab.GetCurSel();
	//把新的页面显示出来
	m_pDialog[m_current_select_tab]->ShowWindow(SW_SHOW);
	*pResult = 0;
}


//void ProcessInfoDlg::OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult) {
//	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
//	// TODO: 在此添加控件通知处理程序代码
//	*pResult = 0;
//}


void ProcessInfoDlg::OnNMCustomdrawList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	LPNMLVCUSTOMDRAW pLVCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);
	*pResult = CDRF_DODEFAULT;

	// First thing - check the draw stage. If it's the control's prepaint   
		// stage, then tell Windows we want messages for every item.   
	if(CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	} else if(CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		// This is the notification message for an item.    
		//处理，将item改变背景颜色   绿色
		//if(0 == pLVCD->nmcd.dwItemSpec || 1 == pLVCD->nmcd.dwItemSpec || 3 == pLVCD->nmcd.dwItemSpec) {   //当前选中的item   
		//	pLVCD->clrTextBk = RGB(255, 108, 108);
		//}
		//if(2 == pLVCD->nmcd.dwItemSpec || 5 == pLVCD->nmcd.dwItemSpec) {   //当前选中的item   
		//	pLVCD->clrTextBk = RGB(228, 207, 126);
		//}
		//if(4 == pLVCD->nmcd.dwItemSpec || 6 == pLVCD->nmcd.dwItemSpec || 7 == pLVCD->nmcd.dwItemSpec) {   //当前选中的item   
		//	pLVCD->clrTextBk = RGB(127, 216, 161);
		//}
		//前一次选中的item，恢复为白色   
		/*else if(m_itemForeSel == pLVCD->nmcd.dwItemSpec) {
			pLVCD->clrTextBk = RGB(绿色);
		}*/

		*pResult = CDRF_DODEFAULT;
	}
}

static int CALLBACK MyCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	// columnsByNumberSort 此数组中的列索引按数字排序,其它按字母排序
    std::unique_ptr<CUIntArray> columnsByNumberSort = std::make_unique<CUIntArray>();
	columnsByNumberSort->Add(0);
	columnsByNumberSort->Add(1);

	return MFCUtil::CListCtrlCompareProc(lParam1, lParam2, lParamSort, columnsByNumberSort.get());
}

void ProcessInfoDlg::OnLvnColumnclickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	mfcutil.LvnColumnclickList(pNMLV, pResult, MyCompareProc, &m_process_info_list);
	*pResult = 0;
}



void ProcessInfoDlg::OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	if((m_current_row_index = pNMItemActivate->iItem) != -1)
	{
		DWORD dwPos = GetMessagePos();
		CPoint point(LOWORD(dwPos), HIWORD(dwPos));
		CMenu menu;
		VERIFY(menu.LoadMenu(IDR_MENU_PROCESS));
		CMenu* popup = menu.GetSubMenu(0);
		ASSERT(popup != NULL);
		popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	}

	*pResult = 0;
}


void ProcessInfoDlg::OnProcessName()
{
    using namespace SubProto;
	CString ruleText = m_process_info_list.GetItemText(m_current_row_index, 2);
    if (!Global::AddPolicy(ENM_POLICY_TYPE_PROCESS_NAME, ENM_PUNISH_TYPE_NO_OPEARATION, ruleText.GetString()))
    {
        MessageBox(TEXT("此进程名称已在黑名单中,请不要重复添加"));
    }
}


void ProcessInfoDlg::OnProcessPath()
{
    using namespace SubProto;
	CString ruleText = m_process_info_list.GetItemText(m_current_row_index, 3);
    if (!Global::AddPolicy(ENM_POLICY_TYPE_PROCESS_NAME, ENM_PUNISH_TYPE_NO_OPEARATION, ruleText.GetString()))
    {
        MessageBox(TEXT("此进程路径已在黑名单中,请不要重复添加"));
    }
}


void ProcessInfoDlg::OnOK()
{
	return;
}


BOOL ProcessInfoDlg::PreTranslateMessage(MSG* pMsg)
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
