﻿#pragma once

/////////////////////////////////////////////////////////////////////////////

#include "ViewList.h"

// COutputList 窗口
class CProcessSubViewWnd : public CDockablePane
{
    // 构造
public:
    CProcessSubViewWnd() noexcept;
    virtual ~CProcessSubViewWnd();
    void UpdateFonts(); 
    void OnCopyValue();
    void OnCopyModulePath();
    void OnContextMenu(CWnd* pWnd, CPoint point);
    void InitModuleWindowView();
    void InitThreadWindowView();
    void InitDirectoryWindowView();
    void FillModuleWindow(const std::vector<ProtocolModuleInfo>& module);
    void FillThreadWindow(const std::map<uint32_t, ProtocolThreadInfo>& threads);
    void FillDirectoryWindow(const std::vector<ProtocolDirectoryInfo>& directories);
    // 特性
protected:
    CMFCTabCtrl	m_wndTabs;
    CViewList m_wndModuleInfo;
    CViewList m_wndThreadInfo;
    CViewList m_wndDirectoryInfo;

protected:

    void AdjustHorzScroll(CListBox& wndListBox);

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);

    DECLARE_MESSAGE_MAP()
};

