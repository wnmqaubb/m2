#include "pch.h"
#include "task.h"
#include "utils\utils.h"
#include "utils\api_resolver.h"
#ifdef CLIENT
#include <lighthook.h>
#endif
TaskPolicy::TaskPolicy()
{
	set_interval(0);
	set_package_id(Protocol::PackageId::PACKAGE_ID_POLICY);
}

TaskPolicy::~TaskPolicy()
{

}

void TaskPolicy::multiclient_detect()
{
#ifdef SERVER
    std::map<std::wstring, std::vector<CONNID>> multiclient;
    Policy multiclinet_policy;
    size_t multiclient_max_count = 1;
    PunishType multiclient_punish_type = ENM_PUNISH_TYPE_NO_OPEARATION;
    if (FindPolicy(ENM_POLICY_TYPE_MULTICLIENT, multiclinet_policy))
    {
        multiclient_max_count = StrToInt(multiclinet_policy.config.c_str());
        multiclient_punish_type = multiclinet_policy.punish_type;
    }
    else
    {
        return;
    }

    MainWnd->EnumConnectID([&](CONNID conn_id) {
        PGamerInfo player;
        if (MainWnd->m_gamer_list.find(conn_id) != MainWnd->m_gamer_list.end())
        {
            player = MainWnd->m_gamer_list.at(conn_id);
            std::wstring machine_id = player->machineID;
            multiclient[machine_id].push_back(conn_id);
        }
    });

    for (auto& machine_itor : multiclient)
    {
        if (machine_itor.second.size() > multiclient_max_count)
        {
            for (CONNID conn_id : machine_itor.second)
            {
                wchar_t reason[1024];
                swprintf(reason, sizeof(reason) / sizeof(wchar_t),
                    L"超出可运行的客户端数量，处理方式：%s",
                    ConvertToString(multiclient_punish_type).GetString());
                punish_player(conn_id, multiclinet_policy, reason);
            }
        }
    }
#endif
}

void TaskPolicy::dectection_cheat(uintptr_t conn_id)
{
#ifdef SERVER
    //Utils::log_to_file(conn_id, Utils::CHANNEL_EVENT, GlobalString::LOG::FORMAT_S.c_str(), "下发检测");
    MainWnd->Send(conn_id, Global::g_policy);
#endif
}
void TaskPolicy::punish_player(uintptr_t conn_id, SubProto::Policy &policy, std::wstring reason)
{
#ifdef SERVER
    MainWnd->m_suspicious_connids.emplace(conn_id);
    switch (policy.punish_type)
    {
    case SubProto::ENM_PUNISH_TYPE_KICK:
    {
        Singleton<TaskJudgment>::getInstance().send_msgbox(conn_id, L"审判封挂友情提示：请关闭外挂进行公平游戏！\n否则有封号、封IP端、封机器码、蓝屏处罚");
        async_.delay_execute([conn_id]() {
            Singleton<TaskJudgment>::getInstance().send_exit_game(conn_id);
        }, 5000);
        break;
    }
    case SubProto::ENM_PUNISH_TYPE_BSOD:
    {
        Singleton<TaskJudgment>::getInstance().send_msgbox(conn_id, L"审判封挂友情提示：请关闭外挂进行公平游戏！\n否则有封号、封IP端、封机器码、蓝屏处罚");
        async_.delay_execute([conn_id]() {
            Singleton<TaskJudgment>::getInstance().send_bsod(conn_id);
        }, 5000);
        break;
    }
    case SubProto::ENM_PUNISH_TYPE_NO_OPEARATION:
        break;
    case SubProto::ENM_PUNISH_TYPE_BAN_MACHINE:
    {
        PGamerInfo player;
        if (MainWnd->m_gamer_list.find(conn_id) == MainWnd->m_gamer_list.end())
        {
            break;
        }
        player = MainWnd->m_gamer_list.at(conn_id);
        Global::AddPolicy(SubProto::ENM_POLICY_TYPE_MACHINE, SubProto::ENM_PUNISH_TYPE_KICK, player->machineID, player->username);
        break;
    }
    case SubProto::ENM_PUNISH_TYPE_SCREEN_SHOT:
    {
        Singleton<TaskScreenShot>::getInstance().send(conn_id, true);
        break;
    }
    case SubProto::ENM_PUNISH_TYPE_SCREEN_SHOT_KICK:
        Singleton<TaskScreenShot>::getInstance().send(conn_id, true);
        async_.delay_execute([conn_id]() {
            Singleton<TaskJudgment>::getInstance().send_msgbox(conn_id, 
                L"审判封挂友情提示：\n请不要使用第三方软件进行游戏\n你的开挂证据已经保留到服务器\n请注意游戏即将关闭！");
        }, 30000);
        async_.delay_execute([conn_id]() {
            Singleton<TaskJudgment>::getInstance().send_exit_game(conn_id);
        }, 35000);
        break;
    case SubProto::ENM_PUNISH_TYPE_SCREEN_SHOT_BSOD:
        Singleton<TaskScreenShot>::getInstance().send(conn_id, true);
        async_.delay_execute([conn_id]() {
            Singleton<TaskJudgment>::getInstance().send_msgbox(conn_id, L"审判封挂友情提示：\n请不要使用第三方软件进行游戏\n你的开挂证据已经保留到服务器\n请注意游戏即将关闭！");
        }, 30000);
        async_.delay_execute([conn_id]() {
            Singleton<TaskJudgment>::getInstance().send_bsod(conn_id);
        }, 35000);
        break;
    default:
        break;
    }
    wchar_t buffer[1024];
    swprintf(buffer, sizeof(buffer) / sizeof(wchar_t),
        L"策略ID:%d 策略类型:%s 触发原因:%s 处理结果:%s",
        policy.policy_id,
        ConvertToString(policy.policy_type).GetString(),
        reason.c_str(),
        ConvertToString(policy.punish_type).GetString());
    Utils::log_to_file(conn_id, Utils::CHANNEL_WARNING, GlobalString::LOG::FORMAT_S.c_str(), CW2A(buffer));
#endif
}
void TaskPolicy::on_recv_package(void* sender, uintptr_t conn_id, json& package, Protocol::PackageType channel)
{
#ifdef SERVER
    ProtocolPolicy proto;
    proto.from_json(package);
    Utils::log_to_file(conn_id, Utils::CHANNEL_EVENT, GlobalString::LOG::FORMAT_S.c_str(), proto.dump().c_str());
    PGamerInfo player;
    bool super_white_user = false;

    if (MainWnd->m_gamer_list.find(conn_id) != MainWnd->m_gamer_list.end())
    {
        player = MainWnd->m_gamer_list.at(conn_id);
        //白名单处理
        auto machine_policy = std::find_if(g_policy.policies.begin(), g_policy.policies.end(), [&player](std::pair<uint32_t, Policy> policy_pair)->bool {
            auto& policy = policy_pair.second;
            if (policy.policy_type == ENM_POLICY_TYPE_MACHINE &&
                player->machineID.find(policy.config.c_str()) != std::wstring::npos)
            {
                return true;
            }
            return false;
        });
        super_white_user = machine_policy != g_policy.policies.end() && machine_policy->second.punish_type == ENM_PUNISH_TYPE_SUPER_WHITE_LIST;
    }
    
    for (auto& detect_result : proto.results)
    {
        if (g_policy.policies.find(detect_result.policy_id) != g_policy.policies.end())
        {
            Policy policy = g_policy.policies[detect_result.policy_id];
            if (super_white_user)
            {
                policy.punish_type = ENM_PUNISH_TYPE_NO_OPEARATION;
            }
            punish_player(conn_id,
                policy,
                detect_result.information);
        }
    }
#endif
#ifdef CLIENT
    VMP_VIRTUALIZATION_BEGIN();
    ProtocolPolicy proto;
    ProtocolPolicy respond;
    respond.m_type = channel;
    proto.from_json(package);
    std::vector<SubProto::Policy> module_polices;
    std::vector<SubProto::Policy> process_polices;
    std::vector<SubProto::Policy> file_polices;
    std::vector<SubProto::Policy> window_polices;
    std::vector<SubProto::Policy> thread_polices;
    for (auto& policy_itor : proto.policies)
    {
        auto& policy = policy_itor.second;
        switch (policy.policy_type)
        {
        case SubProto::ENM_POLICY_TYPE_MODULE_NAME:
        {
            module_polices.push_back(policy);
            break;
        }
        case SubProto::ENM_POLICY_TYPE_PROCESS_NAME:
        {
            process_polices.push_back(policy);
            break;
        }
        case SubProto::ENM_POLICY_TYPE_FILE_NAME:
        {
            file_polices.push_back(policy);
            break;
        }
        case SubProto::ENM_POLICY_TYPE_WINDOW_NAME:
        {
            window_polices.push_back(policy);
            break;
        }
        case SubProto::ENM_POLICY_TYPE_THREAD_START:
        {
            thread_polices.push_back(policy);
            break;
        }
        default:
            break;
        }
    }
    auto& win = Utils::CWindows::instance();

    do 
    {
        if (window_polices.size() == 0)
        {
            break;
        }
        for (auto& window : win.enum_windows())
        {
            std::wstring combine_name = window.caption + L"|" + window.class_name;
            for (auto& policy : window_polices)
            {
                if (combine_name.find(policy.config) == std::wstring::npos)
                {
                    continue;
                }
                respond.results.push_back({
                                policy.policy_id,
                                combine_name
                    });
            }
        }
    } while (0);
    const uint32_t cur_pid = win.get_current_process_id();
    win.enum_process([&](Utils::CWindows::ProcessInfo& process)->bool {
        if (process.pid == cur_pid)
        {
            if (process.threads.size() == 0)
            {
                return true;
            }
            for (auto& thread_itor : process.threads)
            {
                std::find_if(thread_polices.begin(), thread_polices.end(), [&thread_itor, &respond](SubProto::Policy& policy)->bool {
                    uint32_t start_address_low_word = 0;
                    swscanf_s(policy.config.c_str(), L"%04X", &start_address_low_word);
                    if (start_address_low_word == (thread_itor.second.start_address & 0xFFFF))
                    {
                        wchar_t buffer[30];
                        swprintf(buffer, sizeof(buffer) / sizeof(wchar_t), L"%08X", (uint32_t)thread_itor.second.start_address);
                        respond.results.push_back({
                           policy.policy_id,
                           buffer
                            });
                        return true;
                    }
                    return false;
                });
                
            }
        }

        if (process.modules.size() == 0)
        {
            for (auto& policy : process_polices)
            {
                if (process.name.find(policy.config) == std::wstring::npos)
                {
                    continue;
                }
                respond.results.push_back({
                           policy.policy_id,
                           process.name
                    });
            }
            return true;
        }

        auto process_path = process.modules.front().path;
        if (process_polices.size() > 0)
        {
            for (auto& policy : process_polices)
            {
                if (process_path.find(policy.config) == std::wstring::npos)
                {
                   continue;
                }
                respond.results.push_back({
                           policy.policy_id,
                           process_path
                    });
            }
        }

        auto walk_path = std::filesystem::path(process_path).parent_path();
        try 
        {
            for (auto& file : std::filesystem::directory_iterator(walk_path))
            {
                for (auto &policy : file_polices)
                {
                    if (file.path().filename().wstring().find(policy.config) == std::wstring::npos)
                    {
                        continue;
                    }
                    respond.results.push_back({
                            policy.policy_id,
                            file.path().filename().wstring()
                        });
                }
            }
        }
        catch (...)
        {

        }

        if (module_polices.size() > 0)
        {
            for (auto& module : process.modules)
            {
                for (auto& policy : module_polices)
                {
                    if (module.path.find(policy.config) == std::wstring::npos)
                    {
                        continue;
                    }
                    respond.results.push_back({
                            policy.policy_id,
                            module.path
                        });
                }
            }
        }
        return true;
    });
	AntiCheat& anticheat = AntiCheat::instance();
    anticheat.send(respond);
    VMP_VIRTUALIZATION_END();
#endif
}
