
#include "pch.h"
#include "Gate.h"
#include "framework.h"
#include "ViewList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewTree

CViewList::CViewList() noexcept
{
}

CViewList::~CViewList()
{
}

BEGIN_MESSAGE_MAP(CViewList, CListCtrl)
    ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, &CViewList::OnLvnColumnclick)
    ON_NOTIFY_REFLECT(NM_DBLCLK, &CViewList::OnNMDblclk)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewTree 消息处理程序

/**
 * columnsByNumberSort 此数组中的列索引按数字排序,其它按字母排序
 */
void CViewList::LvnColumnclickList(LPNMLISTVIEW pNMLV, LRESULT * pResult, PFNLVCOMPARE pfnCompare)
{
    int iCurSelSubItem = pNMLV->iSubItem;
    // 去掉其它列的排序
    const UINT length = 50;
    // 序号
    if(this->m_nFlags > 0 && this->m_nFlags != pNMLV->iSubItem + 1)
    {
        LVCOLUMN columnOld;
        CString csStr_old("");
        TCHAR szCol_old[length];
        columnOld.pszText = szCol_old;
        columnOld.mask = LVCF_TEXT;
        columnOld.cchTextMax = sizeof(szCol_old);
        this->GetColumn(this->m_nFlags - 1, &columnOld);
        csStr_old = CString(columnOld.pszText);
        if(csStr_old.Right(1) == CString("▼"))
        {
            csStr_old.Replace(TEXT("▼"), TEXT(""));
            this->SetColumnWidth(this->m_nFlags - 1, this->GetColumnWidth(this->m_nFlags - 1) - 10);
        }
        else if(csStr_old.Right(1) == CString("▲"))
        {
            csStr_old.Replace(TEXT("▲"), TEXT(""));
            this->SetColumnWidth(this->m_nFlags - 1, this->GetColumnWidth(this->m_nFlags - 1) - 10);
        }
        columnOld.pszText = csStr_old.GetBuffer();
        this->SetColumn(this->m_nFlags - 1, &columnOld);
    }

    this->m_nFlags = pNMLV->iSubItem + 1;
    for(int i = 0; i < this->GetItemCount(); ++i)
    {
        this->SetItemData(i, i);
    }
    LVCOLUMN Vol;
    CString csStr("");
    TCHAR szCol[length];
    Vol.pszText = szCol;
    Vol.mask = LVCF_TEXT;
    Vol.cchTextMax = sizeof(szCol);
    this->GetColumn(iCurSelSubItem, &Vol);
    csStr = CString(Vol.pszText);

    if(csStr.Right(1) == CString("▼"))
    {
        csStr.Replace(TEXT("▼"), TEXT("▲"));
        Vol.pszText = csStr.GetBuffer();
        this->SetColumn(iCurSelSubItem, &Vol);
    }
    else if(csStr.Right(1) == CString("▲"))
    {
        csStr.Replace(TEXT("▲"), TEXT(""));
        Vol.pszText = csStr.GetBuffer();
        this->SetColumn(iCurSelSubItem, &Vol);
        this->SetColumnWidth(iCurSelSubItem, this->GetColumnWidth(iCurSelSubItem) - 10);
    }
    else
    {
        csStr.Append(TEXT("▼"));
        Vol.pszText = csStr.GetBuffer();
        this->SetColumn(iCurSelSubItem, &Vol);
        this->SetColumnWidth(iCurSelSubItem, this->GetColumnWidth(iCurSelSubItem) + 10);
    }

    this->SortItems(pfnCompare, (LPARAM)this);

}

/**
 * search 搜索的字符串
 * searchOld 上次搜索的字符串
 * list 搜索的对象
 * columnsByNumberSearch 要搜索的列索引
 */
void CViewList::CListCtrlSearch(CString& search)
{
    int m_nFindIndex = -1;
    CString SearchText = search.Trim();
    // 1 如果内容为空，不查找
    if(SearchText.IsEmpty())
        return;

    // 2 根据条件查找、选择数据
    // 本次查询与上次不同
    if(searchOld != SearchText)
        m_nFindIndex = -1;

    // 根据当前选择项，确定查找开始位置
    POSITION pos = this->GetFirstSelectedItemPosition();
    if(pos != NULL)
        m_nFindIndex = this->GetNextSelectedItem(pos);

    // 查找位置过了最后位置，从头再来
    if(m_nFindIndex > this->GetItemCount() - 1)
        m_nFindIndex = -1;

    // (1) 从标记位的下一个到最后位置查找
    bool finded = false;
    for(int i = m_nFindIndex + 1; i < this->GetItemCount(); ++i)
    {
        for(int k = 0; k < columnsByNumberSearch.GetCount(); k++)
        {
            int columnIndex = columnsByNumberSearch.GetAt(k);
            CString ItemText = this->GetItemText(i, columnIndex);
            ItemText.MakeUpper();
            SearchText.MakeUpper();
            if(ItemText.Find(SearchText) == -1)
                continue;
            // 取消已选中的内容
            POSITION pos = this->GetFirstSelectedItemPosition();
            while(pos != NULL)
            {
                int nIndex = this->GetNextSelectedItem(pos);
                this->SetItemState(nIndex, 0, LVIS_SELECTED | LVIS_FOCUSED);
            }

            this->SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
            this->SetFocus();
            this->EnsureVisible(i, FALSE);
            m_nFindIndex = i;
            searchOld = SearchText;
            return;
        }
    }

    // (2) 从开始到标记位查找（前面未找到）
    for(int j = 0; j <= m_nFindIndex; ++j)
    {
        for(int k = 0; k < columnsByNumberSearch.GetCount(); k++)
        {
            int columnIndex = columnsByNumberSearch.GetAt(k);
            CString ItemText = this->GetItemText(j, columnIndex);
            ItemText.MakeUpper();
            SearchText.MakeUpper();
            if(ItemText.Find(SearchText) == -1)
                continue;

            // 取消已选中的内容
            POSITION pos = this->GetFirstSelectedItemPosition();
            while(pos != NULL)
            {
                int nIndex = this->GetNextSelectedItem(pos);
                this->SetItemState(nIndex, 0, LVIS_SELECTED | LVIS_FOCUSED);
            }

            this->SetItemState(j, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
            this->SetFocus();
            this->EnsureVisible(j, FALSE);
            m_nFindIndex = j;
            searchOld = SearchText;
            return;
        }
    }

    AfxMessageBox(TEXT("未找到！"));
}

/**
 * source 复制字符串到剪贴板
 */
void CViewList::CopyText(CString& source)
{
    int len = source.GetLength();
    if(len <= 0)
    {
        return;
    }
    if(!OpenClipboard())
    {
        return;
    }
    EmptyClipboard();
    HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (len + 1));
    if(hglbCopy == NULL)
    {
        CloseClipboard();
        return;
    }
    char* lptstrCopy = (char*)GlobalLock(hglbCopy);
    strcpy_s(lptstrCopy, len + 1, (CStringA)source);
    GlobalUnlock(hglbCopy);
    SetClipboardData(CF_TEXT, hglbCopy);
    CloseClipboard();
}

void CViewList::SetColumnByIntSort(std::initializer_list<uint32_t> column_index_array)
{
    columnsByNumberSort.RemoveAll();
    for (uint32_t i : column_index_array)
    {
        columnsByNumberSort.Add(i);
    }
}

void CViewList::SetColumnBySearch(std::initializer_list<uint32_t> column_index_array)
{
    columnsByNumberSearch.RemoveAll();
    for (uint32_t i : column_index_array)
    {
        columnsByNumberSearch.Add(i);
    }
}


void CViewList::OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    LvnColumnclickList(pNMLV, pResult, OnColumnSortCompareProc);
    *pResult = 0;
}

void CViewList::OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult)
{
    AfxMessageBox(_T("CViewList::OnNMDblclk 双击事件触发"));
    *pResult = 0;
}