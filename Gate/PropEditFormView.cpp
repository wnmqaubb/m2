// PropEditFormView.cpp: 实现文件
//

#include "pch.h"
#include "Gate.h"
#include "PropEditFormView.h"

#include "ConfigSettingSubView.h"
#include "ConfigSettingChildFrm.h"
#include "ConfigSettingDoc.h"
#include "ConfigSettingView.h"
// PropEditFormView

IMPLEMENT_DYNCREATE(CPropEditFormView, CFormView)

CPropEditFormView::CPropEditFormView()
	: CFormView(IDD_PROP_EDIT_VIEW)
{

}

CPropEditFormView::~CPropEditFormView()
{
}


void CPropEditFormView::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_POLICY_TYPE, m_PolicyTypeComboBox);
    DDX_Control(pDX, IDC_COMBO_PUNISH_TYPE, m_PunishTypeComboBox);
    DDX_Control(pDX, IDC_EDIT_POLICY_ID, m_PolicyIdEdit);
    DDX_Control(pDX, IDC_EDIT_POLICY_CONFIG, m_PolicyConfigEdit);
    DDX_Control(pDX, IDC_EDIT_POLICY_COMMENT, m_PolicyCommentEdit);
}

BEGIN_MESSAGE_MAP(CPropEditFormView, CFormView)
    ON_WM_CREATE()
    ON_CBN_DROPDOWN(IDC_COMBO_POLICY_TYPE, &CPropEditFormView::OnCbnDropdownComboPolicyType)
    ON_CBN_DROPDOWN(IDC_COMBO_PUNISH_TYPE, &CPropEditFormView::OnCbnDropdownComboPunishType)
    ON_BN_CLICKED(IDC_BUTTON_POLICY_COMMIT, &CPropEditFormView::OnBnClickedButtonPolicyCommit)
END_MESSAGE_MAP()

// PropEditFormView 诊断

#ifdef _DEBUG
void CPropEditFormView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CPropEditFormView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

#endif
#endif //_DEBUG


// PropEditFormView 消息处理程序


int CPropEditFormView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CFormView::OnCreate(lpCreateStruct) == -1)
        return -1;

    return 0;
}

void CPropEditFormView::OnCbnDropdownComboPunishType()
{
    std::map<PolicyType, std::vector<PunishType>> config = {
        {ENM_POLICY_TYPE_MODULE_NAME,{ENM_PUNISH_TYPE_KICK,ENM_PUNISH_TYPE_NO_OPEARATION,ENM_PUNISH_TYPE_BAN_MACHINE,ENM_PUNISH_TYPE_SCREEN_SHOT,ENM_PUNISH_TYPE_SCREEN_SHOT_KICK}},
        {ENM_POLICY_TYPE_PROCESS_NAME,{ENM_PUNISH_TYPE_KICK,ENM_PUNISH_TYPE_NO_OPEARATION,ENM_PUNISH_TYPE_BAN_MACHINE,ENM_PUNISH_TYPE_SCREEN_SHOT,ENM_PUNISH_TYPE_SCREEN_SHOT_KICK}},
        {ENM_POLICY_TYPE_FILE_NAME,{ENM_PUNISH_TYPE_KICK,ENM_PUNISH_TYPE_NO_OPEARATION,ENM_PUNISH_TYPE_BAN_MACHINE,ENM_PUNISH_TYPE_SCREEN_SHOT,ENM_PUNISH_TYPE_SCREEN_SHOT_KICK}},
        {ENM_POLICY_TYPE_WINDOW_NAME,{ENM_PUNISH_TYPE_KICK,ENM_PUNISH_TYPE_NO_OPEARATION,ENM_PUNISH_TYPE_BAN_MACHINE,ENM_PUNISH_TYPE_SCREEN_SHOT,ENM_PUNISH_TYPE_SCREEN_SHOT_KICK}},
        {ENM_POLICY_TYPE_MACHINE,{ENM_PUNISH_TYPE_KICK,ENM_PUNISH_TYPE_NO_OPEARATION,ENM_PUNISH_TYPE_SUPER_WHITE_LIST,ENM_PUNISH_TYPE_SCREEN_SHOT,ENM_PUNISH_TYPE_SCREEN_SHOT_KICK}},
        {ENM_POLICY_TYPE_MULTICLIENT,{ENM_PUNISH_TYPE_KICK,ENM_PUNISH_TYPE_NO_OPEARATION,ENM_PUNISH_TYPE_BAN_MACHINE,ENM_PUNISH_TYPE_SCREEN_SHOT,ENM_PUNISH_TYPE_SCREEN_SHOT_KICK}},
        {ENM_POLICY_TYPE_SHELLCODE,{ENM_PUNISH_TYPE_KICK,ENM_PUNISH_TYPE_NO_OPEARATION,ENM_PUNISH_TYPE_BAN_MACHINE,ENM_PUNISH_TYPE_SCREEN_SHOT,ENM_PUNISH_TYPE_SCREEN_SHOT_KICK}},
#ifdef GATE_ADMIN
        {ENM_POLICY_TYPE_SCRIPT,{ENM_PUNISH_TYPE_NO_OPEARATION}},
        {ENM_POLICY_TYPE_THREAD_START,{ENM_PUNISH_TYPE_KICK,ENM_PUNISH_TYPE_NO_OPEARATION,ENM_PUNISH_TYPE_BAN_MACHINE,ENM_PUNISH_TYPE_SCREEN_SHOT,ENM_PUNISH_TYPE_SCREEN_SHOT_KICK}},
#endif        
    };
    int last_punish_type = CB_ERR;
    bool last_sel_flag = false;
    CString punish_type;
    if (m_PunishTypeComboBox.GetCurSel() != CB_ERR)
    {
        m_PunishTypeComboBox.GetLBText(m_PunishTypeComboBox.GetCurSel(), punish_type);
        last_punish_type = ConvertToPunishType(punish_type);
    }

    ClearPunishTypeList();
    m_PunishTypeComboBox.ResetContent();
    int nIndex = 0;
    CString policy_type;
    m_PolicyTypeComboBox.GetLBText(m_PolicyTypeComboBox.GetCurSel(), policy_type);
    for (auto punish_type : config[ConvertToPolicyType(policy_type)])
    {
        int index = m_PunishTypeComboBox.InsertString(nIndex++,ConvertToString(punish_type));
        AddPunishTypeToMap(punish_type, index);
        m_PunishTypeComboBox.SetItemData(index, punish_type);
        if (punish_type == last_punish_type)
        {
            last_sel_flag = true;
        }
    }

    if (last_sel_flag)
    {
        m_PunishTypeComboBox.SetCurSel(GetPunishTypeListIndex((PunishType)last_punish_type));
    }
    else
    {
        m_PunishTypeComboBox.SetCurSel(GetPunishTypeListIndex(ENM_PUNISH_TYPE_NO_OPEARATION));

    }
    m_PunishTypeComboBox.UpdateData(TRUE);
}

void CPropEditFormView::OnCbnDropdownComboPolicyType()
{
}


void CPropEditFormView::OnInitialUpdate()
{
    CFormView::OnInitialUpdate();
    int nIndex = 0;
    for (int index = ENM_POLICY_TYPE_MODULE_NAME; index < ENM_POLICY_TYPE_MAX; index++)
    {
        if (index == ENM_POLICY_TYPE_BACK_GAME ||
            index == ENM_POLICY_TYPE_EXIT_GAME ||
            index == ENM_POLICY_TYPE_ACTION_SPEED_WALK ||
            index == ENM_POLICY_TYPE_ACTION_SPEED_HIT ||
            index == ENM_POLICY_TYPE_ACTION_SPEED_SPELL
            ) continue;
#ifndef GATE_ADMIN
        if(index == ENM_POLICY_TYPE_SCRIPT || index == ENM_POLICY_TYPE_THREAD_START) continue;
#endif
        m_PolicyTypeComboBox.InsertString(nIndex++,ConvertToString((PolicyType)index));
    }
    m_PolicyTypeComboBox.SetCurSel(ENM_POLICY_TYPE_MODULE_NAME);
    OnCbnDropdownComboPunishType();
}


void CPropEditFormView::OnBnClickedButtonPolicyCommit()
{
    auto pParent = GetParent();
    if (pParent)
    {
        auto pSettingDoc = reinterpret_cast<CConfigSettingDoc*>(reinterpret_cast<CConfigSettingSubViewWnd*>(pParent)->GetDocument());
        if(pSettingDoc)
        {
            CString cstrPolicyId;
            m_PolicyIdEdit.GetWindowText(cstrPolicyId);
            auto uiPolicyId = StrToInt(cstrPolicyId);
#ifndef GATE_ADMIN
            if (!m_CreateByAdmin && (GATE_POLICY_ID >= uiPolicyId || uiPolicyId > GATE_ADMIN_POLICY_ID))
            {
                AfxMessageBox(TEXT("策略ID为688001～689000之间"), MB_OK);
                return;
            }
#endif
            ProtocolPolicy Policy;
            Policy.policy_id = uiPolicyId;
            CString policy_type;
            m_PolicyTypeComboBox.GetLBText(m_PolicyTypeComboBox.GetCurSel(), policy_type);
            Policy.policy_type = ConvertToPolicyType(policy_type);

            CString punish_type;
            m_PunishTypeComboBox.GetLBText(m_PunishTypeComboBox.GetCurSel(), punish_type);
            Policy.punish_type = ConvertToPunishType(punish_type);
            CString temp;
            m_PolicyConfigEdit.GetWindowText(temp);
            Policy.config = temp.GetBuffer();
            if (Policy.policy_type == ENM_POLICY_TYPE_BACK_GAME ||
                Policy.policy_type == ENM_POLICY_TYPE_EXIT_GAME ||
                Policy.policy_type == ENM_POLICY_TYPE_ACTION_SPEED_WALK ||
                Policy.policy_type == ENM_POLICY_TYPE_ACTION_SPEED_HIT ||
                Policy.policy_type == ENM_POLICY_TYPE_ACTION_SPEED_SPELL                
                )
            {
                bool flag = false;
                int config_int = 0;
                try
                {
                    config_int = std::stoi(Policy.config);
                }
                catch (...)
                {
                    flag = true;
                }

                if (flag || config_int < 0)
                {
                    AfxMessageBox(TEXT("配置必须为大于0的数值"), MB_OK);
                    return;
                }
            }
            m_PolicyCommentEdit.GetWindowText(temp);
            Policy.comment = temp.GetBuffer();
            pSettingDoc->GetPolicy().policies[uiPolicyId] = Policy;
            pSettingDoc->GetView<CConfigSettingView>()->RefreshViewList();
            pSettingDoc->SetTitle(pSettingDoc->GetTitle() + TEXT("*"));
        }
    }
}
