#include <filesystem>
#include "rmc_screen_shot_mgr.h"


RmcScreenShotMgr& RmcScreenShotMgr::instance()
{
	static RmcScreenShotMgr instance_;
	return instance_;
}

bool RmcScreenShotMgr::is_conn_id_exist(unsigned long conn_id)
{
	if (tasks_.find(conn_id) == tasks_.end())
		return false;
	return true;
}

std::unordered_map<std::string, std::shared_ptr<RmcFile>>& RmcScreenShotMgr::get_conn_id(unsigned long conn_id)
{
	return tasks_[conn_id];
}

std::unordered_map<std::string, std::shared_ptr<RmcFile>>& RmcScreenShotMgr::create_conn_id(unsigned long conn_id)
{
	std::unordered_map<std::string, std::shared_ptr<RmcFile>> empty_path_rmc_file_map;
	tasks_.emplace(std::make_pair(conn_id, empty_path_rmc_file_map));
	return get_conn_id(conn_id);
}

bool RmcScreenShotMgr::is_task_exist(unsigned long conn_id, const std::string& guid)
{
	if (tasks_.find(conn_id) == tasks_.end())
		return false;
	if (tasks_[conn_id].find(guid) == tasks_[conn_id].end())
		return false;
	return true;
}

std::weak_ptr<RmcFile> RmcScreenShotMgr::create_task(unsigned long conn_id, const std::string& guid)
{
	if (is_conn_id_exist(conn_id))
	{
		if (!is_task_exist(conn_id, guid))
		{
			get_conn_id(conn_id).emplace(std::make_pair(guid, std::make_shared<RmcFile>(guid)));
		}
	}
	else
	{
		create_conn_id(conn_id).emplace(std::make_pair(guid, std::make_shared<RmcFile>(guid)));
	}
	return get_task(conn_id, guid);
}

std::weak_ptr<RmcFile> RmcScreenShotMgr::get_task(unsigned long conn_id, const std::string& guid)
{
	return tasks_[conn_id][guid];
}

void RmcScreenShotMgr::remove_task(unsigned long conn_id, const std::string& guid)
{
	tasks_[conn_id].erase(tasks_[conn_id].find(guid));
}

