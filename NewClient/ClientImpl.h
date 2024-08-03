#pragma once
#include "Service/AntiCheatClient.h"
#include "ClientPluginMgr.h"

class CClientImpl : public CAntiCheatClient
{
    using super = CAntiCheatClient;
public:
    CClientImpl(asio::io_service& io_);
    virtual void on_recv(unsigned int package_id, const RawProtocolImpl& package, const msgpack::v1::object_handle&);
    virtual void load_uuid();
    virtual void save_uuid(const ProtocolC2SHandShake& handshake);
    fs::path exe_path_;
    fs::path cache_dir_;
    std::unique_ptr<CClientPluginMgr> plugin_mgr_;
};