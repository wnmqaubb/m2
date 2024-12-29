// CPoliceDlg.cpp: 实现文件
//

#include "pch.h"
#include "GateF.h"
#include "CPoliceDlg.h"
#include "Service/Ini_tool.h"


// CPoliceDlg 对话框

IMPLEMENT_DYNAMIC(CPoliceDlg, CDialogEx)

CPoliceDlg::CPoliceDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_POLICES, pParent)
	, m_check_base_policy(TRUE)
	, m_check_better_policy(TRUE)
	, m_check_best_policy(FALSE)
	, m_policy_detect_interval(0)
{

}

CPoliceDlg::~CPoliceDlg()
{
}

void CPoliceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_POLICES, m_list_polices);
	DDX_Check(pDX, IDC_CHECK_BASE_POLICY, m_check_base_policy);
	DDX_Check(pDX, IDC_CHECK_BETTER_POLICY, m_check_better_policy);
	DDX_Check(pDX, IDC_CHECK_BEST_POLICY, m_check_best_policy);
	DDX_Text(pDX, IDC_EDIT_POLICY_DETECT_INTERVAL, m_policy_detect_interval);
	DDV_MinMaxInt(pDX, m_policy_detect_interval, 1, 10);
	DDX_Control(pDX, IDC_SPIN_POLICY_DETECT_INTERVAL, m_spin_detect_interval);
	m_spin_detect_interval.SetRange(1, 10);
	m_spin_detect_interval.SetBuddy(GetDlgItem(IDC_EDIT_POLICY_DETECT_INTERVAL));
}


BEGIN_MESSAGE_MAP(CPoliceDlg, CDialogEx)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(NM_CLICK, IDC_LIST_POLICES, &CPoliceDlg::OnNMClickListPolices)
	ON_EN_KILLFOCUS(IDC_LIST_EDIT, &CPoliceDlg::OnEnKillfocusEditCtrl)
	ON_CBN_KILLFOCUS(IDC_LIST_COMBOBOX_POLICY, &CPoliceDlg::OnCbnKillfocusComboPolicyType)
	ON_CBN_KILLFOCUS(IDC_LIST_COMBOBOX_PUNISH, &CPoliceDlg::OnCbnKillfocusComboPunishType)
	ON_COMMAND(ID_CFG_ADD, &CPoliceDlg::OnConfigAdd)
	ON_COMMAND(ID_CFG_DEL, &CPoliceDlg::OnConfigDel)
	ON_COMMAND(ID_CFG_SAVE, &CPoliceDlg::OnConfigSave)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_POLICY_COMMIT, &CPoliceDlg::OnConfigSave)
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, &CPoliceDlg::OnBnClickedRefreshPolicy)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_POLICY_DETECT_INTERVAL, &CPoliceDlg::OnDeltaposSpin1)
END_MESSAGE_MAP()

BOOL CPoliceDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	InitConfigSettingView();
	// 填充策略类型下拉框
	m_combo_policy_type.Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, CRect(0, 0, 0, 0), this, IDC_LIST_COMBOBOX_POLICY);
	int nIndex = 0;
	for (int index = ENM_POLICY_TYPE_MODULE_NAME; index < ENM_POLICY_TYPE_MAX; index++)
	{
		if (index == ENM_POLICY_TYPE_BACK_GAME ||
			index == ENM_POLICY_TYPE_EXIT_GAME ||
			index == ENM_POLICY_TYPE_ACTION_SPEED_WALK ||
			index == ENM_POLICY_TYPE_ACTION_SPEED_HIT ||
			index == ENM_POLICY_TYPE_ACTION_SPEED_SPELL
			) continue;
		if (index == ENM_POLICY_TYPE_SCRIPT || index == ENM_POLICY_TYPE_THREAD_START) continue;
		m_combo_policy_type.InsertString(nIndex++, ConvertToString((PolicyType)index));
	}
	m_combo_policy_type.SetCurSel(ENM_POLICY_TYPE_MODULE_NAME);

	// 填充处罚类型下拉框
	m_combo_punish_type.Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, CRect(0, 0, 0, 0), this, IDC_LIST_COMBOBOX_PUNISH);
	RefreshViewList();
	m_policy_detect_interval = IniTool::read_ini<int>(".\\jishiyu.ini", "Gate", "Policy_Detect_Interval", 3);
	m_spin_detect_interval.SetPos(m_policy_detect_interval);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CPoliceDlg::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
	CMenu menu;
	menu.LoadMenu(IDR_SETING_RIGHT_MENU);
	CMenu* pSumMenu = menu.GetSubMenu(0);
	theApp.GetContextMenuManager()->ShowPopupMenu(*pSumMenu, point.x, point.y, this, TRUE);
}

void CPoliceDlg::InitConfigSettingView()
{
	//m_list_polices.SetColumnByIntSort({ 0, 1, 2 });
	m_list_polices.SetRedraw(FALSE);
	m_list_polices.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_list_polices.ModifyStyle(0, LVS_REPORT | LVS_EDITLABELS);
	int colIndex = 0;
	m_list_polices.InsertColumn(colIndex++, TEXT("序号"), LVCFMT_LEFT, 38);
	m_list_polices.InsertColumn(colIndex++, TEXT("策略ID"), LVCFMT_LEFT, 50);
	m_list_polices.InsertColumn(colIndex++, TEXT("策略类型"), LVCFMT_LEFT, 80);
	m_list_polices.InsertColumn(colIndex++, TEXT("处理类型"), LVCFMT_LEFT, 110);
	m_list_polices.InsertColumn(colIndex++, TEXT("配置"), LVCFMT_LEFT, 470);
	m_list_polices.InsertColumn(colIndex++, TEXT("备注"), LVCFMT_LEFT, 150);
	m_list_polices.InsertColumn(colIndex++, TEXT("是否管理员创建"), LVCFMT_LEFT, 0);
	m_list_polices.SetRedraw(TRUE);
}

void CPoliceDlg::RefreshViewList()
{
	m_list_polices.SetRedraw(FALSE);
	int colIndex = 0;
	m_list_polices.DeleteAllItems();
	CString connect_id_str, seq;
	const wchar_t* format_d = L"%d";
	const wchar_t* format_3d = L"%03d";
	const wchar_t* empty_wstr = L"";
	int rowNum = 0;
	for (auto [uiPolicyId, Policy] : theApp.m_cfg->policies)
	{
		if (688000 >= uiPolicyId || uiPolicyId > 689050
			|| Policy.policy_type == ENM_POLICY_TYPE_SCRIPT
			|| Policy.policy_type == ENM_POLICY_TYPE_THREAD_START)
		{
			continue;
		}
		if (rowNum % 100 == 0) Sleep(1);
		colIndex = 0;
		CString id;
		CString temp;
		id.Format(format_d, rowNum + 1);
		m_list_polices.InsertItem(rowNum, empty_wstr);
		m_list_polices.SetItemText(rowNum, colIndex++, id);
		temp.Format(format_d, uiPolicyId);
		m_list_polices.SetItemText(rowNum, colIndex++, temp);
		m_list_polices.SetItemText(rowNum, colIndex++, ConvertToString((PolicyType)Policy.policy_type));
		m_list_polices.SetItemText(rowNum, colIndex++, ConvertToString((PunishType)Policy.punish_type));
		m_list_polices.SetItemText(rowNum, colIndex++, Policy.config.c_str());
		m_list_polices.SetItemText(rowNum, colIndex++, Policy.comment.c_str());
		temp.Format(format_d, Policy.create_by_admin);
		m_list_polices.SetItemText(rowNum, colIndex++, temp);
		rowNum++;
	}
	m_list_polices.SetRedraw(TRUE);
}

void CPoliceDlg::OnInitialUpdate()
{
	CPoliceDlg::OnInitialUpdate();
	//GetDocument()->SetView(this);
	RefreshViewList();
}


void CPoliceDlg::OnSize(UINT nType, int cx, int cy)
{
	CPoliceDlg::OnSize(nType, cx, cy);
	AdjustLayout();
}


void CPoliceDlg::AdjustLayout()
{
	if (GetSafeHwnd() == nullptr)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	m_list_polices.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}

void CPoliceDlg::OnListItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

}

uint32_t CPoliceDlg::next_gm_policy_id(std::map<uint32_t, ProtocolPolicy>& policies) {
	uint32_t policy_id = GATE_POLICY_ID + 1;

	while (policies.find(policy_id) != policies.end()) {
		policy_id++;
		if (policy_id >= GATE_ADMIN_POLICY_ID) {
			break;
		}
	}
	return policy_id;
}

void CPoliceDlg::OnConfigAdd()
{
	OnConfigSave();
	theApp.OpenConfig();
	unsigned int uiLastPolicyId = next_gm_policy_id(theApp.m_cfg->policies);
	ProtocolPolicy NewPolicy;
	NewPolicy.policy_id = uiLastPolicyId;
	theApp.m_cfg->policies[uiLastPolicyId] = NewPolicy;
	RefreshViewList();
	OnSelectItem(uiLastPolicyId);
}

void CPoliceDlg::OnSelectItem(int policy_id) {
	for (int i = 0; i < m_list_polices.GetItemCount(); i++)
	{
		CString cstrPolicyId = m_list_polices.GetItemText(i, 1);
		uint32_t uiPolicyId = _ttoi(cstrPolicyId);
		if (uiPolicyId == policy_id) {
			m_list_polices.SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			m_list_polices.EnsureVisible(i, FALSE);
			m_list_polices.SetFocus();
			return;
		}
	}
}

void CPoliceDlg::OnConfigDel()
{
	int selected_up;
	auto selectedRow = (int)m_list_polices.GetFirstSelectedItemPosition() - 1;
	if (selectedRow != -1)
	{
		CString cstrPolicyId = m_list_polices.GetItemText(selectedRow, 1);
		uint32_t uiPolicyId = _ttoi(cstrPolicyId);
		theApp.OpenConfig();
		theApp.m_cfg->policies.erase(uiPolicyId);
		if (selectedRow > 1) {
			CString cstrPolicyId1 = m_list_polices.GetItemText(selectedRow-1, 1);
			selected_up = _ttoi(cstrPolicyId1);
		}
		RefreshViewList();

	}
	OnConfigSave();
	OnSelectItem(selected_up);
}

void CPoliceDlg::OnConfigSave()
{
	ProtocolS2CPolicy cfg;
	for (int i = 0; i < m_list_polices.GetItemCount(); i++)
	{
		CString cstrPolicyId = m_list_polices.GetItemText(i, 1);
		uint32_t uiPolicyId = _ttoi(cstrPolicyId);
		CString policy_type_str = m_list_polices.GetItemText(i, 2);
		PolicyType policy_type = ConvertToPolicyType(policy_type_str);
		CString punish_type_str = m_list_polices.GetItemText(i, 3);
		PunishType punish_type = ConvertToPunishType(punish_type_str);
		CString config_str = m_list_polices.GetItemText(i, 4);
		CString comment_str = m_list_polices.GetItemText(i, 5);
		CString create_by_admin_str = m_list_polices.GetItemText(i, 6);
		bool create_by_admin = _ttoi(create_by_admin_str);
		ProtocolPolicy policy;
		policy.policy_id = uiPolicyId;
		policy.policy_type = policy_type;
		policy.punish_type = punish_type;
		policy.config = config_str;
		policy.comment = comment_str;
		policy.create_by_admin = create_by_admin;
		cfg.policies[uiPolicyId] = policy;
	}
	theApp.m_cfg = std::make_unique<ProtocolS2CPolicy>(std::move(cfg));
	theApp.SaveConfig();
}

void CPoliceDlg::OnNMClickListPolices(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int itemIndex = pNMItemActivate->iItem;
	int subItemIndex = pNMItemActivate->iSubItem;

	if (itemIndex >= 0 && subItemIndex >= 0) {
		EditCell(itemIndex, subItemIndex);
	}
	
	*pResult = 0;
}

CFont* CPoliceDlg::GetCellFont(int nRow, int nCol)
{
	LVITEM lvi;
	lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE | LVIF_IMAGE | LVIF_INDENT | LVIF_GROUPID | LVIF_COLUMNS;
	lvi.iItem = nRow;
	lvi.iSubItem = nCol;
	lvi.pszText = nullptr;
	lvi.cchTextMax = 0;

	if (m_list_polices.GetItem(&lvi)) {
		if (lvi.pszText != nullptr) {
			// 如果指定了字体，返回指定的字体
			CFont* pFont = reinterpret_cast<CFont*>(lvi.lParam);
			//CFont* pFont = dynamic_cast<CFont*>(lvi.lParam);
			if (pFont != nullptr && pFont->GetSafeHandle() != nullptr) {
				return pFont;
			}
		}
	}

	// 返回默认字体
	return m_list_polices.GetFont();
}

void CPoliceDlg::EditCell(int nRow, int nCol)
{
	m_nCurSelRow = nRow;
	m_nCurSelCol = nCol;
	CFont* pOldFont;
	CRect rect;
	int nLength,lineHeight,index;
	m_list_polices.GetSubItemRect(nRow, nCol, LVIR_BOUNDS, rect);

	// 将列表控件的客户区坐标转换为对话框的客户区坐标
	m_list_polices.ClientToScreen(&rect);
	ScreenToClient(&rect);

	// 获取单元格的字体
	CFont* pCellFont = GetCellFont(nRow, nCol);

	switch (nCol) {
		case 1: // 文本框 
		case 4: // 文本框
		case 5: // 文本框
			if (m_editCtrl.m_hWnd) {
				m_editCtrl.DestroyWindow();
			}
			// 创建编辑控件
			if (nCol == 4) {// 多行文本框
				/*CDC dc;
				dc.CreateCompatibleDC(NULL);
				CFont* pOldFont = dc.SelectObject(pCellFont);

				TEXTMETRIC tm;
				dc.GetTextMetrics(&tm);
				lineHeight = tm.tmHeight + tm.tmExternalLeading;

				dc.SelectObject(pOldFont);
				dc.DeleteDC();*/

				//return lineHeight * 5;
				//rect.bottom = rect.top + lineHeight * 5;
				m_editCtrl.Create(WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_MULTILINE | ES_WANTRETURN | WS_EX_CLIENTEDGE, rect, this, IDC_LIST_EDIT);
			}
			else {
				m_editCtrl.Create(WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, rect, this, IDC_LIST_EDIT);
			}
			m_editCtrl.SetWindowText(m_list_polices.GetItemText(nRow, nCol));
			// 应用单元格的字体
			pOldFont = m_editCtrl.GetFont();
			m_editCtrl.SetFont(pCellFont);
			if (pOldFont != nullptr) {
				delete pOldFont;
			}
			// 将光标设置到文本的末尾
			nLength = m_editCtrl.GetWindowTextLength();
			m_editCtrl.SetSel(nLength, nLength);
			// 设置边框颜色
			//SetControlBorderColor(&m_editCtrl, RGB(0, 0, 255));
			m_editCtrl.SetFocus(); 
			m_editCtrl.BringWindowToTop();
			break;
		case 2: // 策略类型下拉框
			m_combo_policy_type.MoveWindow(rect);
			index = m_combo_policy_type.FindString(-1, m_list_polices.GetItemText(nRow, nCol)); 
			if (index != -1)
			{
				m_combo_policy_type.SetCurSel(index);
			}
			m_combo_policy_type.ShowWindow(SW_SHOW);
			// 应用单元格的字体
			m_combo_policy_type.SetFont(pCellFont);
			m_combo_policy_type.SetFocus();
			m_combo_policy_type.BringWindowToTop();
			break;
		case 3: // 处罚类型下拉框			
			m_combo_punish_type.MoveWindow(rect);
			OnCbnDropdownComboPunishType(m_list_polices.GetItemText(nRow, 2));
			index = m_combo_punish_type.FindString(-1, m_list_polices.GetItemText(nRow, nCol));
			if (index != -1)
			{
				m_combo_punish_type.SetCurSel(index);
			}
			m_combo_punish_type.ShowWindow(SW_SHOW);
			// 应用单元格的字体
			m_combo_punish_type.SetFont(pCellFont);
			m_combo_punish_type.SetFocus();
			m_combo_punish_type.BringWindowToTop();
			break;
	}
}

void CPoliceDlg::OnEnKillfocusEditCtrl()
{
    CString strText;
    m_editCtrl.GetWindowText(strText);

    if (m_nCurSelRow >= 0 && m_nCurSelCol >= 0) {
        // 当是多开策略时，需要判断配置的值是否为大于0的整数
        if (m_nCurSelCol == 4) {
            CString policy_type_str = m_list_polices.GetItemText(m_nCurSelRow, 2);
			if (!policy_type_str.IsEmpty() && ConvertToPolicyType(policy_type_str) == ENM_POLICY_TYPE_MULTICLIENT) {
                // 判断配置的值是否为大于0的整数
                int value = _ttoi(strText);
                if (value <= 0) {
                    AfxMessageBox(L"多开限制配置值必须大于0的整数");
                    return;
                }
            }
        }
        m_list_polices.SetItemText(m_nCurSelRow, m_nCurSelCol, strText);
    }

    // 销毁编辑控件
    m_editCtrl.ShowWindow(SW_HIDE);
}

void CPoliceDlg::OnCbnKillfocusComboPunishType()
{
	CString strText;
	m_combo_punish_type.GetLBText(m_combo_punish_type.GetCurSel(), strText);
	if (m_nCurSelRow >= 0 && m_nCurSelCol >= 0) {
		m_list_polices.SetItemText(m_nCurSelRow, m_nCurSelCol, strText);
	}
	m_combo_punish_type.ShowWindow(SW_HIDE);
}

void CPoliceDlg::OnCbnKillfocusComboPolicyType()
{
	CString strText;
	m_combo_policy_type.GetLBText(m_combo_policy_type.GetCurSel(), strText);
	if (m_nCurSelRow >= 0 && m_nCurSelCol >= 0) {
		m_list_polices.SetItemText(m_nCurSelRow, m_nCurSelCol, strText);
	}
	m_combo_policy_type.ShowWindow(SW_HIDE);
}

void CPoliceDlg::SetControlBorderColor(CWnd* pCtrl, COLORREF color)
{
	pCtrl->ModifyStyleEx(0, WS_EX_CLIENTEDGE); // 添加边框样式
	pCtrl->Invalidate(); // 重绘控件

	// 重绘控件以设置边框颜色
	//pCtrl->OnPaint();
	CPaintDC dc(pCtrl);
	CRect rect;
	pCtrl->GetClientRect(&rect);
	dc.Draw3dRect(&rect, color, color);
}

void CPoliceDlg::OnCbnDropdownComboPunishType(CString policy_type)
{
	std::map<PolicyType, std::vector<PunishType>> config = {
		{ENM_POLICY_TYPE_MODULE_NAME,{ENM_PUNISH_TYPE_KICK,ENM_PUNISH_TYPE_NO_OPEARATION,ENM_PUNISH_TYPE_BAN_MACHINE}},
		{ENM_POLICY_TYPE_PROCESS_NAME,{ENM_PUNISH_TYPE_KICK,ENM_PUNISH_TYPE_NO_OPEARATION,ENM_PUNISH_TYPE_BAN_MACHINE}},
		{ENM_POLICY_TYPE_PROCESS_NAME_AND_SIZE,{ENM_PUNISH_TYPE_KICK,ENM_PUNISH_TYPE_NO_OPEARATION,ENM_PUNISH_TYPE_BAN_MACHINE}},
		{ENM_POLICY_TYPE_FILE_NAME,{ENM_PUNISH_TYPE_KICK,ENM_PUNISH_TYPE_NO_OPEARATION,ENM_PUNISH_TYPE_BAN_MACHINE}},
		{ENM_POLICY_TYPE_WINDOW_NAME,{ENM_PUNISH_TYPE_KICK,ENM_PUNISH_TYPE_NO_OPEARATION,ENM_PUNISH_TYPE_BAN_MACHINE}},
		//{ENM_POLICY_TYPE_MACHINE,{ENM_PUNISH_TYPE_KICK,ENM_PUNISH_TYPE_NO_OPEARATION,ENM_PUNISH_TYPE_BAN_MACHINE,ENM_PUNISH_TYPE_SUPER_WHITE_LIST}},
		{ENM_POLICY_TYPE_MULTICLIENT,{ENM_PUNISH_TYPE_KICK,ENM_PUNISH_TYPE_NO_OPEARATION,ENM_PUNISH_TYPE_BAN_MACHINE}},
		{ENM_POLICY_TYPE_SHELLCODE,{ENM_PUNISH_TYPE_KICK,ENM_PUNISH_TYPE_NO_OPEARATION,ENM_PUNISH_TYPE_BAN_MACHINE}},
	};
	int last_punish_type = CB_ERR;
	bool last_sel_flag = false;
	CString punish_type;
	if (m_combo_punish_type.GetCurSel() != CB_ERR)
	{
		m_combo_punish_type.GetLBText(m_combo_punish_type.GetCurSel(), punish_type);
		last_punish_type = ConvertToPunishType(punish_type);
	}

	m_combo_punish_type.ResetContent();
	int nIndex = 0;
	for (auto punish_type : config[ConvertToPolicyType(policy_type)])
	{
		int index = m_combo_punish_type.InsertString(nIndex++, ConvertToString(punish_type));
		//AddPunishTypeToMap(punish_type, index);
		m_combo_punish_type.SetItemData(index, punish_type);
		if (punish_type == last_punish_type)
		{
			last_sel_flag = true;
		}
	}

	/*if (last_sel_flag)
	{
		m_combo_punish_type.SetCurSel(GetPunishTypeListIndex((PunishType)last_punish_type));
	}
	else
	{
		m_combo_punish_type.SetCurSel(GetPunishTypeListIndex(ENM_PUNISH_TYPE_NO_OPEARATION));

	}*/
	m_combo_punish_type.UpdateData(TRUE);
}

HBRUSH CPoliceDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_STATIC)
	{
		pDC->SetTextColor(RGB(250, 0, 0));
	}
	return hbr;
}

void CPoliceDlg::OnBnClickedRefreshPolicy()
{
	theApp.OpenConfig();
	RefreshViewList();
}


void CPoliceDlg::OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// 根据旋转按钮的增减方向来改变整型变量的值
	if (pNMUpDown->iDelta > 0)
	{
		if (m_policy_detect_interval < 10)
		{
			m_policy_detect_interval++;
		}
		else {
			m_policy_detect_interval = 10;
		}
	}
	else
	{
		if (m_policy_detect_interval > 1)
		{
			m_policy_detect_interval--;  // 向下调整（减少1）
		}else {
			m_policy_detect_interval = 1;
		}
	}
	//m_spin_detect_interval.SetPos(m_policy_detect_interval);
	IniTool::write_ini<int>(".\\jishiyu.ini", "Gate", "Policy_Detect_Interval", m_policy_detect_interval);
	UpdateData(FALSE);  // 将改变后的变量值更新显示到文本框中
	*pResult = 0;
}
