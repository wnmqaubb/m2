// OutputDlg.cpp: 实现文件
//

#include "pch.h"
#include "Gate.h"
#include "OutputDlg.h"
#include "afxdialogex.h"


// OutputDlg 对话框

IMPLEMENT_DYNAMIC(COutputDlg, CDialogEx)

COutputDlg::COutputDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_OUTPUT_DIALOG, pParent)
{
#ifndef _WIN32_WCE
	EnableActiveAccessibility();
#endif

}

COutputDlg::~COutputDlg()
{
}

void COutputDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(COutputDlg, CDialogEx)
    ON_WM_CREATE()
    ON_WM_SIZE()
END_MESSAGE_MAP()


// OutputDlg 消息处理程序


BOOL COutputDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // 异常: OCX 属性页应返回 FALSE
}

void COutputDlg::AdjustHorzScroll(CListBox& wndListBox)
{
    CClientDC dc(this);
    CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontRegular);

    int cxExtentMax = 0;

    for (int i = 0; i < wndListBox.GetCount(); i++)
    {
        CString strItem;
        wndListBox.GetText(i, strItem);

        cxExtentMax = std::max(cxExtentMax, (int)dc.GetTextExtent(strItem).cx);
    }

    wndListBox.SetHorizontalExtent(cxExtentMax);
    dc.SelectObject(pOldFont);
}

void COutputDlg::UpdateFonts()
{
    m_wndObserverClientLog.SetFont(&afxGlobalData.fontRegular);
    m_wndServiceLog.SetFont(&afxGlobalData.fontRegular);
    m_wndLogicServerLog.SetFont(&afxGlobalData.fontRegular);
}

void COutputDlg::LogPrint(int type, LPCTSTR format, ...)
{
    CString buf;
    va_list ap;
    va_start(ap, format);
    buf.FormatV(format, ap);
    va_end(ap);
    CTime tm = CTime::GetCurrentTime();
    buf.Format(_T("[%s]%s"), tm.Format(_T("%H:%M:%S")), buf);
    theApp.m_WorkIo.post([this, type, buf]() {
        switch (type)
        {
        case ObserverClientLog:
            m_wndObserverClientLog.AddString(buf);
            AdjustHorzScroll(m_wndObserverClientLog);
            break;
        case ServiceLog:
            m_wndServiceLog.AddString(buf);
            AdjustHorzScroll(m_wndServiceLog);
            break;
        case LogicServerLog:
            m_wndLogicServerLog.AddString(buf);
            AdjustHorzScroll(m_wndLogicServerLog);
            break;
        default:
            break;
        }
    });
}

int COutputDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDialogEx::OnCreate(lpCreateStruct) == -1)
        return -1;

    CRect rectDummy;
    rectDummy.SetRectEmpty();

    // 创建选项卡窗口: 
    if (!m_wndTabs.Create(CMFCTabCtrl::STYLE_3D, rectDummy, this, 1))
    {
        TRACE0("未能创建输出选项卡窗口\n");
        return -1;      // 未能创建
    }

    // 创建输出窗格: 
    const DWORD dwStyle = LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL;

    if (!m_wndObserverClientLog.Create(dwStyle, rectDummy, &m_wndTabs, 2) ||
        !m_wndServiceLog.Create(dwStyle, rectDummy, &m_wndTabs, 3) ||
        !m_wndLogicServerLog.Create(dwStyle, rectDummy, &m_wndTabs, 4))
    {
        TRACE0("未能创建输出窗口\n");
        return -1;      // 未能创建
    }

    CString strTabName;
    BOOL bNameValid;

    // 将列表窗口附加到选项卡: 
    m_wndTabs.AddTab(&m_wndObserverClientLog, _T("管理员日志"), (UINT)0);
    m_wndTabs.AddTab(&m_wndServiceLog, _T("主服务日志"), (UINT)1);
    m_wndTabs.AddTab(&m_wndLogicServerLog, _T("逻辑服务日志"), (UINT)2);
    m_wndTabs.SetActiveTab(2);

    return 0;
}


void COutputDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    // 选项卡控件应覆盖整个工作区: 
	m_wndTabs.SetWindowPos (nullptr, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

COutputList::COutputList() noexcept
{
}

COutputList::~COutputList()
{
}

BEGIN_MESSAGE_MAP(COutputList, CListBox)
    ON_WM_CONTEXTMENU()
    ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
    ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
    ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()

void COutputList::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
    CMenu menu;
    menu.LoadMenu(IDR_MENU_OUTPUT);

    CMenu* pSumMenu = menu.GetSubMenu(0);

    if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
    {
        CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

        if (!pPopupMenu->Create(this, point.x, point.y, (HMENU)pSumMenu->m_hMenu, FALSE, TRUE))
            return;

        ((CMDIFrameWndEx*)AfxGetMainWnd())->OnShowPopupMenu(pPopupMenu);
        UpdateDialogControls(this, FALSE);
    }

    SetFocus();
}

void COutputList::OnEditCopy()
{
    int nSel = GetCurSel();
    if (-1 == nSel)
        return;

    CString strSelAll = _T("");
    GetText(nSel, strSelAll);
    auto source = CT2A(strSelAll);
    int len = strlen(source) + 1;
    HGLOBAL hClip;
    if (OpenClipboard())
    {
        EmptyClipboard();
        hClip = GlobalAlloc(GMEM_MOVEABLE, len);
        char* buff = (char*)GlobalLock(hClip);
        strcpy_s(buff, len, source);
        GlobalUnlock(hClip);
        SetClipboardData(CF_TEXT, hClip);
        CloseClipboard();
        GlobalFree(hClip);
    }
}

void COutputList::OnEditClear()
{
    ResetContent();
}
