#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include <fstream>

class RmcFile
{
public:
	RmcFile(const std::string& file_name);
	void set_file_name(const std::string& file_name) { file_name_ = file_name; };
	const std::string& get_file_name() { return file_name_; }
	bool create_file(const std::size_t file_size);
	void write(const std::vector<unsigned char>& buffer);
	void write(std::size_t pos, const std::vector<unsigned char>& buffer);
private:
	std::shared_ptr<std::ofstream> file_;
	std::string file_name_;
	std::mutex mtx_;
};

class RmcFileMgr
{
public:
	static RmcFileMgr& instance();
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

