#pragma once
#include "AntiCheatClient.h"
#include "ObServerPackage.h"

class CObserverClient : public CAntiCheatClient
{
    using super = CAntiCheatClient;
public:
    using super::send;
    using super::async_send;
    CObserverClient() : is_auth_(false)
    {
        package_mgr_.register_handler(OBPKG_ID_S2C_SET_FIELD, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
            auto req = msg.get().as<ProtocolOBS2OBCSetField>();
            user_data().set_field(NetUtils::hash(req.key.c_str()), req.val);
        });
        package_mgr_.register_handler(OBPKG_ID_S2C_AUTH, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
            auto status = msg.get().as<ProtocolOBS2OBCAuth>().status;
            if (status)
            {
                is_auth_ = true;
                notify_mgr_.dispatch(CLIENT_AUTH_SUCCESS_NOTIFY_ID);
            }
            else
            {
                is_auth_ = false;
                notify_mgr_.dispatch(CLIENT_AUTH_FAILED_NOTIFY_ID);
            }
        });
        notify_mgr_.register_handler(CLIENT_DISCONNECT_NOTIFY_ID, [this]() {
            is_auth_ = false;
            get_gate_notify_mgr().dispatch(CLIENT_DISCONNECT_NOTIFY_ID);
            log(LOG_TYPE_DEBUG, TEXT("失去连接:%s:%s"), Utils::c2w(get_address()).c_str(),
                Utils::c2w(get_port()).c_str());
        });
        notify_mgr_.register_handler(CLIENT_CONNECT_FAILED_NOTIFY_ID, [this]() {
            get_gate_notify_mgr().dispatch(CLIENT_CONNECT_FAILED_NOTIFY_ID);
            log(LOG_TYPE_DEBUG, TEXT("连接失败:%s:%s"), Utils::c2w(get_address()).c_str(),
                Utils::c2w(get_port()).c_str());
        });
        notify_mgr_.register_handler(CLIENT_CONNECT_SUCCESS_NOTIFY_ID, [this]() {
            get_gate_notify_mgr().dispatch(CLIENT_CONNECT_SUCCESS_NOTIFY_ID);
            log(LOG_TYPE_DEBUG, TEXT("建立连接:%s:%s"), Utils::c2w(get_address()).c_str(),
                Utils::c2w(get_port()).c_str());
            ProtocolC2SHandShake handshake;
            memcpy(&handshake.uuid, uuid_.data, sizeof(handshake.uuid));
            super::async_send(&handshake);
            start_timer<int>(CLIENT_HEARTBEAT_TIMER_ID, heartbeat_duration_, [this]() {
                ProtocolC2SHeartBeat heartbeat;
                heartbeat.tick = time(0);
                super::async_send(&heartbeat);
            });
        });
        // 获取到期时间
        notify_mgr_.register_handler(ON_RECV_HANDSHAKE_NOTIFY_ID, [this]() {
            gate_query_vmp_expire();
        });
    }

    void gate_query_vmp_expire()
    {
        ProtocolOBC2OBSAuth auth;
        auth.key = auth_key_;
        super::async_send(&auth);
    }

    virtual void send(std::size_t session_id, RawProtocolImpl& package)
    {
        ProtocolOBC2OBSSend req;
        req.package = package;
        req.package.head.session_id = session_id;
        super::send(&req);
    }

    virtual void send(std::size_t session_id, msgpack::sbuffer& buffer)
    {
        RawProtocolImpl raw_package;
        raw_package.encode(buffer.data(), buffer.size());
        send(session_id, raw_package);
    }

    template <typename T>
    void send(std::size_t session_id, T* package)
    {
        if (!package)
            __debugbreak();
        msgpack::sbuffer buffer;
        msgpack::pack(buffer, *package);
        send(session_id, buffer);
    }

    virtual void async_send(std::size_t session_id, RawProtocolImpl& package)
    {
        ProtocolOBC2OBSSend req;
        req.package = package;
        req.package.head.session_id = session_id;
        super::async_send(&req);
    }

    virtual void async_send(std::size_t session_id, msgpack::sbuffer& buffer)
    {
        RawProtocolImpl raw_package;
        raw_package.encode(buffer.data(), buffer.size());
        async_send(session_id, raw_package);
    }

    template <typename T>
    void async_send(std::size_t session_id, T* package)
    {
        if (!package)
            __debugbreak();
        msgpack::sbuffer buffer;
        msgpack::pack(buffer, *package);
        async_send(session_id, buffer);
    }

    inline NetUtils::EventMgr<notify_handler_t>& get_gate_notify_mgr() { return gate_notify_mgr_; }
    inline void set_auth_key(const std::string& auth_key) { auth_key_ = auth_key; }
    inline const std::string& get_auth_key() { return auth_key_; }
    inline bool is_auth() { return is_auth_; }
private:
    NetUtils::EventMgr<notify_handler_t> gate_notify_mgr_;
    std::string auth_key_;
    bool is_auth_;
};