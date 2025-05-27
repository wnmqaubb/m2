#pragma once

#include "OutputDlg.h"

class CCmdView : public CView
{
protected: // �������л�����
    CCmdView() noexcept;
    DECLARE_DYNCREATE(CCmdView)

    // ����
public:
    CBaseDoc* GetDocument() const;
    CChildFrame* CCmdView::GetParentCChild() const;
    // ����
public:

    // ��д
public:
    virtual void OnDraw(CDC* pDC);  // ��д�Ի��Ƹ���ͼ
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

    // ʵ��
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

    // ���ɵ���Ϣӳ�亯��
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


#ifndef _DEBUG  // GateView.cpp �еĵ��԰汾
inline CBaseDoc* CCmdView::GetDocument() const
{
    return reinterpret_cast<CBaseDoc*>(m_pDocument);
}
#endif

