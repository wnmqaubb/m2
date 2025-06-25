
// GateView.cpp: CProcessView 类的实现
//

#include "pch.h"
#include "framework.h"
#include "Gate.h"

#include "ProcessChildFrm.h"
#include "ProcessDoc.h"
#include "ProcessView.h"
#include "CmdView.h"
#include <atlconv.h>
#include <filesystem>
#include "MainFrm.h"
#include "ClientView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include <ConfigSettingDoc.h>
#include <ConfigSettingView.h>


// CProcessView

IMPLEMENT_DYNCREATE(CProcessView, CView)

BEGIN_MESSAGE_MAP(CProcessView, CView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_CREATE()
	ON_WM_SIZE()
    ON_NOTIFY(LVN_ITEMCHANGED, ID_PROCESS_VIEW, &CProcessView::OnListItemChanged)
    ON_COMMAND(ID_PROCESS_NAME, &CProcessView::OnProcessNameBan)
    ON_COMMAND(ID_PROCESS_PATH, &CProcessView::OnProcessPathBan)
    ON_COMMAND(ID_GET_GAMEUSER_FILE, &CProcessView::OnGetGameUserFile)
    ON_COMMAND(ID_GET_GAMEUSER_FILE_SIGN, &CProcessView::OnGetGameUserFileSign)
END_MESSAGE_MAP()

// CProcessView 构造/析构

CProcessView::CProcessView() noexcept
{
	// TODO: 在此处添加构造代码

}

CProcessView::~CProcessView()
{
}

BOOL CProcessView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CProcessView 绘图

void CProcessView::OnDraw(CDC* pDC)
{
	CView::OnDraw(pDC);
	// TODO: 在此处为本机数据添加绘制代码
}

void CProcessView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CProcessView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
	CMenu menu;
	menu.LoadMenu(IDR_PROCESS_RIGHT_MENU);
	CMenu* pSumMenu = menu.GetSubMenu(0);
	theApp.GetContextMenuManager()->ShowPopupMenu(*pSumMenu, point.x, point.y, this, TRUE);
}


// CProcessView 诊断
#ifdef _DEBUG
void CProcessView::AssertValid() const
{
	CView::AssertValid();
}

void CProcessView::Dump(CDumpContext& dc) const
{
CView::Dump(dc);
}

CProcessDoc* CProcessView::GetDocument() const // 非调试版本是内联的
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CProcessDoc)));
    return (CProcessDoc*)m_pDocument;
}

#endif //_DEBUG

CProcessChildFrame* CProcessView::GetParentCChild() const
{
    ASSERT(GetParent()->IsKindOf(RUNTIME_CLASS(CProcessChildFrame)));
    return (CProcessChildFrame*)GetParent();
}
// CProcessView 消息处理程序


int CProcessView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CView::OnCreate(lpCreateStruct) == -1)
        return -1;



    CRect rectDummy;
    GetClientRect(&rectDummy);

    // 创建视图: 
    const DWORD dwViewStyle = LVS_SINGLESEL | LVS_SHOWSELALWAYS | WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    if (!m_ViewList.Create(dwViewStyle, rectDummy, this, ID_PROCESS_VIEW))
    {
        TRACE0("未能创建列表视图\n");
    }
    return 0;
}


void CProcessView::FillViewList()
{
    m_ViewList.SetColumnByIntSort({ 0, 1, 2 });
    auto& processes = GetDocument()->GetProcesses();
    m_ViewList.SetRedraw(FALSE);
    m_ViewList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_GRIDLINES);
    int colIndex = 0;
    m_ViewList.InsertColumn(colIndex++, TEXT("序号"), LVCFMT_LEFT, 38);
    m_ViewList.InsertColumn(colIndex++, TEXT("PID"), LVCFMT_LEFT, 40);
    m_ViewList.InsertColumn(colIndex++, TEXT("父PID"), LVCFMT_LEFT, 45);
    m_ViewList.InsertColumn(colIndex++, TEXT("进程名称"), LVCFMT_LEFT, 150);
    m_ViewList.InsertColumn(colIndex++, TEXT("进程路径"), LVCFMT_LEFT, 530);
    m_ViewList.InsertColumn(colIndex++, TEXT("进程位数"), LVCFMT_LEFT, 70);
    m_ViewList.DeleteAllItems();
    CString connect_id_str, seq;
    const wchar_t* format_d = L"%d";
    const wchar_t* format_3d = L"%03d";
    const wchar_t* empty_wstr = L"";
    const wchar_t* no_access_wstr = L"无权限";
    const wchar_t* empty_list_wstr = L"模块列表为空";
    const wchar_t* x32_wstr = L"x32";
    const wchar_t* x64_wstr = L"x64";
    int rowNum = 0;
    for (auto& process : processes.data)
    {
        if (rowNum % 100 == 0) Sleep(1);
        colIndex = 0;
        CString id;
        CString temp;
        id.Format(format_d, rowNum + 1);
        m_ViewList.InsertItem(rowNum, empty_wstr);
        m_ViewList.SetItemText(rowNum, colIndex++, id);
        temp.Format(format_d, process.second.pid);
        m_ViewList.SetItemText(rowNum, colIndex++, temp);
        temp.Format(format_d, process.second.parent_pid);
        m_ViewList.SetItemText(rowNum, colIndex++, temp);
        m_ViewList.SetItemText(rowNum, colIndex++, process.second.name.c_str());
        m_ViewList.SetItemText(rowNum, colIndex++, process.second.no_access ? no_access_wstr :
            process.second.modules.empty() ? empty_list_wstr : process.second.modules.front().path.c_str());
        m_ViewList.SetItemText(rowNum, colIndex++, process.second.is_64bits ? x64_wstr : x32_wstr);
        rowNum++;
    }
    m_ViewList.SetRedraw(TRUE);
}

void CProcessView::OnInitialUpdate()
{
    CView::OnInitialUpdate();
    FillViewList();
}


void CProcessView::OnSize(UINT nType, int cx, int cy)
{
    CView::OnSize(nType, cx, cy);
    AdjustLayout();
    // TODO: 在此处添加消息处理程序代码
}


void CProcessView::AdjustLayout()
{
    if (GetSafeHwnd() == nullptr)
    {
        return;
    }

    CRect rectClient;
    GetClientRect(rectClient);

    m_ViewList.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}

void CProcessView::OnListItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    if (pNMListView->uChanged == LVIF_STATE)
    {
        if (pNMListView->uNewState & LVIS_SELECTED)
        {
            int nItem = pNMListView->iItem;
            CString csPid = m_ViewList.GetItemText(nItem, 1);
            uint32_t pid = atoi(CT2A(csPid.GetBuffer()));
            auto& processes = GetDocument()->GetProcesses();
            auto& process = processes.data.find(pid)->second;
            GetParentCChild()->m_wndProcessSubViewWnd.FillModuleWindow(process.modules);
            GetParentCChild()->m_wndProcessSubViewWnd.FillThreadWindow(process.threads);
            GetParentCChild()->m_wndProcessSubViewWnd.FillDirectoryWindow(process.directories);
        }
    }
}

void CProcessView::OnProcessNameBan()
{
    auto selectedRow = (int)m_ViewList.GetFirstSelectedItemPosition() - 1;
    std::wstring process_name;
    if (selectedRow != -1)
    {
        process_name = m_ViewList.GetItemText(selectedRow, 3);
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
        unsigned int uiLastPolicyId = theApp.GetMainFrame()->GetClientView().next_policy_id(Policies);

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
        ScrollToAddByPolicyId(uiLastPolicyId);

    }
}

void CProcessView::ScrollToAddByPolicyId(int policy_id) {
    auto pSettingDoc = ((CConfigSettingDoc*)theApp.m_ConfigDoc);
    if (pSettingDoc->GetView<CConfigSettingView>())
    {
        pSettingDoc->GetView<CConfigSettingView>()->ScrollToAddByPolicyId(policy_id);
    }
}

void CProcessView::OnProcessPathBan()
{
    auto selectedRow = (int)m_ViewList.GetFirstSelectedItemPosition() - 1;
    std::wstring process_path;
    if (selectedRow != -1)
    {
        process_path = m_ViewList.GetItemText(selectedRow, 4);
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
        unsigned int uiLastPolicyId = theApp.GetMainFrame()->GetClientView().next_policy_id(Policies);
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
        ScrollToAddByPolicyId(uiLastPolicyId);
    }

}

void CProcessView::OnGetGameUserFile()
{
	auto selectedRow = (int)m_ViewList.GetFirstSelectedItemPosition() - 1;
	CString process_path;
	if (selectedRow != -1)
	{
		process_path = m_ViewList.GetItemText(selectedRow, 4);
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

void CProcessView::OnGetGameUserFileSign()
{
    // powershell -Command "Get-AuthenticodeSignature -FilePath 'C:\Users\A.exe' | Format-List" -Property SignerCertificate, Status
    auto selectedRow = (int)m_ViewList.GetFirstSelectedItemPosition() - 1;
    CString process_path;
    if (selectedRow != -1)
    {
        process_path = m_ViewList.GetItemText(selectedRow, 4);
        CString cmd;
        cmd.Format(TEXT("powershell -Command \"Get - AuthenticodeSignature -FilePath '%s' | Format-List\" -Property Status"), process_path);
        //theApp.GetMainFrame()->GetClientView().OnCmdView();
        cmd.Replace(_T("\\"), _T("\\\\"));
        theApp.GetMainFrame()->CopyToClipboard(cmd);
    }
    else
    {
        return;
    }
}