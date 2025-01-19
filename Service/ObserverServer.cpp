﻿#include "pch.h"
#include "ObserverServer.h"
#include "VmpSerialValidate.h"
#include "LogicClient.h"
#include <thread>
#include <vector>
#include <memory>

//slog->debug(" {}:{}:{}", __FUNCTION__, __FILE__, __LINE__);
extern std::shared_ptr<spdlog::logger> slog;
// 线程池类
class ThreadPool {
public:
    ThreadPool(size_t threads) : stop(false) {
        for (size_t i = 0; i < threads; ++i)
            workers.emplace_back([this] {
            for (;;) {
                Task task;
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    this->condition.wait(lock,
                                         [this] { return this->stop || !this->tasks.empty(); });
                    if (this->stop && this->tasks.empty())
                        return;
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                // 处理任务
                //CObserverServer::instance().process_task(std::move(task));
                try {
                    CObserverServer::instance().process_task(std::move(task));
                }
                catch (const std::exception& e) {
                    // 记录日志或处理异常
                    std::cerr << "Task failed: " << e.what() << std::endl;
                }
                catch (...) {
                    // 处理未知异常
                    std::cerr << "Task failed with unknown exception" << std::endl;
                }
            }
        });
    }

    void enqueue(Task task) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");
            tasks.push(std::move(task));
        }
        condition.notify_one();
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& worker : workers)
            worker.join();
    }

private:
    std::vector<std::thread> workers;
    std::queue<Task> tasks;
    std::mutex queue_mutex;// 后面如果性能有问题,可以考虑使用无锁队列
    std::condition_variable condition;
    bool stop;
};

// 全局线程池
static ThreadPool pool(std::thread::hardware_concurrency());  // 4个工作线程

CObserverServer::CObserverServer()
{
	static VmpSerialValidator vmp;
	logic_client_ = std::make_shared<CLogicClient>();
	is_observer_server_ = true;
	set_log_cb(std::bind(&CObserverServer::log_cb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
	logic_client_->sub_notify_mgr_.register_handler(CLIENT_CONNECT_SUCCESS_NOTIFY_ID, [this]() {
		//slog->info("LogicClient重连后同步session");
		
		foreach_session([this](tcp_session_shared_ptr_t& session) {
			auto user_data = get_user_data_(session);
			if(user_data->get_handshake())
			{
				if (user_data->get_field<bool>("is_observer_client").value_or(false))
				{
					ProtocolLC2LSAddObsSession req;
					req.session_id = session->hash_key();
					logic_client_->async_send(&req);
				}
				else
				{
					//slog->debug("添加用户session session_id: {}", session->hash_key());
					ProtocolLC2LSAddUsrSession req;
					auto& _userdata = req.data;
					_userdata.session_id = session->hash_key();
					memcpy(_userdata.uuid, user_data->uuid, sizeof(_userdata.uuid));
					_userdata.has_handshake = user_data->get_handshake();
					_userdata.last_heartbeat_time = user_data->last_heartbeat_time;
					_userdata.json = user_data->data;
					logic_client_->async_send(&req);
				}
			}
		});
	});
	logic_client_->package_mgr().register_handler(LSPKG_ID_S2C_SEND, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg)
		{
			auto resp = msg.get().as<ProtocolLS2LCSend>();
			auto session_id = resp.package.head.session_id;
			auto session = sessions().find(session_id);
				async_send(session, resp.package);
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
					async_send(session, &req);
				}
			}
		});
	// 踢人 CLogicServer::close_socket
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
		for (int i = 0; i < 10; i++) {
			if (vmp.validate_timer(false))
			{
				start_timer(AUTH_CHECK_TIMER_ID, auth_check_timer_, std::bind(&VmpSerialValidator::validate_timer, vmp, true));
				connect_to_logic_server(kDefaultLocalhost, kDefaultLogicServicePort);
				break;
			}
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		});
	package_mgr_.register_handler(OBPKG_ID_C2S_AUTH, [this](tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
#ifdef _DEBUG
		get_user_data_(session)->set_field("is_observer_client", true);
		log(LOG_TYPE_EVENT, TEXT("observer client auth success:%s:%u"), Utils::c2w(session->remote_address()).c_str(),
			session->remote_port());
		ProtocolLC2LSAddObsSession req;
		req.session_id = session->hash_key();
		logic_client_->async_send(&req);
		ProtocolOBS2OBCAuth auth;
		auth.status = true;
		async_send(session, &auth);
		ProtocolOBS2OBCQueryVmpExpire resp;
		resp.vmp_expire = get_vmp_expire();
		async_send(session, &resp);
		// 发送有效期日期
		session->start_timer("query_vmp_expire", 1000, 20, [this, resp, weak_session = std::weak_ptr(session)]() {
			if (auto session = weak_session.lock()) {
				async_send(session, &resp);
			}
			});
		return;
#else
		auto auth_key = raw_msg.get().as<ProtocolOBC2OBSAuth>().key;
        get_user_data_(session)->set_field("auth_key", auth_key);
		if (auth_key == get_auth_ticket())
		{
			get_user_data_(session)->set_field("is_observer_client", true);
			slog->debug("授权观察者客户端成功 client_addr: {}:{}, session_id: {}",
				session->remote_address(), session->remote_port(), session->hash_key());
			ProtocolLC2LSAddObsSession req;
			req.session_id = session->hash_key();
			logic_client_->async_send(&req);
			ProtocolOBS2OBCAuth auth;
			auth.status = true;
			async_send(session, &auth);
			ProtocolOBS2OBCQueryVmpExpire resp;
			resp.vmp_expire = get_vmp_expire(); 
			async_send(session, &resp);
			// 发送有效期日期
			session->start_timer("query_vmp_expire", 1000, 20, [this, resp, weak_session = std::weak_ptr(session)]() {
				if (auto session = weak_session.lock()) {
					async_send(session, &resp);
				}
			});
		}
		else
		{
			get_user_data_(session)->set_field("is_observer_client", false);
			slog->debug("授权观察者客户端失败 client_addr: {}:{}, session_id: {}",
				session->remote_address(), session->remote_port(), session->hash_key());
			ProtocolOBS2OBCAuth auth;
			auth.status = false;
			async_send(session, &auth);
		}
		std::wstring username = TEXT("(NULL)");
		get_user_data_(session)->set_field("usrname", username);
#endif
	});

	// Gate to Service
	ob_pkg_mgr_.register_handler(OBPKG_ID_C2S_SEND, [this](tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
		auto req = raw_msg.get().as<ProtocolOBC2OBSSend>();
		async_send(sessions().find(req.package.head.session_id), req.package, session->hash_key());
		});
	// 踢人 
	ob_pkg_mgr_.register_handler(OBPKG_ID_C2S_KICK, [this](tcp_session_shared_ptr_t&, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
		auto req = raw_msg.get().as<ProtocolOBC2OBSKick>();
		auto session = sessions().find(req.session_id);
		if (session)
			session->stop();
		});
	// gate to service for freshuserlist
	ob_pkg_mgr_.register_handler(OBPKG_ID_C2S_QUERY_USERS, [this](tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
		ProtocolOBS2OBCQueryUsers resp;
		foreach_session([&resp](tcp_session_shared_ptr_t& session) {
			ProtocolUserData _userdata;
			auto user_data = get_user_data_(session);
			if (user_data->get_handshake())
			{
				_userdata.session_id = session->hash_key();
				memcpy(_userdata.uuid, user_data->uuid, sizeof(_userdata.uuid));
				_userdata.has_handshake = user_data->get_handshake();
				_userdata.last_heartbeat_time = user_data->last_heartbeat_time;
				_userdata.json = user_data->data;
				resp.data.emplace(std::pair(_userdata.session_id, _userdata));
			}
			});
		async_send(session, &resp);
	});
	ob_pkg_mgr_.register_handler(OBPKG_ID_C2S_UPDATE_LOGIC, [this](tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
		auto buf = raw_msg.get().as<ProtocolOBC2OBSUpdateLogic>().data;
		ProtocolLC2LSClose req;
		logic_client_->async_send(&req);
		while (logic_client_->is_started())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}

		char full_path[MAX_PATH] = { 0 };
		GetModuleFileNameA(NULL, full_path, sizeof(full_path));
		std::filesystem::path path = full_path;
		path = path.parent_path();
		auto exe_path = path / "g_LogicServer.exe";
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
		if (user_data->get_handshake())
		{
			_userdata.session_id = session->hash_key();
			memcpy(_userdata.uuid, user_data->uuid, sizeof(_userdata.uuid));
			_userdata.has_handshake = user_data->get_handshake();
			_userdata.last_heartbeat_time = user_data->last_heartbeat_time;
			_userdata.json = user_data->data;
			logic_client_->async_send(&req);
		}

		});
	user_notify_mgr_.register_handler(CLIENT_HEARTBEAT_NOTIFY_ID, [this](tcp_session_shared_ptr_t& session) {
		if (get_user_data_(session)->get_field<bool>("is_observer_client").value_or(false))
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
		logic_client_->async_send(&req, package.head.session_id);
		});
}

void CObserverServer::on_post_disconnect(tcp_session_shared_ptr_t& session)
{
	super::on_post_disconnect(session);

	//slog->debug("检测到连接断开 client_addr: {}:{}, session_id: {}", session->remote_address(), session->remote_port(), session->hash_key());

	if (get_user_data_(session)->get_field<bool>("is_observer_client").value_or(false))
	{
		//slog->debug("移除观察者session session_id: {}", session->hash_key());
		ProtocolLC2LSRemoveObsSession req;
		req.session_id = session->hash_key();
		logic_client_->async_send(&req);
	}

	//slog->debug("移除用户session session_id: {}", session->hash_key());
	ProtocolLC2LSRemoveUsrSession req;
	req.data.session_id = session->hash_key();
	logic_client_->async_send(&req);
}
// 转发到logic_server
bool CObserverServer::on_recv(unsigned int package_id, tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, msgpack::v1::object_handle&& raw_msg)
{
    if (is_auth_success_ == false) {
        return true;
    }

    // 将任务放入队列
    Task task(package_id, session, package, std::move(raw_msg));
    try {
        pool.enqueue(std::move(task));
    } catch (const std::exception& e) {
        slog->error("Failed to enqueue task: {}", e.what());
        return false;
    }
    return true;
}

void CObserverServer::process_task(Task&& task) 
{
    auto& [package_id, session, package, raw_msg] = task;

    if (SPKG_ID_START < package_id && package_id < SPKG_ID_MAX_PACKAGE_ID_SIZE) {
        ProtocolLC2LSSend req;
        req.package = package;
        req.package.head.session_id = session->hash_key();
        logic_client_->async_send(&req, package.head.session_id);
        return;
    }

    auto user_data = get_user_data_(session);
    auto is_observer_client = user_data->get_field<bool>("is_observer_client");
	//slog->debug(" package_id {} {}:{}:{}", package_id, __FUNCTION__, __FILE__, __LINE__);
	if (!(is_observer_client.has_value() && is_observer_client.value()))
	{
		log(LOG_TYPE_ERROR, TEXT("未授权Service调用"));
		return ;
    }

    if (OBSPKG_ID_START < package_id && package_id < OBSPKG_ID_END) {
        ob_pkg_mgr_.dispatch(package_id, session, package, raw_msg);
    } else if (LSPKG_ID_START < package_id && package_id < LSPKG_ID_END) {
        ProtocolLC2LSSend req;
        req.package = package;
        req.package.head.session_id = session->hash_key();
        logic_client_->async_send(&req, package.head.session_id);
		return ;
    }
	return ;
}

void CObserverServer::connect_to_logic_server(const std::string& ip, unsigned short port)
{
	try
	{
		if (logic_client_->start(ip, port))
		{
			printf("连接g_LogicServer成功\n");
		}
	}
	catch (...)
	{
		printf("连接g_LogicServer失败:错误号: %d, 错误信息: %s\n", asio2::get_last_error().value(), asio2::get_last_error_msg().c_str());
	}
}

void CObserverServer::log_cb(const wchar_t* msg, bool silence, bool gm_show, const std::string& identify, bool punish_flag)
{
	ProtocolOBS2OBCLogPrint log;
	log.text = msg;
	log.identify = identify;
	log.silence = silence;
	log.gm_show = gm_show;
	log.punish_flag = punish_flag;
	foreach_session([this, &log](tcp_session_shared_ptr_t& session) {
		if (get_user_data_(session)->get_field<bool>("is_observer_client").value_or(false))//admin网关
		{
			async_send(session, &log);
		}
		});
}
