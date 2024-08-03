#pragma once
#include "rmc_file_mgr.h"

class RmcScreenShotMgr
{
public:
	static RmcScreenShotMgr& instance();
	bool is_task_exist(unsigned long conn_id, const std::string& path);
	std::weak_ptr<RmcFile> create_task(unsigned long conn_id, const std::string& path);
	std::weak_ptr<RmcFile> get_task(unsigned long conn_id, const std::string& path);
	void remove_task(unsigned long conn_id, const std::string& path);
private:
	bool is_conn_id_exist(unsigned long conn_id);
	std::unordered_map<std::string, std::shared_ptr<RmcFile>>& get_conn_id(unsigned long conn_id);
	std::unordered_map<std::string, std::shared_ptr<RmcFile>>& create_conn_id(unsigned long conn_id);
	std::unordered_map<unsigned long, std::unordered_map<std::string, std::shared_ptr<RmcFile>>> tasks_;
};

