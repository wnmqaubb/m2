
// GateView.cpp: CProcessView 类的实现
//

#include "pch.h"
#include "framework.h"
#include "Gate.h"

#include "ChildFrm.h"
#include "WindowDoc.h"
#include "WindowView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CProcessView

IMPLEMENT_DYNCREATE(CWindowView, CView)

BEGIN_MESSAGE_MAP(CWindowView, CView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_CREATE()
	ON_WM_SIZE()
    ON_NOTIFY(LVN_ITEMCHANGED, ID_WINDOWS_VIEW, &CWindowView::OnListItemChanged)
    ON_COMMAND(ID_WINDOW_TITLE, &CWindowView::OnWindowTitleBan)
    ON_COMMAND(ID_WINDOW_CLASS_NAME, &CWindowView::OnWindowClassNameBan)
    ON_COMMAND(ID_WINDOW_BOTH, &CWindowView::OnWindowBothBan)
END_MESSAGE_MAP()

// CProcessView 构造/析构

CWindowView::CWindowView() noexcept
{
	// TODO: 在此处添加构造代码

}

CWindowView::~CWindowView()
{
}

BOOL CWindowView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式
    
	return CView::PreCreateWindow(cs);
}

// CProcessView 绘图

void CWindowView::OnDraw(CDC* pDC)
{
	CView::OnDraw(pDC);
	// TODO: 在此处为本机数据添加绘制代码
}

void CWindowView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CWindowView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
	CMenu menu;
	menu.LoadMenu(IDR_MENU_WINDOW);
	theApp.GetContextMenuManager()->ShowPopupMenu(menu, point.x, point.y, this, TRUE);
}


// CProcessView 诊断
#ifdef _DEBUG
void CWindowView::AssertValid() const
{
	CView::AssertValid();
}

void CWindowView::Dump(CDumpContext& dc) const
{
CView::Dump(dc);
}

CWindowDoc* CWindowView::GetDocument() const // 非调试版本是内联的
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWindowDoc)));
    return (CWindowDoc*)m_pDocument;
}

CChildFrame* CWindowView::GetParentCChild() const
{
    ASSERT(GetParent()->IsKindOf(RUNTIME_CLASS(CChildFrame)));
    return (CChildFrame*)GetParent();
}

#endif //_DEBUG


// CProcessView 消息处理程序


int CWindowView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CView::OnCreate(lpCreateStruct) == -1)
        return -1;



    CRect rectDummy;
    GetClientRect(&rectDummy);

    // 创建视图: 
    const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    if (!m_ViewList.Create(dwViewStyle, rectDummy, this, ID_WINDOWS_VIEW))
    {
        TRACE0("未能创建列表视图\n");
    }
    return 0;
}


void CWindowView::FillViewList()
{
    auto& windows = GetDocument()->GetWindows();
    m_ViewList.SetRedraw(FALSE);
    m_ViewList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_GRIDLINES);
    int colIndex = 0;
    m_ViewList.InsertColumn(colIndex++, TEXT("序号"), LVCFMT_LEFT, 38);
    m_ViewList.InsertColumn(colIndex++, TEXT("句柄"), LVCFMT_LEFT, 70);
    m_ViewList.InsertColumn(colIndex++, TEXT("标题"), LVCFMT_LEFT, 150);
    m_ViewList.InsertColumn(colIndex++, TEXT("类名"), LVCFMT_LEFT, 250);
    m_ViewList.InsertColumn(colIndex++, TEXT("进程id"), LVCFMT_LEFT, 70);
    m_ViewList.InsertColumn(colIndex++, TEXT("线程id"), LVCFMT_LEFT, 70);
    m_ViewList.InsertColumn(colIndex++, TEXT("隐藏进程"), LVCFMT_LEFT, 70);
    m_ViewList.InsertColumn(colIndex++, TEXT("进程名"), LVCFMT_LEFT, 200);
    m_ViewList.DeleteAllItems();
    int rowNum = 0;
    for (auto& window : windows.data)
    {
        colIndex = 0;
        CString id;
        CString temp;
        id.Format(_T("%d"), rowNum + 1);
        m_ViewList.InsertItem(rowNum, _T(""));
        m_ViewList.SetItemText(rowNum, colIndex++, id);
        temp.Format(_T("%d"), window.hwnd);
        m_ViewList.SetItemText(rowNum, colIndex++, temp);
        m_ViewList.SetItemText(rowNum, colIndex++, window.caption.c_str());
        m_ViewList.SetItemText(rowNum, colIndex++, window.class_name.c_str());
        temp.Format(_T("%d"), window.pid);
        m_ViewList.SetItemText(rowNum, colIndex++, temp);
        temp.Format(_T("%d"), window.tid);
        m_ViewList.SetItemText(rowNum, colIndex++, temp);
        m_ViewList.SetItemText(rowNum, colIndex++, window.is_hide_process ? _T("1") : _T("0"));
        m_ViewList.SetItemText(rowNum, colIndex++, window.process_name.c_str());
        rowNum++;
    }
    m_ViewList.SetRedraw(TRUE);
}

void CWindowView::OnInitialUpdate()
{
    CView::OnInitialUpdate();
    FillViewList();
}


void CWindowView::OnSize(UINT nType, int cx, int cy)
{
    CView::OnSize(nType, cx, cy);
    AdjustLayout();
}


void CWindowView::AdjustLayout()
{
    if (GetSafeHwnd() == nullptr)
    {
        return;
    }

    CRect rectClient;
    GetClientRect(rectClient);

    m_ViewList.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}

void CWindowView::OnListItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    if (pNMListView->uChanged == LVIF_STATE)
    {
        if (pNMListView->uNewState & LVIS_SELECTED)
        {
            //不做处理
        }
    }
}

void CWindowView::OnWindowTitleBan()
{
    auto selectedRow = (int)m_ViewList.GetFirstSelectedItemPosition() - 1;
    std::wstring window_title;
    if (selectedRow != -1)
    {
        window_title = m_ViewList.GetItemText(selectedRow, 2);
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
        unsigned int uiLastPolicyId = 0;
        for (auto[uiPolicyId, Policy] : Policies)
        {
            uiLastPolicyId = uiPolicyId;
        }
        uiLastPolicyId++;
        ProtocolPolicy Policy;
        Policy.policy_id = uiLastPolicyId;
        Policy.punish_type = ENM_PUNISH_TYPE_NO_OPEARATION;
        Policy.policy_type = ENM_POLICY_TYPE_WINDOW_NAME;
        Policy.config = window_title;
        Policies[uiLastPolicyId] = Policy;
        file.close();
        std::ofstream output(theApp.m_cCfgPath, std::ios::out | std::ios::binary);
        str = Cfg->dump();
        output.write(str.data(), str.size());
        output.close();
        theApp.OnServiceSettings();
    }
}


void CWindowView::OnWindowClassNameBan()
{
    auto selectedRow = (int)m_ViewList.GetFirstSelectedItemPosition() - 1;
    std::wstring window_classname;
    if (selectedRow != -1)
    {
        window_classname = m_ViewList.GetItemText(selectedRow, 3);
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
        unsigned int uiLastPolicyId = 0;
        for (auto[uiPolicyId, Policy] : Policies)
        {
            uiLastPolicyId = uiPolicyId;
        }
        uiLastPolicyId++;
        ProtocolPolicy Policy;
        Policy.policy_id = uiLastPolicyId;
        Policy.punish_type = ENM_PUNISH_TYPE_NO_OPEARATION;
        Policy.policy_type = ENM_POLICY_TYPE_WINDOW_NAME;
        Policy.config = window_classname;
        Policies[uiLastPolicyId] = Policy;
        file.close();
        std::ofstream output(theApp.m_cCfgPath, std::ios::out | std::ios::binary);
        str = Cfg->dump();
        output.write(str.data(), str.size());
        output.close();
        theApp.OnServiceSettings();
    }
}


void CWindowView::OnWindowBothBan()
{
    auto selectedRow = (int)m_ViewList.GetFirstSelectedItemPosition() - 1;
    std::wstring window_both;
    if (selectedRow != -1)
    {
        window_both = m_ViewList.GetItemText(selectedRow, 2);
        window_both += L"|";
        window_both += m_ViewList.GetItemText(selectedRow, 3);
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
        unsigned int uiLastPolicyId = 0;
        for (auto[uiPolicyId, Policy] : Policies)
        {
            uiLastPolicyId = uiPolicyId;
        }
        uiLastPolicyId++;
        ProtocolPolicy Policy;
        Policy.policy_id = uiLastPolicyId;
        Policy.punish_type = ENM_PUNISH_TYPE_NO_OPEARATION;
        Policy.policy_type = ENM_POLICY_TYPE_WINDOW_NAME;
        Policy.config = window_both;
        Policies[uiLastPolicyId] = Policy;
        file.close();
        std::ofstream output(theApp.m_cCfgPath, std::ios::out | std::ios::binary);
        str = Cfg->dump();
        output.write(str.data(), str.size());
        output.close();
        theApp.OnServiceSettings();
    }
}
