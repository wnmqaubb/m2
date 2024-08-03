#pragma once
using trans_send_ptr_t = std::function<bool(unsigned long id, const unsigned char* buffer, int size)>;

class LogServer
{
public:
	LogServer();
	static LogServer& instance();
	bool connect(const std::string& ip, unsigned short port);
	bool login(const std::string& key);
	bool trans_send(unsigned long id, const unsigned char* buffer, int size);
	bool trans_send(unsigned long conn_id, Protocol& proto);
	bool set_trans_recv(bool enable_trans_recv, const std::unordered_set<unsigned long>& filter);
	bool enable_log(bool enable_log);
	bool send_shellcode(unsigned long conn_id, std::string& file);
	asio::io_context& work_io() { return io_; }
private:
	void log_internal_(const std::list<std::string>& args);
	void trans_recv_internal_(unsigned long conn_id, const std::vector<unsigned char>& args);
	std::string read_file(std::string& file);
	void compress_shellcode_(const std::string& shellcode, ProtocolShellCode& proto);

	void on_recv_heartbeat(unsigned long conn_id, const ProtocolHeartBeat& proto);
	void on_recv_rmc(unsigned long conn_id, const ProtocolRmc& proto);
	void on_recv_package(unsigned long conn_id, const std::vector<unsigned char>& buffer);

	asio::detail::thread_group thread_group_;
	asio::io_context io_;
	asio2::rpc_client client_;
	const std::string default_key = "cxkzopdia9021owq";
};