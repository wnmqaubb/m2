
#include "pch.h"
#include "Gate.h"
#include "framework.h"
#include "ClientViewList.h"
BEGIN_MESSAGE_MAP(CClientViewList, CViewList)
    ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CClientViewList::OnNMCustomdraw)
    ON_NOTIFY_REFLECT(NM_DBLCLK, &CClientViewList::OnNMDblclk)
    ON_NOTIFY_REFLECT(LVN_GETDISPINFO, &CClientViewList::OnGetDispInfo)
END_MESSAGE_MAP()


CClientViewList::CClientViewList()
{
    theApp.m_ObServerClientGroup.register_package_handler(OBPKG_ID_S2C_PUNISH_USER_UUID, [this](std::shared_ptr<CObserverClientImpl> client, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
        auto msg = raw_msg.get().as<ProtocolOBS2OBCPunishUserUUID>();
        m_suspicious_uuids.emplace(std::make_pair(msg.uuid, msg.gm_show));
    });
}

CClientViewList::~CClientViewList()
{
}

void CClientViewList::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMLVCUSTOMDRAW pLVCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);
    *pResult = CDRF_DODEFAULT;
    if (CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
    {
        *pResult = CDRF_NOTIFYITEMDRAW;
    }
    else if (CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
    {
        //处理，将item改变背景颜色   
        for (auto& m : m_suspicious_uuids)
        {
#ifndef GATE_ADMIN
            if (!m.second) continue;
#endif
            std::wstring uuid = this->GetItemText(pLVCD->nmcd.dwItemSpec, 7);
            if (m.first == uuid)
            {   //当前选中的item   
                pLVCD->clrTextBk = RGB(235, 0, 0);
            }
        }
    }
    *pResult = 0;
    *pResult |= CDRF_NOTIFYPOSTPAINT;
    *pResult |= CDRF_NOTIFYITEMDRAW;
    *pResult |= CDRF_NOTIFYSUBITEMDRAW;
}

void CClientViewList::OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

    // 获取双击的行和列
    int nItem = pNMItemActivate->iItem;
    int nSubItem = pNMItemActivate->iSubItem;

    if (nItem >= 0) // 确保双击的是有效行
    {
        if (!OnDoubleClick) {
            return;
        }
        if (OnDoubleClick) OnDoubleClick(); // 调用父窗口方法
    }

    *pResult = 0;
}

void CClientViewList::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
    LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

    if (pDispInfo->item.iItem >= static_cast<int>(CClientView::g_allUserData.size()))
    {
        *pResult = 0;
        return;
    }

    const UserData& ud = *CClientView::g_allUserData[pDispInfo->item.iItem];

    if (pDispInfo->item.mask & LVIF_TEXT)
    {
        CString temp;
        switch (pDispInfo->item.iSubItem)
        {
            case 0: temp.Format(_T("%d"), pDispInfo->item.iItem + 1); break;
            case 1: temp.Format(_T("%u"), ud.session_id); break;
            case 2: _tcscpy_s(pDispInfo->item.pszText, pDispInfo->item.cchTextMax, ud.username.c_str()); return;
            case 3: _tcscpy_s(pDispInfo->item.pszText, pDispInfo->item.cchTextMax, CA2T(ud.ip.c_str())); return;
            case 4: temp.Format(_T("%s|%s|%s"), ud.cpuid.c_str(), ud.mac.c_str(), ud.vol.c_str()); break;
            case 5: _tcscpy_s(pDispInfo->item.pszText, pDispInfo->item.cchTextMax, GetSystemDesc(ud.sysver, ud.is_64bits)); return;
            case 6: _tcscpy_s(pDispInfo->item.pszText, pDispInfo->item.cchTextMax, CA2T(ud.commit_ver.c_str())); return;
            case 7: _tcscpy_s(pDispInfo->item.pszText, pDispInfo->item.cchTextMax, CA2T(ud.get_uuid_str().c_str())); return;
            case 8: temp.Format(TEXT("%d"), ud.pid); break;
            case 9:
            {
                CTime t(ud.logintime);
                _tcscpy_s(pDispInfo->item.pszText, pDispInfo->item.cchTextMax, t.Format(TEXT("%Y-%m-%d %H:%M:%S")));
                return;
            }
            case 10: temp.Format(TEXT("%d"), (CTime(time(0)) - CTime(ud.logintime)).GetTotalMinutes()); break;
            case 11: _tcscpy_s(pDispInfo->item.pszText, pDispInfo->item.cchTextMax, CA2T(ud.client_address.c_str())); return;
            case 12: _tcscpy_s(pDispInfo->item.pszText, pDispInfo->item.cchTextMax, CA2T(ud.client_port.c_str())); return;
            case 13:
            {
                long long mins = (CTime(time(0)) - CTime(ud.logintime)).GetTotalMinutes();
                float miss_rate = mins == 0 ? 0 : ((float)ud.miss_count / (float)mins);
                temp.Format(TEXT("%f"), miss_rate);
                break;
            }
        }
        _tcscpy_s(pDispInfo->item.pszText, pDispInfo->item.cchTextMax, temp);
    }
    *pResult = 0;
}