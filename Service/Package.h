<<<<<<< HEAD
#pragma once
template <unsigned short pkg_id = 0>
struct ProtocolBase
{
    unsigned short package_id = pkg_id;
};

enum PackageId
{
    PKG_ID_UNDEFINED = 0,
    PKG_ID_C2S_HANDSHAKE,
    PKG_ID_S2C_HEARTBEAT,
    PKG_ID_C2S_HEARTBEAT,
    PKG_ID_S2C_HANDSHAKE,
    PKG_ID_MAX_PACKAGE_ID_SIZE,
};

enum SystemVersion
{
    WINDOWS_ANCIENT = 0,
    WINDOWS_XP = 51,
    WINDOWS_SERVER_2003 = 52,
    WINDOWS_VISTA = 60,
    WINDOWS_7 = 61,
    WINDOWS_8 = 62,
    WINDOWS_8_1 = 63,
    WINDOWS_10 = 100,
    WINDOWS_NEW = MAXLONG
};

struct ProtocolC2SHandShake : ProtocolBase<PKG_ID_C2S_HANDSHAKE>
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
    int system_version;
    bool is_64bit_system;
    std::wstring cpuid;
    std::wstring mac;
    std::wstring volume_serial_number;
    int rev_version = 0;
    std::string commited_hash = "";
    unsigned int pid = 0;
    static std::unique_ptr<ProtocolC2SHandShake> load(const char* buf, size_t len)
    {
        RawProtocolImpl raw_msg;
        if (raw_msg.decode(std::string_view(buf, len)) == false)
        {
            return nullptr;
        }
        auto msg = msgpack::unpack((char*)raw_msg.body.buffer.data(), raw_msg.body.buffer.size());
        auto res = std::make_unique<ProtocolC2SHandShake>(msg.get().as<ProtocolC2SHandShake>());
        return std::move(res);
    }

    std::string dump() const
    {
        msgpack::sbuffer sbuf;
        msgpack::pack(sbuf, *this);
        RawProtocolImpl raw_msg;
        raw_msg.encode(sbuf.data(), sbuf.size());
        return raw_msg.release();
    }

    MSGPACK_DEFINE(package_id, uuid_1, uuid_2, uuid_3, uuid_4,
        system_version,
        is_64bit_system,
        cpuid,
        mac,
        volume_serial_number,
        rev_version,
        commited_hash,
        pid);
};

struct ProtocolS2CHandShake : ProtocolBase<PKG_ID_S2C_HANDSHAKE>
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
    MSGPACK_DEFINE(package_id, uuid_1, uuid_2, uuid_3, uuid_4);
};

struct ProtocolC2SHeartBeat : ProtocolBase<PKG_ID_C2S_HEARTBEAT>
{
    time_t tick;
    MSGPACK_DEFINE(package_id, tick);
};

struct ProtocolS2CHeartBeat : ProtocolBase<PKG_ID_S2C_HEARTBEAT>
{
    time_t tick;
    MSGPACK_DEFINE(package_id, tick);
=======
#pragma once
template <unsigned short pkg_id = 0>
struct ProtocolBase
{
    unsigned short package_id = pkg_id;
};

enum PackageId
{
    PKG_ID_UNDEFINED = 0,
    PKG_ID_C2S_HANDSHAKE,
    PKG_ID_S2C_HEARTBEAT,
    PKG_ID_C2S_HEARTBEAT,
    PKG_ID_S2C_HANDSHAKE,
    PKG_ID_MAX_PACKAGE_ID_SIZE,
};

enum SystemVersion
{
    WINDOWS_ANCIENT = 0,
    WINDOWS_XP = 51,
    WINDOWS_SERVER_2003 = 52,
    WINDOWS_VISTA = 60,
    WINDOWS_7 = 61,
    WINDOWS_8 = 62,
    WINDOWS_8_1 = 63,
    WINDOWS_10 = 100,
    WINDOWS_NEW = MAXLONG
};

struct ProtocolC2SHandShake : ProtocolBase<PKG_ID_C2S_HANDSHAKE>
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
    int system_version;
    bool is_64bit_system;
    std::wstring cpuid;
    std::wstring mac;
    std::wstring volume_serial_number;
    int rev_version = 0;
    std::string commited_hash = "";
    unsigned int pid = 0;
    static std::unique_ptr<ProtocolC2SHandShake> load(const char* buf, size_t len)
    {
        RawProtocolImpl raw_msg;
        if (raw_msg.decode(std::string_view(buf, len)) == false)
        {
            return nullptr;
        }
        auto msg = msgpack::unpack((char*)raw_msg.body.buffer.data(), raw_msg.body.buffer.size());
        auto res = std::make_unique<ProtocolC2SHandShake>(msg.get().as<ProtocolC2SHandShake>());
        return std::move(res);
    }

    std::string dump() const
    {
        msgpack::sbuffer sbuf;
        msgpack::pack(sbuf, *this);
        RawProtocolImpl raw_msg;
        raw_msg.encode(sbuf.data(), sbuf.size());
        return raw_msg.release();
    }

    MSGPACK_DEFINE(package_id, uuid_1, uuid_2, uuid_3, uuid_4,
        system_version,
        is_64bit_system,
        cpuid,
        mac,
        volume_serial_number,
        rev_version,
        commited_hash,
        pid);
};

struct ProtocolS2CHandShake : ProtocolBase<PKG_ID_S2C_HANDSHAKE>
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
    MSGPACK_DEFINE(package_id, uuid_1, uuid_2, uuid_3, uuid_4);
};

struct ProtocolC2SHeartBeat : ProtocolBase<PKG_ID_C2S_HEARTBEAT>
{
    time_t tick;
    MSGPACK_DEFINE(package_id, tick);
};

struct ProtocolS2CHeartBeat : ProtocolBase<PKG_ID_S2C_HEARTBEAT>
{
    time_t tick;
    MSGPACK_DEFINE(package_id, tick);
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9
};