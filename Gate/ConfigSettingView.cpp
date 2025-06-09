
// GateView.cpp: CProcessView ���ʵ��
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

// CProcessView ����/����

CConfigSettingView::CConfigSettingView() noexcept
{
    
}

CConfigSettingView::~CConfigSettingView()
{
}

BOOL CConfigSettingView::PreCreateWindow(CREATESTRUCT& cs)
{
    // TODO: �ڴ˴�ͨ���޸�
    //  CREATESTRUCT cs ���޸Ĵ��������ʽ

    return CView::PreCreateWindow(cs);
}

// CProcessView ��ͼ

void CConfigSettingView::OnDraw(CDC* pDC)
{
    CView::OnDraw(pDC);
    // TODO: �ڴ˴�Ϊ����������ӻ��ƴ���
}

void CConfigSettingView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
    ClientToScreen(&point);
    OnContextMenu(this, point);
}

void CConfigSettingView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
    CMenu menu;
	menu.LoadMenu(IDR_SETING_RIGHT_MENU);
	CMenu* pSumMenu = menu.GetSubMenu(0);
    theApp.GetContextMenuManager()->ShowPopupMenu(*pSumMenu, point.x, point.y, this, TRUE);
}


// CProcessView ���
#ifdef _DEBUG
void CConfigSettingView::AssertValid() const
{
    CView::AssertValid();
}

void CConfigSettingView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}

CConfigSettingDoc* CConfigSettingView::GetDocument() const // �ǵ��԰汾��������
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
// CProcessView ��Ϣ�������


int CConfigSettingView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CView::OnCreate(lpCreateStruct) == -1)
        return -1;



    CRect rectDummy;
    GetClientRect(&rectDummy);

    // ������ͼ: 
    const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | LVS_SHOWSELALWAYS;

    if (!m_ViewList.Create(dwViewStyle, rectDummy, this, ID_CFG_ADD))
    {
        TRACE0("δ�ܴ����б���ͼ\n");
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
    m_ViewList.InsertColumn(colIndex++, TEXT("���"), LVCFMT_LEFT, 38);
    m_ViewList.InsertColumn(colIndex++, TEXT("����ID"), LVCFMT_LEFT, 70);
    m_ViewList.InsertColumn(colIndex++, TEXT("��������"), LVCFMT_LEFT, 80);
    m_ViewList.InsertColumn(colIndex++, TEXT("��������"), LVCFMT_LEFT, 110);
    m_ViewList.InsertColumn(colIndex++, TEXT("����"), LVCFMT_LEFT, 470);
    m_ViewList.InsertColumn(colIndex++, TEXT("��ע"), LVCFMT_LEFT, 150);
    m_ViewList.InsertColumn(colIndex++, TEXT("�Ƿ����Ա����"), LVCFMT_LEFT, 0);
    m_ViewList.SetRedraw(TRUE);
}

void CConfigSettingView::RefreshViewList()
{
    // 1. ���浱ǰ״̬
    int nSelectedItem = m_ViewList.GetNextItem(-1, LVNI_SELECTED);
    int nTopIndex = m_ViewList.GetTopIndex();
    int nSelectedID = -1;

    // ��ȡ��ǰѡ�����ΨһID
    CPoint scrollPos;
    SCROLLINFO si;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_POS;
    if (m_ViewList.GetScrollInfo(SB_VERT, &si))
    {
        scrollPos.y = si.nPos;
    }

    if (nSelectedItem != -1)
    {
        nSelectedID = (int)m_ViewList.GetItemData(nSelectedItem);
    }

    // 3. ˢ���б�
    m_ViewList.SetRedraw(FALSE); // ��ֹ�ػ��������
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
        m_ViewList.SetItemData(rowNum, uiPolicyId);
        rowNum++;
    }
    // 4. �ָ�ѡ��״̬
    int nNewIndex = -1;
    int nItemCount = m_ViewList.GetItemCount();

    if (nSelectedID != -1)
    {
        // ����ԭѡ�����Ƿ񻹴���
        for (int i = 0; i < nItemCount; i++)
        {
            if ((int)m_ViewList.GetItemData(i) == nSelectedID)
            {
                nNewIndex = i;
                break;
            }
        }
    }

    // ���ԭѡ������ڣ�ѡ����һ��
    if (nNewIndex == -1 && nSelectedItem != -1 && nItemCount > 0)
    {
        // ԭѡ���ɾ��������ѡ����һ��
        nNewIndex = std::min(nSelectedItem, nItemCount - 1);
    }

    // 5. ����ѡ��״̬�͹���λ��
    if (nNewIndex != -1)
    {
        // ����ѡ��״̬
        m_ViewList.SetItemState(nNewIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

        // ȷ��ѡ����ɼ�
        m_ViewList.EnsureVisible(nNewIndex, FALSE);
        m_ViewList.SetFocus();

        // �ָ�����λ��
        if (nTopIndex > 0)
        {
            // �����µĶ�������
            int nNewTopIndex = std::min(nTopIndex, std::max(0, m_ViewList.GetItemCount() - m_ViewList.GetCountPerPage()));

            // ʹ��Scroll�����ָ�λ��
            CRect itemRect;
            m_ViewList.GetItemRect(nNewTopIndex, &itemRect, LVIR_BOUNDS);
            m_ViewList.Scroll(CSize(0, itemRect.top + 1 - scrollPos.y));
        }
    }

    m_ViewList.SetRedraw(TRUE); // �����ػ�
    m_ViewList.RedrawWindow();  // ǿ���ػ�
}

void CConfigSettingView::OnSelectItem(int policy_id) {
    for (int i = 0; i < m_ViewList.GetItemCount(); i++)
    {
        CString cstrPolicyId = m_ViewList.GetItemText(i, 1);
        uint32_t uiPolicyId = _ttoi(cstrPolicyId);
        if (uiPolicyId == policy_id) {
            m_ViewList.SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
            m_ViewList.EnsureVisible(i, FALSE);
            m_ViewList.SetFocus();
            return;
        }
    }
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
    // TODO: �ڴ˴������Ϣ����������
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

void CConfigSettingView::ScrollToAddByPolicyId(int policy_id)
{
    for (int i = 0; i < m_ViewList.GetItemCount(); i++)
    {
        CString cstrPolicyId = m_ViewList.GetItemText(i, 1);
        uint32_t uiPolicyId = atoi(CT2A(cstrPolicyId.GetBuffer()));
        if (uiPolicyId == policy_id)
        {   
            m_ViewList.SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
            m_ViewList.EnsureVisible(i, FALSE);
            return;
        }
    }
}