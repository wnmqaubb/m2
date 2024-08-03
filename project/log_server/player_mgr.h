#pragma once

class PlayerMgr
{
public:
	static PlayerMgr& instance();
	bool is_alive(unsigned long id);
	bool exist(unsigned long id) { return players_.find(id) != players_.end(); }
	std::weak_ptr<json> get_player_data(unsigned long id);
	void heartbeat(unsigned long id, const ProtocolHeartBeat& proto);
	void login(unsigned long id, const std::wstring& username);
	void logout(unsigned long id);
	unsigned long get_online_player_count() { return players_.size(); }
	std::unordered_map<unsigned long, std::shared_ptr<json>>& get_players() { return players_; }
private:
	std::mutex mtx_;
	std::unordered_map<unsigned long, std::shared_ptr<json>> players_;
};