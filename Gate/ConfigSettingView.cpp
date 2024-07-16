
// GateView.cpp: CProcessView 类的实现
//

#include "pch.h"
#include "framework.h"
#include "Gate.h"

#include "ConfigSettingChildFrm.h"
#include "ConfigSettingSubView.h"
#include "ConfigSettingDoc.h"
#include "ConfigSettingView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CProcessView

IMPLEMENT_DYNCREATE(CConfigSettingView, CView)

BEGIN_MESSAGE_MAP(CConfigSettingView, CView)
    ON_WM_CONTEXTMENU()
    ON_WM_RBUTTONUP()
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_COMMAND(ID_CFG_ADD, &CConfigSettingView::OnConfigAdd)
    ON_COMMAND(ID_CFG_DEL, &CConfigSettingView::OnConfigDel)
    ON_COMMAND(ID_CFG_SAVE, &CConfigSettingView::OnConfigSave)
    ON_NOTIFY(LVN_ITEMCHANGED, ID_CFG_ADD, &CConfigSettingView::OnListItemChanged)
END_MESSAGE_MAP()

// CProcessView 构造/析构

CConfigSettingView::CConfigSettingView() noexcept
{
    
}

CConfigSettingView::~CConfigSettingView()
{
}

BOOL CConfigSettingView::PreCreateWindow(CREATESTRUCT& cs)
{
    // TODO: 在此处通过修改
    //  CREATESTRUCT cs 来修改窗口类或样式

    return CView::PreCreateWindow(cs);
}

// CProcessView 绘图

void CConfigSettingView::OnDraw(CDC* pDC)
{
    CView::OnDraw(pDC);
    // TODO: 在此处为本机数据添加绘制代码
}

void CConfigSettingView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
    ClientToScreen(&point);
    OnContextMenu(this, point);
}

void CConfigSettingView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
    CMenu menu;
    menu.LoadMenu(IDR_MAINFRAME);
    CMenu* pSumMenu = menu.GetSubMenu(4);
    theApp.GetContextMenuManager()->ShowPopupMenu(*pSumMenu, point.x, point.y, this, TRUE);
}


// CProcessView 诊断
#ifdef _DEBUG
void CConfigSettingView::AssertValid() const
{
    CView::AssertValid();
}

void CConfigSettingView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}

CConfigSettingDoc* CConfigSettingView::GetDocument() const // 非调试版本是内联的
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CConfigSettingDoc)));
    return (CConfigSettingDoc*)m_pDocument;
}

#endif //_DEBUG

CConfigSettingChildFrame* CConfigSettingView::GetParentCChild() const
{
    ASSERT(GetParent()->IsKindOf(RUNTIME_CLASS(CConfigSettingChildFrame)));
    return (CConfigSettingChildFrame*)GetParent();
}
// CProcessView 消息处理程序


int CConfigSettingView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CView::OnCreate(lpCreateStruct) == -1)
        return -1;



    CRect rectDummy;
    GetClientRect(&rectDummy);

    // 创建视图: 
    const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    if (!m_ViewList.Create(dwViewStyle, rectDummy, this, ID_CFG_ADD))
    {
        TRACE0("未能创建列表视图\n");
    }

    InitConfigSettingView();
    return 0;
}

void CConfigSettingView::InitConfigSettingView()
{
    m_ViewList.SetColumnByIntSort({ 0, 1, 2 });
    m_ViewList.SetRedraw(FALSE);
    m_ViewList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_GRIDLINES);
    int colIndex = 0;
    m_ViewList.InsertColumn(colIndex++, TEXT("序号"), LVCFMT_LEFT, 38);
    m_ViewList.InsertColumn(colIndex++, TEXT("策略ID"), LVCFMT_LEFT, 50);
    m_ViewList.InsertColumn(colIndex++, TEXT("策略类型"), LVCFMT_LEFT, 80);
    m_ViewList.InsertColumn(colIndex++, TEXT("处理类型"), LVCFMT_LEFT, 110);
    m_ViewList.InsertColumn(colIndex++, TEXT("配置"), LVCFMT_LEFT, 470);
    m_ViewList.InsertColumn(colIndex++, TEXT("备注"), LVCFMT_LEFT, 150);
    m_ViewList.InsertColumn(colIndex++, TEXT("是否管理员创建"), LVCFMT_LEFT, 0);
    m_ViewList.SetRedraw(TRUE);
}

void CConfigSettingView::RefreshViewList()
{
    m_ViewList.SetRedraw(FALSE);
    auto& m_Policys = GetDocument()->GetPolicy();
    int colIndex = 0;
    m_ViewList.DeleteAllItems();
    CString connect_id_str, seq;
    const wchar_t* format_d = L"%d";
    const wchar_t* format_3d = L"%03d";
    const wchar_t* empty_wstr = L"";
    int rowNum = 0;
    for (auto[uiPolicyId, Policy] : m_Policys.policies)
    {
#ifndef GATE_ADMIN
        if (688000 >= uiPolicyId || uiPolicyId > 689050
            || Policy.policy_type == ENM_POLICY_TYPE_SCRIPT 
            || Policy.policy_type == ENM_POLICY_TYPE_THREAD_START)
        {
            continue;
        }
#endif
        if (rowNum % 100 == 0) Sleep(1);
        colIndex = 0;
        CString id;
        CString temp;
        id.Format(format_d, rowNum + 1);
        m_ViewList.InsertItem(rowNum, empty_wstr);
        m_ViewList.SetItemText(rowNum, colIndex++, id);
        temp.Format(format_d, uiPolicyId);
        m_ViewList.SetItemText(rowNum, colIndex++, temp);
        m_ViewList.SetItemText(rowNum, colIndex++, ConvertToString((PolicyType)Policy.policy_type));
        m_ViewList.SetItemText(rowNum, colIndex++, ConvertToString((PunishType)Policy.punish_type));
        m_ViewList.SetItemText(rowNum, colIndex++, Policy.config.c_str());
        m_ViewList.SetItemText(rowNum, colIndex++, Policy.comment.c_str());
        temp.Format(format_d, Policy.create_by_admin);
        m_ViewList.SetItemText(rowNum, colIndex++, temp);
        rowNum++;
    }
    m_ViewList.SetRedraw(TRUE);
}

void CConfigSettingView::OnInitialUpdate()
{
    CView::OnInitialUpdate();
    GetDocument()->SetView(this);
    RefreshViewList();
}


void CConfigSettingView::OnSize(UINT nType, int cx, int cy)
{
    CView::OnSize(nType, cx, cy);
    AdjustLayout();
    // TODO: 在此处添加消息处理程序代码
}


void CConfigSettingView::AdjustLayout()
{
    if (GetSafeHwnd() == nullptr)
    {
        return;
    }

    CRect rectClient;
    GetClientRect(rectClient);

    m_ViewList.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}

void CConfigSettingView::OnListItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    if (pNMListView->uChanged == LVIF_STATE)
    {
        if (pNMListView->uNewState & LVIS_SELECTED)
        {
            int nItem = pNMListView->iItem;
            CString cstrPolicyId = m_ViewList.GetItemText(nItem, 1);
            CString policy_type_str = m_ViewList.GetItemText(nItem, 2);
            uint32_t uiPolicyId = atoi(CT2A(cstrPolicyId.GetBuffer()));
            PolicyType policy_type = ConvertToPolicyType(policy_type_str);
            if (policy_type == ENM_POLICY_TYPE_BACK_GAME ||
                policy_type == ENM_POLICY_TYPE_EXIT_GAME ||
                policy_type == ENM_POLICY_TYPE_ACTION_SPEED_WALK ||
                policy_type == ENM_POLICY_TYPE_ACTION_SPEED_HIT ||
                policy_type == ENM_POLICY_TYPE_ACTION_SPEED_SPELL
                )
            {
                return;
            }
            GetParentCChild()->m_wndSubViewWnd.FillProp(GetDocument(), GetDocument()->GetPolicy().policies.find(uiPolicyId)->second);
        }
    }
}

void CConfigSettingView::OnConfigAdd()
{
    auto& Policies = GetDocument()->GetPolicy().policies;
    unsigned int uiLastPolicyId = 0;
#ifdef GATE_ADMIN
    uiLastPolicyId = 689000;
#else
    uiLastPolicyId = 688000;
#endif
    for (auto [uiPolicyId, Policy] : Policies)
    {
#ifdef GATE_ADMIN
        if (uiPolicyId > 689000)
        {
            uiLastPolicyId = uiPolicyId;
        }
#else
        if (688000 < uiPolicyId && uiPolicyId < 689001)
        {
            uiLastPolicyId = uiPolicyId;
        }
#endif
    }
    uiLastPolicyId++;
    ProtocolPolicy NewPolicy;
#ifdef GATE_ADMIN
    NewPolicy.create_by_admin = true;
#endif
    NewPolicy.policy_id = uiLastPolicyId;
    Policies[uiLastPolicyId] = NewPolicy;
    RefreshViewList();
    m_ViewList.SetItemState(m_ViewList.GetItemCount() - 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    m_ViewList.EnsureVisible(m_ViewList.GetItemCount() - 1, FALSE);
    
}

void CConfigSettingView::OnConfigDel()
{
    auto selectedRow = (int)m_ViewList.GetFirstSelectedItemPosition() - 1;
    if (selectedRow != -1)
    {
        CString cstrPolicyId = m_ViewList.GetItemText(selectedRow, 1);
        uint32_t uiPolicyId = atoi(CT2A(cstrPolicyId.GetBuffer()));
        GetDocument()->GetPolicy().policies.erase(uiPolicyId);
        RefreshViewList();
    }
}

void CConfigSettingView::OnConfigSave()
{
    GetDocument()->DoFileSave();
}