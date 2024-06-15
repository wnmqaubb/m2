#pragma once

/////////////////////////////////////////////////////////////////////////////
#include "PropEditFormView.h"

// COutputList 窗口
class CConfigSettingSubViewWnd : public CDockablePane
{
    // 构造
public:
    CConfigSettingSubViewWnd() noexcept;

    void UpdateFonts(); 
    void InitPropsWindowView();
    void FillProp(CDocument* pDoc, ProtocolPolicy& Policy);
    CDocument* GetDocument() { return m_pDoc; }
    // 特性
protected:
    CMFCTabCtrl	m_wndTabs;
    CPropEditFormView* m_PropEditForm;
    CDocument* m_pDoc = nullptr;
protected:
    
    void AdjustHorzScroll(CListBox& wndListBox);

    // 实现
public:
    virtual ~CConfigSettingSubViewWnd();
protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);

    DECLARE_MESSAGE_MAP()
};

