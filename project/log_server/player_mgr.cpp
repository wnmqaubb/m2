#include <iostream>
#include <chrono>
#include <mutex>
#include "protocol.h"
#include "utils.h"
#include "player_mgr.h"
#include <set>

PlayerMgr&  PlayerMgr::instance()
{
	static PlayerMgr instance_;
	return instance_;
}

bool PlayerMgr::is_alive(unsigned long conn_id)
{
	if (!exist(conn_id))
		return false;
	json& data = *get_player_data(conn_id).lock();
	auto now_ = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	decltype(now_) last_tick_cout = data["last_tick_count"];
	auto delta = now_ - last_tick_cout;
	if (delta < std::chrono::milliseconds(60 * 1000).count())
	{
		return true;
	}
	return false;
}

std::weak_ptr<json> PlayerMgr::get_player_data(unsigned long conn_id)
{
	if (exist(conn_id))
	{
		return players_[conn_id];
	}
	throw std::make_error_code(std::errc::not_connected);
	return std::make_shared<json>();
}

void PlayerMgr::heartbeat(unsigned long conn_id, const ProtocolHeartBeat& proto)
{
	if (!exist(conn_id))
	{
		login(conn_id, proto.gamer_username);
	}
	json& data = *get_player_data(conn_id).lock();
	data["last_tickcount"] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	data["username"] = to_utf8(proto.gamer_username);
	data["mac"] = to_utf8(proto.mac_address);
	data["volume"] = to_utf8(proto.volume_serial_number);
}

void PlayerMgr::login(unsigned long id, const std::wstring& username)
{
	std::lock_guard<std::mutex> lck(mtx_);
	if (exist(id))
		return;
	std::shared_ptr<json> data_ = std::make_shared<json>();
	players_.insert(std::make_pair(id, data_));
	auto& data = *data_;
	data["username"] = to_utf8(username);
	data["last_tickcount"] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	data["processes"] = std::set<std::string>();
	data["windows"] = std::set<std::string>();
	data["computer_name"] = std::string();
}

void PlayerMgr::logout(unsigned long id)
{
	if (exist(id))
	{
		std::lock_guard<std::mutex> lck(mtx_);
		players_.erase(players_.find(id));
	}
}
