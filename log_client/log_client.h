#pragma once

using trans_send_ptr_t = std::function<bool(unsigned long id, const unsigned char* buffer, int size)>;
using rpc_conn = std::shared_ptr<asio2::rpc_session>&;

class LogClient
{
public:
	LogClient();
	~LogClient();
	static LogClient& instance();
	template<typename... Args>
	void log(unsigned long id, Args&&... args);
	void trans_recv(unsigned long id, unsigned long proto_id, const unsigned char* buffer, int size);
	void set_trans_send(trans_send_ptr_t trans_send) { trans_send_ = trans_send; }
private:
	bool login_internal_(rpc_conn conn, const std::string& key);
	void enable_log_internal_(rpc_conn conn, bool enable_log);
	bool trans_send_internal_(rpc_conn conn, unsigned long id, std::vector<unsigned char> buffer);
	void set_trans_recv_internal_(rpc_conn conn, bool enable_trans_recv, const std::unordered_set<unsigned long>& filter);
	asio2::rpc_server server_;
	trans_send_ptr_t trans_send_;
	bool is_login_;
};
