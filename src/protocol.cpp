#include "pch.h"
#include "protocol.h"
#include <stddef.h>
#include <intrin.h>
#include <string>

Protocol::Protocol()
{
    m_checksum = 0;
#ifdef CLIENT
    m_type = PackageType::PACKAGE_TYPE_ALL;
#else
#ifdef BUILD_ADMIN
    m_type = PackageType::PACKAGE_TYPE_ADMIN;
#else
    m_type = PackageType::PACKAGE_TYPE_ANTICHEAT;
#endif
#endif
    
    m_size = 0;
    m_buffer = nullptr;
    m_raw_size = 0;
}

Protocol::Protocol(const json& target)
    : Protocol()
{
    copy(target);
    m_raw_buffer = pak(&m_raw_size);
}

Protocol::~Protocol()
{
    if (m_buffer)
    {
        delete[] m_buffer;
        m_buffer = nullptr;
    }
}

void Protocol::copy(void* buffer, uint32_t len)
{
    if (m_buffer)
    {
        delete[]m_buffer;
        m_buffer = nullptr;
    }
    m_buffer = new unsigned char[len];
    __movsb(m_buffer, (unsigned char*)buffer, len);
    m_size = len;
}
bool Protocol::unpak(const unsigned char* buffer, uint32_t len)
{
    if (len < offsetof(Protocol, m_buffer))
    {
        return false;
    }
    m_checksum = *(uint32_t*)&buffer[offsetof(Protocol, m_checksum)];
    m_type = *(PackageType*)&buffer[offsetof(Protocol, m_type)];
    m_size = *(PackageType*)&buffer[offsetof(Protocol, m_size)];
    if (m_size != (len - offsetof(Protocol, m_buffer)))
    {
        return false;
    }
    uint32_t hash_val = aphash(buffer + offsetof(Protocol, m_type), len - offsetof(Protocol, m_type));
    if (m_checksum != hash_val)
    {
        return false;
    }
    if (m_buffer)
    {
        delete[] m_buffer;
        m_buffer = nullptr;
    }
    this->m_buffer = new unsigned char[m_size];
    __movsb(m_buffer, &buffer[offsetof(Protocol, m_buffer)], m_size);
    return true;
}

BufferPtr Protocol::pak(uint32_t* len)
{
    uint32_t size = offsetof(Protocol, m_buffer) + m_size;
    std::unique_ptr<unsigned char[]> temp(new unsigned char[size]);
    __movsb(temp.get(), (unsigned char*)this, offsetof(Protocol, m_buffer));
    if (m_size && m_buffer)
    {
        __movsb(&temp[offsetof(Protocol, m_buffer)], m_buffer, m_size);
    }
    m_checksum = aphash(temp.get() + offsetof(Protocol, m_type), size - offsetof(Protocol, m_type));
    *(uint32_t*)&temp[offsetof(Protocol, m_checksum)] = m_checksum;
    *len = size;
    return std::move(temp);
}

void Protocol::copy(json json_)
{
    std::vector<std::uint8_t> buffer = json::to_ubjson(json_);
    copy((void*)buffer.data(), buffer.size());
}

void Protocol::get_json(json& dest)
{
    dest = json::from_ubjson(m_buffer, m_buffer + m_size);
}

namespace SubProto
{
    void to_json(json& j, const Process& p)
    {
        j = json{
            { "pid",p.pid },
            { "path",p.path }
        };
    }




    void from_json(const json& j, Process& p)
    {
        p.pid = j.at("pid").get<uint32_t>();
        p.path = j.at("path").get<std::wstring>();
    }



    void to_json(json& j, const Module& p)
    {
        j = json{
            { "path",p.path }
        };
    }

    void from_json(const json& j, Module& p)
    {
        p.path = j.at("path").get<std::wstring>();
    }

    void to_json(json& j, const Thread& p)
    {
        j = json{
            { "tid",p.tid },
            { "startaddr",p.startaddr },
            { "owner_module",p.owner_module },
        };
    }

    void from_json(const json& j, Thread& p)
    {
        p.tid = j.at("tid").get<uint32_t>();
        p.startaddr = j.at("startaddr").get<uint64_t>();
        p.owner_module = j.at("owner_module").get<std::wstring>();
    }

    void to_json(json& j, const Directory& p)
    {
        j = json{
            {"path",p.path},
            {"is_directory",p.is_directory},
        };
    }
    void from_json(const json& j, Directory& p)
    {
        p.path = j.at("path").get<std::wstring>();
        p.is_directory = j.at("is_directory").get<bool>();
    }

    void to_json(json& j, const Window& p)
    {
        j = json{
            {"caption",p.caption},
            {"class_name",p.class_name},
            {"pid",p.pid},
            {"process_name",p.process_name},
            {"tid",p.tid},
        };
    }
    void from_json(const json& j, Window& p)
    {
        p.caption = j.at("caption").get<std::wstring>();
        p.class_name = j.at("class_name").get<std::wstring>();
        p.pid = j.at("pid").get<uint32_t>();
        p.process_name = j.at("process_name").get<std::wstring>();
        p.tid = j.at("tid").get<uint32_t>();
    }

    void from_json(const json& j, Policy& p)
    {
        p.policy_id = j.at("policy_id").get<uint32_t>();
        p.policy_type = j.at("policy_type").get<PolicyType>();
        p.punish_type = j.at("punish_type").get<PunishType>();
        p.config = j.at("config").get<std::wstring>();
        p.comment = j.find("comment") != j.end() ? j.at("comment").get<std::wstring>() : L"";
    }

    void to_json(json& j, const Policy& p)
    {
        j = json{
            {"policy_id",p.policy_id},
            {"policy_type",p.policy_type},
            {"punish_type",p.punish_type},
            {"config",p.config},
            {"comment",p.comment},
        };
    }

    void from_json(const json& j, DetectResult& p)
    {
        p.policy_id = j.at("policy_id").get<uint32_t>();
        p.information = j.at("information").get<std::wstring>();
    }

    void to_json(json& j, const DetectResult& p)
    {
        j = json{
            {"policy_id",p.policy_id},
            {"information",p.information},
        };
    }

}

