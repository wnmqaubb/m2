#include "pch.h"
#include <asio2/asio2.hpp>
#include "log_client.h"

const std::string default_key = "cxkzopdia9021owq";

struct UserData
{
	bool is_login = false;
	bool enable_log = false;
	bool enable_trans_recv = false;
	std::unordered_set<unsigned long> trans_recv_id_filter;

};

LogClient::LogClient()
	: server_(512, 0x3FFFFFFF, std::thread::hardware_concurrency())
{
	server_.bind("login", &LogClient::login_internal_, this);
	server_.bind("trans_send", &LogClient::trans_send_internal_, this);
	server_.bind("set_trans_recv", &LogClient::set_trans_recv_internal_, this);
	server_.bind("enable_log", &LogClient::enable_log_internal_, this);
	server_.bind_connect([this](rpc_conn session_ptr) {
		debug_log("%s:%u connect", session_ptr->remote_address().c_str(), session_ptr->remote_port());
	});
	server_.bind_disconnect([this](rpc_conn session_ptr) {
		debug_log("%s:%u disconnect", session_ptr->remote_address().c_str(), session_ptr->remote_port());
	});
	server_.start("0.0.0.0", "23229");
}

LogClient::~LogClient()
{

}

LogClient& LogClient::instance()
{
	static LogClient instance_;
	return instance_;
}


bool LogClient::login_internal_(rpc_conn conn, const std::string& key)
{
	if (key == default_key)
	{
		auto user_data = conn->user_data<UserData>();
		user_data.is_login = true;
		conn->user_data<UserData>(std::move(user_data));
		return true;
	}
	return false;
}

void LogClient::enable_log_internal_(rpc_conn conn, bool enable_log)
{
	auto user_data = conn->user_data<UserData>();
	if (user_data.is_login)
	{
		user_data.enable_log = enable_log;
		conn->user_data<UserData>(std::move(user_data));
	}
}

void LogClient::set_trans_recv_internal_(rpc_conn conn, bool enable_trans_recv, const std::unordered_set<unsigned long>& filter)
{
	auto user_data = conn->user_data<UserData>();
	if (user_data.is_login)
	{
		user_data.enable_trans_recv = enable_trans_recv;
		user_data.trans_recv_id_filter = filter;
		conn->user_data<UserData>(std::move(user_data));
	}
}

bool LogClient::trans_send_internal_(rpc_conn conn, unsigned long id, std::vector<unsigned char> buffer)
{
	if (!conn->user_data<UserData>().is_login)
	{
		return false;
	}
	if (trans_send_)
		return trans_send_(id, buffer.data(), buffer.size());
	return false;
}

void LogClient::trans_recv(unsigned long id, unsigned long proto_id, const unsigned char* buffer, int size)
{
	std::vector<unsigned char> buffer_(size);
	memcpy(buffer_.data(), buffer, size);
	server_.foreach_session([&](rpc_conn conn) {
		auto user_data = conn->user_data<UserData>();
		if (user_data.is_login && user_data.enable_trans_recv)
		{
			if (user_data.trans_recv_id_filter.find(proto_id) != user_data.trans_recv_id_filter.end())
			{
				conn->async_call([](std::error_code& ec) {
					if (ec) {
						debug_log(ec.message().c_str());
					}
				}, std::chrono::seconds(5), "trans_recv", id, buffer_);
			}
		}
	});
}

template<typename... Args>
void LogClient::log(unsigned long id, Args&&... args)
{
	std::initializer_list<std::string> args_ = { args... };
	std::list<std::string> vec_args_(args_.begin(), args_.end());
	vec_args_.emplace_front(std::to_string(id));
	server_.foreach_session([&](rpc_conn conn) {
		auto user_data = conn->user_data<UserData>();
		if (user_data.is_login && user_data.enable_log)
		{
			conn->async_call([](std::error_code& ec) {
				if (ec) {
					debug_log(ec.message().c_str());
				}
			}, std::chrono::seconds(5), "log", vec_args_);
		}
	});
}


void DLL_EXPORT log_server_register_trans_send(trans_send_ptr_t trans_send)
{
	LogClient::instance().set_trans_send(trans_send);
}

void DLL_EXPORT log_server_trans_recv(unsigned long id, unsigned long proto_id, const unsigned char* buffer, int size)
{
	LogClient::instance().trans_recv(id, proto_id, buffer, size);
}

template<typename... Args>
void DLL_EXPORT log_server_log(unsigned long id, Args... args)
{
	LogClient::instance().log(id, args...);
}

template<>
void DLL_EXPORT log_server_log(unsigned long id, const char* p1)
{
	LogClient::instance().log(id, p1);
}

