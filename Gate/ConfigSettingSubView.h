#pragma once

/////////////////////////////////////////////////////////////////////////////
#include "PropEditFormView.h"

// COutputList ����
class CConfigSettingSubViewWnd : public CDockablePane
{
    // ����
public:
    CConfigSettingSubViewWnd() noexcept;

    void UpdateFonts(); 
    void InitPropsWindowView();
    void FillProp(CDocument* pDoc, ProtocolPolicy& Policy);
    CDocument* GetDocument() { return m_pDoc; }
    // ����
protected:
    CMFCTabCtrl	m_wndTabs;
    CPropEditFormView* m_PropEditForm;
    CDocument* m_pDoc = nullptr;
protected:
    
    void AdjustHorzScroll(CListBox& wndListBox);

    // ʵ��
public:
    virtual ~CConfigSettingSubViewWnd();
protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);

    DECLARE_MESSAGE_MAP()
};

