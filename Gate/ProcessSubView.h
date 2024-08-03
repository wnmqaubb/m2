#pragma once

/////////////////////////////////////////////////////////////////////////////

#include "ViewList.h"

// COutputList ����
class CProcessSubViewWnd : public CDockablePane
{
    // ����
public:
    CProcessSubViewWnd() noexcept;

    void UpdateFonts(); 
    void InitModuleWindowView();
    void InitThreadWindowView();
    void InitDirectoryWindowView();
    void FillModuleWindow(const std::vector<ProtocolModuleInfo>& module);
    void FillThreadWindow(const std::map<uint32_t, ProtocolThreadInfo>& threads);
    void FillDirectoryWindow(const std::vector<ProtocolDirectoryInfo>& directories);
    // ����
protected:
    CMFCTabCtrl	m_wndTabs;
    CViewList m_wndModuleInfo;
    CViewList m_wndThreadInfo;
    CViewList m_wndDirectoryInfo;

protected:

    void AdjustHorzScroll(CListBox& wndListBox);

    // ʵ��
public:
    virtual ~CProcessSubViewWnd();
protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);

    DECLARE_MESSAGE_MAP()
};

