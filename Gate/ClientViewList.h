#pragma once
#include "ViewList.h"
#include <functional>

class CClientViewList :
    public CViewList
{
private:
    // ��������ҵ���ҵ�uuid
    std::map<std::wstring, bool> m_suspicious_uuids;
    afx_msg virtual void OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult);
    DECLARE_MESSAGE_MAP()
public:
    CClientViewList();
    ~CClientViewList();
    afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
    std::function<void()> OnDoubleClick; // �ص�����
    afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
};

