// CMainBar.cpp: 实现文件
//

#include "pch.h"
#include "Gate.h"
#include "MainBar.h"


// CMainBar 对话框

IMPLEMENT_DYNCREATE(CMainBar, CFormView)

CMainBar::CMainBar()
	: CFormView(IDD_MAINBAR)
{

}

CMainBar::~CMainBar()
{
}

BOOL CMainBar::OnCommand(WPARAM wParam, LPARAM lParam)
{
    GetParent()->SendMessage(WM_COMMAND, wParam, lParam);
    return CFormView::OnCommand(wParam, lParam);
}

void CMainBar::OnInitialUpdate()
{
#ifndef GATE_ADMIN     
     //GetDlgItem(IDC_SCREENSHOT_BUTTON)->ShowWindow(SW_HIDE);
     //GetDlgItem(IDC_PROCESS_BUTTON)->ShowWindow(SW_HIDE);
#else
     GetDlgItem(IDC_REFRESH_LICENSE_BUTTON)->ShowWindow(SW_SHOW);
     GetDlgItem(IDC_BUTTON_SYNC_LICENSE)->ShowWindow(SW_SHOW);
     GetDlgItem(IDC_EXPDATE_STATIC)->ShowWindow(SW_HIDE);
     GetDlgItem(IDC_EXPDATE_TEXT_STATIC)->ShowWindow(SW_HIDE);
#endif
}

HBRUSH CMainBar::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = __super::OnCtlColor(pDC, pWnd, nCtlColor);
    if (IDC_EXPDATE_STATIC == pWnd->GetDlgCtrlID())
    {
        pDC->SetTextColor(RGB(0x85, 0x2C, 0x97));
    }
    return hbr;
}

void CMainBar::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMainBar, CFormView)
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


#ifdef _DEBUG
void CMainBar::AssertValid() const
{
    CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CMainBar::Dump(CDumpContext& dc) const
{
    CFormView::Dump(dc);
}
#endif
#endif //_DEBUG

// CMainBar 消息处理程序
