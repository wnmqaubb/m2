<<<<<<< HEAD

#include "pch.h"
#include "framework.h"

#include "Gate.h"
#include "ProcessSubView.h"
#include "Resource.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COutputBar

CProcessSubViewWnd::CProcessSubViewWnd() noexcept
{
}

CProcessSubViewWnd::~CProcessSubViewWnd()
{
}

BEGIN_MESSAGE_MAP(CProcessSubViewWnd, CDockablePane)
    ON_WM_CREATE()
    ON_WM_SIZE()
END_MESSAGE_MAP()

int CProcessSubViewWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDockablePane::OnCreate(lpCreateStruct) == -1)
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
    const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    if (!m_wndModuleInfo.Create(dwViewStyle, rectDummy, &m_wndTabs, 2) ||
        !m_wndThreadInfo.Create(dwViewStyle, rectDummy, &m_wndTabs, 3) ||
        !m_wndDirectoryInfo.Create(dwViewStyle, rectDummy, &m_wndTabs, 4))
    {
        TRACE0("未能创建列表视图\n");
        return -1;
    }

    UpdateFonts();

    CString strTabName;
    BOOL bNameValid;

    m_wndTabs.AddTab(&m_wndModuleInfo, _T("模块"), (UINT)0);
    m_wndTabs.AddTab(&m_wndThreadInfo, _T("线程"), (UINT)1);
    m_wndTabs.AddTab(&m_wndDirectoryInfo, _T("进程目录"), (UINT)2);

    InitModuleWindowView();
    InitThreadWindowView();
    InitDirectoryWindowView();
    return 0;
}

void CProcessSubViewWnd::OnSize(UINT nType, int cx, int cy)
{
    CDockablePane::OnSize(nType, cx, cy);
    m_wndTabs.SetWindowPos(nullptr, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}

void CProcessSubViewWnd::InitModuleWindowView()
{
    m_wndModuleInfo.SetColumnByIntSort({0, 2});
    m_wndModuleInfo.SetRedraw(FALSE);
    int colIndex = 0;
    m_wndModuleInfo.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_GRIDLINES);
    m_wndModuleInfo.InsertColumn(colIndex++, TEXT("序号"), LVCFMT_LEFT, 38);
    m_wndModuleInfo.InsertColumn(colIndex++, TEXT("模块基址"), LVCFMT_LEFT, 80);
    m_wndModuleInfo.InsertColumn(colIndex++, TEXT("模块大小"), LVCFMT_LEFT, 80);
    m_wndModuleInfo.InsertColumn(colIndex++, TEXT("模块名"), LVCFMT_LEFT, 150);
    m_wndModuleInfo.InsertColumn(colIndex++, TEXT("模块路径"), LVCFMT_LEFT, 530);

    m_wndModuleInfo.SetRedraw(TRUE);
}

void CProcessSubViewWnd::InitThreadWindowView()
{
    m_wndThreadInfo.SetColumnByIntSort({ 0, 1 });
    m_wndThreadInfo.SetRedraw(FALSE);
    int colIndex = 0;
    m_wndThreadInfo.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_GRIDLINES);
    colIndex = 0;
    m_wndThreadInfo.InsertColumn(colIndex++, TEXT("序号"), LVCFMT_LEFT, 38);
    m_wndThreadInfo.InsertColumn(colIndex++, TEXT("线程ID"), LVCFMT_LEFT, 50);
    m_wndThreadInfo.InsertColumn(colIndex++, TEXT("线程入口"), LVCFMT_LEFT, 120);
    m_wndThreadInfo.InsertColumn(colIndex++, TEXT("主线程"), LVCFMT_LEFT, 50);
    m_wndThreadInfo.InsertColumn(colIndex++, TEXT("模块路径"), LVCFMT_LEFT, 530);
    m_wndThreadInfo.SetRedraw(TRUE);
}

void CProcessSubViewWnd::InitDirectoryWindowView()
{
    m_wndDirectoryInfo.SetColumnByIntSort({ 0 });
    m_wndDirectoryInfo.SetRedraw(FALSE);
    int colIndex = 0;
    m_wndDirectoryInfo.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_GRIDLINES);
    colIndex = 0;
    m_wndDirectoryInfo.InsertColumn(colIndex++, TEXT("序号"), LVCFMT_LEFT, 38);
    m_wndDirectoryInfo.InsertColumn(colIndex++, TEXT("类型"), LVCFMT_LEFT, 50);
    m_wndDirectoryInfo.InsertColumn(colIndex++, TEXT("文件名"), LVCFMT_LEFT, 530);
    m_wndDirectoryInfo.SetRedraw(TRUE);
}

void CProcessSubViewWnd::FillModuleWindow(const std::vector<ProtocolModuleInfo>& modules)
{
    m_wndModuleInfo.SetRedraw(FALSE);
    int colIndex = 0;
    m_wndModuleInfo.DeleteAllItems();
    const wchar_t* empty_wstr = _T("");
    const wchar_t* format_d = _T("%d");
    const wchar_t* format_0x08X = _T("0x%08X");
    const wchar_t* format_0xllX = _T("0x%llX");
    int rowNum = 0;
    for (auto& module : modules)
    {
        if (rowNum % 100 == 0) Sleep(1);
        colIndex = 0;
        CString id;
        CString temp;
        id.Format(format_d, rowNum + 1);
        m_wndModuleInfo.InsertItem(rowNum, empty_wstr);
        m_wndModuleInfo.SetItemText(rowNum, colIndex++, id);
        temp.Format(format_0xllX, module.base);
        m_wndModuleInfo.SetItemText(rowNum, colIndex++, temp);
        temp.Format(format_0x08X, module.size_of_image);
        m_wndModuleInfo.SetItemText(rowNum, colIndex++, temp);
        m_wndModuleInfo.SetItemText(rowNum, colIndex++, module.module_name.c_str());
        m_wndModuleInfo.SetItemText(rowNum, colIndex++, module.path.c_str());
        rowNum++;
    }
    m_wndModuleInfo.SetRedraw(TRUE);
}

void CProcessSubViewWnd::FillThreadWindow(const std::map<uint32_t, ProtocolThreadInfo>& threads)
{
    m_wndThreadInfo.SetRedraw(FALSE);
    int colIndex = 0;
    m_wndThreadInfo.DeleteAllItems();
    const wchar_t* empty_wstr = _T("");
    const wchar_t* format_d = _T("%d");
    const wchar_t* format_0xllX = _T("0x%llX");
    const wchar_t* main_thread_wstr = _T("1");
    const wchar_t* child_thread_wstr = _T("0");
    int rowNum = 0;
    for(auto& thread : threads)
    {
        if (rowNum % 100 == 0) Sleep(1);
        colIndex = 0;
        CString id;
        CString temp;
        id.Format(format_d, rowNum + 1);
        m_wndThreadInfo.InsertItem(rowNum, empty_wstr);
        m_wndThreadInfo.SetItemText(rowNum, colIndex++, id);
        temp.Format(format_d, thread.second.tid);
        m_wndThreadInfo.SetItemText(rowNum, colIndex++, temp);
        temp.Format(format_0xllX, thread.second.start_address);
        m_wndThreadInfo.SetItemText(rowNum, colIndex++, temp);
        m_wndThreadInfo.SetItemText(rowNum, colIndex++, (thread.second.is_main_thread ? main_thread_wstr : child_thread_wstr));
        m_wndThreadInfo.SetItemText(rowNum, colIndex++, thread.second.owner_module.c_str());
        rowNum++;
    }
    m_wndThreadInfo.SetRedraw(TRUE);
}

void CProcessSubViewWnd::FillDirectoryWindow(const std::vector<ProtocolDirectoryInfo>& directories)
{
    m_wndDirectoryInfo.SetRedraw(FALSE);
    int colIndex = 0;
    m_wndDirectoryInfo.DeleteAllItems();
    const wchar_t* empty_wstr = _T("");
    const wchar_t* format_d = _T("%d");
    int rowNum = 0;
    for(auto& dir : directories)
    {
        if (rowNum % 100 == 0) Sleep(1);
        colIndex = 0;
        CString id;
        CString temp;
        id.Format(format_d, rowNum + 1);
        m_wndDirectoryInfo.InsertItem(rowNum, empty_wstr);
        m_wndDirectoryInfo.SetItemText(rowNum, colIndex++, id);
        m_wndDirectoryInfo.SetItemText(rowNum, colIndex++, dir.is_directory ? TEXT("[目录]") : TEXT("[文件]"));
        m_wndDirectoryInfo.SetItemText(rowNum, colIndex++, dir.path.c_str());
        rowNum++;
    }
    m_wndDirectoryInfo.SetRedraw(TRUE);
}

void CProcessSubViewWnd::AdjustHorzScroll(CListBox& wndListBox)
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

void CProcessSubViewWnd::UpdateFonts()
{
    m_wndThreadInfo.SetFont(&afxGlobalData.fontRegular);
    m_wndModuleInfo.SetFont(&afxGlobalData.fontRegular);
    m_wndDirectoryInfo.SetFont(&afxGlobalData.fontRegular);
}
=======

#include "pch.h"
#include "framework.h"

#include "Gate.h"
#include "ProcessSubView.h"
#include "Resource.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COutputBar

CProcessSubViewWnd::CProcessSubViewWnd() noexcept
{
}

CProcessSubViewWnd::~CProcessSubViewWnd()
{
}

BEGIN_MESSAGE_MAP(CProcessSubViewWnd, CDockablePane)
    ON_WM_CREATE()
    ON_WM_SIZE()
END_MESSAGE_MAP()

int CProcessSubViewWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDockablePane::OnCreate(lpCreateStruct) == -1)
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
    const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    if (!m_wndModuleInfo.Create(dwViewStyle, rectDummy, &m_wndTabs, 2) ||
        !m_wndThreadInfo.Create(dwViewStyle, rectDummy, &m_wndTabs, 3) ||
        !m_wndDirectoryInfo.Create(dwViewStyle, rectDummy, &m_wndTabs, 4))
    {
        TRACE0("未能创建列表视图\n");
        return -1;
    }

    UpdateFonts();

    CString strTabName;
    BOOL bNameValid;

    m_wndTabs.AddTab(&m_wndModuleInfo, _T("模块"), (UINT)0);
    m_wndTabs.AddTab(&m_wndThreadInfo, _T("线程"), (UINT)1);
    m_wndTabs.AddTab(&m_wndDirectoryInfo, _T("进程目录"), (UINT)2);

    InitModuleWindowView();
    InitThreadWindowView();
    InitDirectoryWindowView();
    return 0;
}

void CProcessSubViewWnd::OnSize(UINT nType, int cx, int cy)
{
    CDockablePane::OnSize(nType, cx, cy);
    m_wndTabs.SetWindowPos(nullptr, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}

void CProcessSubViewWnd::InitModuleWindowView()
{
    m_wndModuleInfo.SetColumnByIntSort({0, 2});
    m_wndModuleInfo.SetRedraw(FALSE);
    int colIndex = 0;
    m_wndModuleInfo.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_GRIDLINES);
    m_wndModuleInfo.InsertColumn(colIndex++, TEXT("序号"), LVCFMT_LEFT, 38);
    m_wndModuleInfo.InsertColumn(colIndex++, TEXT("模块基址"), LVCFMT_LEFT, 80);
    m_wndModuleInfo.InsertColumn(colIndex++, TEXT("模块大小"), LVCFMT_LEFT, 80);
    m_wndModuleInfo.InsertColumn(colIndex++, TEXT("模块名"), LVCFMT_LEFT, 150);
    m_wndModuleInfo.InsertColumn(colIndex++, TEXT("模块路径"), LVCFMT_LEFT, 530);

    m_wndModuleInfo.SetRedraw(TRUE);
}

void CProcessSubViewWnd::InitThreadWindowView()
{
    m_wndThreadInfo.SetColumnByIntSort({ 0, 1 });
    m_wndThreadInfo.SetRedraw(FALSE);
    int colIndex = 0;
    m_wndThreadInfo.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_GRIDLINES);
    colIndex = 0;
    m_wndThreadInfo.InsertColumn(colIndex++, TEXT("序号"), LVCFMT_LEFT, 38);
    m_wndThreadInfo.InsertColumn(colIndex++, TEXT("线程ID"), LVCFMT_LEFT, 50);
    m_wndThreadInfo.InsertColumn(colIndex++, TEXT("线程入口"), LVCFMT_LEFT, 120);
    m_wndThreadInfo.InsertColumn(colIndex++, TEXT("主线程"), LVCFMT_LEFT, 50);
    m_wndThreadInfo.InsertColumn(colIndex++, TEXT("模块路径"), LVCFMT_LEFT, 530);
    m_wndThreadInfo.SetRedraw(TRUE);
}

void CProcessSubViewWnd::InitDirectoryWindowView()
{
    m_wndDirectoryInfo.SetColumnByIntSort({ 0 });
    m_wndDirectoryInfo.SetRedraw(FALSE);
    int colIndex = 0;
    m_wndDirectoryInfo.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_GRIDLINES);
    colIndex = 0;
    m_wndDirectoryInfo.InsertColumn(colIndex++, TEXT("序号"), LVCFMT_LEFT, 38);
    m_wndDirectoryInfo.InsertColumn(colIndex++, TEXT("类型"), LVCFMT_LEFT, 50);
    m_wndDirectoryInfo.InsertColumn(colIndex++, TEXT("文件名"), LVCFMT_LEFT, 530);
    m_wndDirectoryInfo.SetRedraw(TRUE);
}

void CProcessSubViewWnd::FillModuleWindow(const std::vector<ProtocolModuleInfo>& modules)
{
    m_wndModuleInfo.SetRedraw(FALSE);
    int colIndex = 0;
    m_wndModuleInfo.DeleteAllItems();
    const wchar_t* empty_wstr = _T("");
    const wchar_t* format_d = _T("%d");
    const wchar_t* format_0x08X = _T("0x%08X");
    const wchar_t* format_0xllX = _T("0x%llX");
    int rowNum = 0;
    for (auto& module : modules)
    {
        if (rowNum % 100 == 0) Sleep(1);
        colIndex = 0;
        CString id;
        CString temp;
        id.Format(format_d, rowNum + 1);
        m_wndModuleInfo.InsertItem(rowNum, empty_wstr);
        m_wndModuleInfo.SetItemText(rowNum, colIndex++, id);
        temp.Format(format_0xllX, module.base);
        m_wndModuleInfo.SetItemText(rowNum, colIndex++, temp);
        temp.Format(format_0x08X, module.size_of_image);
        m_wndModuleInfo.SetItemText(rowNum, colIndex++, temp);
        m_wndModuleInfo.SetItemText(rowNum, colIndex++, module.module_name.c_str());
        m_wndModuleInfo.SetItemText(rowNum, colIndex++, module.path.c_str());
        rowNum++;
    }
    m_wndModuleInfo.SetRedraw(TRUE);
}

void CProcessSubViewWnd::FillThreadWindow(const std::map<uint32_t, ProtocolThreadInfo>& threads)
{
    m_wndThreadInfo.SetRedraw(FALSE);
    int colIndex = 0;
    m_wndThreadInfo.DeleteAllItems();
    const wchar_t* empty_wstr = _T("");
    const wchar_t* format_d = _T("%d");
    const wchar_t* format_0xllX = _T("0x%llX");
    const wchar_t* main_thread_wstr = _T("1");
    const wchar_t* child_thread_wstr = _T("0");
    int rowNum = 0;
    for(auto& thread : threads)
    {
        if (rowNum % 100 == 0) Sleep(1);
        colIndex = 0;
        CString id;
        CString temp;
        id.Format(format_d, rowNum + 1);
        m_wndThreadInfo.InsertItem(rowNum, empty_wstr);
        m_wndThreadInfo.SetItemText(rowNum, colIndex++, id);
        temp.Format(format_d, thread.second.tid);
        m_wndThreadInfo.SetItemText(rowNum, colIndex++, temp);
        temp.Format(format_0xllX, thread.second.start_address);
        m_wndThreadInfo.SetItemText(rowNum, colIndex++, temp);
        m_wndThreadInfo.SetItemText(rowNum, colIndex++, (thread.second.is_main_thread ? main_thread_wstr : child_thread_wstr));
        m_wndThreadInfo.SetItemText(rowNum, colIndex++, thread.second.owner_module.c_str());
        rowNum++;
    }
    m_wndThreadInfo.SetRedraw(TRUE);
}

void CProcessSubViewWnd::FillDirectoryWindow(const std::vector<ProtocolDirectoryInfo>& directories)
{
    m_wndDirectoryInfo.SetRedraw(FALSE);
    int colIndex = 0;
    m_wndDirectoryInfo.DeleteAllItems();
    const wchar_t* empty_wstr = _T("");
    const wchar_t* format_d = _T("%d");
    int rowNum = 0;
    for(auto& dir : directories)
    {
        if (rowNum % 100 == 0) Sleep(1);
        colIndex = 0;
        CString id;
        CString temp;
        id.Format(format_d, rowNum + 1);
        m_wndDirectoryInfo.InsertItem(rowNum, empty_wstr);
        m_wndDirectoryInfo.SetItemText(rowNum, colIndex++, id);
        m_wndDirectoryInfo.SetItemText(rowNum, colIndex++, dir.is_directory ? TEXT("[目录]") : TEXT("[文件]"));
        m_wndDirectoryInfo.SetItemText(rowNum, colIndex++, dir.path.c_str());
        rowNum++;
    }
    m_wndDirectoryInfo.SetRedraw(TRUE);
}

void CProcessSubViewWnd::AdjustHorzScroll(CListBox& wndListBox)
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

void CProcessSubViewWnd::UpdateFonts()
{
    m_wndThreadInfo.SetFont(&afxGlobalData.fontRegular);
    m_wndModuleInfo.SetFont(&afxGlobalData.fontRegular);
    m_wndDirectoryInfo.SetFont(&afxGlobalData.fontRegular);
}
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9
