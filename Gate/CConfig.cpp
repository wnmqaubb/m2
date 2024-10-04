// CConfig.cpp: 实现文件
//

#include "pch.h"
#include "Gate.h"
#include "CConfig.h"
#include "afxdialogex.h"
#include "ConfigSettingSubView.h"
#include "ConfigSettingChildFrm.h"
#include "ConfigSettingDoc.h"
#include "ConfigSettingView.h"


// CConfig 对话框

IMPLEMENT_DYNAMIC(CConfig, CDialogEx)

CConfig::CConfig(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CONFIG_DIALOG, pParent)
    , m_lazy_back_enable(FALSE)
    , m_lazy_back(0)
    , m_lazy_exit_enable(FALSE)
    , m_lazy_exit(0)
    , m_speed_walk_enable(FALSE)
    , m_speed_hit_enable(FALSE)
    , m_speed_spell_enable(FALSE)
    , m_speed_walk(0)
    , m_speed_hit(0)
    , m_speed_spell(0)
    , m_can_lazy_back_exit_enable(FALSE)
{

}

CConfig::~CConfig()
{
}

void CConfig::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_CHECK_BACK, m_lazy_back_enable);
    DDX_Text(pDX, IDC_EDIT_BACK, m_lazy_back);
    DDX_Check(pDX, IDC_CHECK_EXIT, m_lazy_exit_enable);
    DDX_Text(pDX, IDC_EDIT_EXIT, m_lazy_exit);
    DDX_Check(pDX, IDC_CHECK_SPEED_WALK, m_speed_walk_enable);
    DDX_Check(pDX, IDC_CHECK_SPEED_HIT, m_speed_hit_enable);
    DDX_Check(pDX, IDC_CHECK_SPEED_SPELL, m_speed_spell_enable);
    DDX_Text(pDX, IDC_EDIT_SPEED_WALK, m_speed_walk);
    DDX_Text(pDX, IDC_EDIT_SPEED_HIT, m_speed_hit);
    DDX_Text(pDX, IDC_EDIT_SPEED_SPELL, m_speed_spell);
    DDX_Check(pDX, IDC_CHECK_CAN_BACK_EXIT, m_can_lazy_back_exit_enable);
}


BEGIN_MESSAGE_MAP(CConfig, CDialogEx)
    ON_WM_CLOSE()
END_MESSAGE_MAP()


void CConfig::ShowConfigDlg()
{
    auto& m_Policys = ((CConfigSettingDoc*)theApp.m_ConfigDoc)->GetPolicy();
    for (auto[uiPolicyId, Policy] : m_Policys.policies)
    {
        if (Policy.policy_type == ENM_POLICY_TYPE_BACK_GAME)
        {
            m_lazy_back = Policy.config.empty() ? 0 : std::stoi(Policy.config);
            m_lazy_back_enable = (Policy.punish_type == ENM_PUNISH_TYPE_ENABLE);
            m_can_lazy_back_exit_enable = !Policy.comment.empty();
        }

        if (Policy.policy_type == ENM_POLICY_TYPE_EXIT_GAME)
        {
            m_lazy_exit = Policy.config.empty() ? 0 : std::stoi(Policy.config);
            m_lazy_exit_enable = (Policy.punish_type == ENM_PUNISH_TYPE_ENABLE);
        }

        if (Policy.policy_type == ENM_POLICY_TYPE_ACTION_SPEED_WALK)
        {
            m_speed_walk = Policy.config.empty() ? 0 : std::stoi(Policy.config);
            m_speed_walk_enable = (Policy.punish_type == ENM_PUNISH_TYPE_ENABLE);
        }

        if (Policy.policy_type == ENM_POLICY_TYPE_ACTION_SPEED_HIT)
        {
            m_speed_hit = Policy.config.empty() ? 0 : std::stoi(Policy.config);
            m_speed_hit_enable = (Policy.punish_type == ENM_PUNISH_TYPE_ENABLE);
        }

        if (Policy.policy_type == ENM_POLICY_TYPE_ACTION_SPEED_SPELL)
        {
            m_speed_spell = Policy.config.empty() ? 0 : std::stoi(Policy.config);
            m_speed_spell_enable = (Policy.punish_type == ENM_PUNISH_TYPE_ENABLE);
        }
    }

    UpdateData(false);

    this->CenterWindow();
    this->ShowWindow(SW_SHOW);

    
}
// CConfig 消息处理程序


void CConfig::OnClose()
{
    UpdateData(true);

    uint32_t policy_id_back = 0;
    uint32_t policy_id_exit = 0;
    uint32_t policy_id_walk = 0;
    uint32_t policy_id_hit = 0;
    uint32_t policy_id_spell = 0;
    auto pSettingDoc = ((CConfigSettingDoc*)theApp.m_ConfigDoc);
    auto& m_Policys = pSettingDoc->GetPolicy();

    unsigned int uiLastPolicyId = 0;
#ifdef GATE_ADMIN
    uiLastPolicyId = GATE_ADMIN_POLICY_ID;
#else
    uiLastPolicyId = GATE_POLICY_ID;
#endif

    for (auto&[uiPolicyId, Policy] : m_Policys.policies)
    {
#ifdef GATE_ADMIN
        if (uiPolicyId > GATE_ADMIN_POLICY_ID)
        {
            uiLastPolicyId = uiPolicyId;
        }
#else
        if (GATE_POLICY_ID < uiPolicyId && uiPolicyId < GATE_ADMIN_POLICY_ID)
        {
            uiLastPolicyId = uiPolicyId;
        }
#endif
        if (Policy.policy_type == ENM_POLICY_TYPE_BACK_GAME)
        {
            Policy.config = std::to_wstring(m_lazy_back);
            Policy.punish_type = (m_lazy_back_enable ? ENM_PUNISH_TYPE_ENABLE : ENM_PUNISH_TYPE_DISABLE);
            Policy.comment = (m_can_lazy_back_exit_enable ? L"1" : L"");
            policy_id_back = Policy.policy_id;
            continue;
        }

        if (Policy.policy_type == ENM_POLICY_TYPE_EXIT_GAME)
        {
            Policy.config = std::to_wstring(m_lazy_exit);
            Policy.punish_type = (m_lazy_exit_enable ? ENM_PUNISH_TYPE_ENABLE : ENM_PUNISH_TYPE_DISABLE);
            policy_id_exit = Policy.policy_id;
            continue;
        }

        if (Policy.policy_type == ENM_POLICY_TYPE_ACTION_SPEED_WALK)
        {
            Policy.config = std::to_wstring(m_speed_walk);
            Policy.punish_type = (m_speed_walk_enable ? ENM_PUNISH_TYPE_ENABLE : ENM_PUNISH_TYPE_DISABLE);
            policy_id_walk = Policy.policy_id;
            continue;
        }

        if (Policy.policy_type == ENM_POLICY_TYPE_ACTION_SPEED_HIT)
        {
            Policy.config = std::to_wstring(m_speed_hit);
            Policy.punish_type = (m_speed_hit_enable ? ENM_PUNISH_TYPE_ENABLE : ENM_PUNISH_TYPE_DISABLE);
            policy_id_hit = Policy.policy_id;
            continue;
        }

        if (Policy.policy_type == ENM_POLICY_TYPE_ACTION_SPEED_SPELL)
        {
            Policy.config = std::to_wstring(m_speed_spell);
            Policy.punish_type = (m_speed_spell_enable ? ENM_PUNISH_TYPE_ENABLE : ENM_PUNISH_TYPE_DISABLE);
            policy_id_spell = Policy.policy_id;
            continue;
        }
    }

    bool create_by_admin = false;
#ifdef GATE_ADMIN
    create_by_admin = true;
#endif

    if (!policy_id_back)
    {
        ProtocolPolicy Policy;
        Policy.policy_id = ++uiLastPolicyId;
        Policy.policy_type = ENM_POLICY_TYPE_BACK_GAME;
        Policy.punish_type = (m_lazy_back_enable ? ENM_PUNISH_TYPE_ENABLE : ENM_PUNISH_TYPE_DISABLE);
        Policy.comment = (m_can_lazy_back_exit_enable ? L"1" : L"");
        Policy.config = std::to_wstring(m_lazy_back);
        Policy.create_by_admin = create_by_admin == true;
        m_Policys.policies[uiLastPolicyId] = Policy;
    }

    if (!policy_id_exit)
    {
        ProtocolPolicy Policy;
        Policy.policy_id = ++uiLastPolicyId;
        Policy.policy_type = ENM_POLICY_TYPE_EXIT_GAME;
        Policy.punish_type = (m_lazy_exit_enable ? ENM_PUNISH_TYPE_ENABLE : ENM_PUNISH_TYPE_DISABLE);
        Policy.config = std::to_wstring(m_lazy_exit);
        Policy.create_by_admin = create_by_admin == true;
        m_Policys.policies[uiLastPolicyId] = Policy;
    }

    if (!policy_id_walk)
    {
        ProtocolPolicy Policy;
        Policy.policy_id = ++uiLastPolicyId;
        Policy.policy_type = ENM_POLICY_TYPE_ACTION_SPEED_WALK;
        Policy.punish_type = (m_speed_walk_enable ? ENM_PUNISH_TYPE_ENABLE : ENM_PUNISH_TYPE_DISABLE);
        Policy.config = std::to_wstring(m_speed_walk);
        Policy.create_by_admin = create_by_admin == true;
        m_Policys.policies[uiLastPolicyId] = Policy;
    }

    if (!policy_id_hit)
    {
        ProtocolPolicy Policy;
        Policy.policy_id = ++uiLastPolicyId;
        Policy.policy_type = ENM_POLICY_TYPE_ACTION_SPEED_HIT;
        Policy.punish_type = (m_speed_hit_enable ? ENM_PUNISH_TYPE_ENABLE : ENM_PUNISH_TYPE_DISABLE);
        Policy.config = std::to_wstring(m_speed_hit);
        Policy.create_by_admin = create_by_admin == true;
        m_Policys.policies[uiLastPolicyId] = Policy;
    }

    if (!policy_id_spell)
    {
        ProtocolPolicy Policy;
        Policy.policy_id = ++uiLastPolicyId;
        Policy.policy_type = ENM_POLICY_TYPE_ACTION_SPEED_SPELL;
        Policy.punish_type = (m_speed_spell_enable ? ENM_PUNISH_TYPE_ENABLE : ENM_PUNISH_TYPE_DISABLE);
        Policy.config = std::to_wstring(m_speed_spell);
        Policy.create_by_admin = create_by_admin == true;
        m_Policys.policies[uiLastPolicyId] = Policy;
    }

    if (pSettingDoc->GetView<CConfigSettingView>())
    {
        pSettingDoc->GetView<CConfigSettingView>()->RefreshViewList();
        pSettingDoc->SetTitle(pSettingDoc->GetTitle() + TEXT("*"));
    }
    pSettingDoc->DoFileSave();
    CDialogEx::OnClose();
}
