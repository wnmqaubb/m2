<<<<<<< HEAD
#pragma once
#include "Protocol.h"
#include "ServicePackage.h"

enum ObServerPackageId
{
    OBSPKG_ID_START = 2000,
    OBPKG_ID_C2S_AUTH,
    OBPKG_ID_S2C_AUTH,
    SPKG_SERIALIZE_TYPE_USERDATA,
    OBPKG_ID_C2S_QUERY_USERS,
    OBPKG_ID_C2S_DOWNLOAD_PLUGIN,
    OBPKG_ID_S2C_QUERY_USERS,
    OBPKG_ID_S2C_DOWNLOAD_PLUGIN,
    OBPKG_ID_S2C_OPEN_DOCUMENT,
    OBPKG_ID_C2S_SEND,
    OBPKG_ID_S2C_SEND,
    OBPKG_ID_S2C_LOG,
    OBPKG_ID_C2S_UPDATE_LOGIC,
    OBPKG_ID_S2C_SET_FIELD,
    OBPKG_ID_S2C_QUERY_VMP_EXPIRE,
    OBPKG_ID_S2C_PUNISH_USER_UUID,
	OBPKG_ID_C2S_KICK,
    OBSPKG_ID_END = 2999,
};

enum LogicServerPackageId
{
    LSPKG_ID_START = 3000,
    LSPKG_ID_C2S_SEND,
    LSPKG_ID_S2C_SEND,
    LSPKG_ID_S2C_SET_FIELD,
    LSPKG_ID_C2S_ADD_OBS_SESSION,
    LSPKG_ID_S2C_LOG,
    LSPKG_ID_C2S_REMOVE_OBS_SESSION,
    LSPKG_ID_C2S_ADD_USR_SESSION,
    LSPKG_ID_C2S_REMOVE_USR_SESSION,
    LSPKG_ID_C2S_CLOSE,
    LSPKG_ID_C2S_UPLOAD_CFG,
    LSPKG_ID_C2S_REMOVE_CFG,
    LSPKG_ID_C2S_UPLOAD_PLUGIN,
    LSPKG_ID_C2S_REMOVE_PLUGIN,
    LSPKG_ID_C2S_ADD_LIST,
    LSPKG_ID_C2S_CLEAR_LIST,
	LSPKG_ID_S2C_KICK,
    LSPKG_ID_END = 3999,
};
struct ProtocolUserData : ProtocolBase<SPKG_SERIALIZE_TYPE_USERDATA>
{
    union
    {
        unsigned char uuid[16];
        struct
        {
            unsigned int uuid_1;
            unsigned int uuid_2;
            unsigned int uuid_3;
            unsigned int uuid_4;
        };
    };

    inline std::chrono::system_clock::duration get_heartbeat_duration()
    {
        return std::chrono::system_clock::now() - last_heartbeat_time;
    }
    inline asio2::uuid& get_uuid()
    {
        return *(asio2::uuid*)uuid;
    }
    inline bool is_loaded_plugin() { return is_loaded_plugin_; }
    inline void set_loaded_plugin(bool v) { is_loaded_plugin_ = v; }
    inline int get_send_policy_count() { return send_policy_count_; }
    inline void add_send_policy_count() { send_policy_count_++; }
    inline int get_policy_timeout_times() { return policy_timeout_times_; }
    inline void add_policy_timeout_times() { policy_timeout_times_++; }
    inline void clear_policy_timeout_times() { policy_timeout_times_ = 0; }
    std::unordered_map<unsigned int, std::chrono::system_clock::time_point>& pkg_id_time_map() { return pkg_id_time_map_; }
    inline bool has_been_check_pkg() const { return has_been_check_pkg_; }
    inline void has_been_check_pkg(bool val) { has_been_check_pkg_ = val; }
    inline void update_last_punish_time() { last_punish_time = std::chrono::system_clock::now();}
    inline std::chrono::system_clock::duration get_last_punish_duration()
    {
        return std::chrono::system_clock::now() - last_punish_time;
    }
    inline int get_punish_times() { return punish_times_; }
    inline void add_punish_times() { punish_times_++; }
    inline void clear_punish_times() { punish_times_ = 0; }

    unsigned int session_id = 0;
    bool has_handshake = false;
    std::chrono::system_clock::time_point last_heartbeat_time = std::chrono::system_clock::now();
    json json;
    std::wstring mac;
    std::shared_ptr<asio::steady_timer> policy_recv_timeout_timer_;
    bool is_loaded_plugin_ = false;
    int send_policy_count_ = 0;
    bool has_been_check_pkg_ = false;
    int policy_timeout_times_ = 0;
    int punish_times_ = 0;
    std::chrono::system_clock::time_point last_punish_time = std::chrono::system_clock::now();
    std::unordered_map<unsigned int, std::chrono::system_clock::time_point> pkg_id_time_map_;
    MSGPACK_DEFINE(uuid_1, uuid_2, uuid_3, uuid_4, session_id, has_handshake, last_heartbeat_time, json);
};
struct ProtocolOBS2OBCAuth : ProtocolBase<OBPKG_ID_S2C_AUTH>
{
    bool status;
    MSGPACK_DEFINE(package_id, status);
};
struct ProtocolOBC2OBSAuth : ProtocolBase<OBPKG_ID_C2S_AUTH>
{
    std::string key;
    MSGPACK_DEFINE(package_id, key);
};

struct ProtocolOBC2OBSQueryUsers : ProtocolBase<OBPKG_ID_C2S_QUERY_USERS>
{
    MSGPACK_DEFINE(package_id);
};

struct ProtocolOBC2OBSKick : ProtocolBase<OBPKG_ID_C2S_KICK>
{
	unsigned int session_id;
	MSGPACK_DEFINE(package_id, session_id);
};

struct ProtocolLS2LCKick : ProtocolBase<LSPKG_ID_S2C_KICK>
{
	unsigned int session_id;
	MSGPACK_DEFINE(package_id, session_id);
};

struct ProtocolOBS2OBCQueryUsers : ProtocolBase<OBPKG_ID_S2C_QUERY_USERS>
{
    std::unordered_map<unsigned int, ProtocolUserData> data;
    MSGPACK_DEFINE(package_id, data);
};

struct ProtocolOBS2OBCQueryVmpExpire : ProtocolBase<OBPKG_ID_S2C_QUERY_VMP_EXPIRE>
{
    std::wstring vmp_expire;
    MSGPACK_DEFINE(package_id, vmp_expire);
};

struct ProtocolOBS2OBCPunishUserUUID : ProtocolBase<OBPKG_ID_S2C_PUNISH_USER_UUID>
{
    std::wstring uuid;
    bool gm_show = false;
    MSGPACK_DEFINE(package_id, uuid, gm_show);
};

struct ProtocolOBS2OBCOpenDocument : ProtocolBase<OBPKG_ID_S2C_OPEN_DOCUMENT>
{
    std::wstring path;
    MSGPACK_DEFINE(package_id, path);
};

struct ProtocolLC2LSSend : ProtocolBase<LSPKG_ID_C2S_SEND>
{
    RawProtocolImpl package;
    MSGPACK_DEFINE(package_id, package.head.desc, package.head.session_id, package.head.sz, package.body.hash, package.body.buffer);
};

struct ProtocolLS2LCSend : ProtocolBase<LSPKG_ID_S2C_SEND>
{
    RawProtocolImpl package;
    MSGPACK_DEFINE(package_id, package.head.desc, package.head.session_id, package.head.sz, package.body.hash, package.body.buffer);
};
struct ProtocolOBC2OBSSend : ProtocolBase<OBPKG_ID_C2S_SEND>
{
    RawProtocolImpl package;
    MSGPACK_DEFINE(package_id, package.head.desc, package.head.session_id, package.head.sz, package.body.hash, package.body.buffer);
};

struct ProtocolOBS2OBCSend : ProtocolBase<OBPKG_ID_S2C_SEND>
{
    RawProtocolImpl package;
    MSGPACK_DEFINE(package_id, package.head.desc, package.head.session_id, package.head.sz, package.body.hash, package.body.buffer);
};

struct ProtocolLS2LCSetField : ProtocolBase<LSPKG_ID_S2C_SET_FIELD>
{
    unsigned int session_id;
    std::string key;
    std::wstring val;
    MSGPACK_DEFINE(package_id, session_id, key, val);
};

struct ProtocolOBS2OBCSetField : ProtocolBase<OBPKG_ID_S2C_SET_FIELD>
{
    unsigned int session_id;
    std::string key;
    std::wstring val;
    MSGPACK_DEFINE(package_id, session_id, key, val);
};

struct ProtocolLC2LSAddObsSession : ProtocolBase<LSPKG_ID_C2S_ADD_OBS_SESSION>
{
    unsigned int session_id;
    MSGPACK_DEFINE(package_id, session_id);
};

struct ProtocolLSLCLogPrint : ProtocolBase<LSPKG_ID_S2C_LOG>
{
    std::wstring text;
    bool silence = false;
    std::string identify;
    bool gm_show = false;
    MSGPACK_DEFINE(package_id, text, silence, identify, gm_show);
};
struct ProtocolOBS2OBCLogPrint : ProtocolBase<OBPKG_ID_S2C_LOG>
{
    std::wstring text;
    bool silence = false;
    bool gm_show = false;
    std::string identify;
    MSGPACK_DEFINE(package_id, text, silence, gm_show, identify);
};

struct ProtocolLC2LSRemoveObsSession : ProtocolBase<LSPKG_ID_C2S_REMOVE_OBS_SESSION>
{
    unsigned int session_id;
    MSGPACK_DEFINE(package_id, session_id);
};

struct ProtocolLC2LSAddUsrSession : ProtocolBase<LSPKG_ID_C2S_ADD_USR_SESSION>
{
    ProtocolUserData data;
    MSGPACK_DEFINE(package_id, data);
};

struct ProtocolLC2LSRemoveUsrSession : ProtocolBase<LSPKG_ID_C2S_REMOVE_USR_SESSION>
{
    ProtocolUserData data;
    MSGPACK_DEFINE(package_id, data);
};

struct ProtocolLC2LSClose : ProtocolBase<LSPKG_ID_C2S_CLOSE>
{
    MSGPACK_DEFINE(package_id);
};

struct ProtocolOBC2OBSUpdateLogic : ProtocolBase<OBPKG_ID_C2S_UPDATE_LOGIC>
{
    std::vector<uint8_t> data;
    MSGPACK_DEFINE(package_id, data);
};
struct ProtocolOBC2LSUploadConfig : ProtocolBase<LSPKG_ID_C2S_UPLOAD_CFG>
{
    std::string file_name;
    std::vector<uint8_t> data;
    MSGPACK_DEFINE(package_id, file_name, data);
};
struct ProtocolOBC2LSRemoveConfig : ProtocolBase<LSPKG_ID_C2S_REMOVE_CFG>
{
    std::string file_name;
    MSGPACK_DEFINE(package_id, file_name);
};
struct ProtocolOBC2LSUploadPlugin : ProtocolBase<LSPKG_ID_C2S_UPLOAD_PLUGIN>
{
    std::string file_name;
    std::vector<uint8_t> data;
    MSGPACK_DEFINE(package_id, file_name, data);
};
struct ProtocolOBC2LSRemovePlugin : ProtocolBase<LSPKG_ID_C2S_REMOVE_PLUGIN>
{
    std::string file_name;
    MSGPACK_DEFINE(package_id, file_name);
};
struct ProtocolOBC2LSAddList : ProtocolBase<LSPKG_ID_C2S_ADD_LIST>
{
    std::string file_name;
    std::string text;
    MSGPACK_DEFINE(package_id, file_name, text);
};
struct ProtocolOBC2LSClearList : ProtocolBase<LSPKG_ID_C2S_CLEAR_LIST>
{
    std::string file_name;
    MSGPACK_DEFINE(package_id, file_name);
};

namespace msgpack {
    MSGPACK_API_VERSION_NAMESPACE(v1) {
        namespace detail {

            template <>
            struct packer_serializer<msgpack::v1::sbuffer, json> {
                static msgpack::packer<msgpack::v1::sbuffer>& pack(msgpack::packer<msgpack::v1::sbuffer>& o, const json& v) {
                    o << json::to_msgpack(v);
                    return o;
                }
            };
        } 
        namespace adaptor {

            template <>
            inline
                msgpack::object const&
                adaptor::convert<json, void>::operator()(msgpack::object const& o, json& v) const {
                v = json::from_msgpack(o.as<std::vector<uint8_t>>());
                return o;
            }
        } // namespace adaptor
    } // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
=======
#pragma once
#include "Protocol.h"
#include "ServicePackage.h"

enum ObServerPackageId
{
    OBSPKG_ID_START = 2000,
    OBPKG_ID_C2S_AUTH,
    OBPKG_ID_S2C_AUTH,
    SPKG_SERIALIZE_TYPE_USERDATA,
    OBPKG_ID_C2S_QUERY_USERS,
    OBPKG_ID_C2S_DOWNLOAD_PLUGIN,
    OBPKG_ID_S2C_QUERY_USERS,
    OBPKG_ID_S2C_DOWNLOAD_PLUGIN,
    OBPKG_ID_S2C_OPEN_DOCUMENT,
    OBPKG_ID_C2S_SEND,
    OBPKG_ID_S2C_SEND,
    OBPKG_ID_S2C_LOG,
    OBPKG_ID_C2S_UPDATE_LOGIC,
    OBPKG_ID_S2C_SET_FIELD,
    OBPKG_ID_S2C_QUERY_VMP_EXPIRE,
    OBPKG_ID_S2C_PUNISH_USER_UUID,
	OBPKG_ID_C2S_KICK,
    OBSPKG_ID_END = 2999,
};

enum LogicServerPackageId
{
    LSPKG_ID_START = 3000,
    LSPKG_ID_C2S_SEND,
    LSPKG_ID_S2C_SEND,
    LSPKG_ID_S2C_SET_FIELD,
    LSPKG_ID_C2S_ADD_OBS_SESSION,
    LSPKG_ID_S2C_LOG,
    LSPKG_ID_C2S_REMOVE_OBS_SESSION,
    LSPKG_ID_C2S_ADD_USR_SESSION,
    LSPKG_ID_C2S_REMOVE_USR_SESSION,
    LSPKG_ID_C2S_CLOSE,
    LSPKG_ID_C2S_UPLOAD_CFG,
    LSPKG_ID_C2S_REMOVE_CFG,
    LSPKG_ID_C2S_UPLOAD_PLUGIN,
    LSPKG_ID_C2S_REMOVE_PLUGIN,
    LSPKG_ID_C2S_ADD_LIST,
    LSPKG_ID_C2S_CLEAR_LIST,
	LSPKG_ID_S2C_KICK,
    LSPKG_ID_END = 3999,
};
struct ProtocolUserData : ProtocolBase<SPKG_SERIALIZE_TYPE_USERDATA>
{
    union
    {
        unsigned char uuid[16];
        struct
        {
            unsigned int uuid_1;
            unsigned int uuid_2;
            unsigned int uuid_3;
            unsigned int uuid_4;
        };
    };

    inline std::chrono::system_clock::duration get_heartbeat_duration()
    {
        return std::chrono::system_clock::now() - last_heartbeat_time;
    }
    inline asio2::uuid& get_uuid()
    {
        return *(asio2::uuid*)uuid;
    }
    inline bool is_loaded_plugin() { return is_loaded_plugin_; }
    inline void set_loaded_plugin(bool v) { is_loaded_plugin_ = v; }
    inline int get_send_policy_count() { return send_policy_count_; }
    inline void add_send_policy_count() { send_policy_count_++; }
    inline int get_policy_timeout_times() { return policy_timeout_times_; }
    inline void add_policy_timeout_times() { policy_timeout_times_++; }
    inline void clear_policy_timeout_times() { policy_timeout_times_ = 0; }
    std::unordered_map<unsigned int, std::chrono::system_clock::time_point>& pkg_id_time_map() { return pkg_id_time_map_; }
    inline bool has_been_check_pkg() const { return has_been_check_pkg_; }
    inline void has_been_check_pkg(bool val) { has_been_check_pkg_ = val; }
    inline void update_last_punish_time() { last_punish_time = std::chrono::system_clock::now();}
    inline std::chrono::system_clock::duration get_last_punish_duration()
    {
        return std::chrono::system_clock::now() - last_punish_time;
    }
    inline int get_punish_times() { return punish_times_; }
    inline void add_punish_times() { punish_times_++; }
    inline void clear_punish_times() { punish_times_ = 0; }

    unsigned int session_id = 0;
    bool has_handshake = false;
    std::chrono::system_clock::time_point last_heartbeat_time = std::chrono::system_clock::now();
    json json;
    std::wstring mac;
    std::shared_ptr<asio::steady_timer> policy_recv_timeout_timer_;
    bool is_loaded_plugin_ = false;
    int send_policy_count_ = 0;
    bool has_been_check_pkg_ = false;
    int policy_timeout_times_ = 0;
    int punish_times_ = 0;
    std::chrono::system_clock::time_point last_punish_time = std::chrono::system_clock::now();
    std::unordered_map<unsigned int, std::chrono::system_clock::time_point> pkg_id_time_map_;
    MSGPACK_DEFINE(uuid_1, uuid_2, uuid_3, uuid_4, session_id, has_handshake, last_heartbeat_time, json);
};
struct ProtocolOBS2OBCAuth : ProtocolBase<OBPKG_ID_S2C_AUTH>
{
    bool status;
    MSGPACK_DEFINE(package_id, status);
};
struct ProtocolOBC2OBSAuth : ProtocolBase<OBPKG_ID_C2S_AUTH>
{
    std::string key;
    MSGPACK_DEFINE(package_id, key);
};

struct ProtocolOBC2OBSQueryUsers : ProtocolBase<OBPKG_ID_C2S_QUERY_USERS>
{
    MSGPACK_DEFINE(package_id);
};

struct ProtocolOBC2OBSKick : ProtocolBase<OBPKG_ID_C2S_KICK>
{
	unsigned int session_id;
	MSGPACK_DEFINE(package_id, session_id);
};

struct ProtocolLS2LCKick : ProtocolBase<LSPKG_ID_S2C_KICK>
{
	unsigned int session_id;
	MSGPACK_DEFINE(package_id, session_id);
};

struct ProtocolOBS2OBCQueryUsers : ProtocolBase<OBPKG_ID_S2C_QUERY_USERS>
{
    std::unordered_map<unsigned int, ProtocolUserData> data;
    MSGPACK_DEFINE(package_id, data);
};

struct ProtocolOBS2OBCQueryVmpExpire : ProtocolBase<OBPKG_ID_S2C_QUERY_VMP_EXPIRE>
{
    std::wstring vmp_expire;
    MSGPACK_DEFINE(package_id, vmp_expire);
};

struct ProtocolOBS2OBCPunishUserUUID : ProtocolBase<OBPKG_ID_S2C_PUNISH_USER_UUID>
{
    std::wstring uuid;
    bool gm_show = false;
    MSGPACK_DEFINE(package_id, uuid, gm_show);
};

struct ProtocolOBS2OBCOpenDocument : ProtocolBase<OBPKG_ID_S2C_OPEN_DOCUMENT>
{
    std::wstring path;
    MSGPACK_DEFINE(package_id, path);
};

struct ProtocolLC2LSSend : ProtocolBase<LSPKG_ID_C2S_SEND>
{
    RawProtocolImpl package;
    MSGPACK_DEFINE(package_id, package.head.desc, package.head.session_id, package.head.sz, package.body.hash, package.body.buffer);
};

struct ProtocolLS2LCSend : ProtocolBase<LSPKG_ID_S2C_SEND>
{
    RawProtocolImpl package;
    MSGPACK_DEFINE(package_id, package.head.desc, package.head.session_id, package.head.sz, package.body.hash, package.body.buffer);
};
struct ProtocolOBC2OBSSend : ProtocolBase<OBPKG_ID_C2S_SEND>
{
    RawProtocolImpl package;
    MSGPACK_DEFINE(package_id, package.head.desc, package.head.session_id, package.head.sz, package.body.hash, package.body.buffer);
};

struct ProtocolOBS2OBCSend : ProtocolBase<OBPKG_ID_S2C_SEND>
{
    RawProtocolImpl package;
    MSGPACK_DEFINE(package_id, package.head.desc, package.head.session_id, package.head.sz, package.body.hash, package.body.buffer);
};

struct ProtocolLS2LCSetField : ProtocolBase<LSPKG_ID_S2C_SET_FIELD>
{
    unsigned int session_id;
    std::string key;
    std::wstring val;
    MSGPACK_DEFINE(package_id, session_id, key, val);
};

struct ProtocolOBS2OBCSetField : ProtocolBase<OBPKG_ID_S2C_SET_FIELD>
{
    unsigned int session_id;
    std::string key;
    std::wstring val;
    MSGPACK_DEFINE(package_id, session_id, key, val);
};

struct ProtocolLC2LSAddObsSession : ProtocolBase<LSPKG_ID_C2S_ADD_OBS_SESSION>
{
    unsigned int session_id;
    MSGPACK_DEFINE(package_id, session_id);
};

struct ProtocolLSLCLogPrint : ProtocolBase<LSPKG_ID_S2C_LOG>
{
    std::wstring text;
    bool silence = false;
    std::string identify;
    bool gm_show = false;
    MSGPACK_DEFINE(package_id, text, silence, identify, gm_show);
};
struct ProtocolOBS2OBCLogPrint : ProtocolBase<OBPKG_ID_S2C_LOG>
{
    std::wstring text;
    bool silence = false;
    bool gm_show = false;
    std::string identify;
    MSGPACK_DEFINE(package_id, text, silence, gm_show, identify);
};

struct ProtocolLC2LSRemoveObsSession : ProtocolBase<LSPKG_ID_C2S_REMOVE_OBS_SESSION>
{
    unsigned int session_id;
    MSGPACK_DEFINE(package_id, session_id);
};

struct ProtocolLC2LSAddUsrSession : ProtocolBase<LSPKG_ID_C2S_ADD_USR_SESSION>
{
    ProtocolUserData data;
    MSGPACK_DEFINE(package_id, data);
};

struct ProtocolLC2LSRemoveUsrSession : ProtocolBase<LSPKG_ID_C2S_REMOVE_USR_SESSION>
{
    ProtocolUserData data;
    MSGPACK_DEFINE(package_id, data);
};

struct ProtocolLC2LSClose : ProtocolBase<LSPKG_ID_C2S_CLOSE>
{
    MSGPACK_DEFINE(package_id);
};

struct ProtocolOBC2OBSUpdateLogic : ProtocolBase<OBPKG_ID_C2S_UPDATE_LOGIC>
{
    std::vector<uint8_t> data;
    MSGPACK_DEFINE(package_id, data);
};
struct ProtocolOBC2LSUploadConfig : ProtocolBase<LSPKG_ID_C2S_UPLOAD_CFG>
{
    std::string file_name;
    std::vector<uint8_t> data;
    MSGPACK_DEFINE(package_id, file_name, data);
};
struct ProtocolOBC2LSRemoveConfig : ProtocolBase<LSPKG_ID_C2S_REMOVE_CFG>
{
    std::string file_name;
    MSGPACK_DEFINE(package_id, file_name);
};
struct ProtocolOBC2LSUploadPlugin : ProtocolBase<LSPKG_ID_C2S_UPLOAD_PLUGIN>
{
    std::string file_name;
    std::vector<uint8_t> data;
    MSGPACK_DEFINE(package_id, file_name, data);
};
struct ProtocolOBC2LSRemovePlugin : ProtocolBase<LSPKG_ID_C2S_REMOVE_PLUGIN>
{
    std::string file_name;
    MSGPACK_DEFINE(package_id, file_name);
};
struct ProtocolOBC2LSAddList : ProtocolBase<LSPKG_ID_C2S_ADD_LIST>
{
    std::string file_name;
    std::string text;
    MSGPACK_DEFINE(package_id, file_name, text);
};
struct ProtocolOBC2LSClearList : ProtocolBase<LSPKG_ID_C2S_CLEAR_LIST>
{
    std::string file_name;
    MSGPACK_DEFINE(package_id, file_name);
};

namespace msgpack {
    MSGPACK_API_VERSION_NAMESPACE(v1) {
        namespace detail {

            template <>
            struct packer_serializer<msgpack::v1::sbuffer, json> {
                static msgpack::packer<msgpack::v1::sbuffer>& pack(msgpack::packer<msgpack::v1::sbuffer>& o, const json& v) {
                    o << json::to_msgpack(v);
                    return o;
                }
            };
        } 
        namespace adaptor {

            template <>
            inline
                msgpack::object const&
                adaptor::convert<json, void>::operator()(msgpack::object const& o, json& v) const {
                v = json::from_msgpack(o.as<std::vector<uint8_t>>());
                return o;
            }
        } // namespace adaptor
    } // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9
} // namespace msgpack