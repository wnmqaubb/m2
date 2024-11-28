
// GateView.h: CProcessView 类的接口
//

#pragma once

#include "ViewList.h"
#include "ConfigSettingChildFrm.h"


class CConfigSettingView : public CView
{
protected: // 仅从序列化创建
	CConfigSettingView() noexcept;
	DECLARE_DYNCREATE(CConfigSettingView)

	// 特性
public:
	CConfigSettingDoc* GetDocument() const;
	CConfigSettingChildFrame* GetParentCChild() const;
	// 操作
public:

	// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	// 实现
public:
	virtual ~CConfigSettingView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
public:
	CViewList m_ViewList;
	void RefreshViewList();
protected:
	void AdjustLayout();
	void InitConfigSettingView();
	// 生成的消息映射函数
protected:
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnListItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnConfigAdd();
	afx_msg void OnConfigDel();
	afx_msg void OnConfigSave();

public:
	afx_msg void ScrollToAddByPolicyId(int policy_id);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	virtual void OnInitialUpdate();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // GateView.cpp 中的调试版本
inline CConfigSettingDoc* CConfigSettingView::GetDocument() const
{
	return reinterpret_cast<CConfigSettingDoc*>(m_pDocument);
}
#endif

