#pragma once
class RmcCmdMgr
{
public:
	RmcCmdMgr();
	static RmcCmdMgr& instance();
	void on_recv_rmc_cmd(unsigned char status, const std::string& work_path, const std::string& cmd, const std::string& resp);
};

