<<<<<<< HEAD
#pragma once
#include "ViewList.h"
class CClientViewList :
    public CViewList
{
private:
    // ��������ҵ���ҵ�uuid
    std::map<std::wstring, bool> m_suspicious_uuids;
public:
    CClientViewList();
    ~CClientViewList();
    DECLARE_MESSAGE_MAP()
    afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
};

=======
#pragma once
#include "ViewList.h"
class CClientViewList :
    public CViewList
{
private:
    // ��������ҵ���ҵ�uuid
    std::map<std::wstring, bool> m_suspicious_uuids;
public:
    CClientViewList();
    ~CClientViewList();
    DECLARE_MESSAGE_MAP()
    afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
};

>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9
