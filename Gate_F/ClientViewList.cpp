﻿
#include "pch.h"
#include "GateF.h"
#include "framework.h"
#include "ClientViewList.h"

BEGIN_MESSAGE_MAP(CGateFDlgList, CViewList)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CGateFDlgList::OnNMCustomdraw)
	ON_NOTIFY_REFLECT(NM_DBLCLK, &CGateFDlgList::OnClientListCtrlDblClick)
    ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()


CGateFDlgList::CGateFDlgList()
{
	theApp.m_ObServerClientGroup.register_package_handler(OBPKG_ID_S2C_PUNISH_USER_UUID, [this](std::shared_ptr<CObserverClientImpl> client, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
		auto msg = raw_msg.get().as<ProtocolOBS2OBCPunishUserUUID>();
		m_suspicious_uuids.emplace(std::make_pair(msg.uuid, msg.gm_show));
	});
}

CGateFDlgList::~CGateFDlgList()
{
}

void CGateFDlgList::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
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
            if (!m.second) continue;
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

// 双击查看进程详细信息
void CGateFDlgList::OnClientListCtrlDblClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	// 获取当前被双击项的信息
	int nItem = pNMItemActivate->iItem;
	if (nItem != -1)
	{
		theApp.GetMainFrame()->m_games_dlg->OnQueryProcess();
	}
	*pResult = 0;
}