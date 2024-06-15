#pragma once
#include <WinDef.h>
#include <WinUser.h>
class MFCUtil
{
public:
	void LvnColumnclickList(LPNMLISTVIEW pNMLV, LRESULT *pResult, PFNLVCOMPARE pfnCompare, CListCtrl* pListCtrl);
	/**
	 * search �������ַ���
	 * searchOld �ϴ��������ַ���
	 * list �����Ķ���
	 * colunmIndexs Ҫ������������
	 */
	void CListCtrlSearch(CString & search, CString & searchOld, CListCtrl & list, CUIntArray & colunmIndexs);

	 void CopyText(CString & source);

	/**
	 * columnsByNumberSort �������е�����������������,��������ĸ����
	 */
	static int CListCtrlCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort, CUIntArray* columnsByNumberSort)
	{
		CListCtrl* pListCtrl = (CListCtrl*)lParamSort;
		int m_current_select_isubitem = pListCtrl->m_nFlags - 1;
		CString strItem1 = pListCtrl->GetItemText(lParam1, m_current_select_isubitem);
		CString strItem2 = pListCtrl->GetItemText(lParam2, m_current_select_isubitem);
		LVCOLUMN Vol;
		CString csStr("");
		const UINT length = 50;
		TCHAR szCol[length];
		Vol.pszText = szCol;
		Vol.mask = LVCF_TEXT;
		Vol.cchTextMax = sizeof(szCol);
		pListCtrl->GetColumn(m_current_select_isubitem, &Vol);
		csStr = CString(Vol.pszText);
		bool sort_by_number = false;
		for(int i = 0; i < columnsByNumberSort->GetCount(); i++)
		{
			if(m_current_select_isubitem == columnsByNumberSort->GetAt(i))
			{
				sort_by_number = true;
				break;
			}
		}
		// ����������
		if(sort_by_number)
		{
			int n1 = _ttoi(strItem1);
			int n2 = _ttoi(strItem2);
			if(csStr.Right(1) == CString("��"))
			{
				return n1 > n2 ? -1 : 1;
			} else
			{
				return n1 > n2 ? 1 : -1;
			}
		}
		// ����ĸ����
		else
		{
			if(csStr.Right(1) == CString("��"))
			{
				return strItem2.CompareNoCase(strItem1);
			} else
			{
				return strItem1.CompareNoCase(strItem2);
			}
		}
	}
};

