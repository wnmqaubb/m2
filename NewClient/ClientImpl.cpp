<<<<<<< HEAD
#include "pch.h"
#include "ClientImpl.h"
#include "version.build"

#define log(x,...)

using namespace Utils;

CClientImpl::CClientImpl(asio::io_service& io_) : super(io_)
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
    plugin_mgr_ = std::make_unique<CClientPluginMgr>(cache_dir_);
    plugin_mgr_->set_client_instance(this);

    notify_mgr().register_handler(CLIENT_DISCONNECT_NOTIFY_ID, [this]() {
        log(LOG_TYPE_DEBUG, TEXT("失去连接"));
    });
    notify_mgr().register_handler(CLIENT_CONNECT_FAILED_NOTIFY_ID, [this]() {
        log(LOG_TYPE_DEBUG, TEXT("连接失败"));
    });
    notify_mgr().register_handler(CLIENT_CONNECT_SUCCESS_NOTIFY_ID, [this]() {
        log(LOG_TYPE_DEBUG, TEXT("握手"));
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
        auto self(shared_from_this());
        start_timer(CLIENT_HEARTBEAT_TIMER_ID, heartbeat_duration(), [self, this]() {
            ProtocolC2SHeartBeat heartbeat;
            heartbeat.tick = time(0);
            send(&heartbeat);
            log(LOG_TYPE_DEBUG, TEXT("发送心跳"));
        });
    });
    notify_mgr().register_handler(ON_RECV_HANDSHAKE_NOTIFY_ID, [this]() {
        ProtocolC2SQueryPlugin req;
        send(&req);
        log(LOG_TYPE_DEBUG, TEXT("查询插件列表"));
    });
    notify_mgr().register_handler(ON_RECV_HEARTBEAT_NOTIFY_ID, [this]() {
        log(LOG_TYPE_DEBUG, TEXT("接收心跳"));
#if 0
        ProtocolC2SQueryPlugin req;
        send(&req);
        log(Debug, TEXT("查询插件列表"));
#endif
    });
    start_timer(QUERY_PLUGIN_LIST_TIMER_ID, std::chrono::minutes(2), [this]() {
        ProtocolC2SQueryPlugin req;
        send(&req);
        log(LOG_TYPE_DEBUG, TEXT("查询插件列表"));
    });
    notify_mgr().register_handler(CLIENT_START_NOTIFY_ID, [this]() {
        log(LOG_TYPE_DEBUG, TEXT("客户端初始化成功"));
        user_data().set_field(sysver_field_id, (int)CWindows::instance().get_system_version());
        user_data().set_field(is_64bits_field_id, CWindows::instance().is_64bits_system());
        user_data().set_field(cpuid_field_id, Utils::HardwareInfo::get_cpuid());
        user_data().set_field(mac_field_id, Utils::HardwareInfo::get_mac_address());
        user_data().set_field(vol_field_id, Utils::HardwareInfo::get_volume_serial_number());
        user_data().set_field(rev_version_field_id, (int)REV_VERSION);
        user_data().set_field(commited_hash_field_id, std::string(VER2STR(COMMITED_HASH)));
        this->load_uuid();
    });
    package_mgr().register_handler(SPKG_ID_S2C_QUERY_PLUGIN, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
        auto msg = raw_msg.get().as<ProtocolS2CQueryPlugin>();
        log(LOG_TYPE_DEBUG, TEXT("收到插件列表：%d"), msg.plugin_list.size());
        auto& resp = raw_msg.get().as<ProtocolS2CQueryPlugin>();
        for (auto[plugin_hash, plugin_name] : resp.plugin_list)
        {
            if (plugin_mgr_->is_plugin_cache_exist(plugin_hash))
            {
                if (!plugin_mgr_->is_plugin_loaded(plugin_hash))
                {
                    super::io().post([this, plugin_hash = plugin_hash, plugin_name = plugin_name]() {
                        if (plugin_mgr_->load_plugin(plugin_hash, plugin_name))
                        {
                            log(LOG_TYPE_DEBUG, TEXT("加载缓存插件成功：%s"), Utils::String::c2w(plugin_name).c_str());
                        }
                        else
                        {
                            log(LOG_TYPE_DEBUG, TEXT("加载缓存插件失败：%s"), Utils::String::c2w(plugin_name).c_str());
                        }
                    });
                }
            }
            else
            {
                ProtocolC2SDownloadPlugin req;
                req.plugin_hash = plugin_hash;
                send(&req);
            }
        }
    });

    package_mgr().register_handler(SPKG_ID_S2C_DOWNLOAD_PLUGIN, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        auto& resp = msg.get().as<ProtocolS2CDownloadPlugin>();
        if (plugin_mgr_->is_plugin_cache_exist(resp.plugin_hash))
        {
            return;
        }
        plugin_mgr_->save_plugin(package, resp);
        super::io().post([this, plugin_hash = resp.plugin_hash, plugin_name = resp.plugin_name]() {
            if (plugin_mgr_->load_plugin(plugin_hash, plugin_name))
            {
                log(LOG_TYPE_DEBUG, TEXT("加载远程插件成功：%s"), Utils::String::c2w(plugin_name).c_str());
            }
            else
            {
                log(LOG_TYPE_DEBUG, TEXT("加载远程插件成功：%s"), Utils::String::c2w(plugin_name).c_str());
            }
        });
    });

    package_mgr().register_handler(SPKG_ID_S2C_CHECK_PLUGIN, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        ProtocolC2SCheckPlugin resp;
        resp.plugin_list = plugin_mgr_->get_plugin_list();
        send(&resp, package.head.session_id);
    });

    package_mgr().register_handler(SPKG_ID_S2C_PUNISH, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        switch (msg.get().as<ProtocolS2CPunish>().type)
        {
        case PunishType::ENM_PUNISH_TYPE_BSOD:
            stop();
            Utils::CWindows::instance().bsod();
            break;
        case PunishType::ENM_PUNISH_TYPE_KICK:
            stop();
            Utils::CWindows::instance().exit_process();
            break;
        default:
            break;
        }
    });
}

void CClientImpl::on_recv(unsigned int package_id, const RawProtocolImpl& package, const msgpack::v1::object_handle&)
{
    log(LOG_TYPE_DEBUG, TEXT("收到:%d"), package_id);
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
=======
#include "pch.h"
#include "ClientImpl.h"
#include "version.build"

#define log(x,...)

using namespace Utils;

CClientImpl::CClientImpl(asio::io_service& io_) : super(io_)
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
    plugin_mgr_ = std::make_unique<CClientPluginMgr>(cache_dir_);
    plugin_mgr_->set_client_instance(this);

    notify_mgr().register_handler(CLIENT_DISCONNECT_NOTIFY_ID, [this]() {
        log(LOG_TYPE_DEBUG, TEXT("失去连接"));
    });
    notify_mgr().register_handler(CLIENT_CONNECT_FAILED_NOTIFY_ID, [this]() {
        log(LOG_TYPE_DEBUG, TEXT("连接失败"));
    });
    notify_mgr().register_handler(CLIENT_CONNECT_SUCCESS_NOTIFY_ID, [this]() {
        log(LOG_TYPE_DEBUG, TEXT("握手"));
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
        auto self(shared_from_this());
        start_timer(CLIENT_HEARTBEAT_TIMER_ID, heartbeat_duration(), [self, this]() {
            ProtocolC2SHeartBeat heartbeat;
            heartbeat.tick = time(0);
            send(&heartbeat);
            log(LOG_TYPE_DEBUG, TEXT("发送心跳"));
        });
    });
    notify_mgr().register_handler(ON_RECV_HANDSHAKE_NOTIFY_ID, [this]() {
        ProtocolC2SQueryPlugin req;
        send(&req);
        log(LOG_TYPE_DEBUG, TEXT("查询插件列表"));
    });
    notify_mgr().register_handler(ON_RECV_HEARTBEAT_NOTIFY_ID, [this]() {
        log(LOG_TYPE_DEBUG, TEXT("接收心跳"));
#if 0
        ProtocolC2SQueryPlugin req;
        send(&req);
        log(Debug, TEXT("查询插件列表"));
#endif
    });
    start_timer(QUERY_PLUGIN_LIST_TIMER_ID, std::chrono::minutes(2), [this]() {
        ProtocolC2SQueryPlugin req;
        send(&req);
        log(LOG_TYPE_DEBUG, TEXT("查询插件列表"));
    });
    notify_mgr().register_handler(CLIENT_START_NOTIFY_ID, [this]() {
        log(LOG_TYPE_DEBUG, TEXT("客户端初始化成功"));
        user_data().set_field(sysver_field_id, (int)CWindows::instance().get_system_version());
        user_data().set_field(is_64bits_field_id, CWindows::instance().is_64bits_system());
        user_data().set_field(cpuid_field_id, Utils::HardwareInfo::get_cpuid());
        user_data().set_field(mac_field_id, Utils::HardwareInfo::get_mac_address());
        user_data().set_field(vol_field_id, Utils::HardwareInfo::get_volume_serial_number());
        user_data().set_field(rev_version_field_id, (int)REV_VERSION);
        user_data().set_field(commited_hash_field_id, std::string(VER2STR(COMMITED_HASH)));
        this->load_uuid();
    });
    package_mgr().register_handler(SPKG_ID_S2C_QUERY_PLUGIN, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
        auto msg = raw_msg.get().as<ProtocolS2CQueryPlugin>();
        log(LOG_TYPE_DEBUG, TEXT("收到插件列表：%d"), msg.plugin_list.size());
        auto& resp = raw_msg.get().as<ProtocolS2CQueryPlugin>();
        for (auto[plugin_hash, plugin_name] : resp.plugin_list)
        {
            if (plugin_mgr_->is_plugin_cache_exist(plugin_hash))
            {
                if (!plugin_mgr_->is_plugin_loaded(plugin_hash))
                {
                    super::io().post([this, plugin_hash = plugin_hash, plugin_name = plugin_name]() {
                        if (plugin_mgr_->load_plugin(plugin_hash, plugin_name))
                        {
                            log(LOG_TYPE_DEBUG, TEXT("加载缓存插件成功：%s"), Utils::String::c2w(plugin_name).c_str());
                        }
                        else
                        {
                            log(LOG_TYPE_DEBUG, TEXT("加载缓存插件失败：%s"), Utils::String::c2w(plugin_name).c_str());
                        }
                    });
                }
            }
            else
            {
                ProtocolC2SDownloadPlugin req;
                req.plugin_hash = plugin_hash;
                send(&req);
            }
        }
    });

    package_mgr().register_handler(SPKG_ID_S2C_DOWNLOAD_PLUGIN, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        auto& resp = msg.get().as<ProtocolS2CDownloadPlugin>();
        if (plugin_mgr_->is_plugin_cache_exist(resp.plugin_hash))
        {
            return;
        }
        plugin_mgr_->save_plugin(package, resp);
        super::io().post([this, plugin_hash = resp.plugin_hash, plugin_name = resp.plugin_name]() {
            if (plugin_mgr_->load_plugin(plugin_hash, plugin_name))
            {
                log(LOG_TYPE_DEBUG, TEXT("加载远程插件成功：%s"), Utils::String::c2w(plugin_name).c_str());
            }
            else
            {
                log(LOG_TYPE_DEBUG, TEXT("加载远程插件成功：%s"), Utils::String::c2w(plugin_name).c_str());
            }
        });
    });

    package_mgr().register_handler(SPKG_ID_S2C_CHECK_PLUGIN, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        ProtocolC2SCheckPlugin resp;
        resp.plugin_list = plugin_mgr_->get_plugin_list();
        send(&resp, package.head.session_id);
    });

    package_mgr().register_handler(SPKG_ID_S2C_PUNISH, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        switch (msg.get().as<ProtocolS2CPunish>().type)
        {
        case PunishType::ENM_PUNISH_TYPE_BSOD:
            stop();
            Utils::CWindows::instance().bsod();
            break;
        case PunishType::ENM_PUNISH_TYPE_KICK:
            stop();
            Utils::CWindows::instance().exit_process();
            break;
        default:
            break;
        }
    });
}

void CClientImpl::on_recv(unsigned int package_id, const RawProtocolImpl& package, const msgpack::v1::object_handle&)
{
    log(LOG_TYPE_DEBUG, TEXT("收到:%d"), package_id);
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
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9
