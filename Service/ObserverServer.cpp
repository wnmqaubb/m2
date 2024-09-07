#include "pch.h"
#include "ObserverServer.h"
#include "VmpSerialValidate.h"

CObserverServer::CObserverServer()
{
    static VmpSerialValidator vmp(this);
    logic_client_ = std::make_shared<CLogicClient>();
    is_observer_server_ = true;
    set_log_cb(std::bind(&CObserverServer::log_cb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    logic_client_->sub_notify_mgr_.register_handler(CLIENT_CONNECT_SUCCESS_NOTIFY_ID, [this]() {
        foreach_session([this](tcp_session_shared_ptr_t& session) {
            auto user_data = get_user_data_(session);
            if (user_data->has_handshake && user_data->get_field<bool>("is_observer_client"))
            {
                ProtocolLC2LSAddObsSession req;
                req.session_id = session->hash_key();
                logic_client_->send(&req);
            }
            else
            {
                ProtocolLC2LSAddUsrSession req;
                auto& _userdata = req.data;
                _userdata.session_id = session->hash_key();
                memcpy(_userdata.uuid, user_data->uuid, sizeof(_userdata.uuid));
                _userdata.has_handshake = user_data->has_handshake;
                _userdata.last_heartbeat_time = user_data->last_heartbeat_time;
                _userdata.json = user_data->data;
                logic_client_->send(&req);
            }
        });
    });
    logic_client_->package_mgr().register_handler(LSPKG_ID_S2C_SEND, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg)
    {
        auto resp = msg.get().as<ProtocolLS2LCSend>();
        auto session_id = resp.package.head.session_id;
        auto session = sessions().find(session_id);
        send(session, resp.package);
    });
    logic_client_->package_mgr().register_handler(LSPKG_ID_S2C_SET_FIELD, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg)
    {
        auto param = msg.get().as<ProtocolLS2LCSetField>();
        auto session = sessions().find(param.session_id);
        if (session)
        {
            get_user_data_(session)->set_field(param.key, param.val);
            // 发给管理员网关
            if (get_user_data_(session)->get_field<bool>("is_observer_client"))
            {
                ProtocolOBS2OBCSetField req;
                req.session_id = param.session_id;
                req.key = param.key;
                req.val = param.val;
                send(session, &req);
            }
        }
    });
	logic_client_->package_mgr().register_handler(LSPKG_ID_S2C_KICK, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg)
	{
		auto param = msg.get().as<ProtocolLS2LCKick>();
		auto session = sessions().find(param.session_id);
		if (session)
		{
			session->stop();
		}
	});
	
    notify_mgr_.register_handler(SERVER_START_NOTIFY_ID, [this]() {
        io().context().post(std::bind(&VmpSerialValidator::validate_timer, vmp, false));
        start_timer(AUTH_CHECK_TIMER_ID, auth_check_timer_, std::bind(&VmpSerialValidator::validate_timer, vmp, true));
        connect_to_logic_server(kDefaultLocalhost, kDefaultLogicServicePort);
    });
    package_mgr_.register_handler(OBPKG_ID_C2S_AUTH, [this](tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
#if _DEBUG
        get_user_data_(session)->set_field("is_observer_client", true);
        log(LOG_TYPE_EVENT, TEXT("observer client auth success:%s:%u"), Utils::c2w(session->remote_address()).c_str(),
            session->remote_port());
        ProtocolLC2LSAddObsSession req;
        req.session_id = session->hash_key();
        logic_client_->send(&req);
        ProtocolOBS2OBCAuth auth;
        auth.status = true;
        send(session, &auth); 
        ProtocolOBS2OBCQueryVmpExpire resp;
        resp.vmp_expire = get_vmp_expire(); 
        send(session, &resp);
        return;
#else
        auto auth_key = raw_msg.get().as<ProtocolOBC2OBSAuth>().key;
        get_user_data_(session)->set_field("auth_key", auth_key);
        if (auth_key == auth_ticket_)
        {
            get_user_data_(session)->set_field("is_observer_client", true);
            log(LOG_TYPE_EVENT, TEXT("observer client auth success:%s:%u"), Utils::c2w(session->remote_address()).c_str(),
                session->remote_port());
            ProtocolLC2LSAddObsSession req;
            req.session_id = session->hash_key();
            logic_client_->send(&req);
            ProtocolOBS2OBCAuth auth;
            auth.status = true;
            send(session, &auth);
            ProtocolOBS2OBCQueryVmpExpire resp;
            resp.vmp_expire = get_vmp_expire();
            send(session, &resp);
        }
        else
        {
            get_user_data_(session)->set_field("is_observer_client", false);
            log(LOG_TYPE_ERROR, TEXT("observer client auth fail:%s:%u"), Utils::c2w(session->remote_address()).c_str(),
                session->remote_port());
            ProtocolOBS2OBCAuth auth;
            auth.status = false;
            send(session, &auth);
        }
        std::wstring username = TEXT("(NULL)");
        get_user_data_(session)->set_field("usrname", username);
#endif
    });
    // Gate to Service
    ob_pkg_mgr_.register_handler(OBPKG_ID_C2S_SEND, [this](tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
        auto req = raw_msg.get().as<ProtocolOBC2OBSSend>();
        send(sessions().find(req.package.head.session_id), req.package, session->hash_key());
    });
	ob_pkg_mgr_.register_handler(OBPKG_ID_C2S_KICK, [this](tcp_session_shared_ptr_t&, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
		auto req = raw_msg.get().as<ProtocolOBC2OBSKick>();
		auto session = sessions().find(req.session_id);
		if (session)
			session->stop();
	});
    // gate to service for freshuserlist
    ob_pkg_mgr_.register_handler(OBPKG_ID_C2S_QUERY_USERS, [this](tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
        ProtocolOBS2OBCQueryUsers resp;
        foreach_session([&resp](tcp_session_shared_ptr_t& session) -> void {
            ProtocolUserData _userdata;
            auto user_data = get_user_data_(session);
            if (user_data->has_handshake)
            {
                _userdata.session_id = session->hash_key();
                memcpy(_userdata.uuid, user_data->uuid, sizeof(_userdata.uuid));
                _userdata.has_handshake = user_data->has_handshake;
                _userdata.last_heartbeat_time = user_data->last_heartbeat_time;
                _userdata.json = user_data->data;
                resp.data.emplace(std::pair(_userdata.session_id, _userdata));
            }
        });
        send(session, &resp);
    });
    ob_pkg_mgr_.register_handler(OBPKG_ID_C2S_UPDATE_LOGIC, [this](tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
        auto buf = raw_msg.get().as<ProtocolOBC2OBSUpdateLogic>().data;
        ProtocolLC2LSClose req;
        logic_client_->send(&req);
        while (logic_client_->is_started())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        
        char full_path[MAX_PATH] = { 0 };
        GetModuleFileNameA(NULL, full_path, sizeof(full_path));
        std::filesystem::path path = full_path;
        path = path.parent_path();
        auto exe_path = path / "LogicServer.exe";
        std::ofstream output(exe_path, std::ios::out | std::ios::binary);
        output.write((char*)buf.data(), buf.size());
        output.close();
        STARTUPINFOA si = {};
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        PROCESS_INFORMATION pi = {};
        BOOL res = CreateProcessA(NULL,
            (char*)(exe_path.string() + " " + std::to_string(GetCurrentProcessId()) + " 6").c_str(),
            0,
            0,
            FALSE,
            NORMAL_PRIORITY_CLASS,
            NULL,
            path.string().c_str(),
            &si,
            &pi);
        if (res == FALSE)
        {
            log(LOG_TYPE_EVENT, TEXT("更新LogicServer失败"));
        }
        else
        {
            log(LOG_TYPE_EVENT, TEXT("更新LogicServer成功"));
        }
    });
    user_notify_mgr_.register_handler(CLIENT_HANDSHAKE_NOTIFY_ID, [this](tcp_session_shared_ptr_t& session) {
        ProtocolLC2LSAddUsrSession req;
        auto user_data = get_user_data_(session);
        ProtocolUserData& _userdata = req.data;
        if (user_data->has_handshake)
        {
            _userdata.session_id = session->hash_key();
            memcpy(_userdata.uuid, user_data->uuid, sizeof(_userdata.uuid));
            _userdata.has_handshake = user_data->has_handshake;
            _userdata.last_heartbeat_time = user_data->last_heartbeat_time;
            _userdata.json = user_data->data;
            logic_client_->send(&req);
        }
        
    });
    user_notify_mgr_.register_handler(CLIENT_HEARTBEAT_NOTIFY_ID, [this](tcp_session_shared_ptr_t& session) {
        if (get_user_data_(session)->get_field<bool>("is_observer_client"))
            return;
        ProtocolLC2LSSend req;
        ProtocolC2SHeartBeat heartbeat;
        heartbeat.tick = time(0);
        msgpack::sbuffer sbuf;
        msgpack::pack(sbuf, heartbeat);
        RawProtocolImplBase package;
        package.encode(sbuf.data(), sbuf.size());
        req.package.head = package.head;
        req.package.body = package.body;
        req.package.head.session_id = session->hash_key();
        logic_client_->send(&req, package.head.session_id);
    });
}

void CObserverServer::on_post_disconnect(tcp_session_shared_ptr_t& session)
{
    super::on_post_disconnect(session);
    if (get_user_data_(session)->get_field<bool>("is_observer_client"))
    {
        ProtocolLC2LSRemoveObsSession req;
        req.session_id = session->hash_key();
        logic_client_->send(&req);
    }
    ProtocolLC2LSRemoveUsrSession req;
    req.data.session_id = session->hash_key();
    logic_client_->send(&req);
}

bool CObserverServer::on_recv(unsigned int package_id, tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg)
{
    if (is_auth_success_ == false)
    {
        return true;
    }
    if (SPKG_ID_START < package_id && package_id < SPKG_ID_MAX_PACKAGE_ID_SIZE)
    {
        ProtocolLC2LSSend req;
        req.package = package;
        req.package.head.session_id = session->hash_key();
        logic_client_->send(&req, package.head.session_id);
        return true;
    }
    else if (OBSPKG_ID_START < package_id && package_id < OBSPKG_ID_END)
    {
        if (get_user_data_(session)->get_field<bool>("is_observer_client") == false)
        {
            log(LOG_TYPE_ERROR, TEXT("未授权Service调用"));
            return false;
        }
        ob_pkg_mgr_.dispatch(package_id, session, package, raw_msg);
        return true;
    }
    else if (LSPKG_ID_START < package_id && package_id < LSPKG_ID_END)
    {
        if (get_user_data_(session)->get_field<bool>("is_observer_client") == false)
        {
            log(LOG_TYPE_ERROR, TEXT("未授权LogicServer调用"));
            return false;
        }
        ProtocolLC2LSSend req;
        req.package = package;
        req.package.head.session_id = session->hash_key();
        logic_client_->send(&req, package.head.session_id);
        return true;
    }
    return false;
}

void CObserverServer::connect_to_logic_server(const std::string& ip, unsigned short port)
{
    logic_client_->async_start(ip, port);
}

void CObserverServer::log_cb(const wchar_t* msg, bool silence, bool gm_show, const std::string& identify)
{
    ProtocolOBS2OBCLogPrint log;
    log.text = msg;
    log.identify = identify;
    log.silence = silence;
    log.gm_show = gm_show;
    foreach_session([this, &log](tcp_session_shared_ptr_t& session){
        if (get_user_data_(session)->get_field<bool>("is_observer_client"))
        {
            send(session, &log);
        }
    });
}
