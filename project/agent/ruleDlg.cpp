// ProcessModuleRule.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "agent.h"
#include "ruleDlg.h"
#include "afxdialogex.h"
#include <filesystem>

// ProcessModuleRule 对话框
using namespace SubProto;
using namespace Global;

ProtocolPolicy Global::g_policy;
std::wstring Global::g_error_text;

IMPLEMENT_DYNAMIC(RuleDlg, CDialogEx)

RuleDlg::RuleDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_RULE_LIST_DLG, pParent)
    , m_comment(_T(""))
    , m_search_text(_T(""))
{
    m_CurrentSeletedPolicyId = -1;
    Global::LoadConfig();
}

RuleDlg::~RuleDlg()
{
    
}

bool Global::AddPolicy(SubProto::PolicyType type, SubProto::PunishType punish_type, std::wstring config, std::wstring comment)
{
    std::map<SubProto::PolicyType, std::vector<SubProto::Policy>> policy_type_map;
    for (auto& policy_itor : g_policy.policies)
    {
        auto& policy = policy_itor.second;
        policy_type_map[policy.policy_type].push_back(policy);
    }
    std::map<SubProto::PolicyType, size_t> limit = 
    {
        {ENM_POLICY_TYPE_MODULE_NAME,-1},
        {ENM_POLICY_TYPE_PROCESS_NAME,-1},
        {ENM_POLICY_TYPE_FILE_NAME,-1},
        {ENM_POLICY_TYPE_WINDOW_NAME,-1},
        {ENM_POLICY_TYPE_MACHINE,-1},
        {ENM_POLICY_TYPE_MULTICLIENT,1},
        {ENM_POLICY_TYPE_SHELLCODE,-1},
        {ENM_POLICY_TYPE_THREAD_START,-1}
    };
    if (policy_type_map[type].size() + 1 > limit[type])
    {
        g_error_text = L"该策略类型不支持多个";
        return false;
    }

    if (std::find_if(policy_type_map[type].begin(), policy_type_map[type].end(), [&](SubProto::Policy& policy) {
        return policy.config == config;
    }) != policy_type_map[type].end())
    {
        g_error_text = L"配置重复添加";
        return false;
    }

    SubProto::Policy policy;
    uint32_t id = g_policy.policies.size();
    while (g_policy.policies.find(id) != g_policy.policies.end())
    {
        id++;
    }
    policy.policy_id = id;
    policy.policy_type = type;
    policy.punish_type = punish_type;
    policy.config = config;
    policy.comment = comment;
    g_policy.policies.emplace(std::make_pair(policy.policy_id, policy));
    return true;
}

void Global::LoadConfig()
{
    std::filesystem::path path(std::filesystem::current_path() / "config.json");
    if (!std::filesystem::exists(path))
    {
        return;
    }
    try
    {
        std::ifstream file(path, std::ios::in | std::ios::binary);
        g_policy.from_json(json::parse(file));
    }
    catch (...)
    {

    }
}

void Global::SaveConfig()
{
    std::filesystem::path path(std::filesystem::current_path() / "config.json");
    std::ofstream file(path, std::ios::in | std::ios::binary | std::ios::trunc);
    json json_;
    g_policy.to_json(json_);
    file << json_.dump();
}

void RuleDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_CONFIG, m_PolicyList);
    DDX_Control(pDX, IDC_COMBO_POLICY_TYPE, m_PolicyTypeComboBox);
    DDX_Control(pDX, IDC_COMBO_PUNISH_TYPE, m_PunishTypeComboBox);
    DDX_Control(pDX, IDC_EDIT_CONFIG, m_ConfigEdit);
    DDX_Text(pDX, IDC_EDIT_COMMENT, m_comment);
    DDX_Text(pDX, IDC_EDIT_RULE_SEARCH, m_search_text);
}


BEGIN_MESSAGE_MAP(RuleDlg, CDialogEx)
	ON_WM_SHOWWINDOW()
    ON_BN_CLICKED(IDC_BUTTON_ADD, &RuleDlg::OnBnClickedButtonAdd)
    ON_BN_CLICKED(IDC_BUTTON_EDIT, &RuleDlg::OnBnClickedButtonEdit)
    ON_NOTIFY(NM_CLICK, IDC_LIST_CONFIG, &RuleDlg::OnNMClickListConfig)
    ON_BN_CLICKED(IDC_BUTTON_REMOVE, &RuleDlg::OnBnClickedButtonRemove)
    ON_WM_CLOSE()
    ON_WM_WINDOWPOSCHANGED()
    ON_CBN_DROPDOWN(IDC_COMBO_PUNISH_TYPE, &RuleDlg::OnCbnDropdownComboPunishType)
    ON_CBN_DROPDOWN(IDC_COMBO_POLICY_TYPE, &RuleDlg::OnCbnDropdownComboPolicyType)
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_BUTTON_ONLINE_GAMER_SEARCH, &RuleDlg::OnBnClickedButtonOnlineGamerSearch)
END_MESSAGE_MAP()


// ProcessModuleRule 消息处理程序

BOOL RuleDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_agentDlg = (CagentDlg*)theApp.m_pMainWnd;
    int colIndex = 0;
    m_PolicyList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_GRIDLINES);
    m_PolicyList.InsertColumn(colIndex++, TEXT("序号"), LVCFMT_LEFT, 38);
    m_PolicyList.InsertColumn(colIndex++, TEXT("策略ID"), LVCFMT_LEFT, 60);
    m_PolicyList.InsertColumn(colIndex++, TEXT("策略类型"), LVCFMT_LEFT, 75);
    m_PolicyList.InsertColumn(colIndex++, TEXT("处理类型"), LVCFMT_LEFT, 70);
    m_PolicyList.InsertColumn(colIndex++, TEXT("配置"), LVCFMT_LEFT, 530);
    m_PolicyList.InsertColumn(colIndex++, TEXT("备注"), LVCFMT_LEFT, 330);

    for (int index = ENM_POLICY_TYPE_MODULE_NAME; index < ENM_POLICY_TYPE_MAX; index++)
    {
        m_PolicyTypeComboBox.InsertString(index, ConvertToString((PolicyType)index));
    }
    m_PolicyTypeComboBox.SetCurSel(ENM_POLICY_TYPE_MODULE_NAME);
    OnCbnDropdownComboPunishType();
#ifdef BUILD_ADMIN
    SetTimer(Protocol::PACKAGE_ID_POLICY, 1000 * 60 * 5, nullptr);
#else
    SetTimer(Protocol::PACKAGE_ID_POLICY, 1000 * 30, nullptr);
#endif
    return TRUE;
}


void RuleDlg::UpdateConfigList()
{
    m_PolicyList.DeleteAllItems();
    uint32_t i = 0;
    for (auto& policy_itor : g_policy.policies)
    {
        auto& policy = policy_itor.second;
        m_PolicyList.InsertItem(i, TEXT(""));
        int colIndex = 0;
        m_PolicyList.SetItemText(i, colIndex++, std::to_wstring(i + 1).c_str());
        m_PolicyList.SetItemText(i, colIndex++, std::to_wstring(policy.policy_id).c_str());
        m_PolicyList.SetItemText(i, colIndex++, ConvertToString(policy.policy_type));
        m_PolicyList.SetItemText(i, colIndex++, ConvertToString(policy.punish_type));
        m_PolicyList.SetItemText(i, colIndex++, policy.config.c_str());
        m_PolicyList.SetItemText(i, colIndex++, policy.comment.c_str());
        i++;
    }
}

void RuleDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);
}

CString Global::ConvertToString(SubProto::PolicyType type)
{
    std::map<SubProto::PolicyType, LPCTSTR> policy_type_str = {
        {ENM_POLICY_TYPE_MODULE_NAME,TEXT("模块名检测")},
        {ENM_POLICY_TYPE_PROCESS_NAME,TEXT("进程名检测")},
        {ENM_POLICY_TYPE_FILE_NAME,TEXT("文件路径")},
        {ENM_POLICY_TYPE_WINDOW_NAME,TEXT("窗口名")},
        {ENM_POLICY_TYPE_MACHINE,TEXT("机器码")},
        {ENM_POLICY_TYPE_MULTICLIENT,TEXT("多开限制")},
        {ENM_POLICY_TYPE_SHELLCODE,TEXT("云代码")},
        {ENM_POLICY_TYPE_THREAD_START,TEXT("线程特征")}
    };
    if (policy_type_str.find(type) != policy_type_str.end())
    {
        return policy_type_str[type];
    }
    else
    {
        return TEXT("内部策略");
    }
}
CString Global::ConvertToString(SubProto::PunishType type)
{
    std::map<SubProto::PunishType, LPCTSTR> punish_type_str = {
        {ENM_PUNISH_TYPE_KICK,TEXT("退出游戏")},
        {ENM_PUNISH_TYPE_BSOD,TEXT("蓝屏")},
        {ENM_PUNISH_TYPE_NO_OPEARATION,TEXT("不处理")},
        {ENM_PUNISH_TYPE_SUPER_WHITE_LIST,TEXT("白名单")},
        {ENM_PUNISH_TYPE_BAN_MACHINE,TEXT("封机器")},
        {ENM_PUNISH_TYPE_SCREEN_SHOT,TEXT("截图")},
        {ENM_PUNISH_TYPE_SCREEN_SHOT_KICK,TEXT("截图+退出游戏")},
        {ENM_PUNISH_TYPE_SCREEN_SHOT_BSOD,TEXT("截图+蓝屏")},
    };
    if (punish_type_str.find(type) != punish_type_str.end())
    {
        return punish_type_str[type];
    }
    else
    {
        return TEXT("未定义");
    }
}

void RuleDlg::OnBnClickedButtonAdd()
{
    CString config;
    m_ConfigEdit.GetWindowText(config);
    UpdateData(TRUE);
    if (!AddPolicy((SubProto::PolicyType)m_PolicyTypeComboBox.GetCurSel(), (SubProto::PunishType)m_PunishTypeComboBox.GetItemData(m_PunishTypeComboBox.GetCurSel()), config.GetString(),m_comment.GetString()))
    {
        MessageBox(g_error_text.c_str());
        return;
    }
    UpdateConfigList();
}

void RuleDlg::OnBnClickedButtonEdit()
{
    // TODO: 在此添加控件通知处理程序代码
    if (m_CurrentSeletedPolicyId != -1 && g_policy.policies.find(m_CurrentSeletedPolicyId) != g_policy.policies.end())
    {
        g_policy.policies[m_CurrentSeletedPolicyId].policy_type = (SubProto::PolicyType)m_PolicyTypeComboBox.GetCurSel();
        g_policy.policies[m_CurrentSeletedPolicyId].punish_type = (SubProto::PunishType)m_PunishTypeComboBox.GetItemData(m_PunishTypeComboBox.GetCurSel());
        CString config;
        m_ConfigEdit.GetWindowText(config);
        UpdateData(TRUE);
        g_policy.policies[m_CurrentSeletedPolicyId].config = config.GetString();
        g_policy.policies[m_CurrentSeletedPolicyId].comment = m_comment.GetString();
    }
    
    UpdateConfigList();
}

void RuleDlg::OnBnClickedButtonRemove()
{
    // TODO: 在此添加控件通知处理程序代码
    if (m_CurrentSeletedPolicyId != -1 && g_policy.policies.find(m_CurrentSeletedPolicyId) != g_policy.policies.end())
    {
        g_policy.policies.erase(m_CurrentSeletedPolicyId);
    }
    UpdateConfigList();
}


void RuleDlg::OnNMClickListConfig(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    // TODO: 在此添加控件通知处理程序代码
    if (pNMItemActivate->iItem != -1)
    {
        m_CurrentSeletedPolicyId = StrToInt(m_PolicyList.GetItemText(pNMItemActivate->iItem, 1));
        m_PolicyTypeComboBox.SetCurSel(g_policy.policies[m_CurrentSeletedPolicyId].policy_type);
        m_PunishTypeComboBox.SetCurSel(GetPunishTypeListIndex(g_policy.policies[m_CurrentSeletedPolicyId].punish_type));
        m_ConfigEdit.SetWindowText(g_policy.policies[m_CurrentSeletedPolicyId].config.c_str());
        m_comment = g_policy.policies[m_CurrentSeletedPolicyId].comment.c_str();
        UpdateData(FALSE);
    }
    *pResult = 0;
}




void RuleDlg::OnClose()
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    SaveConfig();
    CDialogEx::OnClose();
}

void RuleDlg::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
    CDialogEx::OnWindowPosChanged(lpwndpos);
    if (lpwndpos->flags & SWP_SHOWWINDOW)
    {
        UpdateConfigList();
    }
    // TODO: 在此处添加消息处理程序代码
}
void RuleDlg::ClearPunishTypeList()
{
    m_PunishTypeListMap.clear();
}
void RuleDlg::AddPunishTypeToMap(SubProto::PunishType type,int index)
{
    m_PunishTypeListMap.emplace(std::make_pair(type, index));
}
int RuleDlg::GetPunishTypeListIndex(SubProto::PunishType type)
{
    return m_PunishTypeListMap[type];
}

void RuleDlg::OnCbnDropdownComboPunishType()
{
    std::map<SubProto::PolicyType, std::vector<SubProto::PunishType>> config = {
        {ENM_POLICY_TYPE_MODULE_NAME,{ENM_PUNISH_TYPE_KICK,ENM_PUNISH_TYPE_BSOD,ENM_PUNISH_TYPE_NO_OPEARATION,ENM_PUNISH_TYPE_BAN_MACHINE,ENM_PUNISH_TYPE_SCREEN_SHOT,ENM_PUNISH_TYPE_SCREEN_SHOT_KICK,ENM_PUNISH_TYPE_SCREEN_SHOT_BSOD}},
        {ENM_POLICY_TYPE_PROCESS_NAME,{ENM_PUNISH_TYPE_KICK,ENM_PUNISH_TYPE_BSOD,ENM_PUNISH_TYPE_NO_OPEARATION,ENM_PUNISH_TYPE_BAN_MACHINE,ENM_PUNISH_TYPE_SCREEN_SHOT,ENM_PUNISH_TYPE_SCREEN_SHOT_KICK,ENM_PUNISH_TYPE_SCREEN_SHOT_BSOD}},
        {ENM_POLICY_TYPE_FILE_NAME,{ENM_PUNISH_TYPE_KICK,ENM_PUNISH_TYPE_BSOD,ENM_PUNISH_TYPE_NO_OPEARATION,ENM_PUNISH_TYPE_BAN_MACHINE,ENM_PUNISH_TYPE_SCREEN_SHOT,ENM_PUNISH_TYPE_SCREEN_SHOT_KICK,ENM_PUNISH_TYPE_SCREEN_SHOT_BSOD}},
        {ENM_POLICY_TYPE_WINDOW_NAME,{ENM_PUNISH_TYPE_KICK,ENM_PUNISH_TYPE_BSOD,ENM_PUNISH_TYPE_NO_OPEARATION,ENM_PUNISH_TYPE_BAN_MACHINE,ENM_PUNISH_TYPE_SCREEN_SHOT,ENM_PUNISH_TYPE_SCREEN_SHOT_KICK,ENM_PUNISH_TYPE_SCREEN_SHOT_BSOD}},
        {ENM_POLICY_TYPE_MACHINE,{ENM_PUNISH_TYPE_KICK,ENM_PUNISH_TYPE_BSOD,ENM_PUNISH_TYPE_NO_OPEARATION,ENM_PUNISH_TYPE_SUPER_WHITE_LIST,ENM_PUNISH_TYPE_SCREEN_SHOT,ENM_PUNISH_TYPE_SCREEN_SHOT_KICK,ENM_PUNISH_TYPE_SCREEN_SHOT_BSOD}},
        {ENM_POLICY_TYPE_MULTICLIENT,{ENM_PUNISH_TYPE_KICK,ENM_PUNISH_TYPE_BSOD,ENM_PUNISH_TYPE_NO_OPEARATION,ENM_PUNISH_TYPE_BAN_MACHINE,ENM_PUNISH_TYPE_SCREEN_SHOT,ENM_PUNISH_TYPE_SCREEN_SHOT_KICK,ENM_PUNISH_TYPE_SCREEN_SHOT_BSOD}},
        {ENM_POLICY_TYPE_SHELLCODE,{ENM_PUNISH_TYPE_KICK,ENM_PUNISH_TYPE_BSOD,ENM_PUNISH_TYPE_NO_OPEARATION,ENM_PUNISH_TYPE_BAN_MACHINE,ENM_PUNISH_TYPE_SCREEN_SHOT,ENM_PUNISH_TYPE_SCREEN_SHOT_KICK,ENM_PUNISH_TYPE_SCREEN_SHOT_BSOD}},
        {ENM_POLICY_TYPE_THREAD_START,{ENM_PUNISH_TYPE_KICK,ENM_PUNISH_TYPE_BSOD,ENM_PUNISH_TYPE_NO_OPEARATION,ENM_PUNISH_TYPE_BAN_MACHINE,ENM_PUNISH_TYPE_SCREEN_SHOT,ENM_PUNISH_TYPE_SCREEN_SHOT_KICK,ENM_PUNISH_TYPE_SCREEN_SHOT_BSOD}},
    };
    ClearPunishTypeList();
    m_PunishTypeComboBox.ResetContent();
    int n = 0;
    for (auto punish_type : config[(SubProto::PolicyType)m_PolicyTypeComboBox.GetCurSel()])
    {
        int index = m_PunishTypeComboBox.InsertString(n++, ConvertToString(punish_type));
        AddPunishTypeToMap(punish_type, index);
        m_PunishTypeComboBox.SetItemData(index, punish_type);
    }
    m_PunishTypeComboBox.SetCurSel(GetPunishTypeListIndex(ENM_PUNISH_TYPE_NO_OPEARATION));
}

void RuleDlg::OnCbnDropdownComboPolicyType()
{
    m_PunishTypeComboBox.SetCurSel(GetPunishTypeListIndex(ENM_PUNISH_TYPE_NO_OPEARATION));
}


void RuleDlg::OnTimer(UINT_PTR nIDEvent)
{
    Singleton<TaskPolicy>::getInstance().multiclient_detect();
    MainWnd->EnumConnectID([](CONNID conn_id) {
        Singleton<TaskPolicy>::getInstance().async_.delay_execute_rule([conn_id]() {
            Singleton<TaskPolicy>::getInstance().dectection_cheat(conn_id);
        }, INTERVAL_DETECTION_RULE);
    });
    SaveConfig();
    CDialogEx::OnTimer(nIDEvent);
}


void RuleDlg::OnBnClickedButtonOnlineGamerSearch()
{
    CUIntArray columnIndexs;
    columnIndexs.Add(4);
    columnIndexs.Add(5);
    UpdateData(TRUE);
    mfcutil.CListCtrlSearch(m_search_text, m_search_text_old, m_PolicyList, columnIndexs);
}
