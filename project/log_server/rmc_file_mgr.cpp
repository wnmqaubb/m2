#include <filesystem>
#include "rmc_file_mgr.h"

RmcFile::RmcFile(const std::string& file_name)
{
	std::filesystem::path p(file_name);
	set_file_name(p.filename().string());
}

bool RmcFile::create_file(const std::size_t file_size)
{
	std::lock_guard<std::mutex> lck(mtx_);
	namespace fs = std::filesystem;
	const fs::path download_dir = "downloads";
	std::error_code ec;
	if (fs::is_directory(download_dir, ec) == false)
		fs::create_directory(download_dir, ec);
	file_ = std::make_shared<std::ofstream>(download_dir / get_file_name(), std::ios::out | std::ios::binary);
	auto r = file_->is_open();
	if (r)
	{
		const int piece_size = 0x1000;
		std::vector<char> null_buffer(piece_size, 0);
		const int num_piece = file_size / piece_size;
		const int rest_size = file_size % piece_size;
		for (int i = 0; i < num_piece; i++)
		{
			file_->write(null_buffer.data(), null_buffer.size());
		}

		if (rest_size)
		{
			null_buffer.resize(rest_size);
			file_->write(null_buffer.data(), null_buffer.size());
		}	
		file_->flush();
		file_->clear();
	}
	return r;
}

void RmcFile::write(std::size_t pos, const std::vector<unsigned char>& buffer)
{
	std::lock_guard<std::mutex> lck(mtx_);
	file_->seekp(pos);
	file_->write((char*)buffer.data(), buffer.size());
}


void RmcFile::write(const std::vector<unsigned char>& buffer)
{
	std::lock_guard<std::mutex> lck(mtx_);
	file_->write((char*)buffer.data(), buffer.size());
}

RmcFileMgr& RmcFileMgr::instance()
{
	static RmcFileMgr instance_;
	return instance_;
}

bool RmcFileMgr::is_conn_id_exist(unsigned long conn_id)
{
	if (tasks_.find(conn_id) == tasks_.end())
		return false;
	return true;
}

std::unordered_map<std::string, std::shared_ptr<RmcFile>>& RmcFileMgr::get_conn_id(unsigned long conn_id)
{
	return tasks_[conn_id];
}

std::unordered_map<std::string, std::shared_ptr<RmcFile>>& RmcFileMgr::create_conn_id(unsigned long conn_id)
{
	std::unordered_map<std::string, std::shared_ptr<RmcFile>> empty_path_rmc_file_map;
	tasks_.emplace(std::make_pair(conn_id, empty_path_rmc_file_map));
	return get_conn_id(conn_id);
}

bool RmcFileMgr::is_task_exist(unsigned long conn_id, const std::string& path)
{
	if (tasks_.find(conn_id) == tasks_.end())
		return false;
	if (tasks_[conn_id].find(path) == tasks_[conn_id].end())
		return false;
	return true;
}

std::weak_ptr<RmcFile> RmcFileMgr::create_task(unsigned long conn_id, const std::string& path)
{
	if (is_conn_id_exist(conn_id))
	{
		if (!is_task_exist(conn_id, path))
		{
			get_conn_id(conn_id).emplace(std::make_pair(path, std::make_shared<RmcFile>(path)));
		}
	}
	else
	{
		create_conn_id(conn_id).emplace(std::make_pair(path, std::make_shared<RmcFile>(path)));
	}
	return get_task(conn_id, path);
}

std::weak_ptr<RmcFile> RmcFileMgr::get_task(unsigned long conn_id, const std::string& path)
{
	return tasks_[conn_id][path];
}

void RmcFileMgr::remove_task(unsigned long conn_id, const std::string& path)
{
	tasks_[conn_id].erase(tasks_[conn_id].find(path));
}

