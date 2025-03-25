#pragma once
#include "ViewList.h"
#include <map>
class CGateFDlgList :
    public CViewList
{
private:
    // 检测出用外挂的玩家的uuid
    std::map<std::wstring, bool> m_suspicious_uuids;
public:
    CGateFDlgList();
    ~CGateFDlgList();
    DECLARE_MESSAGE_MAP()
    afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
    void OnClientListCtrlDblClick(NMHDR* pNMHDR, LRESULT* pResult);
};

