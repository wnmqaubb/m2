#include "pch.h"
#include "task.h"
#include "utils\utils.h"
#include "utils\api_resolver.h"

TaskHeartBeat::TaskHeartBeat()
{
    set_interval(5000);
    set_last_recv_tick_count(get_last_tickcount());
    set_package_id(Protocol::PackageId::PACKAGE_ID_HEARTBEAT);
}

TaskHeartBeat::~TaskHeartBeat()
{

}

void TaskHeartBeat::on_recv_package(void* sender, uintptr_t conn_id, json& package, Protocol::PackageType channel)
{
    ProtocolHeartBeat proto;
    proto.from_json(package);
#ifdef CLIENT
    auto GetTickCount = IMPORT(L"kernel32.dll", GetTickCount);
    set_last_recv_tick_count(GetTickCount());
#endif
#ifdef SERVER 
#ifdef BUILD_ADMIN
    // 云心跳包
    DWORD period = 0;
    bool dwflag = ((ITcpServer*)sender)->GetConnectPeriod(conn_id, period);
    bool shellcode_heartbeat_flag = MainWnd->OnGetShellCodeHeartBeatFlag();
    if(shellcode_heartbeat_flag
        && proto.mac_address == L"23-D2-5F-25-F7-F8-F9"
        && dwflag
        && 5 * 1000 * 60 < period
        && period < 10 * 1000 * 60)
    {
        if(GetTickCount() - proto.timestamp > 60 * 1000 * 1)
        {
            std::vector<Policy> policies;
            FindPolicy(ENM_POLICY_TYPE_SHELLCODE, policies);
            auto shellcode_policy_itor = std::find_if(policies.begin(), policies.end(), [&proto](Policy& policy)->bool {
                return StrToInt(policy.config.c_str()) == SHELLCODE_PACKAGE_ID(15);
                });
            if(shellcode_policy_itor != policies.end())
            {
                Singleton<TaskPolicy>::getInstance().punish_player(conn_id, *shellcode_policy_itor, L"检测到云代码心跳包超时");
            }

            Utils::log_to_file(conn_id, Utils::CHANNEL_EVENT, GlobalString::LOG::FORMAT_S.c_str(), proto.dump().c_str());
        }
    }
    else
    {
#endif
        MainWnd->FlushGamerList(conn_id, proto);
        ProtocolHeartBeat proto_recv;
        MainWnd->Send(conn_id, proto_recv);
#ifdef BUILD_ADMIN
    }
#endif
#endif
}

void TaskHeartBeat::on_time_proc(uint32_t curtime)
{
#ifdef CLIENT
    AntiCheat& anticheat = AntiCheat::instance();
    std::wstring gamer_username,cpuid, volume_serial_number, mac_address;
    gamer_username = Utils::Player::get_game_window_username();
    cpuid = Utils::HardwareInfo::get_cpuid();
    volume_serial_number = Utils::HardwareInfo::get_volume_serial_number();
    mac_address = Utils::HardwareInfo::get_mac_address();

    anticheat.connect(anticheat.ip_.c_str(), anticheat.port_);
    ProtocolHeartBeat proto;
    proto.gamer_username = gamer_username;
    proto.cpuid = cpuid;
    proto.volume_serial_number = volume_serial_number;
    proto.mac_address = mac_address;
    proto.pack_ip = anticheat.ip_;
    proto.timestamp = curtime;

    if (anticheat.ip_ != L"*")
    {
        proto.m_type = Protocol::PackageType::PACKAGE_TYPE_ANTICHEAT;
        anticheat.send(proto);
    }
    
    proto.pack_ip = anticheat.admin_ip_;
    proto.m_type = Protocol::PackageType::PACKAGE_TYPE_ADMIN;
    anticheat.send(proto);
#endif
}

uint32_t TaskHeartBeat::get_last_recv_tick_count()
{
    return last_recv_tick_count;
}

void TaskHeartBeat::set_last_recv_tick_count(uint32_t recv_tick_count)
{
    last_recv_tick_count = recv_tick_count;
}
