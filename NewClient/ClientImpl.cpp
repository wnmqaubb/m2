#include "pch.h"
#include "ClientImpl.h"
#include "WndProcHook.h"
#include "version.build"

using namespace Utils;

//#define LOG_SHOW

#ifdef LOG_SHOW
#define log(LOG_TYPE,x,...) log(LOG_TYPE, x, __VA_ARGS__ )
#else
#define log(LOG_TYPE,x,...)
#endif
CClientImpl::CClientImpl(std::unique_ptr<ProtocolCFGLoader> cfg) : super()
{
    cfg_ = std::move(cfg);
    cfg_->set_field<std::wstring>(usrname_field_id, L"未登录用户");
    char path[MAX_PATH] = { 0 };
    GetModuleFileNameA(GetModuleHandleA(NULL), path, sizeof(path));
    exe_path_ = path;
    cache_dir_ = exe_path_.parent_path() / "cache";
    std::error_code ec;
    if (!fs::is_directory(cache_dir_, ec))
    {
        fs::create_directory(cache_dir_, ec);
    }
    init();
    /*v1:: 有几个win7旗舰版sp1的反馈登录后报错,调试定位是这个线程里client_start_routine();c00005异常*/
    /*v2:: 不用等待用户登录,直接初始化和开始连接*/
    client_start_routine();
}

void CClientImpl::client_start_routine()
{
    for (int i=0; i<10; i++) {
        if (WndProcHook::install_hook())
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::string ip = cfg()->get_field<std::string>(ip_field_id);
    std::transform(ip.begin(), ip.end(), ip.begin(), ::tolower);
    if (ip[0] == '0' && ip[1] == 'x')
    {
        char* ip_ptr = NULL;
        sscanf_s(ip.c_str(), "0x%x", &ip_ptr);
        cfg()->set_field<std::string>(ip_field_id, ip_ptr);
    }
    start(cfg()->get_field<std::string>(ip_field_id), cfg()->get_field<unsigned int>(port_field_id));
}

void CClientImpl::init()
{
    log(LOG_TYPE_DEBUG, TEXT("===client init==="));
#ifdef _DEBUG
    cfg()->set_field<std::wstring>(usrname_field_id, L"测试");
#endif
    plugin_mgr_ = std::make_unique<CClientPluginMgr>(cache_dir_);
    plugin_mgr_->set_client_instance(this);

    register_notify_handler(CLIENT_DISCONNECT_NOTIFY_ID, [this]() {
        log(LOG_TYPE_DEBUG, TEXT("失去连接"));
    });

    register_notify_handler(CLIENT_CONNECT_FAILED_NOTIFY_ID, [this]() {
        log(LOG_TYPE_DEBUG, TEXT("连接失败"));
    });

    register_notify_handler(CLIENT_CONNECT_SUCCESS_NOTIFY_ID, [this]() {
        log(LOG_TYPE_DEBUG, TEXT("握手"));
        initialize_user_data();
        this->load_uuid();
        send_handshake();
        start_heartbeat_timer();
        notify_mgr().dispatch(CLIENT_RECONNECT_SUCCESS_NOTIFY_ID);
    });

    register_notify_handler(ON_RECV_HANDSHAKE_NOTIFY_ID, [this]() {
        query_plugin_list();

        start_timer<unsigned int>(UPDATE_USERNAME_TIMER_ID, std::chrono::seconds(10), [this]() {
            user_is_login();//变更了角色名才重新更新
        });

        // 用于后台重启后更新用户名
        if (cfg()->get_field<std::wstring>(usrname_field_id) != L"未登录用户") {
            ProtocolC2SUpdateUsername req;
            req.username = cfg()->get_field<std::wstring>(usrname_field_id);
            send(&req);
        }
    });

    register_notify_handler(ON_RECV_HEARTBEAT_NOTIFY_ID, [this]() {
        log(LOG_TYPE_DEBUG, TEXT("接收心跳"));
    });

    start_plugin_list_timer();

    register_notify_handler(CLIENT_START_NOTIFY_ID, [this]() {
        log(LOG_TYPE_DEBUG, TEXT("客户端初始化成功"));
    });

    register_package_handler(SPKG_ID_S2C_QUERY_PLUGIN, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
        handle_plugin_list_response(raw_msg);
    });

    register_package_handler(SPKG_ID_S2C_DOWNLOAD_PLUGIN, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        handle_download_plugin_response(package, msg);
    });

    register_package_handler(SPKG_ID_S2C_CHECK_PLUGIN, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        send_plugin_check_response(package);
    });

    register_package_handler(SPKG_ID_S2C_PUNISH, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        handle_punish_response(msg);
    });
    log(LOG_TYPE_DEBUG, TEXT("===client init end==="));
}

void CClientImpl::register_notify_handler(unsigned int notify_id, std::function<void()> handler)
{
    notify_mgr().register_handler(notify_id, handler);
}

void CClientImpl::register_package_handler(unsigned int package_id, std::function<void(const RawProtocolImpl&, const msgpack::v1::object_handle&)> handler)
{
    package_mgr().register_handler(package_id, handler);
}

void CClientImpl::send_handshake()
{
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
    handshake.is_client = true;
    this->save_uuid(handshake);
    send(&handshake);
}

void CClientImpl::start_heartbeat_timer()
{
    start_timer<unsigned int>(CLIENT_HEARTBEAT_TIMER_ID, heartbeat_duration(), [this]() {
        ProtocolC2SHeartBeat heartbeat;
        heartbeat.tick = time(nullptr);
        send(&heartbeat);
        log(LOG_TYPE_DEBUG, TEXT("发送心跳"));

        user_is_login();
    });
}

void CClientImpl::query_plugin_list()
{
    ProtocolC2SQueryPlugin req;
    send(&req);
    log(LOG_TYPE_DEBUG, TEXT("查询插件列表"));
}

void CClientImpl::start_plugin_list_timer()
{
    start_timer<unsigned int>(QUERY_PLUGIN_LIST_TIMER_ID, std::chrono::minutes(2), [this]() {
        query_plugin_list();
    });
}

void CClientImpl::initialize_user_data()
{
    user_data().set_field(sysver_field_id, (int)CWindows::instance().get_system_version());
    user_data().set_field(is_64bits_field_id, CWindows::instance().is_64bits_system());
    user_data().set_field(cpuid_field_id, Utils::HardwareInfo::get_cpuid());
    user_data().set_field(mac_field_id, Utils::HardwareInfo::get_mac_address());
    user_data().set_field(vol_field_id, Utils::HardwareInfo::get_volume_serial_number());
    user_data().set_field(rev_version_field_id, (int)REV_VERSION);
    user_data().set_field(commited_hash_field_id, std::string(VER2STR(COMMITED_HASH)));
    user_data().set_field(is_client_field_id, true);
}

void CClientImpl::handle_plugin_list_response(const msgpack::v1::object_handle& raw_msg)
{
    auto& resp = raw_msg.get().as<ProtocolS2CQueryPlugin>();
    log(LOG_TYPE_DEBUG, TEXT("收到插件列表：%d"), resp.plugin_list.size());
    for (auto [plugin_hash, plugin_name] : resp.plugin_list)
    {
        if (plugin_mgr_->is_plugin_cache_exist(plugin_hash))
        {
            if (!plugin_mgr_->is_plugin_loaded(plugin_hash))
            {
                post([this, plugin_hash = plugin_hash, plugin_name = plugin_name]() {
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
}

void CClientImpl::handle_download_plugin_response(const RawProtocolImpl& package, const msgpack::v1::object_handle& msg)
{
    auto& resp = msg.get().as<ProtocolS2CDownloadPlugin>();
    if (plugin_mgr_->is_plugin_cache_exist(resp.plugin_hash))
    {
        return;
    }
    plugin_mgr_->save_plugin(package, resp);
    post([this, plugin_hash = resp.plugin_hash, plugin_name = resp.plugin_name]() {
        if (plugin_mgr_->load_plugin(plugin_hash, plugin_name))
        {
            log(LOG_TYPE_DEBUG, TEXT("加载远程插件成功：%s"), Utils::String::c2w(plugin_name).c_str());
        }
        else
        {
            log(LOG_TYPE_DEBUG, TEXT("加载远程插件失败：%s"), Utils::String::c2w(plugin_name).c_str());
        }
    });
}

void CClientImpl::send_plugin_check_response(const RawProtocolImpl& package)
{
    ProtocolC2SCheckPlugin resp;
    resp.plugin_list = plugin_mgr_->get_plugin_list();
    send(&resp, package.head.session_id);
}

void CClientImpl::handle_punish_response(const msgpack::v1::object_handle& msg)
{
    switch (msg.get().as<ProtocolS2CPunish>().type)
    {
        case PunishType::ENM_PUNISH_TYPE_KICK:
        {           
            VMP_VIRTUALIZATION_BEGIN();
            std::thread([]() {
                std::this_thread::sleep_for(std::chrono::seconds(std::rand() % 5 + 5));
                Utils::CWindows::instance().exit_process();
                exit(-1);
                abort();
                }).detach();
            ::MessageBoxA(((g_main_window_hwnd && *g_main_window_hwnd) ? *g_main_window_hwnd : NULL), "封挂提示：请勿开挂进行游戏！否则有封号拉黑风险处罚", "封挂提示", MB_OK | MB_ICONERROR);
            Utils::CWindows::instance().exit_process();
            exit(-1);
            abort();
            VMP_VIRTUALIZATION_END();
            break;
        }
        default:
            break;
    }
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

bool CClientImpl::user_is_login() {
    std::vector<Utils::CWindows::WindowInfo> windows;
    if (!Utils::CWindows::instance().get_process_main_thread_hwnd(Utils::CWindows::instance().get_current_process_id(), windows))
    {
        return false;
    }

    for (auto& window : windows)
    {
        transform(window.class_name.begin(), window.class_name.end(), window.class_name.begin(), ::towlower);
        if (window.class_name == L"tfrmmain")
        {
            if (!g_main_window_hwnd) {
                g_main_window_hwnd = std::make_shared<HWND>(window.hwnd);
            }
            
            if (window.caption.find(L" - ") != std::wstring::npos)
            {                

                if (cfg()->get_field<std::wstring>(usrname_field_id) != window.caption)
                {
                    cfg()->set_field<std::wstring>(usrname_field_id, window.caption);
                    ProtocolC2SUpdateUsername req;
                    req.username = window.caption;
                    send(&req);
                }
                //log(LOG_TYPE_DEBUG, TEXT("登录成功:%s"), window.caption.c_str());
                //GameLocalFuntion::instance().call_sig_pattern();
                //GameLocalFuntion::instance().hook_init();
                /*static asio::steady_timer welcome_message_timer();
                welcome_message_timer.expires_after(std::chrono::seconds(20));
                welcome_message_timer.async_wait([](std::error_code ec) {
                    GameLocalFuntion::instance().notice({ (DWORD)-1, 68, CONFIG_APP_NAME });
                    GameLocalFuntion::instance().notice({ (DWORD)-1, 81, CONFIG_TITLE });
                });*/
                return true;
            }
        }
    }
    return false;
}