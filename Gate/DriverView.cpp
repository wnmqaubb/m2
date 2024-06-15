<<<<<<< HEAD
﻿
// GateView.cpp: CProcessView 类的实现
//

#include "pch.h"
#include "framework.h"
#include "Gate.h"

#include "ChildFrm.h"
#include "DriverDoc.h"
#include "DriverView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CProcessView

IMPLEMENT_DYNCREATE(CDriverView, CView)

BEGIN_MESSAGE_MAP(CDriverView, CView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_CREATE()
	ON_WM_SIZE()
    ON_NOTIFY(LVN_ITEMCHANGED, ID_DRIVER_VIEW, &CDriverView::OnListItemChanged)
END_MESSAGE_MAP()

// CProcessView 构造/析构

CDriverView::CDriverView() noexcept
{
	// TODO: 在此处添加构造代码

}

CDriverView::~CDriverView()
{
}

BOOL CDriverView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式
    
	return CView::PreCreateWindow(cs);
}

// CProcessView 绘图

void CDriverView::OnDraw(CDC* pDC)
{
	CView::OnDraw(pDC);
	// TODO: 在此处为本机数据添加绘制代码
}

void CDriverView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}


// CProcessView 诊断
#ifdef _DEBUG
void CDriverView::AssertValid() const
{
	CView::AssertValid();
}

void CDriverView::Dump(CDumpContext& dc) const
{
CView::Dump(dc);
}

CDriverDoc* CDriverView::GetDocument() const // 非调试版本是内联的
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDriverDoc)));
    return (CDriverDoc*)m_pDocument;
}

CChildFrame* CDriverView::GetParentCChild() const
{
    ASSERT(GetParent()->IsKindOf(RUNTIME_CLASS(CChildFrame)));
    return (CChildFrame*)GetParent();
}

#endif //_DEBUG


// CProcessView 消息处理程序


int CDriverView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CView::OnCreate(lpCreateStruct) == -1)
        return -1;



    CRect rectDummy;
    GetClientRect(&rectDummy);

    // 创建视图: 
    const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    if (!m_ViewList.Create(dwViewStyle, rectDummy, this, ID_DRIVER_VIEW))
    {
        TRACE0("未能创建列表视图\n");
    }
    return 0;
}


void CDriverView::FillViewList()
{
    auto& drivers = GetDocument()->GetDrivers();
    m_ViewList.SetRedraw(FALSE);
    m_ViewList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_GRIDLINES);
    int colIndex = 0;
    m_ViewList.InsertColumn(colIndex++, TEXT("序号"), LVCFMT_LEFT, 38);
    m_ViewList.InsertColumn(colIndex++, TEXT("基址"), LVCFMT_LEFT, 100);
    m_ViewList.InsertColumn(colIndex++, TEXT("大小"), LVCFMT_LEFT, 150);
    m_ViewList.InsertColumn(colIndex++, TEXT("名称"), LVCFMT_LEFT, 530);
    m_ViewList.DeleteAllItems();
    int rowNum = 0;
    for (auto& driver : drivers.data)
    {
        colIndex = 0;
        CString id;
        CString temp;
        id.Format(_T("%d"), rowNum + 1);
        m_ViewList.InsertItem(rowNum, _T(""));
        m_ViewList.SetItemText(rowNum, colIndex++, id);
        temp.Format(_T("0x%llX"), driver.image_base);
        m_ViewList.SetItemText(rowNum, colIndex++, temp);
        temp.Format(_T("0x%08X"), driver.image_size);
        m_ViewList.SetItemText(rowNum, colIndex++, temp);
        CString image_name;
        if (driver.image_name.find(L"netfilter2") != std::wstring::npos)
        {
            image_name.Format(_T("***%s***"), driver.image_name.c_str());
        }
        else
        {
            image_name = driver.image_name.c_str();
        }
        m_ViewList.SetItemText(rowNum, colIndex++, image_name);
        rowNum++;
    }
    m_ViewList.SetRedraw(TRUE);
}

void CDriverView::OnInitialUpdate()
{
    CView::OnInitialUpdate();
    FillViewList();
}


void CDriverView::OnSize(UINT nType, int cx, int cy)
{
    CView::OnSize(nType, cx, cy);
    AdjustLayout();
}


void CDriverView::AdjustLayout()
{
    if (GetSafeHwnd() == nullptr)
    {
        return;
    }

    CRect rectClient;
    GetClientRect(rectClient);

    m_ViewList.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}

void CDriverView::OnListItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    if (pNMListView->uChanged == LVIF_STATE)
    {
        if (pNMListView->uNewState & LVIS_SELECTED)
        {
            //不做处理
        }
    }
=======
﻿
// GateView.cpp: CProcessView 类的实现
//

#include "pch.h"
#include "framework.h"
#include "Gate.h"

#include "ChildFrm.h"
#include "DriverDoc.h"
#include "DriverView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CProcessView

IMPLEMENT_DYNCREATE(CDriverView, CView)

BEGIN_MESSAGE_MAP(CDriverView, CView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_CREATE()
	ON_WM_SIZE()
    ON_NOTIFY(LVN_ITEMCHANGED, ID_DRIVER_VIEW, &CDriverView::OnListItemChanged)
END_MESSAGE_MAP()

// CProcessView 构造/析构

CDriverView::CDriverView() noexcept
{
	// TODO: 在此处添加构造代码

}

CDriverView::~CDriverView()
{
}

BOOL CDriverView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式
    
	return CView::PreCreateWindow(cs);
}

// CProcessView 绘图

void CDriverView::OnDraw(CDC* pDC)
{
	CView::OnDraw(pDC);
	// TODO: 在此处为本机数据添加绘制代码
}

void CDriverView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}


// CProcessView 诊断
#ifdef _DEBUG
void CDriverView::AssertValid() const
{
	CView::AssertValid();
}

void CDriverView::Dump(CDumpContext& dc) const
{
CView::Dump(dc);
}

CDriverDoc* CDriverView::GetDocument() const // 非调试版本是内联的
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDriverDoc)));
    return (CDriverDoc*)m_pDocument;
}

CChildFrame* CDriverView::GetParentCChild() const
{
    ASSERT(GetParent()->IsKindOf(RUNTIME_CLASS(CChildFrame)));
    return (CChildFrame*)GetParent();
}

#endif //_DEBUG


// CProcessView 消息处理程序


int CDriverView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CView::OnCreate(lpCreateStruct) == -1)
        return -1;



    CRect rectDummy;
    GetClientRect(&rectDummy);

    // 创建视图: 
    const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    if (!m_ViewList.Create(dwViewStyle, rectDummy, this, ID_DRIVER_VIEW))
    {
        TRACE0("未能创建列表视图\n");
    }
    return 0;
}


void CDriverView::FillViewList()
{
    auto& drivers = GetDocument()->GetDrivers();
    m_ViewList.SetRedraw(FALSE);
    m_ViewList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_GRIDLINES);
    int colIndex = 0;
    m_ViewList.InsertColumn(colIndex++, TEXT("序号"), LVCFMT_LEFT, 38);
    m_ViewList.InsertColumn(colIndex++, TEXT("基址"), LVCFMT_LEFT, 100);
    m_ViewList.InsertColumn(colIndex++, TEXT("大小"), LVCFMT_LEFT, 150);
    m_ViewList.InsertColumn(colIndex++, TEXT("名称"), LVCFMT_LEFT, 530);
    m_ViewList.DeleteAllItems();
    int rowNum = 0;
    for (auto& driver : drivers.data)
    {
        colIndex = 0;
        CString id;
        CString temp;
        id.Format(_T("%d"), rowNum + 1);
        m_ViewList.InsertItem(rowNum, _T(""));
        m_ViewList.SetItemText(rowNum, colIndex++, id);
        temp.Format(_T("0x%llX"), driver.image_base);
        m_ViewList.SetItemText(rowNum, colIndex++, temp);
        temp.Format(_T("0x%08X"), driver.image_size);
        m_ViewList.SetItemText(rowNum, colIndex++, temp);
        CString image_name;
        if (driver.image_name.find(L"netfilter2") != std::wstring::npos)
        {
            image_name.Format(_T("***%s***"), driver.image_name.c_str());
        }
        else
        {
            image_name = driver.image_name.c_str();
        }
        m_ViewList.SetItemText(rowNum, colIndex++, image_name);
        rowNum++;
    }
    m_ViewList.SetRedraw(TRUE);
}

void CDriverView::OnInitialUpdate()
{
    CView::OnInitialUpdate();
    FillViewList();
}


void CDriverView::OnSize(UINT nType, int cx, int cy)
{
    CView::OnSize(nType, cx, cy);
    AdjustLayout();
}


void CDriverView::AdjustLayout()
{
    if (GetSafeHwnd() == nullptr)
    {
        return;
    }

    CRect rectClient;
    GetClientRect(rectClient);

    m_ViewList.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}

void CDriverView::OnListItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    if (pNMListView->uChanged == LVIF_STATE)
    {
        if (pNMListView->uNewState & LVIS_SELECTED)
        {
            //不做处理
        }
    }
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9
}