#include "pch.h"

#include "AntiCheatServer.h"
#include "NetUtils.h"

#define ENABLE_PROXY_TUNNEL 0
#if ENABLE_PROXY_TUNNEL
#include "ProxyTunnel.h"
#endif

#if ENABLE_PROXY_TUNNEL
//todo �����ͷ�
class GameProxyTunnel : public ProxyTunnel
{
public:
	using super = ProxyTunnel;
    using super::super;
    using recv_callback_t = std::function<void(const std::vector<char>& buf, const std::error_code& ec, std::size_t length)>;
	const std::string kProxyServerAddr = "192.168.59.133";
	const int kProxyServerPort = 7000;
    inline void set_recv_callback(recv_callback_t cb) { recv_callback_ = cb; }
	void start()
	{
		super::start(kProxyServerAddr, kProxyServerPort);
	}
    void on_connect(const std::error_code& ec)
    {
        log(LOG_TYPE_DEBUG, TEXT("��Ϸͨ���������� %s:%d"),
            Utils::c2w(get_address()).c_str(),
            get_port());
    }
    void on_disconnect(const std::error_code& ec)
    {
        log(LOG_TYPE_DEBUG, TEXT("��Ϸͨ��ʧȥ���� %s:%d"),
            Utils::c2w(get_address()).c_str(),
            get_port());
    }
    void on_recv(const std::vector<char>& buf, const std::error_code& ec, std::size_t length)
    {
        if (recv_callback_)
            recv_callback_(buf, ec, length);
    }
    void on_send(const std::error_code& ec, std::size_t length)
    {
        log(LOG_TYPE_DEBUG, TEXT("��Ϸͨ��������Ϣ %s:%d ����:%d %s"),
            Utils::c2w(get_address()).c_str(),
            get_port(),
            length,
			Utils::c2w(ec.message()).c_str()
			);
    }
private:
    recv_callback_t recv_callback_;
};
#endif


#if ENABLE_GAME_MATCH_ROLE
//��Ϸ����߼�����ʱ����
class match_role
{
public:
    explicit match_role(char start_tag, char end_tag) : 
        start_tag_(start_tag),
        end_tag_(end_tag)
    {}

    template <typename Iterator>
    std::pair<Iterator, bool> operator()(Iterator begin, Iterator end) const
    {
        Iterator i = begin;
        while (i != end)
        {
            if (*i != start_tag_)
                return std::pair(begin, true); // head character is not #, return and kill the client
            i++;
            if (i == end) break;
			while (i != end)
			{
				if(*i == end_tag_)
					return std::pair(i, true);
				i++;
			}
            break;
        }
        return std::pair(begin, false);
    }

private:
    char start_tag_;
    char end_tag_;
};
#else
#include "Protocol.h"
#endif



CAntiCheatServer::CAntiCheatServer()
	: super()
{
	//��֤��أ�serverless �ƺ���+���ݿ�
	auth_check_timer_ = std::chrono::minutes(24 * 60);
    auth_url_ = "";//"http://service-4v2g0j35-1305025354.sh.apigw.tencentcs.com/release/check";
	auth_ticket_ = "1234";

	uuid_check_duration_ = std::chrono::seconds(5);
	heartbeat_check_duration_ = std::chrono::seconds(60);
	heartbeat_timeout_ = std::chrono::seconds(120);
	
    is_observer_server_ = false;
    is_logic_server_ = false;
    is_auth_success_ = false;
    set_log_level(LOG_TYPE_DEBUG | LOG_TYPE_EVENT | LOG_TYPE_ERROR);
}

CAntiCheatServer::~CAntiCheatServer()
{
    package_mgr_.clear_handler();
}

bool CAntiCheatServer::start(const std::string& listen_addr, int port)
{
#if ENABLE_PROXY_TUNNEL
	enable_proxy_tunnel(true);
#else
	enable_proxy_tunnel(false);
#endif
	if (super::super::start(listen_addr, port, RawProtocolImpl()))
	{
		return true;
	}
	return false;
}

bool CAntiCheatServer::check_timer(bool slience)
{
#if 0
    std::string body;
    std::string msg;
    try {
        std::error_code ec;
        asio2::http_client http_client;
        auto res = http_client.execute(auth_url_ + "?action=login&ticket=" + auth_ticket_, ec);
        body = res.body();
        auto json_data = json::parse(body);
        unsigned int status = json_data["status"];
        msg = json_data["msg"];
        msg = Utils::from_utf8(msg);
        if (status)
        {
            is_auth_success_ = true;
            if (!slience) log(Event, TEXT("%s"), Utils::c2w(msg).c_str());
            return true;
        }
        else
        {
            is_auth_success_ = false;
            log(Event, TEXT("%s"), Utils::c2w(msg).c_str());
            return false;
        }
    }
    catch (...)
    {
        log(Event, TEXT("��֤���������� %s"), Utils::c2w(Utils::from_utf8(body)).c_str());
    }
#endif
	return false;
}

void CAntiCheatServer::on_accept(tcp_session_shared_ptr_t& session)
{
    log(LOG_TYPE_DEBUG, TEXT("���� %s:%d"),
        Utils::c2w(session->remote_address()).c_str(),
        session->remote_port());
    get_user_data(session)->set_field("is_local_client", session->remote_address() == "127.0.0.1");
}

void CAntiCheatServer::on_init()
{
    log(LOG_TYPE_EVENT, TEXT("�׽��ֳ�ʼ���ɹ�"));
}

void CAntiCheatServer::on_post_disconnect(tcp_session_shared_ptr_t& session)
{
    log(LOG_TYPE_DEBUG, TEXT("�Ͽ����� %s:%d"),
        Utils::c2w(session->remote_address()).c_str(),
        session->remote_port());
	if (is_enable_proxy_tunnel())
	{
#if ENABLE_PROXY_TUNNEL
		get_user_data(session).game_proxy_tunnel->stop();
#endif
	}
}

void CAntiCheatServer::on_start(error_code_t ec)
{
	if (!ec)
	{
		log(LOG_TYPE_EVENT, TEXT("��ʼ����:%s:%d %s"),
			Utils::c2w(listen_address()).c_str(),
			listen_port(),
			Utils::c2w(ec.message()).c_str());
        notify_mgr_.dispatch(SERVER_START_NOTIFY_ID);
	}
	else
	{
		log(LOG_TYPE_EVENT, TEXT("����ʧ��:%s:%d %s"),
			Utils::c2w(listen_address()).c_str(),
			listen_port(),
			Utils::c2w(ec.message()).c_str());
	}
}

void CAntiCheatServer::on_stop(error_code_t ec)
{
    log(LOG_TYPE_EVENT, TEXT("ֹͣ����:%s:%d %s"),
        Utils::c2w(listen_address()).c_str(),
        listen_port(),
        Utils::c2w(ec.message()).c_str());
}


void CAntiCheatServer::start_timer(unsigned int timer_id, std::chrono::system_clock::duration duration, std::function<void()> handler)
{
    super::start_timer(timer_id, duration, [this, handler = std::move(handler)](){
        handler();
    });
}

void CAntiCheatServer::stop_timer(unsigned int timer_id)
{
    super::stop_timer(timer_id);
}

void CAntiCheatServer::log(int type, LPCTSTR format, ...)
{
    static std::mutex mtx;
    std::lock_guard<std::mutex> lck(mtx);
    if ((log_level_ & type) == 0)
        return;
    std::time_t now_time = time(0);
    TCHAR date_str[MAX_PATH] = { 0 };
    TCHAR time_str[MAX_PATH] = { 0 };
    tm tm_;
    localtime_s(&tm_, &now_time);
    wcsftime(time_str, sizeof(time_str) / sizeof(time_str[0]) - 1, TEXT("%H:%M:%S"), &tm_);
    std::wcout << time_str << ":";

    TCHAR buffer[1024];
    va_list ap;
    va_start(ap, format);
    _vsnwprintf_s(buffer, sizeof(buffer) / sizeof(buffer[0]) - 1, format, ap);
    va_end(ap);

    switch (type)
    {
    case LOG_TYPE_DEBUG:
        std::wcout << L"[Debug]";
        break;
    case LOG_TYPE_EVENT:
        std::wcout << L"[Event]";
        break;
    case LOG_TYPE_ERROR:
        std::wcout << L"[Error]";
        break;
    default:
        break;
    }
    wprintf(TEXT("%s\n"), buffer);

    if (log_cb_)
        log_cb_(buffer, false, true, "");
}

void CAntiCheatServer::user_log(int type, bool silense, bool gm_show, const std::string& identify, LPCTSTR format, ...)
{
    static std::mutex mtx;
    std::lock_guard<std::mutex> lck(mtx);
    if ((log_level_ & type) == 0)
        return;
    std::time_t now_time = time(0);
    TCHAR date_str[MAX_PATH] = { 0 };
    TCHAR time_str[MAX_PATH] = { 0 };
    tm tm_;
    localtime_s(&tm_, &now_time);
    wcsftime(time_str, sizeof(time_str) / sizeof(time_str[0]) - 1, TEXT("%H:%M:%S"), &tm_);
    std::wcout << time_str << ":";

    TCHAR buffer[1024];
    va_list ap;
    va_start(ap, format);
    _vsnwprintf_s(buffer, sizeof(buffer) / sizeof(buffer[0]) - 1, format, ap);
    va_end(ap);

    switch (type)
    {
    case LOG_TYPE_DEBUG:
        std::wcout << L"[Debug]";
        break;
    case LOG_TYPE_EVENT:
        std::wcout << L"[Event]";
        break;
    case LOG_TYPE_ERROR:
        std::wcout << L"[Error]";
        break;
    default:
        break;
    }
    wprintf(TEXT("%s\n"), buffer);

    if (log_cb_)
        log_cb_(buffer, silense, gm_show, identify);
}

void CAntiCheatServer::close(unsigned int session_id)
{
	sessions().find(session_id)->stop();
}


void CAntiCheatServer::on_recv(tcp_session_shared_ptr_t& session, std::string_view sv)
{
#ifdef _DEBUG
    _on_recv(session, sv);
#else
    try
    {
        _on_recv(session, sv);
    }
    catch (...)
    {
        log(LOG_TYPE_DEBUG, TEXT("�հ������쳣:%s:%d ����:%d"),
            Utils::c2w(session->remote_address()).c_str(),
            session->remote_port(),
            sv.size());
    }
#endif
}


void CAntiCheatServer::_on_recv(tcp_session_shared_ptr_t& session, std::string_view sv)
{
    if (sv.size() == 0)
    {
        log(LOG_TYPE_ERROR, TEXT("Э��ײ����:%s:%d ����:%d"),
            Utils::c2w(session->remote_address()).c_str(),
            session->remote_port(),
            sv.size());
        session->stop();
        return;
    }
    if (is_enable_proxy_tunnel())
    {
#if ENABLE_PROXY_TUNNEL
        get_user_data(session).game_proxy_tunnel->send(sv);
#endif
    }
    RawProtocolImpl package;
#ifdef _DEBUG
    if (is_logic_server())
    {
        if (!package.decode(sv))
        {
            log(LOG_TYPE_DEBUG, TEXT("���У��ʧ��:%s:%d ����:%d"),
                Utils::c2w(session->remote_address()).c_str(),
                session->remote_port(),
                sv.size());
            return;
        }
    }
    else
    {
        if (!package.decode(sv))
        {
            log(LOG_TYPE_DEBUG, TEXT("���У��ʧ��:%s:%d ����:%d"),
                Utils::c2w(session->remote_address()).c_str(),
                session->remote_port(),
                sv.size());
            return;
        }
    }
#else
    if (!package.decode(sv))
    {
        log(LOG_TYPE_DEBUG, TEXT("���У��ʧ��:%s:%d ����:%d"),
            Utils::c2w(session->remote_address()).c_str(),
            session->remote_port(),
            sv.size());
        return;
    }
#endif
    

    auto raw_msg = msgpack::unpack((char*)package.body.buffer.data(), package.body.buffer.size());
    if (raw_msg.get().type != msgpack::type::ARRAY) throw msgpack::type_error();
    if (raw_msg.get().via.array.size < 1) throw msgpack::type_error();
    if (raw_msg.get().via.array.ptr[0].type != msgpack::type::POSITIVE_INTEGER) throw msgpack::type_error();
    const auto package_id = raw_msg.get().via.array.ptr[0].as<unsigned int>();
    auto user_data = get_user_data(session);
    if (package.head.step != user_data->step + 1)
    {
        user_data->set_field("miss_count", user_data->get_field<int>("miss_count") + 1);
        log(LOG_TYPE_DEBUG, TEXT("[%s:%d] ���ֶ������طŹ���"),
            Utils::c2w(session->remote_address()).c_str(),
            session->remote_port());
    }
    user_data->step = package.head.step;

    if (user_data->has_handshake == false && package_id == PackageId::PKG_ID_C2S_HANDSHAKE)
    {
        auto msg = raw_msg.get().as<ProtocolC2SHandShake>();
        on_recv_handshake(session, package, msg);
        return;
    }

    if (user_data->has_handshake == false)
    {
        log(LOG_TYPE_ERROR, TEXT("[%s:%d] δ�����û�"),
            Utils::c2w(session->remote_address()).c_str(),
            session->remote_port());
        return;
    }

    if (package_id == PackageId::PKG_ID_C2S_HEARTBEAT)
    {
        auto msg = raw_msg.get().as<ProtocolC2SHeartBeat>();
        on_recv_heartbeat(session, package, msg);
        return;
    }
#if 0
    log(Debug, TEXT("�յ����ݰ�:%s:%d ����:%d"),
        Utils::c2w(session->remote_address()).c_str(),
        session->remote_port(),
        sv.size());
#endif
    if (package_mgr_.dispatch(package_id, session, package, raw_msg))
    {
        return;
    }
    if (!on_recv(package_id, session, package, raw_msg))
    {
        log(LOG_TYPE_ERROR, TEXT("[%s:%d] δ֪��id %d"),
            Utils::c2w(session->remote_address()).c_str(),
            session->remote_port(),
            package_id);
    }
}

void CAntiCheatServer::on_post_connect(tcp_session_shared_ptr_t& session)
{
	log(LOG_TYPE_DEBUG, TEXT("�������� %s:%d"),
		Utils::c2w(session->remote_address()).c_str(),
		session->remote_port());
#if ENABLE_PROXY_TUNNEL
	if (is_enable_proxy_tunnel())
	{
		auto userdata = get_user_data(session);
		userdata.game_proxy_tunnel->start();
		userdata.game_proxy_tunnel->set_recv_callback([this, session_id = session->hash_key()](const std::vector<char>& buf, const std::error_code& ec, std::size_t length){
			log(LOG_TYPE_DEBUG, TEXT("��Ϸͨ���յ���Ϣ ����:%d"),
				length);
			this->send(session_id, buf.data(), length);
		});
	}
#endif
	//���ֳ�ʱ���
	session->start_timer((unsigned int)UUID_CHECK_TIMER_ID, uuid_check_duration_, [this, session]() {
		auto userdata = get_user_data(session);
		if (userdata->has_handshake == false)
		{
			session->stop();
			return;
		}
	});
}

void CAntiCheatServer::on_recv_heartbeat(tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const ProtocolC2SHeartBeat& msg)
{
	auto userdata = get_user_data(session);
	userdata->update_heartbeat_time();
    ProtocolS2CHeartBeat resp;
    resp.tick = msg.tick;
    send(session, &resp);
    user_notify_mgr_.dispatch(CLIENT_HEARTBEAT_NOTIFY_ID, session);
}

void CAntiCheatServer::on_recv_handshake(tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const ProtocolC2SHandShake& msg)
{
    log(LOG_TYPE_DEBUG, TEXT("���� %s:%d"),
        Utils::c2w(session->remote_address()).c_str(),
        session->remote_port());
	auto userdata = get_user_data(session);
	memcpy(userdata->uuid, msg.uuid, sizeof(msg.uuid));
	userdata->has_handshake = true;
	//���ֳ�ʱ���ȡ��
    session->stop_timer((unsigned int)UUID_CHECK_TIMER_ID);
	//������ʱ���
	session->start_timer((unsigned int)HEARTBEAT_CHECK_TIMER_ID, heartbeat_check_duration_, [this, session]() {
		auto userdata = get_user_data(session);
		auto duration = userdata->get_heartbeat_duration();
		if (duration > heartbeat_timeout_)
		{
			log(LOG_TYPE_ERROR, TEXT("%s:%d ������ʱ%d��"), Utils::c2w(session->remote_address()).c_str(),
				session->remote_port(), std::chrono::duration_cast<std::chrono::seconds>(duration).count());
			session->stop();
		}
	});

    userdata->set_field("sysver", msg.system_version);
    userdata->set_field("64bits", msg.is_64bit_system);
    userdata->set_field("cpuid", msg.cpuid);
    userdata->set_field("mac", msg.mac);
    userdata->set_field("vol", msg.volume_serial_number);
    userdata->set_field("rev_ver", msg.rev_version);
    userdata->set_field("commit_ver", msg.commited_hash);
    userdata->set_field("ip", session->remote_address());
    userdata->set_field("logintime", time(0));
    userdata->set_field("pid", msg.pid);

    user_notify_mgr_.dispatch(CLIENT_HANDSHAKE_NOTIFY_ID, session);
    ProtocolS2CHandShake resp;
    memcpy(resp.uuid, msg.uuid, sizeof(resp.uuid));
	send(session, &resp);
}
