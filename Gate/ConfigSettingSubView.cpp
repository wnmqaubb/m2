#include "pch.h"
#include "framework.h"
#include "Gate.h"
#include "ConfigSettingSubView.h"
#include "Resource.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COutputBar

CConfigSettingSubViewWnd::CConfigSettingSubViewWnd() noexcept
{
    m_PropEditForm = (CPropEditFormView*)(RUNTIME_CLASS(CPropEditFormView)->CreateObject());
}

CConfigSettingSubViewWnd::~CConfigSettingSubViewWnd()
{
}

BEGIN_MESSAGE_MAP(CConfigSettingSubViewWnd, CDockablePane)
    ON_WM_CREATE()
    ON_WM_SIZE()
END_MESSAGE_MAP()

int CConfigSettingSubViewWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDockablePane::OnCreate(lpCreateStruct) == -1)
        return -1;

    CRect rectDummy;
    rectDummy.SetRectEmpty();

    // 确保 RichEdit 已初始化
    if (!AfxInitRichEdit2()) {
        AfxMessageBox(L"RichEdit 初始化失败");
        return -1;
    }    

    // 创建输出窗格: 
    const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE;

    m_PropEditForm->Create(NULL, NULL, dwViewStyle, rectDummy, this, NULL, NULL);

    UpdateFonts();

    return 0;
}

void CConfigSettingSubViewWnd::OnSize(UINT nType, int cx, int cy)
{
    CDockablePane::OnSize(nType, cx, cy);
    if (GetSafeHwnd() == nullptr) {
        return;
    }

    CRect rectClient;
    GetClientRect(rectClient);

    // 确保窗口已创建
    if (m_PropEditForm->GetSafeHwnd()) {
        m_PropEditForm->SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
    }
}

void CConfigSettingSubViewWnd::FillProp(CDocument* pDoc, ProtocolPolicy& Policy)
{
    m_pDoc = pDoc;
    CString temp;
    temp.Format(TEXT("%d"), Policy.policy_id);
#ifndef GATE_ADMIN
    m_PropEditForm->m_PolicyIdEdit.SetReadOnly(Policy.create_by_admin);
#endif
    m_PropEditForm->m_PolicyIdEdit.SetWindowText(temp);
    m_PropEditForm->m_PolicyTypeComboBox.SetCurSel(Policy.policy_type);
    m_PropEditForm->m_PunishTypeComboBox.SetCurSel(Policy.punish_type);
    m_PropEditForm->m_PolicyConfigEdit.SetWindowText(Policy.config.c_str());
    m_PropEditForm->m_PolicyCommentEdit.SetWindowText(Policy.comment.c_str());
    m_PropEditForm->m_CreateByAdmin = Policy.create_by_admin;
}

void CConfigSettingSubViewWnd::AdjustHorzScroll(CListBox& wndListBox)
{
    CClientDC dc(this);
    CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontRegular);

    int cxExtentMax = 0;

    for (int i = 0; i < wndListBox.GetCount(); i++)
    {
        CString strItem;
        wndListBox.GetText(i, strItem);

        cxExtentMax = std::max(cxExtentMax, (int)dc.GetTextExtent(strItem).cx);
    }

    wndListBox.SetHorizontalExtent(cxExtentMax);
    dc.SelectObject(pOldFont);
}

void CConfigSettingSubViewWnd::UpdateFonts()
{/*
    m_wndProps->SetFont(&afxGlobalData.fontRegular);*/
}