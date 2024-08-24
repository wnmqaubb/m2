#pragma once
#include "AntiCheatClient.h"
#include "ObServerPackage.h"

class CLogicClient : public CAntiCheatClient
{
    using super = CAntiCheatClient;
public:
    CLogicClient()
        : super()
    {
        notify_mgr_.register_handler(CLIENT_CONNECT_SUCCESS_NOTIFY_ID, [this]() {
            ProtocolC2SHandShake handshake;
            memcpy(&handshake.uuid, uuid_.data, sizeof(handshake.uuid));
            send(&handshake);
			start_timer<unsigned int>(CLIENT_HEARTBEAT_TIMER_ID, heartbeat_duration_, [this]() {
				ProtocolC2SHeartBeat heartbeat;
				heartbeat.tick = time(0);
				send(&heartbeat);
				});
            sub_notify_mgr_.dispatch(CLIENT_CONNECT_SUCCESS_NOTIFY_ID);
        });
    }
    virtual void on_recv(unsigned int package_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg)
    {
        if (on_recv_logic_package)
            on_recv_logic_package(package_id, package, raw_msg);
    }
    std::function<void(unsigned int package_id, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg)> on_recv_logic_package;
    NetUtils::EventMgr<notify_handler_t> sub_notify_mgr_;
};