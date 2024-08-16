#pragma once
//class CServerPluginMgr
//{
//public:
//    RawProtocolImpl get_plugin(unsigned int plugin_hash);
//    bool is_plugin_file_hash_exist(unsigned int file_hash);
//    void add_plugin(unsigned int file_hash, ProtocolS2CDownloadPlugin& plugin);
//    void remove_plugin(unsigned int file_hash);
//    void reload_all_plugin();
//    void create_plugin_file(const std::string& file_name, std::vector<uint8_t>& data);
//    void remove_plugin_file(const std::string& file_name);
//    std::set<unsigned int> get_plugin_file_hash_set();
//    ProtocolS2CQueryPlugin get_plugin_hash_set();
//
//protected:
//    std::shared_mutex mtx_;
//    std::unordered_map<unsigned int, std::pair<RawProtocolImpl, ProtocolS2CDownloadPlugin>> plugin_cache_;
//};


class CServerPolicyMgr
{
public:
    ProtocolS2CPolicy get_policy();
    std::unique_ptr<ProtocolPolicy> find_policy(PolicyType policy_type);
    std::unique_ptr<ProtocolPolicy> find_policy(unsigned int policy_id);
    bool is_policy_file_hash_exist(unsigned int file_hash);
    void add_policy(unsigned int file_hash, ProtocolS2CPolicy& policy);
    void add_policy(ProtocolPolicy& policy);
    void remove_policy(unsigned int file_hash);
    void reload_all_policy();
    void create_policy_file(const std::string& file_name, std::vector<uint8_t>& data);
    void remove_policy_file(const std::string& file_name);
    std::set<unsigned int> get_policy_file_hash_set();
    void on_policy_reload();
    int get_multi_client_limit_count() const { return multi_client_limit_count_; }
    PunishType get_multi_client_limit_punish_type() const { return multi_client_limit_punish_type_; }
    ProtocolPolicy& get_multi_client_policy() { return multi_client_policy_; }
    std::unordered_map<unsigned int, ProtocolPolicy> get_mac_ban_policy() { std::shared_lock<std::shared_mutex> lck(mtx_); return mac_ban_policy_; }
    bool is_ip_or_mac_ban(const std::wstring& val, ProtocolPolicy& policy) 
    { 
        std::shared_lock<std::shared_mutex> lck(mtx_);
        auto hash_val = NetUtils::hash(val.c_str(), val.size());
        if (mac_ban_policy_.find(hash_val) != mac_ban_policy_.end())
        {
            policy = mac_ban_policy_[hash_val];
            return true;
        }
        return false;
    }
    bool is_ip_or_mac_in_super_white_list(const std::wstring& val, ProtocolPolicy& policy)
    {
        std::shared_lock<std::shared_mutex> lck(mtx_);
        auto hash_val = NetUtils::hash(val.c_str(), val.size());
        if (mac_white_policy_.find(hash_val) != mac_white_policy_.end())
        {
            policy = mac_white_policy_[hash_val];
            return true;
        }
        return false;
    }
protected:
    std::shared_mutex mtx_;
    std::unordered_map<unsigned int, ProtocolS2CPolicy> policy_cache_;
    ProtocolS2CPolicy policy_;
    int multi_client_limit_count_ = 999;
    PunishType multi_client_limit_punish_type_ = ENM_PUNISH_TYPE_NO_OPEARATION;
    ProtocolPolicy multi_client_policy_;
    std::unordered_map<unsigned int, ProtocolPolicy> mac_ban_policy_;
    std::unordered_map<unsigned int, ProtocolPolicy> mac_white_policy_;
};