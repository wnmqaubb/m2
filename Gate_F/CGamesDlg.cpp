// GamesDlg.cpp: 实现文件
//

#include "pch.h"
#include "GateF.h"
#include "afxdialogex.h"
#include "CGamesDlg.h"

// GamesDlg 对话框
IMPLEMENT_DYNAMIC(CGamesDlg, CDialogEx)

CGamesDlg::CGamesDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_GAMES, pParent)
{

}

CGamesDlg::~CGamesDlg()
{
}

void CGamesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_GAMES, m_list_games);
}


void CGamesDlg::OnCommand(UINT nID)
{

}

BEGIN_MESSAGE_MAP(CGamesDlg, CDialogEx)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(IDC_BUTTON_SEARCH, &CGamesDlg::OnBnClickedOnlineGamerSearch)
	ON_COMMAND(ID_EXIT_GAME, &CGamesDlg::OnExitGame)
	ON_COMMAND(ID_IP_BAN, &CGamesDlg::OnIpBan)
	ON_COMMAND(ID_MAC_BAN, &CGamesDlg::OnMacBan)
	ON_COMMAND(ID_ROLENAME_BAN, &CGamesDlg::OnRoleNameBan)
	ON_COMMAND(ID_IP_WHITE_ADD, &CGamesDlg::OnIpWhiteAdd)
	ON_COMMAND(ID_MAC_WHITE_ADD, &CGamesDlg::OnMacWhiteAdd)
	ON_COMMAND(ID_ROLENAME_WHITE_ADD, &CGamesDlg::OnRoleNameWhiteAdd)
	ON_BN_CLICKED(ID_REFRESH_USERS, &CGamesDlg::OnRefreshUsers)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

// GamesDlg 消息处理程序

void CGamesDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if (pWnd == &m_list_games)
	{
		CPoint clientPoint = point;
		m_list_games.ScreenToClient(&clientPoint);
		UINT flags;
		int item = m_list_games.HitTest(clientPoint, &flags);
		if (item != -1)
		{
			// 显示右键菜单
			CMenu menu;
			menu.LoadMenu(IDR_MENU_USERS_RIGHT);
			menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
		}
	}
}

BOOL CGamesDlg::PreTranslateMessage(MSG* pMsg)
{
	if (WM_KEYFIRST <= pMsg->message && pMsg->message <= WM_KEYLAST)
	{
		if (pMsg->wParam == VK_RETURN)
		{
			OnBnClickedOnlineGamerSearch();
			this->ShowWindow(SW_SHOW);
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

inline CString GetSystemDesc(int SysVer, bool is64bits)
{
	CString result;
	switch (SysVer)
	{
		case WINDOWS_ANCIENT:
			result = "Ancient";
			break;
		case WINDOWS_XP:
			result = "WinXp";
			break;
		case WINDOWS_SERVER_2003:
			result = "Win2003";
			break;
		case WINDOWS_VISTA:
			result = "Vista";
			break;
		case WINDOWS_7:
			result = "Win7";
			break;
		case WINDOWS_8:
			result = "Win8";
			break;
		case WINDOWS_8_1:
			result = "Win8.1";
			break;
		case WINDOWS_10:
			result = "Win10";
			break;
		case WINDOWS_NEW:
			result = "New";
			break;
		default:
			result = "Unknown";
			break;
	}
	result.Append(is64bits ? _T("(x64)") : _T("(x86)"));
	return result;
}

void CGamesDlg::OnRefreshUsers()
{
	static size_t szUserCount = 0;
	theApp.m_ObServerClientGroup.register_package_handler(OBPKG_ID_S2C_QUERY_USERS, [this](std::shared_ptr<CObserverClientImpl> client, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg)
		{
			auto msg = raw_msg.get().as<ProtocolOBS2OBCQueryUsers>();
			theApp.m_WorkIo.post([this, msg, client]() {
				client->set_user_count(0);
				client->session_ids().clear();
				CTime tCur(time(0));
				for (auto& user_data : msg.data)
				{
					auto json_data = user_data.second.json;
					if (json_data.find("is_observer_client") != json_data.end())
					{
						bool is_observer_client = json_data["is_observer_client"];
						if (is_observer_client)
						{
							continue;
						}
					}
					client->set_user_count(client->get_user_count() + 1);
					client->session_ids().emplace(user_data.second.session_id);
					szUserCount++;
					int rowNum = m_list_games.GetItemCount();
					int colIndex = 0;

					CString temp;
					temp.Format(_T("%d"), rowNum + 1);
					m_list_games.InsertItem(rowNum, _T(""));
					//////
					m_list_games.SetItemText(rowNum, colIndex++, temp);
					temp.Format(_T("%u"), user_data.second.session_id);
					m_list_games.SetItemText(rowNum, colIndex++, temp);
					std::wstring username = json_data.find("usrname") == json_data.end() ? TEXT("(NULL)") : json_data["usrname"];
					m_list_games.SetItemText(rowNum, colIndex++, username.c_str()); OutputDebugString(username.c_str());
					std::string ip = json_data["ip"];
					m_list_games.SetItemText(rowNum, colIndex++, CA2T(ip.c_str()));

					std::wstring cpuid = json_data["cpuid"];
					std::wstring mac = json_data["mac"];
					std::wstring vol = json_data["vol"];
					temp.Format(_T("%s|%s|%s"), cpuid.c_str(),
						mac.c_str(),
						vol.c_str());
					int sysver = json_data["sysver"];
					int is_64bits = json_data["64bits"];
					m_list_games.SetItemText(rowNum, colIndex++, temp);
					m_list_games.SetItemText(rowNum, colIndex++, GetSystemDesc(sysver, is_64bits));
					time_t tm = json_data["logintime"];
					CTime t(tm);
					m_list_games.SetItemText(rowNum, colIndex++, t.Format(TEXT("%Y-%m-%d %H:%M:%S")));
					auto tDelta = tCur - t;
					temp.Format(TEXT("%d"), tDelta.GetTotalMinutes());
					m_list_games.SetItemText(rowNum, colIndex++, temp);
				}
				theApp.GetMainFrame()->SetStatusBar(ID_INDICATOR_USERS_COUNT, std::to_wstring(szUserCount).c_str());
				});
		});
	m_list_games.DeleteAllItems();
	szUserCount = 0;
	ProtocolOBC2OBSQueryUsers req;
	theApp.m_ObServerClientGroup.send(&req);
}

void CGamesDlg::OnQueryProcess()
{
	ProtocolS2CQueryProcess msg;
	SendCurrentSelectedUserCommand(&msg);
}

void CGamesDlg::OnQueryDrivers()
{
	ProtocolS2CQueryDriverInfo msg;
	SendCurrentSelectedUserCommand(&msg);
}

void CGamesDlg::OnQueryShellCode()
{
	ProtocolS2CCheckPlugin msg;
	SendCurrentSelectedUserCommand(&msg);
}
void CGamesDlg::OnQueryScreenShot()
{
	ProtocolS2CQueryScreenShot msg;
	SendCurrentSelectedUserCommand(&msg);
}

void CGamesDlg::OnQueryWindows()
{
	ProtocolS2CQueryWindows msg;
	SendCurrentSelectedUserCommand(&msg);
}

void CGamesDlg::OnBnClickedOnlineGamerSearch()
{
	UpdateData(TRUE);
	CString text;
	GetDlgItemTextW(IDC_EDIT_SEARCH, text);
	if (text.GetLength() > 0)
	{
		m_list_games.CListCtrlSearch(text);
	}
}

template<typename T>
void CGamesDlg::SendCurrentSelectedUserCommand(T* package)
{
	auto selectedRow = (int)m_list_games.GetFirstSelectedItemPosition() - 1;
	if (selectedRow != -1)
	{
		const size_t session_id = atoi(CT2A(m_list_games.GetItemText(selectedRow, 1)));
		const std::string ip = CT2A(m_list_games.GetItemText(selectedRow, 11));
		const int port = atoi(CT2A(m_list_games.GetItemText(selectedRow, 12)));
		theApp.m_ObServerClientGroup(ip, port)->send(session_id, package);
	}
	else
	{
		AfxMessageBox(TEXT("请选择一个玩家!"), MB_OK);
	}
}

void CGamesDlg::OnExitGame()
{
	ProtocolS2CPunish req;
	req.type = PunishType::ENM_PUNISH_TYPE_KICK;
	SendCurrentSelectedUserCommand(&req);
}

void CGamesDlg::OnIpBan()
{
	auto selectedRow = (int)m_list_games.GetFirstSelectedItemPosition() - 1;
	CString ip;
	if (selectedRow != -1)
	{
		ip = m_list_games.GetItemText(selectedRow, 3);
	}
	else
	{
		return;
	}
	OnWhiteOrBlackAdd(ip, &(theApp.GetMainFrame()->m_anticheat_dlg->m_list_black_ip));
}

void CGamesDlg::OnMacBan()
{
	auto selectedRow = (int)m_list_games.GetFirstSelectedItemPosition() - 1;
	CString strMac;
	if (selectedRow != -1)
	{
		strMac = m_list_games.GetItemText(selectedRow, 4);
	}
	else
	{
		return;
	}
	OnWhiteOrBlackAdd(strMac, &(theApp.GetMainFrame()->m_anticheat_dlg->m_list_black_machine));
}

void CGamesDlg::OnRoleNameBan()
{
	auto selectedRow = (int)m_list_games.GetFirstSelectedItemPosition() - 1;
	CString str_rolename;
	if (selectedRow != -1)
	{
		str_rolename = m_list_games.GetItemText(selectedRow, 2);
		int nPos = str_rolename.Find(_T(" - "));
		if (nPos != -1)
		{
			str_rolename = str_rolename.Mid(nPos + 3);
		}
	}
	else
	{
		return;
	}
	OnWhiteOrBlackAdd(str_rolename, &(theApp.GetMainFrame()->m_anticheat_dlg->m_list_black_rolename));
}

void CGamesDlg::OnIpWhiteAdd()
{
	auto selectedRow = (int)m_list_games.GetFirstSelectedItemPosition() - 1;
	CString strIP;
	if (selectedRow != -1)
	{
		strIP = m_list_games.GetItemText(selectedRow, 3);
	}
	else
	{
		return;
	}

	OnWhiteOrBlackAdd(strIP, &(theApp.GetMainFrame()->m_anticheat_dlg->m_list_white_ip));
}

void CGamesDlg::OnMacWhiteAdd()
{
	auto selectedRow = (int)m_list_games.GetFirstSelectedItemPosition() - 1;
	CString strMac;
	if (selectedRow != -1)
	{
		strMac = m_list_games.GetItemText(selectedRow, 4);
	}
	else
	{
		return;
	}
	OnWhiteOrBlackAdd(strMac, &(theApp.GetMainFrame()->m_anticheat_dlg->m_list_white_machine));
}

void CGamesDlg::OnRoleNameWhiteAdd()
{
	auto selectedRow = (int)m_list_games.GetFirstSelectedItemPosition() - 1;
	CString str_rolename;
	if (selectedRow != -1)
	{
		str_rolename = m_list_games.GetItemText(selectedRow, 2);
		int nPos = str_rolename.Find(_T(" - "));
		if (nPos != -1)
		{
			str_rolename = str_rolename.Mid(nPos + 3);
		}
	}
	else
	{
		return;
	}

	OnWhiteOrBlackAdd(str_rolename, &(theApp.GetMainFrame()->m_anticheat_dlg->m_list_white_rolename));
}

void CGamesDlg::OnWhiteOrBlackAdd(const CString& list_name, CListBox* m_current_list)
{
	auto mainFrame = theApp.GetMainFrame();
	mainFrame->m_anticheat_dlg->m_current_list = m_current_list;
	mainFrame->m_anticheat_dlg->OnAddToList(list_name);
	mainFrame->SwitchToTab(1);
	mainFrame->m_anticheat_dlg->ShowWindow(SW_SHOW);
}

HBRUSH CGamesDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	if (IDC_EXPDATE_STATIC == pWnd->GetDlgCtrlID())
	{
		pDC->SetTextColor(RGB(0x95, 0x3C, 0x97));
	}
	return hbr;
}
