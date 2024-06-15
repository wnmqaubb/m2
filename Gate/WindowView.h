<<<<<<< HEAD
﻿
// GateView.h: CProcessView 类的接口
//

#pragma once

#include "ViewList.h"

class CWindowView : public CView
{
protected: // 仅从序列化创建
	CWindowView() noexcept;
	DECLARE_DYNCREATE(CWindowView)

// 特性
public:
	CWindowDoc* GetDocument() const;
    CChildFrame* CWindowView::GetParentCChild() const;
// 操作
public:

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// 实现
public:
	virtual ~CWindowView();
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
	
    afx_msg void OnWindowTitleBan();
    afx_msg void OnWindowClassNameBan();
    afx_msg void OnWindowBothBan();
};

#ifndef _DEBUG  // GateView.cpp 中的调试版本
inline CWindowDoc* CWindowView::GetDocument() const
   { return reinterpret_cast<CWindowDoc*>(m_pDocument); }
#endif

=======
﻿
// GateView.h: CProcessView 类的接口
//

#pragma once

#include "ViewList.h"

class CWindowView : public CView
{
protected: // 仅从序列化创建
	CWindowView() noexcept;
	DECLARE_DYNCREATE(CWindowView)

// 特性
public:
	CWindowDoc* GetDocument() const;
    CChildFrame* CWindowView::GetParentCChild() const;
// 操作
public:

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// 实现
public:
	virtual ~CWindowView();
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
	
    afx_msg void OnWindowTitleBan();
    afx_msg void OnWindowClassNameBan();
    afx_msg void OnWindowBothBan();
};

#ifndef _DEBUG  // GateView.cpp 中的调试版本
inline CWindowDoc* CWindowView::GetDocument() const
   { return reinterpret_cast<CWindowDoc*>(m_pDocument); }
#endif

>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9
