#include "pch.h"
#include "ClientImpl.h"
#include "version.build"
#include "TaskBasic.h"

using namespace Utils;

CClientImpl::CClientImpl(/*asio::io_service& io_*/) : super(/*io_*/)
{
    char path[MAX_PATH] = { 0 };
    GetModuleFileNameA(GetModuleHandleA(NULL), path, sizeof(path));
    exe_path_ = path;
    cache_dir_ = exe_path_.parent_path() / "cache";
    std::error_code ec;
    if (fs::is_directory(cache_dir_, ec) == false)
    {
        fs::create_directory(cache_dir_, ec);
    }
    notify_mgr().register_handler(CLIENT_DISCONNECT_NOTIFY_ID, [this]() {
        LOG("失去连接");
    });
    notify_mgr().register_handler(CLIENT_CONNECT_FAILED_NOTIFY_ID, [this]() {
        auto ec = asio2::get_last_error();
        LOG("连接失败: %s ", ec.message().c_str());
    });
    notify_mgr().register_handler(CLIENT_CONNECT_SUCCESS_NOTIFY_ID, [this]() {
        LOG("握手");
        ProtocolC2SHandShake handshake;
        memcpy(&handshake.uuid, uuid().data, sizeof(handshake.uuid));
        handshake.system_version = std::any_cast<int>(user_data().get_field(sysver_field_id));
        handshake.is_64bit_system = std::any_cast<bool>(user_data().get_field(is_64bits_field_id));
        handshake.cpuid = std::any_cast<std::wstring>(user_data().get_field(cpuid_field_id));
        handshake.mac = std::any_cast<std::wstring>(user_data().get_field(mac_field_id));
        handshake.volume_serial_number = std::any_cast<std::wstring>(user_data().get_field(vol_field_id));
        handshake.rev_version = std::any_cast<int>(user_data().get_field(rev_version_field_id));
        handshake.commited_hash = std::any_cast<std::string>(user_data().get_field(commited_hash_field_id));
        handshake.pid = GetCurrentProcessId();
        this->save_uuid(handshake);
        send(&handshake);
		start_timer<unsigned int>(CLIENT_HEARTBEAT_TIMER_ID, heartbeat_duration(), [this]() {
			ProtocolC2SHeartBeat heartbeat;
			heartbeat.tick = time(0);
			send(&heartbeat);
			LOG("发送心跳");
			});
		post([this]() {
            if (!is_loaded_plugin())
            {
			    LoadPlugin(this);
            }
		},std::chrono::milliseconds(200));
        // 发送用户名 防止断开后重连时网关用户名为空
        notify_mgr().dispatch (CLIENT_RECONNECT_SUCCESS_NOTIFY_ID);
    });

	notify_mgr().register_handler(ON_RECV_HEARTBEAT_NOTIFY_ID, [this]() {
		LOG("接收心跳");
		});

    notify_mgr().register_handler(CLIENT_START_NOTIFY_ID, [this]() {
        LOG("客户端初始化成功");
        user_data().set_field(sysver_field_id, (int)CWindows::instance().get_system_version());
        user_data().set_field(is_64bits_field_id, CWindows::instance().is_64bits_system());
        user_data().set_field(cpuid_field_id, Utils::HardwareInfo::get_cpuid());
        user_data().set_field(mac_field_id, Utils::HardwareInfo::get_mac_address());
        user_data().set_field(vol_field_id, Utils::HardwareInfo::get_volume_serial_number());
        user_data().set_field(rev_version_field_id, (int)REV_VERSION);
        user_data().set_field(commited_hash_field_id, std::string(VER2STR(COMMITED_HASH)));
        this->load_uuid();
    });
    package_mgr().register_handler(SPKG_ID_S2C_PUNISH, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        switch (msg.get().as<ProtocolS2CPunish>().type)
        {
        case PunishType::ENM_PUNISH_TYPE_KICK:
            Utils::CWindows::instance().exit_process();
            break;
        default:
            break;
        }
	});
}

void CClientImpl::on_recv(unsigned int package_id, const RawProtocolImpl& package, const msgpack::v1::object_handle&)
{
    LOG("收到:%d", package_id);
}

void CClientImpl::load_uuid()
{
    std::wstring volume_serial_number = std::any_cast<std::wstring>(user_data().get_field(vol_field_id));
    unsigned int volume_serial_number_hash_val = ApiResolver::hash(volume_serial_number.c_str(), volume_serial_number.size());
    std::ifstream file(cache_dir_ / std::to_string(volume_serial_number_hash_val).c_str(), std::ios::in | std::ios::binary);
    if (file.is_open())
    {
        std::stringstream ss;
        ss << file.rdbuf();
        auto str = ss.str();
        auto handshake = ProtocolC2SHandShake::load(str.data(), str.size());
        if (handshake)
        {
            memcpy(&uuid_, handshake->uuid, sizeof(handshake->uuid));
        }
        file.close();
    }
}

void CClientImpl::save_uuid(const ProtocolC2SHandShake& handshake)
{
    std::wstring volume_serial_number = std::any_cast<std::wstring>(user_data().get_field(vol_field_id));
    unsigned int volume_serial_number_hash_val = ApiResolver::hash(volume_serial_number.c_str(), volume_serial_number.size()); 
    std::ofstream file(cache_dir_ / std::to_string(volume_serial_number_hash_val).c_str(), std::ios::out | std::ios::binary);
    if (file.is_open())
    {
        auto str = handshake.dump();
        file.write(str.data(), str.size());
        file.close();
    }
}