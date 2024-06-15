<<<<<<< HEAD
#pragma once
#include <filesystem>
#include <fstream>
#include "Service/AntiCheatClient.h"

namespace fs = std::filesystem;
using plugin_hash_t = unsigned int;

class CClientPluginMgr
{
public:
    CClientPluginMgr(fs::path cache_dir_);
    virtual void set_client_instance(CAntiCheatClient* instance);
    virtual bool load_plugin(plugin_hash_t plugin_hash, const std::string& filename);
    virtual std::string get_plugin_cache_str(plugin_hash_t plugin_hash);
    virtual bool is_plugin_cache_exist(plugin_hash_t plugin_hash);
    virtual bool is_plugin_loaded(plugin_hash_t plugin_hash);
    virtual void save_plugin(const RawProtocolImpl& package, const ProtocolS2CDownloadPlugin& msg);
    const std::map<plugin_hash_t, ProtocolModuleInfo>& get_plugin_list() { return plugin_list_; };
protected:
    fs::path cache_dir_;
    CAntiCheatClient* instance_;
    std::shared_mutex mtx_;
    std::map<plugin_hash_t, ProtocolModuleInfo> plugin_list_;
=======
#pragma once
#include <filesystem>
#include <fstream>
#include "Service/AntiCheatClient.h"

namespace fs = std::filesystem;
using plugin_hash_t = unsigned int;

class CClientPluginMgr
{
public:
    CClientPluginMgr(fs::path cache_dir_);
    virtual void set_client_instance(CAntiCheatClient* instance);
    virtual bool load_plugin(plugin_hash_t plugin_hash, const std::string& filename);
    virtual std::string get_plugin_cache_str(plugin_hash_t plugin_hash);
    virtual bool is_plugin_cache_exist(plugin_hash_t plugin_hash);
    virtual bool is_plugin_loaded(plugin_hash_t plugin_hash);
    virtual void save_plugin(const RawProtocolImpl& package, const ProtocolS2CDownloadPlugin& msg);
    const std::map<plugin_hash_t, ProtocolModuleInfo>& get_plugin_list() { return plugin_list_; };
protected:
    fs::path cache_dir_;
    CAntiCheatClient* instance_;
    std::shared_mutex mtx_;
    std::map<plugin_hash_t, ProtocolModuleInfo> plugin_list_;
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9
};