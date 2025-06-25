#pragma once

/////////////////////////////////////////////////////////////////////////////
#include "PropEditFormView.h"

// COutputList 窗口
class CConfigSettingSubViewWnd : public CDockablePane
{
    // 构造
public:
    CConfigSettingSubViewWnd() noexcept;
    virtual ~CConfigSettingSubViewWnd();

    void UpdateFonts(); 
    void FillProp(CDocument* pDoc, ProtocolPolicy& Policy);
    CDocument* GetDocument() { return m_pDoc; }
    // 特性
protected:
    CPropEditFormView m_PropEditForm;
    CDocument* m_pDoc = nullptr;    
    void AdjustHorzScroll(CListBox& wndListBox);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);

    DECLARE_MESSAGE_MAP()
};

