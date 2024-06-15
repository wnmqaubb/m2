<<<<<<< HEAD
﻿
#pragma once

/////////////////////////////////////////////////////////////////////////////
// CViewTree 窗口

class CViewList : public CListCtrl
{
// 构造
public:
	CViewList() noexcept;
    void SetColumnByIntSort(std::initializer_list<uint32_t> column_index_array);
    void SetColumnBySearch(std::initializer_list<uint32_t> column_index_array);
    /**
     * search 搜索的字符串
     * searchOld 上次搜索的字符串
     * list 搜索的对象
     * colunmIndexs 要搜索的列索引
     */
    void CListCtrlSearch(CString & search);
    // 重写
protected:
    void LvnColumnclickList(LPNMLISTVIEW pNMLV, LRESULT *pResult, PFNLVCOMPARE pfnCompare);

    void CopyText(CString & source);
    /**
     * columnsByNumberSort 此数组中的列索引按数字排序,其它按字母排序
     */
    static int CompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
    {
        CViewList* pListCtrl = (CViewList*)lParamSort;
        int iCurSelItem = pListCtrl->m_nFlags - 1;
        CString szItem1 = pListCtrl->GetItemText(lParam1, iCurSelItem);
        CString szItem2 = pListCtrl->GetItemText(lParam2, iCurSelItem);
        LVCOLUMN Vol;
        CString szColumnTitle("");
        const UINT length = 50;
        TCHAR szCol[length];
        Vol.pszText = szCol;
        Vol.mask = LVCF_TEXT;
        Vol.cchTextMax = sizeof(szCol);
        pListCtrl->GetColumn(iCurSelItem, &Vol);
        szColumnTitle = CString(Vol.pszText);
        bool isSortByNumber = false;
        for(int i = 0; i < pListCtrl->columnsByNumberSort.GetCount(); i++)
        {
            if(iCurSelItem == pListCtrl->columnsByNumberSort.GetAt(i))
            {
                isSortByNumber = true;
                break;
            }
        }
        // 按数字排序
        if(isSortByNumber)
        {
            int n1 = _ttoi(szItem1);
            int n2 = _ttoi(szItem2);
            if(szColumnTitle.Right(1) == CString("▼"))
            {
                return n1 > n2 ? -1 : 1;
            }
            else
            {
                return n1 > n2 ? 1 : -1;
            }
        }
        // 按字母排序
        else
        {
            if(szColumnTitle.Right(1) == CString("▼"))
            {
                return szItem2.CompareNoCase(szItem1);
            }
            else
            {
                return szItem1.CompareNoCase(szItem2);
            }
        }
    }
// 实现
public:
	virtual ~CViewList();

protected:
	DECLARE_MESSAGE_MAP()
private:
    // columnsByNumberSort 此数组中的列索引按数字排序,其它按字母排序
    CUIntArray columnsByNumberSort;
    CUIntArray columnsByNumberSearch;
    CString searchOld;
    static int CALLBACK OnColumnSortCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
    {
        return CompareProc(lParam1, lParam2, lParamSort);
    }

public:
    afx_msg void OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult);
};
=======
﻿
#pragma once

/////////////////////////////////////////////////////////////////////////////
// CViewTree 窗口

class CViewList : public CListCtrl
{
// 构造
public:
	CViewList() noexcept;
    void SetColumnByIntSort(std::initializer_list<uint32_t> column_index_array);
    void SetColumnBySearch(std::initializer_list<uint32_t> column_index_array);
    /**
     * search 搜索的字符串
     * searchOld 上次搜索的字符串
     * list 搜索的对象
     * colunmIndexs 要搜索的列索引
     */
    void CListCtrlSearch(CString & search);
    // 重写
protected:
    void LvnColumnclickList(LPNMLISTVIEW pNMLV, LRESULT *pResult, PFNLVCOMPARE pfnCompare);

    void CopyText(CString & source);
    /**
     * columnsByNumberSort 此数组中的列索引按数字排序,其它按字母排序
     */
    static int CompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
    {
        CViewList* pListCtrl = (CViewList*)lParamSort;
        int iCurSelItem = pListCtrl->m_nFlags - 1;
        CString szItem1 = pListCtrl->GetItemText(lParam1, iCurSelItem);
        CString szItem2 = pListCtrl->GetItemText(lParam2, iCurSelItem);
        LVCOLUMN Vol;
        CString szColumnTitle("");
        const UINT length = 50;
        TCHAR szCol[length];
        Vol.pszText = szCol;
        Vol.mask = LVCF_TEXT;
        Vol.cchTextMax = sizeof(szCol);
        pListCtrl->GetColumn(iCurSelItem, &Vol);
        szColumnTitle = CString(Vol.pszText);
        bool isSortByNumber = false;
        for(int i = 0; i < pListCtrl->columnsByNumberSort.GetCount(); i++)
        {
            if(iCurSelItem == pListCtrl->columnsByNumberSort.GetAt(i))
            {
                isSortByNumber = true;
                break;
            }
        }
        // 按数字排序
        if(isSortByNumber)
        {
            int n1 = _ttoi(szItem1);
            int n2 = _ttoi(szItem2);
            if(szColumnTitle.Right(1) == CString("▼"))
            {
                return n1 > n2 ? -1 : 1;
            }
            else
            {
                return n1 > n2 ? 1 : -1;
            }
        }
        // 按字母排序
        else
        {
            if(szColumnTitle.Right(1) == CString("▼"))
            {
                return szItem2.CompareNoCase(szItem1);
            }
            else
            {
                return szItem1.CompareNoCase(szItem2);
            }
        }
    }
// 实现
public:
	virtual ~CViewList();

protected:
	DECLARE_MESSAGE_MAP()
private:
    // columnsByNumberSort 此数组中的列索引按数字排序,其它按字母排序
    CUIntArray columnsByNumberSort;
    CUIntArray columnsByNumberSearch;
    CString searchOld;
    static int CALLBACK OnColumnSortCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
    {
        return CompareProc(lParam1, lParam2, lParamSort);
    }

public:
    afx_msg void OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult);
};
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9
