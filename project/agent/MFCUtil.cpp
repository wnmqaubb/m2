#include "pch.h"
#include "framework.h"
#include "MFCUtil.h"
#include <strsafe.h>

/**
 * columnsByNumberSort �������е�����������������,��������ĸ����
 */
void MFCUtil::LvnColumnclickList(LPNMLISTVIEW pNMLV, LRESULT * pResult, PFNLVCOMPARE pfnCompare, CListCtrl* pListCtrl)
{
	int m_current_select_isubitem = pNMLV->iSubItem;
	// ȥ�������е�����
	const UINT length = 50;
	// ���
	if(pListCtrl->m_nFlags > 0 && pListCtrl->m_nFlags != pNMLV->iSubItem + 1)
	{
		LVCOLUMN Vol_old;
		CString csStr_old("");
		TCHAR szCol_old[length];
		Vol_old.pszText = szCol_old;
		Vol_old.mask = LVCF_TEXT;
		Vol_old.cchTextMax = sizeof(szCol_old);
		pListCtrl->GetColumn(pListCtrl->m_nFlags - 1, &Vol_old);
		csStr_old = CString(Vol_old.pszText);
		if(csStr_old.Right(1) == CString("��"))
		{
			csStr_old.Replace(TEXT("��"), TEXT(""));
			pListCtrl->SetColumnWidth(pListCtrl->m_nFlags - 1, pListCtrl->GetColumnWidth(pListCtrl->m_nFlags - 1) - 10);
		} else if(csStr_old.Right(1) == CString("��"))
		{
			csStr_old.Replace(TEXT("��"), TEXT(""));
			pListCtrl->SetColumnWidth(pListCtrl->m_nFlags - 1, pListCtrl->GetColumnWidth(pListCtrl->m_nFlags - 1) - 10);
		}
		Vol_old.pszText = csStr_old.GetBuffer();
		pListCtrl->SetColumn(pListCtrl->m_nFlags - 1, &Vol_old);
	}

	pListCtrl->m_nFlags = pNMLV->iSubItem + 1;
	for(int i = 0; i < pListCtrl->GetItemCount(); ++i)
	{
		pListCtrl->SetItemData(i, i);
	}
	LVCOLUMN Vol;
	CString csStr("");
	TCHAR szCol[length];
	Vol.pszText = szCol;
	Vol.mask = LVCF_TEXT;
	Vol.cchTextMax = sizeof(szCol);
	pListCtrl->GetColumn(m_current_select_isubitem, &Vol);
	csStr = CString(Vol.pszText);

	if(csStr.Right(1) == CString("��"))
	{
		csStr.Replace(TEXT("��"), TEXT("��"));
		Vol.pszText = csStr.GetBuffer();
		pListCtrl->SetColumn(m_current_select_isubitem, &Vol);
	} else if(csStr.Right(1) == CString("��"))
	{
		csStr.Replace(TEXT("��"), TEXT(""));
		Vol.pszText = csStr.GetBuffer();
		pListCtrl->SetColumn(m_current_select_isubitem, &Vol);
		pListCtrl->SetColumnWidth(m_current_select_isubitem, pListCtrl->GetColumnWidth(m_current_select_isubitem) - 10);
	} else
	{
		csStr.Append(TEXT("��"));
		Vol.pszText = csStr.GetBuffer();
		pListCtrl->SetColumn(m_current_select_isubitem, &Vol);
		pListCtrl->SetColumnWidth(m_current_select_isubitem, pListCtrl->GetColumnWidth(m_current_select_isubitem) + 10);
	}

	pListCtrl->SortItems(pfnCompare, (LPARAM)pListCtrl);

}

/**
 * search �������ַ���
 * searchOld �ϴ��������ַ���
 * list �����Ķ���
 * colunmIndexs Ҫ������������
 */
void MFCUtil::CListCtrlSearch(CString& search, CString& searchOld, CListCtrl& list, CUIntArray& colunmIndexs)
{
	int m_nFindIndex;
	CString SearchText = search.Trim();
	// 1 �������Ϊ�գ�������
	if(SearchText.IsEmpty())
		return;

	// 2 �����������ҡ�ѡ������
	// ���β�ѯ���ϴβ�ͬ
	if(searchOld != SearchText)
		m_nFindIndex = -1;

	// ���ݵ�ǰѡ���ȷ�����ҿ�ʼλ��
	POSITION pos = list.GetFirstSelectedItemPosition();
	if(pos != NULL)
		m_nFindIndex = list.GetNextSelectedItem(pos);

	// ����λ�ù������λ�ã���ͷ����
	if(m_nFindIndex > list.GetItemCount() - 1)
		m_nFindIndex = -1;

	// (1) �ӱ��λ����һ�������λ�ò���
	bool finded = false;
	for(int i = m_nFindIndex + 1; i < list.GetItemCount(); ++i)
	{
		for(int k = 0; k < colunmIndexs.GetCount(); k++)
		{
			int columnIndex = colunmIndexs.GetAt(k);
			CString ItemText = list.GetItemText(i, columnIndex);
			ItemText.MakeUpper();
			SearchText.MakeUpper();
			if(ItemText.Find(SearchText) == -1)
				continue;
			// ȡ����ѡ�е�����
			POSITION pos = list.GetFirstSelectedItemPosition();
			while(pos != NULL)
			{
				int nIndex = list.GetNextSelectedItem(pos);
				list.SetItemState(nIndex, 0, LVIS_SELECTED | LVIS_FOCUSED);
			}

			list.SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			list.SetFocus();
			list.EnsureVisible(i, FALSE);
			m_nFindIndex = i;
			searchOld = SearchText;
			return;
		}
	}

	// (2) �ӿ�ʼ�����λ���ң�ǰ��δ�ҵ���
	for(int j = 0; j <= m_nFindIndex; ++j)
	{
		for(int k = 0; k < colunmIndexs.GetCount(); k++)
		{
			int columnIndex = colunmIndexs.GetAt(k);
			CString ItemText = list.GetItemText(j, columnIndex);
			ItemText.MakeUpper();
			SearchText.MakeUpper();
			if(ItemText.Find(SearchText) == -1)
				continue;

			// ȡ����ѡ�е�����
			POSITION pos = list.GetFirstSelectedItemPosition();
			while(pos != NULL)
			{
				int nIndex = list.GetNextSelectedItem(pos);
				list.SetItemState(nIndex, 0, LVIS_SELECTED | LVIS_FOCUSED);
			}

			list.SetItemState(j, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			list.SetFocus();
			list.EnsureVisible(j, FALSE);
			m_nFindIndex = j;
			searchOld = SearchText;
			return;
		}
	}

	AfxMessageBox(TEXT("δ�ҵ���"));
}

/**
 * source �����ַ�����������
 */
void MFCUtil::CopyText(CString& source)
{
	int len = source.GetLength();
	if(len <= 0)
	{
		return;
	}
	if(!OpenClipboard(nullptr))
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