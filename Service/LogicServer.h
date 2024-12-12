#pragma once
#include "AntiCheatClient.h"
#include "AntiCheatServer.h"
#include "ObserverClient.h"
#include "ObServerPackage.h"
#include "ObserverServer.h"
#include "ServerPluginMgr.h"

#define CONFIG_APP_NAME "及时雨"
extern std::filesystem::path g_cur_dir;

class CObsSessionMgr
{
    using sessions_container_t = std::set<unsigned int>;
public:
    inline sessions_container_t sessions() { std::shared_lock<std::shared_mutex> lck(mtx_); return obs_sessions_; }
    inline bool exist(unsigned int session_id) { std::shared_lock<std::shared_mutex> lck(mtx_);  return obs_sessions_.find(session_id) != obs_sessions_.end(); }
    inline void add_session(unsigned int session_id) { std::unique_lock<std::shared_mutex> lck(mtx_); obs_sessions_.emplace(session_id); }
    inline void remove_session(unsigned int session_id) { std::unique_lock<std::shared_mutex> lck(mtx_); if (obs_sessions_.find(session_id) != obs_sessions_.end()) obs_sessions_.erase(session_id); }
private:
    std::shared_mutex mtx_;
    sessions_container_t obs_sessions_;
};

class CSessionMgr
{
    using sessions_container_t = std::unordered_map<unsigned int, std::shared_ptr<ProtocolUserData>>;
    using sessions_mac_container_t = std::unordered_map<unsigned int, sessions_container_t>;
public:
    std::shared_ptr<ProtocolUserData> get_user_data(unsigned int session_id)
    {
        std::shared_lock<std::shared_mutex> lck(mtx_);
        if (sessions_.find(session_id) == sessions_.end())
            return nullptr;
        return sessions_[session_id];
    }
    void foreach_session(std::function<void(std::shared_ptr<ProtocolUserData>&)> cb)
    {
        std::shared_lock<std::shared_mutex> lck(mtx_);
        for(auto [session_id , user_data] : sessions_)
        {
            cb(user_data);
        }
    }
    int get_machine_count(const std::wstring& mac)
    {
        std::shared_lock<std::shared_mutex> lck(mtx_);
        auto mac_hash = NetUtils::hash(mac.c_str(), mac.size());
        return sessions_mac_[mac_hash].size();
    }
    void add_session(unsigned int session_id, const ProtocolUserData& user_data)
    {
        std::unique_lock<std::shared_mutex> lck(mtx_); 
        auto shared_user_data = std::make_shared<ProtocolUserData>(user_data);
        shared_user_data->mac = shared_user_data->json.at("cpuid").get<std::wstring>()
            + L"|" + shared_user_data->json.at("mac").get<std::wstring>()
            + L"|" + shared_user_data->json.at("vol").get<std::wstring>();
        sessions_.emplace(std::make_pair(session_id, shared_user_data));
        auto mac_hash = NetUtils::hash(shared_user_data->mac.c_str(), shared_user_data->mac.size());
        sessions_mac_[mac_hash][session_id] = shared_user_data;
    }
    void remove_session(unsigned int session_id) 
    { 
        std::unique_lock<std::shared_mutex> lck(mtx_); 
        if (sessions_.find(session_id) != sessions_.end()) 
        {
            auto user_data = sessions_[session_id];
            auto mac_hash = NetUtils::hash(user_data->mac.c_str(), user_data->mac.size());
            if (sessions_mac_[mac_hash].find(session_id) != sessions_mac_[mac_hash].end())
            {
                sessions_mac_[mac_hash].erase(session_id);
                if (sessions_mac_[mac_hash].empty())
                {
                    sessions_mac_.erase(mac_hash);
                }
            }
            sessions_.erase(session_id);
        }
    }
private:
    std::shared_mutex mtx_;
    sessions_container_t sessions_;
    sessions_mac_container_t sessions_mac_;
};
class CLogicServer : public CAntiCheatServer
{
    using super = CAntiCheatServer;
public:
    void send_policy(std::shared_ptr<ProtocolUserData>& user_data, tcp_session_shared_ptr_t& session, unsigned int session_id);
    CLogicServer();
	void clear_txt(const std::string& file_name);
	void write_txt(const std::string& file_name, const std::string& str, bool is_from_add_list = false);
	virtual void send(tcp_session_shared_ptr_t& session, unsigned int session_id, const RawProtocolImpl& package)
    {
        ProtocolLS2LCSend resp;
        resp.package = package;
        resp.package.head.session_id = session_id;
        super::send(session, &resp);
    }

    virtual void send(tcp_session_shared_ptr_t& session, unsigned int session_id, msgpack::sbuffer& buffer)
    {
        RawProtocolImpl package;
        package.encode(buffer.data(), buffer.size());
        send(session, session_id, package);
    }

    template <typename T>
    void send(tcp_session_shared_ptr_t& session, unsigned int session_id, T* package)
    {
        if (!package)
            __debugbreak();
        msgpack::sbuffer buffer;
        msgpack::pack(buffer, *package);
        send(session, session_id, buffer);
    }
    virtual void set_field(tcp_session_shared_ptr_t& session, unsigned int session_id, const std::string& key, const std::wstring& val)
    {
        ProtocolLS2LCSetField req;
        req.session_id = session_id;
        req.key = key;
        req.val = val;
        super::send(session, &req);
    }
    virtual void write_img(unsigned int session_id, std::vector<uint8_t>& data);
    /**
     * msg: 日志信息
     * silence: 是否显示到界面日志窗口
     * gm_show: 是否显示到gm
     * identify: 玩家uuid标识符
     * punish_flag: 是否是惩罚log
     */
    virtual void log_cb(const wchar_t* msg, bool silence, bool gm_show, const std::string& identify, bool punish_flag);
    virtual void punish(tcp_session_shared_ptr_t& session, unsigned int session_id, ProtocolPolicy& policy, const std::wstring& comment, const std::wstring& comment_2 = L"");
    virtual bool is_svip(unsigned int session_id);
    virtual void detect(tcp_session_shared_ptr_t& session, unsigned int session_id);
	virtual void close_socket(tcp_session_shared_ptr_t& session, unsigned int session_id);
	std::string trim_user_name(const std::string& username_);
	inline CObsSessionMgr& obs_sessions_mgr() { return obs_sessions_mgr_; }
    inline CSessionMgr& usr_sessions_mgr() { return usr_sessions_mgr_; }
	inline int get_policy_detect_interval() { return policy_detect_interval_; }
	inline void set_policy_detect_interval(int interval) { policy_detect_interval_ = interval; }
private:
    void OnlineCheck();
protected:
    using observer_package_type = std::function<void(tcp_session_shared_ptr_t& session, unsigned int ob_session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle&)>;
    CServerPluginMgr plugin_mgr_;
    CServerPolicyMgr policy_mgr_;
    NetUtils::EventMgr<observer_package_type> ob_pkg_mgr_;
    NetUtils::EventMgr<observer_package_type> policy_pkg_mgr_;
    CObsSessionMgr obs_sessions_mgr_;
    CSessionMgr usr_sessions_mgr_;
    int policy_detect_interval_;
};