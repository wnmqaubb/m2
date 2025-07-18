#pragma once
#include "AntiCheatClient.h"
#include "AntiCheatServer.h"
#include "ObserverClient.h"
#include "ObServerPackage.h"
#include "LogicClient.h"

class CObserverServer : public CAntiCheatServer
{
    using super = CAntiCheatServer;
public:
    CObserverServer();
    virtual void on_post_disconnect(tcp_session_shared_ptr_t& session);
    virtual bool on_recv(unsigned int package_id, tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg);
    virtual void connect_to_logic_server(const std::string& ip, unsigned short port);;
    virtual void log_cb(const wchar_t* msg, bool silence, bool gm_show, const std::string& identify);
    void logic_client_stop() { if (logic_client_) { logic_client_->stop_all_timers(); logic_client_->stop(); } };
    void stop() { logic_client_->stop(); super::stop(); }
    std::wstring& get_vmp_expire() { return vmp_expire_; }
protected:
    std::shared_ptr<CLogicClient> logic_client_;
    NetUtils::EventMgr<package_handler_t> ob_pkg_mgr_;
    std::wstring vmp_expire_;
};