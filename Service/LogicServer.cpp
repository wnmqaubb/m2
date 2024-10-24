#include "pch.h"
#include "LogicServer.h"
#include "../version.build"
#include "vmprotect/VMProtectSDK.h"
std::filesystem::path g_cur_dir;

#if 1
#define ENABLE_POLICY_TIMEOUT_CHECK
#define ENABLE_POLICY_TIMEOUT_CHECK_TIMES 3
#define ENABLE_DETAIL_USER_LOGIN_LOGOUT_LOG
#endif

int main(int argc, char** argv)
{
	VMProtectBeginVirtualization(__FUNCTION__);
	CreateMutex(NULL, FALSE, TEXT("mtx_logic_server"));
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		return 0;
	}
	g_cur_dir = std::filesystem::path(argv[0]).parent_path();
	setlocale(LC_CTYPE, "");
	CLogicServer server;
	if (!server.start(kDefaultLocalhost, kDefaultLogicServicePort))
	{
		if (asio2::get_last_error())
			printf("CLogicServer启动失败:错误号: %d, 错误信息: %s\n", asio2::get_last_error().value(), asio2::get_last_error_msg().c_str());
		return 1;
	}
	if (argc == 2 || argc == 3)
	{
		if (argc == 3)
		{
			char log_level = std::stoi(argv[2]);
			server.set_log_level(log_level);
		}
		unsigned int ppid = std::stoi(argv[1]);
		HANDLE phandle = OpenProcess(PROCESS_VM_OPERATION | SYNCHRONIZE, FALSE, ppid);
		WaitForSingleObject(phandle, INFINITE);
		server.stop();
	}
#ifndef _DEBUG
	server.set_log_level(LOG_TYPE_EVENT | LOG_TYPE_ERROR);
#endif
	while (!server.is_stopped())
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	return 0;
	VMProtectEnd();
}
#define REGISTER_TRANSPORT(PKG_ID) \
ob_pkg_mgr_.register_handler(PKG_ID, [this](tcp_session_shared_ptr_t& session, unsigned int ob_session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {\
    if(obs_sessions_mgr().exist(ob_session_id) == false) return;\
    ProtocolOBS2OBCSend req;\
    req.package = package;\
    send(session, ob_session_id, &req);\
});

enum LogicTimerId {
	DEFINE_TIMER_ID(PLUGIN_RELOAD_TIMER_ID),
	DEFINE_TIMER_ID(ONLINE_CHECK_TIMER_ID),
};

void CLogicServer::send_policy(std::shared_ptr<ProtocolUserData>& user_data, tcp_session_shared_ptr_t& session, unsigned int session_id)
{
	VMProtectBeginVirtualization(__FUNCTION__);
#if defined(ENABLE_POLICY_TIMEOUT_CHECK)
	if (!user_data->policy_recv_timeout_timer_)
	{
		user_data->policy_recv_timeout_timer_ = std::make_shared<asio::steady_timer>(io().context());
	}
#endif
	send(session, session_id, &policy_mgr_.get_policy());
	user_data->add_send_policy_count();
#if defined(ENABLE_POLICY_TIMEOUT_CHECK)
	do
	{
		user_data->policy_recv_timeout_timer_->expires_from_now(std::chrono::minutes(3));
		user_data->policy_recv_timeout_timer_->async_wait([this, user_data = user_data, service_session_id = session->hash_key()](std::error_code ec) {
			if (ec != asio::error::operation_aborted)
			{
				user_log(LOG_TYPE_EVENT, true, false, user_data->get_uuid().str(), TEXT("策略收到超时:%d"), user_data->session_id);
				user_data->add_policy_timeout_times();
				if (user_data->get_policy_timeout_times() >= ENABLE_POLICY_TIMEOUT_CHECK_TIMES)
				{
					auto service_session = sessions().find(service_session_id);
					if (service_session)
						close_socket(service_session, user_data->session_id);
					user_log(LOG_TYPE_EVENT, true, false, user_data->get_uuid().str(), TEXT("策略收到严重超时，请手动处罚:%d"), user_data->session_id);
					std::wstring usr_name = user_data->json.find("usrname") != user_data->json.end() ? user_data->json.at("usrname").get<std::wstring>() : L"(NULL)";
					//write_txt(".\\恶性开挂人员名单.txt", trim_user_name(Utils::w2c(usr_name)));
				}
			}
			else
			{
				user_data->clear_policy_timeout_times();
			}
		});
	} while (0);
#endif
	VMProtectEnd();
}
CLogicServer::CLogicServer()
{
	VMProtectBeginVirtualization(__FUNCTION__);
	is_logic_server_ = true;
	set_log_cb(std::bind(&CLogicServer::log_cb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
	start_timer(PLUGIN_RELOAD_TIMER_ID, std::chrono::seconds(1), [this]() {
		try {
			//plugin_mgr_.reload_all_plugin();
			policy_mgr_.reload_all_policy();
		}
		catch (...)
		{

		}
		});
	start_timer(ONLINE_CHECK_TIMER_ID, std::chrono::seconds(40), [this]() {
		OnlineCheck();
		});
	package_mgr_.register_handler(LSPKG_ID_C2S_CLOSE, [this](tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
		this->stop();
		exit(0);
		});
	// service ---> logic server连接
	package_mgr_.register_handler(LSPKG_ID_C2S_ADD_OBS_SESSION, [this](tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
		auto req = msg.get().as<ProtocolLC2LSAddObsSession>();
		obs_sessions_mgr().add_session(req.session_id);
		set_field(session, req.session_id, "logic_ver", TEXT(FILE_VERSION_STR));
		});
	package_mgr_.register_handler(LSPKG_ID_C2S_REMOVE_OBS_SESSION, [this](tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
		auto req = msg.get().as<ProtocolLC2LSRemoveObsSession>();
		obs_sessions_mgr().remove_session(req.session_id);
		});
	package_mgr_.register_handler(LSPKG_ID_C2S_ADD_USR_SESSION, [this](tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
		auto req = msg.get().as<ProtocolLC2LSAddUsrSession>();
		usr_sessions_mgr().add_session(req.data.session_id, req.data);
		auto user_data = usr_sessions_mgr().get_user_data(req.data.session_id);
		if (user_data)
		{
			// 延迟发送策略，防止用户连接过慢导致策略发送失败
			session->post([this, user_data, session]() mutable {
				send_policy(user_data, session, user_data->session_id);
				//user_log(LOG_TYPE_EVENT, true, false, user_data->get_uuid().str(), TEXT("延迟发送策略"));
			}, std::chrono::seconds(std::rand() % 20 + 10));

			std::wstring json_dump;
			try
			{
				json_dump = Utils::c2w(user_data->json.dump());
			}
			catch (...)
			{

			}
			user_log(LOG_TYPE_EVENT, true, false, user_data->get_uuid().str(), TEXT("建立连接并下发策略:%s"), json_dump.c_str());
		}
		detect(session, req.data.session_id);
		});
	package_mgr_.register_handler(LSPKG_ID_C2S_REMOVE_USR_SESSION, [this](tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
		auto req = msg.get().as<ProtocolLC2LSRemoveUsrSession>();
		auto user_data = usr_sessions_mgr().get_user_data(req.data.session_id);
		if (user_data) {
#if defined(ENABLE_DETAIL_USER_LOGIN_LOGOUT_LOG)
			user_log(LOG_TYPE_EVENT, true, false, user_data->get_uuid().str(), TEXT("断开连接:%d"), user_data->session_id);
#endif
#if defined(ENABLE_POLICY_TIMEOUT_CHECK)
			if (user_data->policy_recv_timeout_timer_) user_data->policy_recv_timeout_timer_->cancel();
#endif
		}
		usr_sessions_mgr().remove_session(req.data.session_id);
		});
	// service ---> logic server  转发协议
	package_mgr_.register_handler(LSPKG_ID_C2S_SEND, [this](tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
		auto req = msg.get().as<ProtocolLC2LSSend>().package;
		auto raw_msg = msgpack::unpack((char*)req.body.buffer.data(), req.body.buffer.size());
		if (raw_msg.get().type != msgpack::type::ARRAY) throw msgpack::type_error();
		if (raw_msg.get().via.array.size < 1) throw msgpack::type_error();
		if (raw_msg.get().via.array.ptr[0].type != msgpack::type::POSITIVE_INTEGER) throw msgpack::type_error();
		const auto package_id = raw_msg.get().via.array.ptr[0].as<unsigned int>();
		if (package_id == LSPKG_ID_C2S_SEND)
		{
			log(LOG_TYPE_ERROR, TEXT("嵌套转发"));
			return;
		}

		auto user_data = usr_sessions_mgr().get_user_data(package.head.session_id);
		if (user_data)
		{
			user_data->pkg_id_time_map()[package_id] = std::chrono::system_clock::now();
		}

		if (!policy_pkg_mgr_.dispatch(package_id | package.head.session_id, session, package.head.session_id, req, raw_msg))
			ob_pkg_mgr_.dispatch(package_id, session, package.head.session_id, req, raw_msg);
		});

	//REGISTER_TRANSPORT(SPKG_ID_C2S_CHECK_PLUGIN);
	REGISTER_TRANSPORT(SPKG_ID_C2S_QUERY_PROCESS);
	REGISTER_TRANSPORT(SPKG_ID_C2S_QUERY_DRIVERINFO);
	REGISTER_TRANSPORT(SPKG_ID_C2S_QUERY_WINDOWSINFO);
	REGISTER_TRANSPORT(SPKG_ID_C2S_QUERY_SCREENSHOT);
	ob_pkg_mgr_.register_handler(PKG_ID_C2S_HEARTBEAT, [this](tcp_session_shared_ptr_t& session, unsigned int ob_session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
		auto user_data = usr_sessions_mgr().get_user_data(package.head.session_id);
		if (user_data)
		{
			// 每3分钟检测一次各个包是否超时
			if (user_data->get_heartbeat_duration() > std::chrono::minutes(3))
			{
				detect(session, package.head.session_id);
				send_policy(user_data, session, package.head.session_id);

				if (user_data->has_been_check_pkg())
				{
					if (user_data->pkg_id_time_map().count(689060) == 0)
					{
						user_log(LOG_TYPE_EVENT, true, false, user_data->get_uuid().str(), TEXT("水仙超时:%d"), user_data->session_id);
						//write_txt(".\\恶性开挂人员名单.txt", trim_user_name(Utils::w2c(usr_name)));
					}
					else
					{
						auto dura_689060 = std::chrono::system_clock::now() - user_data->pkg_id_time_map()[689060];
						if (dura_689060 > std::chrono::minutes(5))
						{
							user_log(LOG_TYPE_EVENT, true, false, user_data->get_uuid().str(), TEXT("689060超时:%d s"), std::chrono::duration_cast<std::chrono::seconds>(dura_689060).count());
						}
					}
					if (user_data->pkg_id_time_map().count(689051) == 0)
					{
						user_log(LOG_TYPE_EVENT, true, false, user_data->get_uuid().str(), TEXT("机器码收到超时:%d"), user_data->session_id);
						close_socket(session, user_data->session_id);
						std::wstring usr_name = user_data->json.find("usrname") != user_data->json.end() ? user_data->json.at("usrname").get<std::wstring>() : L"(NULL)";
						//write_txt(".\\恶性开挂人员名单.txt", trim_user_name(Utils::w2c(usr_name)));
					}
					else
					{
						auto dura_689051 = std::chrono::system_clock::now() - user_data->pkg_id_time_map()[689051];
						if (dura_689051 > std::chrono::minutes(5))
						{
							user_log(LOG_TYPE_EVENT, true, false, user_data->get_uuid().str(), TEXT("689051超时:%d s"), std::chrono::duration_cast<std::chrono::seconds>(dura_689051).count());
							std::wstring usr_name = user_data->json.find("usrname") != user_data->json.end() ? user_data->json.at("usrname").get<std::wstring>() : L"(NULL)";
						}
					}

					if (user_data->pkg_id_time_map().count(SPKG_ID_C2S_QUERY_PROCESS) == 0)
					{
						user_log(LOG_TYPE_EVENT, true, false, user_data->get_uuid().str(), TEXT("查看进程收到超时:%d"), user_data->session_id);
						close_socket(session, user_data->session_id);
						std::wstring usr_name = user_data->json.find("usrname") != user_data->json.end() ? user_data->json.at("usrname").get<std::wstring>() : L"(NULL)";
						//write_txt(".\\恶性开挂人员名单.txt", trim_user_name(Utils::w2c(usr_name)));
					}
				}
				else
				{
					ProtocolS2CQueryProcess req;
					send(session, package.head.session_id, &req);
					user_data->has_been_check_pkg(true);
					//user_log(LOG_TYPE_EVENT, true, false, user_data->get_uuid().str(), TEXT("下发查看进程:%d"), user_data->session_id);
				}

				user_data->last_heartbeat_time = std::chrono::system_clock::now();
			}
		}

		});

	ob_pkg_mgr_.register_handler(SPKG_ID_C2S_UPDATE_USER_NAME, [this](tcp_session_shared_ptr_t& session, unsigned int ob_session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
		auto& req = msg.get().as<ProtocolC2SUpdateUsername>();
		auto user_data = usr_sessions_mgr().get_user_data(package.head.session_id);
		if (user_data)
		{
			// 发给service
			set_field(session, package.head.session_id, "usrname", req.username);
			user_data->json["usrname"] = req.username;
			user_log(LOG_TYPE_EVENT, true, false, user_data->get_uuid().str(), TEXT("用户登录:%s sid:[%d]"), req.username.c_str(), user_data->session_id);
		}
		});
	ob_pkg_mgr_.register_handler(SPKG_ID_C2S_TASKECHO, [this](tcp_session_shared_ptr_t& session, unsigned int ob_session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
		auto& req = msg.get().as<ProtocolC2STaskEcho>();
		auto policy = policy_mgr_.find_policy(req.task_id);
		std::wstring reason = Utils::c2w(req.text);
		auto user_data = usr_sessions_mgr().get_user_data(package.head.session_id);
		if (user_data)
		{			
			user_data->pkg_id_time_map()[req.task_id] = std::chrono::system_clock::now();
		}

		if (req.is_cheat && policy)
		{
			punish(session, package.head.session_id, *policy, reason.c_str());
			return;
		}


		if (user_data)
		{
			std::wstring usr_name = user_data->json.find("usrname") != user_data->json.end() ? user_data->json.at("usrname").get<std::wstring>() : L"(NULL)";
			std::wstring reason = Utils::c2w(req.text);
			bool gm_show = 688000 < req.task_id && req.task_id < 689000;
			bool silence = req.task_id != 0;

			if (req.task_id == 689999 && req.is_cheat)
			{
				write_txt(".\\恶性开挂人员名单.txt", trim_user_name(Utils::w2c(usr_name)));
				user_log(LOG_TYPE_EVENT, silence, false, user_data->get_uuid().str(), TEXT("玩家:%s 策略ID:%d 日志:多次脚本错误定性为外挂,已写入恶性名单!"), usr_name.c_str(), req.task_id, reason.c_str());
			}

			//user_log(LOG_TYPE_EVENT, silence, gm_show, user_data->get_uuid().str(), TEXT("玩家:%s 策略ID:%d 日志:%s"), usr_name.c_str(), req.task_id, reason.c_str());
#if defined(ENABLE_POLICY_TIMEOUT_CHECK)
			if (user_data->policy_recv_timeout_timer_) user_data->policy_recv_timeout_timer_->cancel(); 
#endif
		}
		});
	ob_pkg_mgr_.register_handler(SPKG_ID_C2S_POLICY, [this](tcp_session_shared_ptr_t& session, unsigned int ob_session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
		auto& req = msg.get().as<ProtocolC2SPolicy>();
		auto user_data = usr_sessions_mgr().get_user_data(package.head.session_id);
#if defined(ENABLE_POLICY_TIMEOUT_CHECK)
		if (user_data)
		{
			if (user_data->policy_recv_timeout_timer_)
			{
				user_data->policy_recv_timeout_timer_->cancel();
			}
		}
#endif
		ProtocolC2SPolicy resp;
		if (req.results.size())
		{
			for (auto row : req.results)
			{
				auto policy = policy_mgr_.find_policy(row.policy_id);
				if (policy)
				{
					punish(session, package.head.session_id, *policy, row.information);
					return;
				}
				else
				{
					resp.results.push_back(row);
				}
			}
		}
		if (resp.results.size())
		{
			ProtocolOBS2OBCSend board_cast;
			msgpack::sbuffer sbuf;
			msgpack::pack(sbuf, resp);
			board_cast.package.encode(sbuf.data(), sbuf.size());

			foreach_session([this, &board_cast](tcp_session_shared_ptr_t& session) {
				for (auto session_id : obs_sessions_mgr().sessions())
				{
					send(session, session_id, &board_cast);
				}
				});
		}
		});
	REGISTER_TRANSPORT(SPKG_ID_C2S_RMC_CREATE_CMD);
	REGISTER_TRANSPORT(SPKG_ID_C2S_RMC_DOWNLOAD_FILE);
	REGISTER_TRANSPORT(SPKG_ID_C2S_RMC_UPLOAD_FILE);
	REGISTER_TRANSPORT(SPKG_ID_C2S_RMC_ECHO);
	// 更新管理员策略
	ob_pkg_mgr_.register_handler(LSPKG_ID_C2S_UPLOAD_CFG, [this](tcp_session_shared_ptr_t& session, unsigned int ob_session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
		auto& req = msg.get().as<ProtocolOBC2LSUploadConfig>();
		policy_mgr_.create_policy_file(req.file_name, req.data);
		log(LOG_TYPE_EVENT, TEXT("更新策略:%s"), Utils::c2w(req.file_name).c_str());
		});
	ob_pkg_mgr_.register_handler(LSPKG_ID_C2S_REMOVE_CFG, [this](tcp_session_shared_ptr_t& session, unsigned int ob_session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
		auto& req = msg.get().as<ProtocolOBC2LSRemoveConfig>();
		policy_mgr_.remove_policy_file(req.file_name);
		log(LOG_TYPE_EVENT, TEXT("卸载策略:%s"), Utils::c2w(req.file_name).c_str());
		});
	//ob_pkg_mgr_.register_handler(LSPKG_ID_C2S_UPLOAD_PLUGIN, [this](tcp_session_shared_ptr_t& session, unsigned int ob_session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
	//    auto& req = msg.get().as<ProtocolOBC2LSUploadPlugin>();
	//    plugin_mgr_.create_plugin_file(req.file_name, req.data);
	//    log(LOG_TYPE_EVENT, TEXT("更新插件:%s"), Utils::c2w(req.file_name).c_str());
	//});
	//ob_pkg_mgr_.register_handler(LSPKG_ID_C2S_REMOVE_PLUGIN, [this](tcp_session_shared_ptr_t& session, unsigned int ob_session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
	//    auto& req = msg.get().as<ProtocolOBC2LSRemovePlugin>();
	//    plugin_mgr_.remove_plugin_file(req.file_name);
	//    log(LOG_TYPE_EVENT, TEXT("卸载插件:%s"), Utils::c2w(req.file_name).c_str());
	//});
	ob_pkg_mgr_.register_handler(LSPKG_ID_C2S_ADD_LIST, [this](tcp_session_shared_ptr_t& session, unsigned int ob_session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
		auto& req = msg.get().as<ProtocolOBC2LSAddList>();
		write_txt(req.file_name, req.text, true);
		});
	ob_pkg_mgr_.register_handler(LSPKG_ID_C2S_CLEAR_LIST, [this](tcp_session_shared_ptr_t& session, unsigned int ob_session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
		auto& req = msg.get().as<ProtocolOBC2LSClearList>();
		clear_txt(req.file_name);
		});
	VMProtectEnd();
}

void CLogicServer::clear_txt(const std::string& file_name)
{
	std::ofstream list(file_name, std::ios::out);
	if (list.is_open())
	{
		list << "";
		list.close();
		log(LOG_TYPE_EVENT, TEXT("清空列表[%s]"), Utils::c2w(file_name).c_str());
	}
}
class counter
{
public:
	counter(std::chrono::system_clock::duration dura, int limit)
		:dura_(dura), count_(0), limit_(limit), last_tick_(std::chrono::system_clock::now())
	{
	}
	std::chrono::system_clock::duration get_duration()
	{
		return std::chrono::system_clock::now() - last_tick_;
	}
	void update_last_tick()
	{
		last_tick_ = std::chrono::system_clock::now();
	}
	bool count(std::function<void()> cb)
	{
		bool r = false;
		count_++;
		if (count_ > limit_)
		{
			if (get_duration() > dura_)
			{
				cb();
				r = true;
			}
			count_ = 0;
			update_last_tick();
		}
		return r;
	}
	std::chrono::system_clock::time_point last_tick_;
	const std::chrono::system_clock::duration dura_;
	int count_;
	const int limit_;

};
void CLogicServer::write_txt(const std::string& file_name, const std::string& str, bool is_from_add_list)
{
	static counter limit_counter(std::chrono::seconds(30), 5);
	static bool is_enable_write_txt = true;
	if (!is_from_add_list)
	{
		if (limit_counter.count([this, file_name = file_name]() {
			if (is_enable_write_txt == true)
			{
				clear_txt(file_name);
			}
			is_enable_write_txt = false;
			}))
		{
			log(LOG_TYPE_EVENT, TEXT("触发写入列表频率上限，暂时关闭写列表功能"));
			return;
		}
		if (!is_enable_write_txt)
			return;
	}

	std::ofstream list(file_name, std::ios::out | std::ios::app);
	if (list.is_open())
	{
		list << str << std::endl;
		list.close();
		log(LOG_TYPE_EVENT, TEXT("加入列表[%s]:%s"), Utils::c2w(file_name).c_str(), Utils::c2w(str).c_str());
	}
}

void CLogicServer::write_img(unsigned int session_id, std::vector<uint8_t>& data)
{
	namespace fs = std::filesystem;
	std::time_t now_time = time(0);
	TCHAR date_str[MAX_PATH] = { 0 };
	TCHAR time_str[MAX_PATH] = { 0 };
	tm tm_;
	localtime_s(&tm_, &now_time);
	wcsftime(time_str, sizeof(time_str) / sizeof(time_str[0]) - 1, TEXT("%I_%M_%S"), &tm_);
	wcsftime(date_str, sizeof(date_str) / sizeof(date_str[0]) - 1, TEXT("%y_%m_%d"), &tm_);

	fs::path save_dir = g_cur_dir / L"log" / date_str;
	std::error_code ec;
	if (fs::is_directory(save_dir, ec) == false)
	{
		fs::create_directories(save_dir, ec);
	}

	std::wstring user_name = L"未命名";
	try
	{
		user_name = usr_sessions_mgr().get_user_data(session_id)->json.at("usrname").get<std::wstring>();
	}
	catch (...)
	{
		user_name = L"未命名";
	}

	std::wstring file_name = user_name + L"_" + std::wstring(time_str) + L".jpg";
	std::ofstream output(save_dir / file_name, std::ios::out | std::ios::binary);
	if (output.is_open())
	{
		output.write((char*)data.data(), data.size());
		output.close();
	}
}

void CLogicServer::log_cb(const wchar_t* msg, bool silence, bool gm_show, const std::string& identify, bool punish_flag)
{
	ProtocolLSLCLogPrint log;
	log.text = msg;
	log.identify = identify;
	log.silence = silence;
	log.gm_show = gm_show;
	log.punish_flag = punish_flag;
	foreach_session([this, &log](tcp_session_shared_ptr_t& session) {
		for (auto session_id : obs_sessions_mgr().sessions())
		{
			send(session, session_id, &log);
		}
		});
}

void CLogicServer::punish(tcp_session_shared_ptr_t& session, unsigned int session_id, ProtocolPolicy& policy, const std::wstring& comment, const std::wstring& comment_2)
{
	static std::map<PunishType, wchar_t*> punish_type_str = {
	   {ENM_PUNISH_TYPE_KICK,TEXT("退出游戏")},
	   {ENM_PUNISH_TYPE_NO_OPEARATION,TEXT("不处理")},
	   {ENM_PUNISH_TYPE_SUPER_WHITE_LIST,TEXT("白名单")},
	   {ENM_PUNISH_TYPE_BAN_MACHINE,TEXT("封机器")},
	   {ENM_PUNISH_TYPE_SCREEN_SHOT,TEXT("截图")},
	   {ENM_PUNISH_TYPE_SCREEN_SHOT_KICK,TEXT("截图+退出游戏")},
	};
	static std::map<PolicyType, wchar_t*> policy_type_str = {
		{ENM_POLICY_TYPE_MODULE_NAME,TEXT("模块名检测")},
		{ENM_POLICY_TYPE_PROCESS_NAME,TEXT("进程名检测")},
		{ENM_POLICY_TYPE_FILE_NAME,TEXT("文件路径")},
		{ENM_POLICY_TYPE_WINDOW_NAME,TEXT("窗口名")},
		{ENM_POLICY_TYPE_MACHINE,TEXT("机器码")},
		{ENM_POLICY_TYPE_MULTICLIENT,TEXT("多开限制")},
		/*{ENM_POLICY_TYPE_SHELLCODE,TEXT("云代码")},*/
		{ENM_POLICY_TYPE_SCRIPT,TEXT("脚本")},
		{ENM_POLICY_TYPE_THREAD_START,TEXT("线程特征")}
	};
	auto user_data = usr_sessions_mgr().get_user_data(session_id);
	bool gm_show = 688000 < policy.policy_id && policy.policy_id < 689000;
	if (user_data)
	{
		std::string ip = user_data->json.find("ip") != user_data->json.end() ? user_data->json.at("ip").get<std::string>() : "(NULL)";
		std::wstring usr_name = user_data->json.find("usrname") != user_data->json.end() ? user_data->json.at("usrname").get<std::wstring>() : L"(NULL)";
		ProtocolPolicy while_list_policy;
		if (policy_mgr_.is_ip_or_mac_in_super_white_list(Utils::c2w(ip), while_list_policy))
		{
			user_log(LOG_TYPE_EVENT, true, gm_show, user_data->get_uuid().str(), TEXT("超级白名单玩家:%s 策略类型:%s 策略id:%d 处罚类型:%s 处罚原因:%s|%s"),
				usr_name.c_str(),
				policy_type_str[(PolicyType)policy.policy_type],
				policy.policy_id,
				punish_type_str[ENM_PUNISH_TYPE_NO_OPEARATION],
				comment.c_str(),
				comment_2.c_str()
			);
			return;
		}
		else if (policy_mgr_.is_ip_or_mac_in_super_white_list(user_data->mac, while_list_policy))
		{
			user_log(LOG_TYPE_EVENT, true, gm_show, user_data->get_uuid().str(), TEXT("超级白名单玩家:%s 策略类型:%s 策略id:%d 处罚类型:%s 处罚原因:%s|%s"),
				usr_name.c_str(),
				policy_type_str[(PolicyType)policy.policy_type],
				policy.policy_id,
				punish_type_str[ENM_PUNISH_TYPE_NO_OPEARATION],
				comment.c_str(),
				comment_2.c_str()
			);
			return;
		}

		ProtocolOBS2OBCPunishUserUUID resp;
		resp.uuid = Utils::c2w(user_data->get_uuid().str());
		foreach_session([this, &resp](tcp_session_shared_ptr_t& session) {
			for (auto session_id : obs_sessions_mgr().sessions())
			{
				send(session, session_id, &resp);
			}
			});
		// 处罚玩家写到GM的开挂玩家列表.txt
		punish_log(TEXT("处罚玩家:%s 处罚类型:%s 处罚原因:%s|%s"),
			usr_name.c_str(),
			punish_type_str[(PunishType)policy.punish_type],
			comment.c_str(),
			comment_2.c_str()
		);

		user_log(LOG_TYPE_EVENT, false, gm_show, user_data->get_uuid().str(), TEXT("处罚玩家:%s 策略类型:%s 策略id:%d 处罚类型:%s 处罚原因:%s|%s"),
			usr_name.c_str(),
			policy_type_str[(PolicyType)policy.policy_type],
			policy.policy_id,
			punish_type_str[(PunishType)policy.punish_type],
			comment.c_str(),
			comment_2.c_str()
		);

		if (policy.punish_type != ENM_PUNISH_TYPE_NO_OPEARATION)
		{
			if (user_data->get_punish_times() == 0)
			{
				user_data->add_punish_times();
			}
			else
			{
				if (user_data->get_last_punish_duration() > std::chrono::minutes(2))
				{
					user_data->add_punish_times();
					user_data->update_last_punish_time();
				}
			}

			if (user_data->get_punish_times() >= 3)
			{
				user_log(LOG_TYPE_EVENT, true, false, user_data->get_uuid().str(), TEXT("处罚失效，请手动处罚:%d"), user_data->session_id);
				std::wstring usr_name = user_data->json.find("usrname") != user_data->json.end() ? user_data->json.at("usrname").get<std::wstring>() : L"(NULL)";
				//write_txt(".\\恶性开挂人员名单.txt", trim_user_name(Utils::w2c(usr_name)));
			}
		}

		switch (policy.punish_type)
		{
			case ENM_PUNISH_TYPE_KICK:
			{
				ProtocolS2CPunish resp;
				resp.type = policy.punish_type;
				send(session, session_id, &resp);
				break;
			}
			case ENM_PUNISH_TYPE_SCREEN_SHOT:
			{
				ProtocolS2CQueryScreenShot req;
				send(session, session_id, &req);
				policy_pkg_mgr_.register_handler(SPKG_ID_C2S_QUERY_SCREENSHOT | session_id, [this](tcp_session_shared_ptr_t& session, unsigned int ob_session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
					write_img(package.head.session_id, msg.get().as<ProtocolC2SQueryScreenShot>().data);
					io().context().post([this, session_id = package.head.session_id]() {
						policy_pkg_mgr_.remove_handler(SPKG_ID_C2S_QUERY_SCREENSHOT | session_id);
						});
					});
				break;
			}
			case ENM_PUNISH_TYPE_SCREEN_SHOT_KICK:
			{
				ProtocolS2CQueryScreenShot req;
				send(session, session_id, &req);
				policy_pkg_mgr_.register_handler(SPKG_ID_C2S_QUERY_SCREENSHOT | session_id, [this](tcp_session_shared_ptr_t& session, unsigned int ob_session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
					write_img(package.head.session_id, msg.get().as<ProtocolC2SQueryScreenShot>().data);
					io().context().post([this, session = session->hash_key(), session_id = package.head.session_id]() {
						policy_pkg_mgr_.remove_handler(SPKG_ID_C2S_QUERY_SCREENSHOT | session_id);
						ProtocolS2CPunish resp;
						resp.type = PunishType::ENM_PUNISH_TYPE_KICK;
						send(sessions().find(session), session_id, &resp);
						});
					});
				break;
			}
			case ENM_PUNISH_TYPE_BAN_MACHINE:
			{
				ProtocolS2CPunish resp;
				resp.type = PunishType::ENM_PUNISH_TYPE_KICK;
				send(session, session_id, &resp);
				ProtocolPolicy policy;
				policy.punish_type = PunishType::ENM_PUNISH_TYPE_KICK;
				policy.policy_type = PolicyType::ENM_POLICY_TYPE_MACHINE;
				policy.config = usr_sessions_mgr().get_user_data(session_id)->mac;
				policy_mgr_.add_policy(policy);
				break;
			}
			default:
				break;
		}
	}
	else
	{
		log(LOG_TYPE_ERROR, TEXT("处罚玩家失败"));
	}
}

void CLogicServer::detect(tcp_session_shared_ptr_t& session, unsigned int session_id)
{
	VMProtectBeginVirtualization(__FUNCTION__);
	auto user_data = usr_sessions_mgr().get_user_data(session_id);
	if (user_data)
	{
		auto mac = user_data->mac;
		std::string ip = user_data->json["ip"];
		std::wstring wstr_ip = Utils::c2w(ip);
		auto cur_usr_machine_count = usr_sessions_mgr().get_machine_count(mac);
		if (cur_usr_machine_count > policy_mgr_.get_multi_client_limit_count())
		{
			punish(session, session_id, policy_mgr_.get_multi_client_policy(), L"超出运行客户端限定");
			return;
		}
		ProtocolPolicy policy;
		if (policy_mgr_.is_ip_or_mac_ban(wstr_ip, policy))
		{
			punish(session, session_id, policy, L"封IP", wstr_ip);
		}
		if (policy_mgr_.is_ip_or_mac_ban(mac, policy))
		{
			punish(session, session_id, policy, L"封机器码", mac);
		}
	}
	VMProtectEnd();
}

void CLogicServer::close_socket(tcp_session_shared_ptr_t& session, unsigned int session_id)
{
	VMProtectBeginVirtualization(__FUNCTION__);
	ProtocolLS2LCKick req;
	req.session_id = session_id;
	super::send(session, &req);
	VMProtectEnd();
}
std::string CLogicServer::trim_user_name(const std::string& username_)
{
	std::string username = username_;
	if (username.empty() || username == "(NULL)")
	{
		return "";
	}
	size_t pos = username.find(" - ");
	if (pos != std::string::npos)
	{
		username.replace(pos, 3, "-");
	}
	return username;
	VMProtectEnd();
}

void CLogicServer::OnlineCheck()
{
	VMProtectBeginVirtualization(__FUNCTION__);
	try
	{
		std::filesystem::path online_path = g_cur_dir;
		online_path /= CONFIG_APP_NAME"网关在线玩家.txt";
		std::ofstream online(online_path, std::ios::out | std::ios::binary | std::ios::trunc);

		std::string gamer, username;
		usr_sessions_mgr().foreach_session([&username, &online](std::shared_ptr<ProtocolUserData>& user_data) {
			if (user_data->json.find("usrname") == user_data->json.end()) return;
			username = Utils::w2c(user_data->json.at("usrname").get<std::wstring>());
			if (username.empty() || username == "(NULL)")
			{
				return;
			}
			size_t pos = username.find(" - ");
			if (pos != std::string::npos)
			{
				username.replace(pos, 3, "-");
			}
			online << username << "\r\n";
			});

		online.flush();
		online.close();
	}
	catch (...)
	{
		log(LOG_TYPE_ERROR, TEXT("---网关在线玩家写入出错,请检测文件是否存在!"));
	}
	VMProtectEnd();
}
