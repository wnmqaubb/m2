#include "pch.h"
#include "LogicServer.h"
#include "ServerPluginMgr.h"

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
    {
        std::unique_lock<std::shared_mutex> lck(mtx_);
        if (policy_cache_.find(file_hash) == policy_cache_.end())
        {
            policy_cache_.emplace(std::make_pair(file_hash, policy));
            policy_.policies.clear();
            for (auto[file_hash, policy] : policy_cache_)
            {
                for (auto[policy_id, sub_policy] : policy.policies)
                {
                    policy_.policies[policy_id] = sub_policy;
                }
            }
            printf("加载配置：%08X\n", file_hash);
        }
    }
    on_policy_reload();
}

void CServerPolicyMgr::add_policy(ProtocolPolicy& policy)
{
    if (policy.policy_type == ENM_POLICY_TYPE_MACHINE)
    {
        std::shared_lock<std::shared_mutex> lck(mtx_);
        if(mac_ban_policy_.find(NetUtils::hash(policy.config.c_str(), policy.config.size())) != mac_ban_policy_.end())
            return;
    }
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
    {
        std::unique_lock<std::shared_mutex> lck(mtx_);
        if (policy_cache_.find(file_hash) != policy_cache_.end())
        {
            policy_cache_.erase(file_hash);
            policy_.policies.clear();
            for (auto[file_hash, policy] : policy_cache_)
            {
                for (auto[policy_id, sub_policy] : policy.policies)
                {
                    policy_.policies[policy_id] = sub_policy;
                }
            }
            printf("卸载配置：%08X\n", file_hash);
        }
    }
    on_policy_reload();
}

void CServerPolicyMgr::reload_all_policy()
{
    namespace fs = std::filesystem;
    static fs::path police_cfg[]{ "config.cfg" ,"jishiyu.cfg" };
    std::error_code ec;
    fs::path dir = g_cur_dir;
    if (fs::is_directory(dir, ec))
    {
        std::map<unsigned int, fs::path> new_policy_list;
        for (auto& file_path : police_cfg)
        {
            /*if (file_path.path().filename() != "config.cfg")
                continue;*/
            auto cfg_file_path = g_cur_dir / file_path;
            std::ifstream file(cfg_file_path, std::ios::in | std::ios::binary);
            if (file.is_open())
            {
                file.seekg(sizeof(RawProtocolHead), file.beg);
                unsigned int file_hash = 0;
                file.read((char*)&file_hash, sizeof(file_hash));
                file.close();
                new_policy_list.emplace(std::make_pair(file_hash, cfg_file_path));
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
    if (path.extension() != ".cfg") return;

    // 1. 先写入临时文件
    std::filesystem::path temp_path = path.parent_path() / (file_name + ".tmp");
    {
        std::ofstream output(temp_path, std::ios::binary);
        if (!output) return;
        output.write(reinterpret_cast<const char*>(data.data()), data.size());
        if (!output) { // 检查写入是否成功
            std::filesystem::remove(temp_path); // 失败则删除临时文件
            return;
        }
    } // 确保文件句柄关闭

    // 2. 原子替换原文件（跨平台安全）
    std::error_code ec;
    std::filesystem::rename(temp_path, path, ec);
    if (ec) {
        std::filesystem::remove(temp_path); // 替换失败则清理临时文件
    }
  //  std::filesystem::path path = g_cur_dir / file_name;
  //  if (path.extension() == ".cfg")
  //  {
  //      // 合并策略文件
		//std::filesystem::path g_cfg_path = g_cur_dir / "config.cfg";

		//std::ifstream file(g_cfg_path, std::ios::in | std::ios::binary);
		//if (!file.is_open())
		//	return;
		//std::stringstream ss;
		//ss << file.rdbuf();
		//file.close();
		//auto str = ss.str();
		//auto policies = ProtocolS2CPolicy::load(str.data(), str.size());
  //      // 加载新策略
		//auto up_policies = ProtocolS2CPolicy::load(reinterpret_cast<char*>(data.data()), data.size());

  //      // 合并
  //      for (auto [policy_id_gm, policy_gm] : policies->policies)// gm本地策略
  //      {
  //          // admin下发的策略
  //          if (up_policies->policies.find(policy_id_gm) != up_policies->policies.end()) {
  //              // 本地策略
  //              if (policy_id_gm < 689000) {
  //                  bool is_same = false;
  //                  // 合并策略
		//			for (auto [policy_id, policy] : up_policies->policies)
		//			{
  //                      // 一样的不处理
  //                      if (policy.policy_type == policy_gm.policy_type && policy.config == policy_gm.config) {
  //                          is_same = true;
  //                          break;
  //                      }
		//			}

  //                  // 不一样的config值,
		//			if(!is_same) {
		//				uint32_t new_policy_id = 688000 + 1;

		//				while (up_policies->policies.find(new_policy_id) != up_policies->policies.end()) {
  //                          new_policy_id++;
		//					if (new_policy_id >= 689000) {
		//						break;
		//					}
		//				}

  //                      policy_gm.policy_id = new_policy_id;
		//				up_policies->policies[new_policy_id] = policy_gm;
		//			}
  //                  
  //              }
  //          }
  //          else {
  //              up_policies->policies[policy_id_gm] = policy_gm;
  //          }
  //      }


  //      auto buf = up_policies->dump();

  //      std::ofstream output(path, std::ios::out | std::ios::binary);
  //      output.write((char*)buf.data(), buf.size());
  //      output.close();
  //  }
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
        multi_client_limit_count_ = atoi(Utils::w2c(multi_client_limit_policy->config).c_str());
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
        mac_ban_policy_.clear();
        mac_white_policy_.clear();
        for (auto[policy_id, policy] : policy_.policies)
        {
            if (policy.policy_type == ENM_POLICY_TYPE_MACHINE)
            {
                if (policy.punish_type == ENM_PUNISH_TYPE_SUPER_WHITE_LIST)
                {
                    mac_white_policy_.emplace(std::make_pair(NetUtils::hash(policy.config.c_str(), policy.config.size()), policy));
                }
                else
                {
                    mac_ban_policy_.emplace(std::make_pair(NetUtils::hash(policy.config.c_str(), policy.config.size()), policy));
                }
            }
        }
    }
}
