
// GateView.h: CProcessView 类的接口
//

#pragma once

#include "ViewList.h"

class CProcessView : public CView
{
protected: // 仅从序列化创建
	CProcessView() noexcept;
	DECLARE_DYNCREATE(CProcessView)

// 特性
public:
	CProcessDoc* GetDocument() const;
    CProcessChildFrame* GetParentCChild() const;
// 操作
public:

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// 实现
public:
	virtual ~CProcessView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CViewList m_ViewList;
	void FillViewList();
	void AdjustLayout();

    // 生成的消息映射函数
protected:
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnListItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	
	virtual void OnInitialUpdate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	
    afx_msg void OnProcessNameBan();
	afx_msg void OnProcessNameAndSizeBan();
	void ScrollToAddByPolicyId(int policy_id);
	afx_msg void OnProcessPathBan();
};

#ifndef _DEBUG  // GateView.cpp 中的调试版本
inline CProcessDoc* CProcessView::GetDocument() const
   { return reinterpret_cast<CProcessDoc*>(m_pDocument); }
#endif

