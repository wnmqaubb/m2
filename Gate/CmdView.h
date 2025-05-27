#pragma once

#include "OutputDlg.h"

class CCmdView : public CView
{
protected: // 仅从序列化创建
    CCmdView() noexcept;
    DECLARE_DYNCREATE(CCmdView)

    // 特性
public:
    CBaseDoc* GetDocument() const;
    CChildFrame* CCmdView::GetParentCChild() const;
    // 操作
public:

    // 重写
public:
    virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

    // 实现
public:
    virtual ~CCmdView();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    COutputList m_OutputList;
    CEdit m_CmdEdit;
    void AdjustLayout();

    // 生成的消息映射函数
protected:
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    void OnDelayedPaste();
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    DECLARE_MESSAGE_MAP()
public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnCmdEditReturn();
    virtual void OnInitialUpdate();
    void OnPaste();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    void Echo(LPCTSTR text);
};


#ifndef _DEBUG  // GateView.cpp 中的调试版本
inline CBaseDoc* CCmdView::GetDocument() const
{
    return reinterpret_cast<CBaseDoc*>(m_pDocument);
}
#endif

