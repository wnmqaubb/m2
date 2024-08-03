// SearchBar.cpp: 实现文件
//

#include "pch.h"
#include "Gate.h"
#include "SearchBar.h"


// SearchBar

IMPLEMENT_DYNCREATE(CSearchBar, CFormView)

CSearchBar::CSearchBar()
	: CFormView(IDD_SEARCHBAR)
{

}

CSearchBar::~CSearchBar()
{
}

void CSearchBar::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BOOL CSearchBar::OnCommand(WPARAM wParam, LPARAM lParam)
{
    GetParent()->SendMessage(WM_COMMAND, wParam, lParam);
    return CFormView::OnCommand(wParam, lParam);
}

BEGIN_MESSAGE_MAP(CSearchBar, CFormView)
END_MESSAGE_MAP()


// SearchBar 诊断

#ifdef _DEBUG
void CSearchBar::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CSearchBar::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// SearchBar 消息处理程序
