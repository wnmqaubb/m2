#pragma once
#include <stdint.h>
#include <sstream>
#include <json/json.hpp>
#include "global_string.h"

using json = nlohmann::json;
using BufferPtr = std::unique_ptr<unsigned char[]>;

namespace Utils
{
    std::wstring string2wstring(std::string str, unsigned int CodePage);
    std::string wstring2string(std::wstring wstr, unsigned int CodePage);
}

class Protocol
{
public:
    typedef enum _PackageType
    {
        PACKAGE_TYPE_GAME = 0,
        PACKAGE_TYPE_ANTICHEAT,
        PACKAGE_TYPE_ADMIN,
        PACKAGE_TYPE_ALL
    }PackageType;
    typedef enum _PackageId
    {
        PACKAGE_ID_HEARTBEAT = 0,
        PACKAGE_ID_STATICDECTECTION,
        PACKAGE_ID_SCREENSHOT,
        PACKAGE_ID_PROCESSINFO,
        PACKAGE_ID_HARDWARE,
        PACKAGE_ID_SHELLCODE,
        PACKAGE_ID_JUDGMENT,
        PACKAGE_ID_EXAMPLE,
        PACKAGE_ID_POLICY,
        PACKAGE_ID_API_HOOK,
        PACKAGE_ID_SHELLCODE_START = 9000,
		PACKAGE_ID_RMC = 9900,
        PACKAGE_ID_UNDEFINED = 9999
    }PackageId;

    Protocol();
    Protocol(const json& target);
    Protocol(const Protocol &proto) = default;

    Protocol(Protocol &&proto) : m_raw_buffer(std::move(proto.m_raw_buffer))
    {
    }

    Protocol& operator=(Protocol &&proto)
    {
        m_raw_buffer = std::move(proto.m_raw_buffer);
        return *this;
    }

    ~Protocol();

    constexpr unsigned int aphash(const unsigned char *str, uint32_t len)
    {
        unsigned int hash = 0;
        for (uint32_t i = 0; i < len; i++)
        {
            if ((i & 1) == 0)
            {
                hash ^= ((hash << 7) ^ (str[i]) ^ (hash >> 3));
            }
            else
            {
                hash ^= (~((hash << 11) ^ (str[i]) ^ (hash >> 5)));
            }
        }
        return (hash & 0x7FFFFFFF);
    }
    void copy(void* buffer, uint32_t len);
    void copy(json target);
    void get_json(json& dest);
    virtual void to_json(json& j) {};
    virtual void from_json(const json& j) {};
    bool unpak(const unsigned char* buffer, uint32_t len);
    BufferPtr pak(uint32_t* len);
    unsigned char* get_raw_buffer() const
    {
        return m_raw_buffer.get();
    }
    uint32_t get_raw_size() const
    {
        return m_raw_size;
    }

    uint32_t id;
    uint32_t m_checksum;
    PackageType m_type;
    uint32_t m_size;
    unsigned char* m_buffer;
private:
    BufferPtr m_raw_buffer;
    uint32_t m_raw_size;
};

#define SHELLCODE_PACKAGE_ID(id) (Protocol::PackageId)(Protocol::PackageId::PACKAGE_ID_SHELLCODE_START + id)

class ProtocolHeartBeat : public Protocol
{
public:
    ProtocolHeartBeat()
    {
        id = PackageId::PACKAGE_ID_HEARTBEAT;
    }
    virtual void to_json(json& j)
    {
        j = json{
            {"id", id},
            {"gamer_username", gamer_username},
            {"cpuid", cpuid},
            {"volume_serial_number", volume_serial_number},
            {"mac_address", mac_address},
            {"pack_ip", pack_ip},
            {"timestamp", timestamp}
        };
    }
    virtual void from_json(const json& j) 
    {
        id = j.at("id").get<uint32_t>();
        gamer_username = j.at("gamer_username").get<std::wstring>();
        cpuid = j.at("cpuid").get<std::wstring>();
        volume_serial_number = j.at("volume_serial_number").get<std::wstring>();
        mac_address = j.at("mac_address").get<std::wstring>();
        pack_ip = j.at("pack_ip").get<std::wstring>();
        timestamp = j.at("timestamp").get<uint32_t>();
    }
    std::string dump()
    {
        std::wstringstream output;
        output << id << "|"
            << gamer_username << "|"
            << cpuid << "|"
            << volume_serial_number << "|"
            << mac_address << "|"
            << pack_ip << "|"
            << timestamp;
        return Utils::wstring2string(output.str(), 0);
    }

    std::wstring gamer_username;
    std::wstring cpuid;
    std::wstring volume_serial_number;
    std::wstring mac_address;
    std::wstring pack_ip;
    uint32_t timestamp;
};


class ProtocolHardWare : public Protocol
{
public:
    ProtocolHardWare()
    {
        id = PackageId::PACKAGE_ID_HARDWARE;
    }
    virtual void to_json(json& j)
    {
        j = json{
            {GlobalString::JsonProperty::ID, id},
            { GlobalString::JsonProperty::CPUID, cpuid },
            { GlobalString::JsonProperty::VOLUME_SERIAL_NUMBER, volume_serial_number },
            { GlobalString::JsonProperty::MAC_ADDRESS, mac_address },
        };
    }
    virtual void from_json(const json& j)
    {
        id = j.at(GlobalString::JsonProperty::ID).get<uint32_t>();
        cpuid = j.at(GlobalString::JsonProperty::CPUID).get<std::wstring>();
        volume_serial_number = j.at(GlobalString::JsonProperty::VOLUME_SERIAL_NUMBER).get<std::wstring>();
        mac_address = j.at(GlobalString::JsonProperty::MAC_ADDRESS).get<std::wstring>();
    }
    std::string dump()
    {
        std::wstringstream output;
        output << id << GlobalString::JsonProperty::WSEPARATOR.c_str()
            << cpuid << GlobalString::JsonProperty::WSEPARATOR.c_str()
            << volume_serial_number << GlobalString::JsonProperty::WSEPARATOR.c_str()
            << mac_address << GlobalString::JsonProperty::WSEPARATOR.c_str();
        return Utils::wstring2string(output.str(), 0);
    }

    std::wstring cpuid;
    std::wstring volume_serial_number;
    std::wstring mac_address;
};


class ProtocolJudgMent : public Protocol
{
public:
    ProtocolJudgMent()
    {
        id = PackageId::PACKAGE_ID_JUDGMENT;
    }
    virtual void to_json(json& j)
    {
        j = json{
            { GlobalString::JsonProperty::ID, id },
            { GlobalString::JsonProperty::SUBID, subid },
            { GlobalString::JsonProperty::DATA, data},
            { GlobalString::JsonProperty::LAZY_TIME, lazy_time},
        };
    }
    virtual void from_json(const json& j)
    {
        id = j.at(GlobalString::JsonProperty::ID).get<uint32_t>();
        subid = j.at(GlobalString::JsonProperty::SUBID).get<std::string>();
        data = j.at(GlobalString::JsonProperty::DATA).get<std::wstring>();
        lazy_time = j.at(GlobalString::JsonProperty::LAZY_TIME).get<uint32_t>();
    }
    std::string dump()
    {
        std::stringstream output;
        output << id << GlobalString::JsonProperty::SEPARATOR.c_str()
            << subid << GlobalString::JsonProperty::SEPARATOR.c_str()
            << Utils::wstring2string(data, 0)
            << GlobalString::JsonProperty::SEPARATOR.c_str() << lazy_time;
        return output.str();
    }

    std::string subid;
    std::wstring data;
    uint32_t lazy_time;
};


class ProtocolScreenShot : public Protocol
{
public:
    ProtocolScreenShot()
    {
        id = PackageId::PACKAGE_ID_SCREENSHOT;
    }
    virtual void to_json(json& j)
    {
        j = json{
            { GlobalString::JsonProperty::ID, id },
            { GlobalString::JsonProperty::STATUS, status },
            { GlobalString::JsonProperty::SILENT, silent },
            { GlobalString::JsonProperty::UNCOMPRESS_SIZE, uncompress_size },
            { GlobalString::JsonProperty::DATA, data },
        };
    }
    virtual void from_json(const json& j)
    {
        id = j.at(GlobalString::JsonProperty::ID).get<uint32_t>();
        status = j.at(GlobalString::JsonProperty::STATUS).get<int>();
        silent = j.at(GlobalString::JsonProperty::SILENT).get<bool>();
        uncompress_size = j.at(GlobalString::JsonProperty::UNCOMPRESS_SIZE).get<size_t>();
        data = j.at(GlobalString::JsonProperty::DATA).get<std::vector<uint8_t>>();
    }
    std::string dump()
    {
        std::stringstream output;
        output << id << GlobalString::JsonProperty::SEPARATOR.c_str()
            << status << GlobalString::JsonProperty::SEPARATOR.c_str()
            << uncompress_size;
        return output.str();
    }

    int status;
    bool silent;
    size_t uncompress_size;
    std::vector<uint8_t> data;
};

class ProtocolShellCode : public Protocol
{
public:
    ProtocolShellCode()
    {
        id = PackageId::PACKAGE_ID_SHELLCODE;
    }
    virtual void to_json(json& j)
    {
        j = json{
            { GlobalString::JsonProperty::ID, id},
            { GlobalString::JsonProperty::STATUS, status },
            { GlobalString::JsonProperty::UNCOMPRESS_SIZE, uncompress_size },
            { GlobalString::JsonProperty::DATA, data },
        };
    }
    virtual void from_json(const json& j)
    {
        id = j.at(GlobalString::JsonProperty::ID).get<uint32_t>();
        status = j.at(GlobalString::JsonProperty::STATUS).get<int>();
        uncompress_size = j.at(GlobalString::JsonProperty::UNCOMPRESS_SIZE).get<size_t>();
        data = j.at(GlobalString::JsonProperty::DATA).get<std::vector<uint8_t>>();
    }
    std::string dump()
    {
        std::stringstream output;
        output << id << GlobalString::JsonProperty::SEPARATOR.c_str()
            << status << GlobalString::JsonProperty::SEPARATOR.c_str()
            << uncompress_size;
        return output.str();
    }

    int status;
    size_t uncompress_size;
    std::vector<uint8_t> data;
};

class ProtocolShellCodeInstance : public Protocol
{
public:
    ProtocolShellCodeInstance()
    {
        id = PackageId::PACKAGE_ID_SHELLCODE_START;
    }
    virtual void to_json(json& j)
    {
        j = json{
            {"id", id},
            {"is_cheat", is_cheat},
            {"reason", reason},
        };
        Protocol::to_json(j);
    }
    virtual void from_json(const json& j)
    {
        id = j.at("id").get<uint32_t>();
        is_cheat = j.at("is_cheat").get<bool>();
        reason = j.at("reason").get<std::wstring>();
    }
    std::string dump()
    {
        std::wstringstream output;
        output << id << "|"
            << is_cheat << "|"
            << reason;
        return Utils::wstring2string(output.str(), 0);
    }

    bool is_cheat;
    std::wstring reason;
};

namespace SubProto
{
    struct Process
    {
        uint32_t pid;
        std::wstring path;
    };
    struct Module
    {
        std::wstring path;
    };

    struct Thread
    {
        uint32_t tid;
        uint64_t startaddr;
        std::wstring owner_module;
    };
    struct Directory
    {
        std::wstring path;
        bool is_directory;
    };
    struct Window
    {
        std::wstring caption;
        std::wstring class_name;
        uint32_t pid;
        std::wstring process_name;
        uint32_t tid;
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
        ENM_POLICY_TYPE_THREAD_START,
        ENM_POLICY_TYPE_MAX,
    };
    enum PunishType
    {
        ENM_PUNISH_TYPE_KICK = 0,
        ENM_PUNISH_TYPE_BSOD,
        ENM_PUNISH_TYPE_NO_OPEARATION,
        ENM_PUNISH_TYPE_BAN_MACHINE,
        ENM_PUNISH_TYPE_SCREEN_SHOT,
        ENM_PUNISH_TYPE_SCREEN_SHOT_KICK,
        ENM_PUNISH_TYPE_SCREEN_SHOT_BSOD,
        ENM_PUNISH_TYPE_SUPER_WHITE_LIST,
        ENM_PUNISH_TYPE_MAX
    };

    struct Policy
    {
        uint32_t policy_id;
        PolicyType policy_type;
        PunishType punish_type;
        std::wstring config;
        std::wstring comment;
    };

    struct DetectResult
    {
        uint32_t policy_id;
        std::wstring information;
    };
    void to_json(json& j, const Process& p);
    void from_json(const json& j, Process& p);
    void to_json(json& j, const Module& p);
    void from_json(const json& j, Module& p);
    void to_json(json& j, const Thread& p);
    void from_json(const json& j, Thread& p);
    void to_json(json& j, const Directory& p);
    void from_json(const json& j, Directory& p);
    void to_json(json& j, const Window& p);
    void from_json(const json& j, Window& p);
    void to_json(json& j, const Policy& p);
    void from_json(const json& j, Policy& p);
    void to_json(json& j, const DetectResult& p);
    void from_json(const json& j, DetectResult& p);
}
class ProtocolProcessInfo : public Protocol
{
public:

    ProtocolProcessInfo()
    {
        id = PackageId::PACKAGE_ID_PROCESSINFO;
    }
    virtual void to_json(json& j)
    {
        j = json{
            { GlobalString::JsonProperty::ID, id},
            { GlobalString::JsonProperty::SUBID, subid },
            { GlobalString::JsonProperty::PID, pid },
            { GlobalString::JsonProperty::PROCESSES, processes },
            { GlobalString::JsonProperty::MODULES, modules },
            { GlobalString::JsonProperty::THREADS, threads},
            { GlobalString::JsonProperty::DIRS, dirs},
            { GlobalString::JsonProperty::WINDOWS, windows},
        };
    }
    virtual void from_json(const json& j)
    {
        id = j.at(GlobalString::JsonProperty::ID).get<uint32_t>();
        subid = j.at(GlobalString::JsonProperty::SUBID).get<std::string>();
        pid = j.at(GlobalString::JsonProperty::PID).get<uint32_t>();
        processes = j.at(GlobalString::JsonProperty::PROCESSES).get<std::vector<SubProto::Process>>();
        modules = j.at(GlobalString::JsonProperty::MODULES).get<std::vector<SubProto::Module>>();
        threads = j.at(GlobalString::JsonProperty::THREADS).get<std::vector<SubProto::Thread>>();
        dirs = j.at(GlobalString::JsonProperty::DIRS).get<std::vector<SubProto::Directory>>();
        windows = j.at(GlobalString::JsonProperty::WINDOWS).get<std::vector<SubProto::Window>>();
    }
    std::string dump()
    {
        std::stringstream result;
        result << id << GlobalString::JsonProperty::SEPARATOR.c_str()
            << subid << GlobalString::JsonProperty::SEPARATOR.c_str()
            << pid << GlobalString::JsonProperty::SEPARATOR.c_str()
            << processes.size() << GlobalString::JsonProperty::SEPARATOR.c_str()
            << modules.size() << GlobalString::JsonProperty::SEPARATOR.c_str()
            << threads.size() << GlobalString::JsonProperty::SEPARATOR.c_str()
            << dirs.size() << GlobalString::JsonProperty::SEPARATOR.c_str()
            << windows.size() << GlobalString::JsonProperty::SEPARATOR.c_str();
        return result.str();
    }

    std::string subid;
    uint32_t pid;
    std::vector<SubProto::Process> processes;
    std::vector<SubProto::Module> modules;
    std::vector<SubProto::Thread> threads;
    std::vector<SubProto::Directory> dirs;
    std::vector<SubProto::Window> windows;
};

class ProtocolPolicy : public Protocol
{
public:

    ProtocolPolicy()
    {
        id = PackageId::PACKAGE_ID_POLICY;
    }
    virtual void to_json(json& j)
    {
        j = json{
            { GlobalString::JsonProperty::ID, id},
            { GlobalString::JsonProperty::POLICIES, policies },
            { GlobalString::JsonProperty::RESULTS, results },
        };
    }
    virtual void from_json(const json& j)
    {
        id = j.at(GlobalString::JsonProperty::ID).get<uint32_t>();
        policies = j.at(GlobalString::JsonProperty::POLICIES).get<std::map<uint32_t, SubProto::Policy>>();
        results = j.at(GlobalString::JsonProperty::RESULTS).get<std::vector<SubProto::DetectResult>>();
    }
    std::string dump()
    {
        std::stringstream result;
        result << id << GlobalString::JsonProperty::SEPARATOR.c_str()
            << policies.size() << GlobalString::JsonProperty::SEPARATOR.c_str()
            << results.size() << GlobalString::JsonProperty::SEPARATOR.c_str();
        return result.str();
    }

    std::map<uint32_t, SubProto::Policy> policies;
    std::vector<SubProto::DetectResult> results;
};


class ProtocolApiHook : public Protocol
{
public:

    ProtocolApiHook()
    {
        id = PackageId::PACKAGE_ID_API_HOOK;
    }
    virtual void to_json(json& j)
    {
        j = json{
            { GlobalString::JsonProperty::ID, id },
            { "hash", api_hash },
            { "params", params },
        };
    }
    virtual void from_json(const json& j)
    {
        id = j.at(GlobalString::JsonProperty::ID).get<uint32_t>();
        api_hash = j.at("hash").get<uint32_t>();
        params = j.at("params").get<std::vector<std::wstring>>();
    }
    std::string dump()
    {
        std::wstringstream result;
        result << id << GlobalString::JsonProperty::WSEPARATOR.c_str()
            << api_hash << GlobalString::JsonProperty::WSEPARATOR.c_str();
        for (auto& param : params)
        {
            result << param.c_str() << GlobalString::JsonProperty::WSEPARATOR.c_str();
        }
        return Utils::wstring2string(result.str(), 0);
    }

    uint32_t api_hash;
    std::vector<std::wstring> params;
};
