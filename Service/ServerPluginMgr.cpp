#include "pch.h"
#include "LogicServer.h"
#include "ServerPluginMgr.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_set>
#include <utility>
#include <vector>
#include <msgpack/v1/object_fwd.hpp>
#include <msgpack/v1/pack.hpp>
#include <msgpack/v1/sbuffer.hpp>
#include <msgpack/v3/unpack.hpp>
#include "NetUtils.h"
#include "Protocol.h"
#include "SubServicePackage.h"

// 优化后的插件管理实现
RawProtocolImpl CServerPluginMgr::get_plugin(unsigned int plugin_hash) {
    constexpr int SHARD_COUNT = 256;
    size_t shard_idx = plugin_hash % SHARD_COUNT;
    PluginShard& shard = plugin_shards_[shard_idx];
    std::shared_lock lock(shard.mtx);
    
    tbb::concurrent_hash_map<unsigned int, 
        std::pair<RawProtocolImpl, ProtocolS2CDownloadPlugin>>::const_accessor ca;
    if (shard.plugins.find(ca, plugin_hash)) {
        return ca->second.first;
    }
    return RawProtocolImpl();
}

bool CServerPluginMgr::is_plugin_file_hash_exist(unsigned int file_hash) {
    constexpr int SHARD_COUNT = 256;
    size_t shard_idx = file_hash % SHARD_COUNT;
    PluginShard& shard = plugin_shards_[shard_idx];
    std::shared_lock lock(shard.mtx);
    
    tbb::concurrent_hash_map<unsigned int, 
        std::pair<RawProtocolImpl, ProtocolS2CDownloadPlugin>>::const_accessor ca;
    return shard.plugins.find(ca, file_hash);
}

void CServerPluginMgr::add_plugin(unsigned int file_hash, ProtocolS2CDownloadPlugin& plugin) {
    constexpr int SHARD_COUNT = 256;
    size_t shard_idx = file_hash % SHARD_COUNT;
    PluginShard& shard = plugin_shards_[shard_idx];
    std::unique_lock lock(shard.mtx);

    tbb::concurrent_hash_map<unsigned int, 
        std::pair<RawProtocolImpl, ProtocolS2CDownloadPlugin>>::accessor acc;
    if (shard.plugins.insert(acc, file_hash)) {
        msgpack::sbuffer buffer;
        msgpack::pack(buffer, plugin);
        RawProtocolImpl raw_package;
        raw_package.encode(buffer.data(), buffer.size());
        acc->second = std::make_pair(raw_package, plugin);
        printf("加载插件:%s\n", plugin.plugin_name.c_str());
    }
}

void CServerPluginMgr::remove_plugin(unsigned int file_hash) {
    constexpr int SHARD_COUNT = 256;
    size_t shard_idx = file_hash % SHARD_COUNT;
    PluginShard& shard = plugin_shards_[shard_idx];
    std::unique_lock lock(shard.mtx);

    tbb::concurrent_hash_map<unsigned int, 
        std::pair<RawProtocolImpl, ProtocolS2CDownloadPlugin>>::accessor acc;
    if (shard.plugins.find(acc, file_hash)) {
        printf("卸载插件:%s\n", acc->second.second.plugin_name.c_str());
        shard.plugins.erase(acc);
    }
}


void CServerPluginMgr::reload_all_plugin()
{
    namespace fs = std::filesystem;
    std::error_code ec;
    fs::path dir = g_cur_dir / "plugin";
    if (fs::is_directory(dir, ec))
    {
        std::map<unsigned int, fs::path> new_plugin_list;
        for (auto& file_path : std::filesystem::directory_iterator(dir))
        {
            if (file_path.path().extension() != ".dll")
                continue;
            std::ifstream file(file_path.path(), std::ios::in | std::ios::binary);
            if (file.is_open())
            {
                file.seekg(sizeof(RawProtocolHead), file.beg);
                unsigned int file_hash = 0;
                file.read((char*)&file_hash, sizeof(file_hash));
                file.close();
                new_plugin_list.emplace(std::make_pair(file_hash, file_path));
            }
        }

        for (auto loaded_plugin_file_hash : get_plugin_file_hash_set())
        {
            if (new_plugin_list.find(loaded_plugin_file_hash) == new_plugin_list.end())
            {
                remove_plugin(loaded_plugin_file_hash);
            }
        }
        for (auto[file_hash, plugin_path] : new_plugin_list)
        {
            if (!is_plugin_file_hash_exist(file_hash))
            {
                std::ifstream file(plugin_path, std::ios::in | std::ios::binary);
                if (file.is_open())
                {
                    RawProtocolImpl package;
                    std::stringstream ss;
                    ss << file.rdbuf();
                    file.close();
                    auto str = ss.str();
                    std::string_view sv(str.data(), str.size());
                    package.decode(sv);
                    try
                    {
                        auto msg = msgpack::unpack((char*)package.body.buffer.data(), package.body.buffer.size());
                        add_plugin(file_hash, msg.get().as<ProtocolS2CDownloadPlugin>());
                    }
                    catch (...)
                    {
                        fs::remove(plugin_path);
                    }
                }
            }
        }
    }
}

void CServerPluginMgr::create_plugin_file(const std::string& file_name, std::vector<uint8_t>& data)
{
    std::filesystem::path path = g_cur_dir / "plugin";
    std::error_code ec;
    if (std::filesystem::is_directory(path, ec) == false)
    {
        std::filesystem::create_directory(path, ec);
    }
    path = path / file_name;
    if (path.extension() == ".dll")
    {
        std::ofstream output(path, std::ios::out | std::ios::binary);
        output.write((char*)data.data(), data.size());
        output.close();
    }
}

void CServerPluginMgr::remove_plugin_file(const std::string& file_name)
{
    std::filesystem::path path = g_cur_dir / "plugin";
    std::error_code ec;
    if (std::filesystem::is_directory(path, ec) == false)
    {
        std::filesystem::create_directory(path, ec);
    }
    path = path / file_name;
    if (path.extension() == ".dll")
    {
        std::filesystem::remove(path);
    }
}

std::set<unsigned int> CServerPluginMgr::get_plugin_file_hash_set() {
    std::set<unsigned int> result;
    for(int i=0; i<256; ++i) {
        PluginShard& shard = plugin_shards_[i];
        std::shared_lock lock(shard.mtx);
        tbb::concurrent_hash_map<unsigned int, 
            std::pair<RawProtocolImpl, ProtocolS2CDownloadPlugin>>::iterator it;
        for (it = shard.plugins.begin(); it != shard.plugins.end(); ++it) {
            result.emplace(it->first);
        }
    }
    return result;
}

ProtocolS2CQueryPlugin CServerPluginMgr::get_plugin_hash_set() {
    ProtocolS2CQueryPlugin result;
    for (auto& shard : plugin_shards_) {
        std::shared_lock lock(shard.mtx);
        tbb::concurrent_hash_map<unsigned int, 
            std::pair<RawProtocolImpl, ProtocolS2CDownloadPlugin>>::const_iterator it;
        for (it = shard.plugins.begin(); it != shard.plugins.end(); ++it) {
            result.plugin_list.emplace_back(
                it->second.second.plugin_hash, 
                it->second.second.plugin_name);
        }
    }
    return result;
}

ProtocolS2CPolicy CServerPolicyMgr::get_policy()
{
    std::shared_lock<std::shared_mutex> lck(mtx_);
    return policy_;
}

std::unique_ptr<ProtocolPolicy> CServerPolicyMgr::find_policy(unsigned int policy_id)
{
    std::shared_lock<std::shared_mutex> lck(mtx_);
    if (policy_.policies.find(policy_id) != policy_.policies.end())
    {
        return std::make_unique<ProtocolPolicy>(policy_.policies[policy_id]);
    }
    return nullptr;
}

std::unique_ptr<ProtocolPolicy> CServerPolicyMgr::find_policy(PolicyType policy_type)
{
    std::shared_lock<std::shared_mutex> lck(mtx_);
    for(auto [policy_id, policy] : policy_.policies)
    {
        if (policy.policy_type == policy_type)
        {
            return std::make_unique<ProtocolPolicy>(policy);
        }
    }
    return nullptr;
}

bool CServerPolicyMgr::is_policy_file_hash_exist(unsigned int file_hash)
{
    std::shared_lock<std::shared_mutex> lck(mtx_);
    return policy_cache_.find(file_hash) != policy_cache_.end();
}

void CServerPolicyMgr::add_policy(unsigned int file_hash, ProtocolS2CPolicy& policy)
{
	std::unique_lock<std::shared_mutex> lck(mtx_);
	if (policy_cache_.find(file_hash) == policy_cache_.end())
	{
		policy_cache_.emplace(std::make_pair(file_hash, policy));
		policy_.policies.clear();
		for (auto [file_hash, policy] : policy_cache_)
		{
			for (auto [policy_id, sub_policy] : policy.policies)
			{
				policy_.policies[policy_id] = sub_policy;
			}
		}
		printf("加载配置：%08X\n", file_hash);
	}
}

void CServerPolicyMgr::add_policy(ProtocolPolicy& policy)
{
    std::filesystem::path g_cfg_path = g_cur_dir / "config.cfg";

    std::ifstream file(g_cfg_path, std::ios::in | std::ios::binary);
    if (!file.is_open())
        return;
    std::stringstream ss;
    ss << file.rdbuf();
    file.close();
    auto str = ss.str();
    auto policies = ProtocolS2CPolicy::load(str.data(), str.size());
    unsigned int new_policy_id = 0;
    for (auto[policy_id, policy] : policies->policies)
    {
        new_policy_id = policy_id;
    }
    new_policy_id++;
    policy.policy_id = new_policy_id;
    policies->policies[new_policy_id] = policy;
    auto buf = policies->dump();
    std::ofstream output(g_cfg_path, std::ios::out | std::ios::binary);
    output.write(buf.data(), buf.size());
    output.close();
}

void CServerPolicyMgr::remove_policy(unsigned int file_hash)
{
	std::unique_lock<std::shared_mutex> lck(mtx_);
	if (policy_cache_.find(file_hash) != policy_cache_.end())
	{
		policy_cache_.erase(file_hash);
		policy_.policies.clear();
		for (auto [file_hash, policy] : policy_cache_)
		{
			for (auto [policy_id, sub_policy] : policy.policies)
			{
				policy_.policies[policy_id] = sub_policy;
			}
		}
		printf("卸载配置：%08X\n", file_hash);
	}
}

void CServerPolicyMgr::reload_all_policy()
{
    namespace fs = std::filesystem;
    std::error_code ec;
    fs::path dir = g_cur_dir;
    if (fs::is_directory(dir, ec))
    {
        std::map<unsigned int, fs::path> new_policy_list;
        for (auto& file_path : std::filesystem::directory_iterator(dir))
        {
            if (file_path.path().extension() != ".cfg")
                continue;
            std::ifstream file(file_path.path(), std::ios::in | std::ios::binary);
            if (file.is_open())
            {
                file.seekg(sizeof(RawProtocolHead), file.beg);
                unsigned int file_hash = 0;
                file.read((char*)&file_hash, sizeof(file_hash));
                file.close();
                new_policy_list.emplace(std::make_pair(file_hash, file_path));
            }
        }

        for (auto loaded_policy_file_hash : get_policy_file_hash_set())
        {
            if (new_policy_list.find(loaded_policy_file_hash) == new_policy_list.end())
            {
                remove_policy(loaded_policy_file_hash);
            }
        }
        for (auto[file_hash, policy_path] : new_policy_list)
        {
            if (!is_policy_file_hash_exist(file_hash))
            {
                std::ifstream file(policy_path, std::ios::in | std::ios::binary);
                if (file.is_open())
                {
                    RawProtocolImpl package;
                    std::stringstream ss;
                    ss << file.rdbuf();
                    file.close();
                    auto str = ss.str();
                    std::string_view sv(str.data(), str.size());
                    package.decode(sv);
                    try 
                    {
                        auto msg = msgpack::unpack((char*)package.body.buffer.data(), package.body.buffer.size());
                        add_policy(file_hash, msg.get().as<ProtocolS2CPolicy>());
                    }
                    catch (msgpack::type_error err)
                    {
                        file.close();
                        fs::remove(policy_path);
                    }
                }
            }
        }
    }
}

void CServerPolicyMgr::create_policy_file(const std::string& file_name, std::vector<uint8_t>& data)
{
    std::filesystem::path path = g_cur_dir / file_name;
    if (path.extension() == ".cfg")
    {
        std::ofstream output(path, std::ios::out | std::ios::binary);
        output.write((char*)data.data(), data.size());
        output.close();
    }
}

void CServerPolicyMgr::remove_policy_file(const std::string& file_name)
{
    std::filesystem::path path = g_cur_dir / file_name;
    if (path.extension() == ".cfg")
    {
        std::filesystem::remove(path);
    }
}

std::set<unsigned int> CServerPolicyMgr::get_policy_file_hash_set()
{
    std::shared_lock<std::shared_mutex> lck(mtx_);
    std::set<unsigned int> result;
    for (auto itor : policy_cache_)
    {
        result.emplace(itor.first);
    }
    return result;
}

void CServerPolicyMgr::on_policy_reload()
{
    auto multi_client_limit_policy = find_policy(ENM_POLICY_TYPE_MULTICLIENT);
    if (multi_client_limit_policy)
    {
        multi_client_limit_count_ = _wtoi(multi_client_limit_policy->config.c_str());
        multi_client_limit_punish_type_ = (PunishType)multi_client_limit_policy->punish_type;
        multi_client_policy_ = *multi_client_limit_policy;
    }
    else
    {
        multi_client_limit_count_ = 999;
        multi_client_limit_punish_type_ = ENM_PUNISH_TYPE_NO_OPEARATION;
    }

    {
		std::unique_lock<std::shared_mutex> lck(mtx_);
		mac_ban_set_.clear();
		ip_ban_set_.clear();
		rolename_ban_set_.clear();
		mac_white_set_.clear();
		ip_white_set_.clear();
		rolename_white_set_.clear();
		std::string path;
		path = "机器码黑名单.txt";
		read_file_white_and_black(g_cur_dir / path, mac_ban_set_);
		path = "IP黑名单.txt";
		read_file_white_and_black(g_cur_dir / path, ip_ban_set_);
		path = "角色名黑名单.txt";
		read_file_white_and_black(g_cur_dir / path, rolename_ban_set_);
        path = "机器码白名单.txt";
		read_file_white_and_black(g_cur_dir / path, mac_white_set_);
		path = "IP白名单.txt";
		read_file_white_and_black(g_cur_dir / path, ip_white_set_);
		path = "角色名白名单.txt";
		read_file_white_and_black(g_cur_dir / path, rolename_white_set_);
	}
}

void CServerPolicyMgr::read_file_white_and_black(const std::filesystem::path& filePath, std::unordered_set<unsigned int>& white_and_black_list)
{
	if (filePath.empty())
		return;
	std::ifstream inFile(filePath);
	if (inFile.is_open())
	{
		std::string line_txt;
		while (std::getline(inFile, line_txt))
		{
			white_and_black_list.emplace(NetUtils::hash(line_txt.c_str(), line_txt.size()));
		}
		inFile.close();
	}
}
// 白名单优先级高于黑名单
bool CServerPolicyMgr::is_svip(const std::string& mac, const std::string& ip, const std::string& rolename) 
{
	std::shared_lock<std::shared_mutex> lck(mtx_);
    if (mac_white_set_.find(NetUtils::hash(mac.c_str(), mac.size())) != mac_white_set_.end()) {
        return true;
    }
    if (ip_white_set_.find(NetUtils::hash(ip.c_str(), ip.size())) != ip_white_set_.end()) {
        return true;
    }
    if (rolename_white_set_.find(NetUtils::hash(rolename.c_str(), rolename.size())) != rolename_white_set_.end()) {
        return true;
    }
    return false;
}

// 黑名单
bool CServerPolicyMgr::is_ban(const std::string& mac, const std::string& ip, const std::string& rolename)
{
	std::shared_lock<std::shared_mutex> lck(mtx_);
	if (mac_ban_set_.find(NetUtils::hash(mac.c_str(), mac.size())) != mac_ban_set_.end()) {
		return true;
	}
	if (ip_ban_set_.find(NetUtils::hash(ip.c_str(), ip.size())) != ip_ban_set_.end()) {
		return true;
	}
	if (rolename_ban_set_.find(NetUtils::hash(rolename.c_str(), rolename.size())) != rolename_ban_set_.end()) {
		return true;
	}
	return false;
}
