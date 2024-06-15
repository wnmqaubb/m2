#pragma once
#include "ViewList.h"
class CClientViewList :
    public CViewList
{
private:
    // 检测出用外挂的玩家的uuid
    std::map<std::wstring, bool> m_suspicious_uuids;
public:
    CClientViewList();
    ~CClientViewList();
    DECLARE_MESSAGE_MAP()
    afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
};

