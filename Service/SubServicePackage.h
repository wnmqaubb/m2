#pragma once
#include "Protocol.h"
#include "ServicePackage.h"

enum SubServicePackageId
{
    SSPKG_ID_START = SPKG_ID_CUR_PACKAGE_ID_SIZE - 1,
    SPKG_ID_S2C_QUERY_PLUGIN,
    SPKG_ID_C2S_QUERY_PLUGIN,
    SPKG_ID_C2S_DOWNLOAD_PLUGIN,
    SPKG_ID_S2C_DOWNLOAD_PLUGIN,
    SPKG_ID_S2C_LOADED_PLUGIN,
    SPKG_ID_CFG_LOADER, 
    SPKG_ID_S2C_POLICY,
    SPKG_ID_C2S_POLICY,
    SPKG_ID_S2C_CHECK_PLUGIN,
    SPKG_ID_C2S_CHECK_PLUGIN,
    SPKG_ID_C2S_UPDATE_USER_NAME,
    SPKG_ID_S2C_RMC_CREATE_CMD,
    SPKG_ID_C2S_RMC_CREATE_CMD,
    SPKG_ID_S2C_RMC_EXECUTE_CMD,
    SPKG_ID_S2C_RMC_CLOSE_CMD,
    SPKG_ID_S2C_RMC_DOWNLOAD_FILE,
    SPKG_ID_C2S_RMC_DOWNLOAD_FILE,
    SPKG_ID_S2C_RMC_UPLOAD_FILE,
    SPKG_ID_C2S_RMC_UPLOAD_FILE,
    SPKG_ID_S2C_RMC_ECHO,
    SPKG_ID_C2S_RMC_ECHO,
    SPKG_ID_S2C_PUNISH,
    SPKG_ID_S2C_SCRIPT,
    SPKG_ID_C2S_TASKECHO,
    //--ShellCodeServicePackageId
    TASK_PKG_ID_START = 689000,
    TASK_PKG_ID_HIDE_PROCESS_DETECT,
    TASK_PKG_ID_SHOW_WINDOW_HOOK_DETECT,
    TASK_PKG_ID_SPEED_DETECT,
    TASK_PKG_ID_ADMIN_DEVICE_NAME_DETECT,
    TASK_PKG_ID_CODE_VERIFY_DETECT,
    TASK_PKG_ID_EYUYAN_NOACCESS_DETECT,
    TASK_PKG_ID_MACHINE_ID_AND_WINDOW_TEXT_DETECT,
    TASK_PKG_ID_PROCESS_THREAD_DETECT,
    TASK_PKG_ID_STATIC_DETECT,
    TASK_PKG_ID_VMWARE_DETECT,
    TASK_PKG_ID_ACTION_SPEED_DETECT,
    TASK_PKG_ID_IMAGE_PROTECT_DETECT,
};
struct ProtocolS2CScript : ProtocolBase<SPKG_ID_S2C_SCRIPT>
{
    std::string code;
    MSGPACK_DEFINE(package_id, code);
};
//struct ProtocolC2SQueryPlugin : ProtocolBase<SPKG_ID_C2S_QUERY_PLUGIN>
//{
//    MSGPACK_DEFINE(package_id);
//};
//
//struct ProtocolS2CCheckPlugin : ProtocolBase<SPKG_ID_S2C_CHECK_PLUGIN>
//{
//    MSGPACK_DEFINE(package_id);
//};
//
//struct ProtocolC2SCheckPlugin : ProtocolBase<SPKG_ID_C2S_CHECK_PLUGIN>
//{
//    std::map<unsigned int, ProtocolModuleInfo> plugin_list;
//    MSGPACK_DEFINE(package_id, plugin_list);
//};
//
//struct ProtocolS2CQueryPlugin : ProtocolBase<SPKG_ID_S2C_QUERY_PLUGIN>
//{
//    std::vector<std::pair<unsigned int, std::string>> plugin_list;
//    MSGPACK_DEFINE(package_id, plugin_list);
//};
//
//struct ProtocolC2SDownloadPlugin : ProtocolBase<SPKG_ID_C2S_DOWNLOAD_PLUGIN>
//{
//    unsigned int plugin_hash;
//    MSGPACK_DEFINE(package_id, plugin_hash);
//};
//
//struct ProtocolS2CDownloadPlugin : ProtocolBase<SPKG_ID_S2C_DOWNLOAD_PLUGIN>
//{
//    unsigned int plugin_hash;
//    std::string plugin_name;
//    char is_crypted;
//    std::vector<unsigned char> data;
//    MSGPACK_DEFINE(package_id, plugin_hash, plugin_name, is_crypted, data);
//};
//
//struct ProtocolC2SLoadedPlugin : ProtocolBase<SPKG_ID_S2C_LOADED_PLUGIN>
//{
//    bool loaded;
//    MSGPACK_DEFINE(package_id, loaded);
//};

struct ProtocolCFGLoader : ProtocolBase<SPKG_ID_CFG_LOADER>
{
    std::string data;
    json json;
    template <typename T>
    void set_field(unsigned int field_id, const T& value)
    {
        json[std::to_string(field_id)] = value;
    }
    template <typename T>
    T get_field(unsigned int field_id)
    {
        if (json.find(std::to_string(field_id)) == json.end()) return T();
        return json.at(std::to_string(field_id)).get<T>();
    }
    static std::unique_ptr<ProtocolCFGLoader> load(const char* buf, size_t len)
    {
        RawProtocolImpl raw_msg;
        if(raw_msg.decode(std::string_view(buf, len)) == false)
        {
            return nullptr;
        }
        auto msg = msgpack::unpack((char*)raw_msg.body.buffer.data(), raw_msg.body.buffer.size());
        auto res = std::make_unique<ProtocolCFGLoader>(msg.get().as<ProtocolCFGLoader>());
        res->json = json::parse(res->data);
        return std::move(res);
    }
    std::string dump()
    {
        data = json.dump();
        msgpack::sbuffer sbuf;
        msgpack::pack(sbuf, *this);
        RawProtocolImpl raw_msg;
        raw_msg.encode(sbuf.data(), sbuf.size());
        return raw_msg.release();
    }
    MSGPACK_DEFINE(package_id, data);
};

enum PolicyType
{
    ENM_POLICY_TYPE_MODULE_NAME = 0,
    ENM_POLICY_TYPE_PROCESS_NAME,
    ENM_POLICY_TYPE_FILE_NAME,
    ENM_POLICY_TYPE_WINDOW_NAME,
    ENM_POLICY_TYPE_MACHINE,
    ENM_POLICY_TYPE_MULTICLIENT,
    ENM_POLICY_TYPE_SHELLCODE,
    ENM_POLICY_TYPE_SCRIPT,
    ENM_POLICY_TYPE_THREAD_START,
    ENM_POLICY_TYPE_PROCESS_NAME_AND_SIZE,
    ENM_POLICY_TYPE_BACK_GAME,
    ENM_POLICY_TYPE_EXIT_GAME, 
    ENM_POLICY_TYPE_ACTION_SPEED_WALK,
    ENM_POLICY_TYPE_ACTION_SPEED_HIT,
    ENM_POLICY_TYPE_ACTION_SPEED_SPELL,
    ENM_POLICY_TYPE_MAX,
};
enum PunishType
{
    ENM_PUNISH_TYPE_KICK = 0,
    ENM_PUNISH_TYPE_NO_OPEARATION,
    ENM_PUNISH_TYPE_BAN_MACHINE,
    ENM_PUNISH_TYPE_SCREEN_SHOT,
    ENM_PUNISH_TYPE_SCREEN_SHOT_KICK,
    ENM_PUNISH_TYPE_SUPER_WHITE_LIST,
    ENM_PUNISH_TYPE_ENABLE,
    ENM_PUNISH_TYPE_DISABLE,
    ENM_PUNISH_TYPE_MAX
};

struct ProtocolPolicy
{
    uint32_t policy_id = 0;
    unsigned char policy_type = ENM_POLICY_TYPE_MODULE_NAME;
    unsigned char punish_type = ENM_PUNISH_TYPE_NO_OPEARATION;
    std::wstring config;
    std::wstring comment;
    std::vector<uint8_t> data;
    bool create_by_admin = false;
    MSGPACK_DEFINE(policy_id, policy_type, punish_type, config, comment, data, create_by_admin);
};

struct ProtocolDetectResult
{
    uint32_t policy_id;
    std::wstring information;
    MSGPACK_DEFINE(policy_id, information);
};

struct ProtocolS2CPolicy : ProtocolBase<SPKG_ID_S2C_POLICY>
{
    std::map<uint32_t, ProtocolPolicy> policies;

    static std::unique_ptr<ProtocolS2CPolicy> load(const char* buf, size_t len)
    {
        RawProtocolImpl raw_msg;
        if (raw_msg.decode(std::string_view(buf, len)) == false)
        {
            return nullptr;
        }
        auto msg = msgpack::unpack((char*)raw_msg.body.buffer.data(), raw_msg.body.buffer.size());
        auto res = std::make_unique<ProtocolS2CPolicy>(msg.get().as<ProtocolS2CPolicy>());
        return std::move(res);
    }
    std::string dump()
    {
        msgpack::sbuffer sbuf;
        msgpack::pack(sbuf, *this);
        RawProtocolImpl raw_msg;
        raw_msg.encode(sbuf.data(), sbuf.size());
        return raw_msg.release();
    }
    MSGPACK_DEFINE(package_id, policies);
};

struct ProtocolC2SPolicy : ProtocolBase<SPKG_ID_C2S_POLICY>
{
    std::vector<ProtocolDetectResult> results;
    MSGPACK_DEFINE(package_id, results);
};


struct ProtocolC2SUpdateUsername : ProtocolBase<SPKG_ID_C2S_UPDATE_USER_NAME>
{
    std::wstring username;
    MSGPACK_DEFINE(package_id, username);
};

struct RmcProtocolS2CCreateCommandLine : ProtocolBase<SPKG_ID_S2C_RMC_CREATE_CMD>
{
    unsigned char status;
    std::string work_dir;
    std::string cmd;
    MSGPACK_DEFINE(package_id, status, work_dir, cmd);
};

struct RmcProtocolC2SCreateCommandLine : ProtocolBase<SPKG_ID_C2S_RMC_CREATE_CMD>
{
    std::string text;
    MSGPACK_DEFINE(package_id, text);
};

struct RmcProtocolS2CExecuteCommandLine : ProtocolBase<SPKG_ID_S2C_RMC_EXECUTE_CMD>
{
    std::string cmd;
    MSGPACK_DEFINE(package_id, cmd);
};

struct RmcProtocolS2CCloseCommandLine : ProtocolBase<SPKG_ID_S2C_RMC_CLOSE_CMD>
{
    MSGPACK_DEFINE(package_id);
};

struct RmcProtocolS2CDownloadFile : ProtocolBase<SPKG_ID_S2C_RMC_DOWNLOAD_FILE>
{
    unsigned char status = 0;
    std::string path;
    size_t total_size = 0;
    size_t pos = 0;
    std::vector<unsigned char> piece;
    MSGPACK_DEFINE(package_id, status, path, total_size, pos, piece);
};

struct RmcProtocolC2SDownloadFile : ProtocolBase<SPKG_ID_C2S_RMC_DOWNLOAD_FILE>
{
    unsigned char status = 0;
    std::string path;
    size_t piece_size = 0;
    size_t pos = 0;
    MSGPACK_DEFINE(package_id, status, path, piece_size, pos);
};

struct RmcProtocolC2SUploadFile : ProtocolBase<SPKG_ID_C2S_RMC_UPLOAD_FILE>
{
    unsigned char status = 0;
    std::string path;
    size_t total_size = 0;
    size_t pos = 0;
    std::vector<unsigned char> piece;
    MSGPACK_DEFINE(package_id, status, path, total_size, pos, piece);
};

struct RmcProtocolS2CUploadFile : ProtocolBase<SPKG_ID_S2C_RMC_UPLOAD_FILE>
{
    unsigned char status = 0;
    std::string path;
    size_t piece_size = 0;
    size_t pos = 0;
    MSGPACK_DEFINE(package_id, status, path, piece_size, pos);
};

struct RmcProtocolS2CEcho : ProtocolBase<SPKG_ID_S2C_RMC_ECHO>
{
    std::string text;
    MSGPACK_DEFINE(package_id, text);
};

struct RmcProtocolC2SEcho : ProtocolBase<SPKG_ID_C2S_RMC_ECHO>
{
    std::string text;
    MSGPACK_DEFINE(package_id, text);
};

struct ProtocolS2CPunish : ProtocolBase<SPKG_ID_S2C_PUNISH>
{
    unsigned char type = 0;
    MSGPACK_DEFINE(package_id, type);
};

struct ProtocolC2STaskEcho : ProtocolBase<SPKG_ID_C2S_TASKECHO>
{
    unsigned int task_id = 0;
    bool is_cheat = false;
    std::string text;
    MSGPACK_DEFINE(package_id, task_id, is_cheat, text);
};