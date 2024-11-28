
// GateView.h: CProcessView ��Ľӿ�
//

#pragma once

#include "ViewList.h"
#include "ConfigSettingChildFrm.h"


class CConfigSettingView : public CView
{
protected: // �������л�����
	CConfigSettingView() noexcept;
	DECLARE_DYNCREATE(CConfigSettingView)

	// ����
public:
	CConfigSettingDoc* GetDocument() const;
	CConfigSettingChildFrame* GetParentCChild() const;
	// ����
public:

	// ��д
public:
	virtual void OnDraw(CDC* pDC);  // ��д�Ի��Ƹ���ͼ
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	// ʵ��
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
	// ���ɵ���Ϣӳ�亯��
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

#ifndef _DEBUG  // GateView.cpp �еĵ��԰汾
inline CConfigSettingDoc* CConfigSettingView::GetDocument() const
{
	return reinterpret_cast<CConfigSettingDoc*>(m_pDocument);
}
#endif

