#pragma once
#include "Service/AntiCheatClient.h"
#include "ClientPluginMgr.h"

class CClientImpl : public CAntiCheatClient
{
public:
    using super = CAntiCheatClient;
	CClientImpl(std::unique_ptr<ProtocolCFGLoader> cfg);
    void init();
    void client_start_routine();
    void register_notify_handler(unsigned int notify_id, std::function<void()> handler);
    void register_package_handler(unsigned int package_id, std::function<void(const RawProtocolImpl &, const msgpack::v1::object_handle &)> handler);
    void send_handshake();
    void start_heartbeat_timer();
    void query_plugin_list();
    void start_plugin_list_timer();
    void initialize_user_data();
    void handle_plugin_list_response(const msgpack::v1::object_handle &raw_msg);
    void handle_download_plugin_response(const RawProtocolImpl &package, const msgpack::v1::object_handle &msg);
    void send_plugin_check_response(const RawProtocolImpl &package);
    void handle_punish_response(const msgpack::v1::object_handle &msg);
    bool user_is_login();
    virtual void on_recv(unsigned int package_id, const RawProtocolImpl &package, const msgpack::v1::object_handle &);
    virtual void load_uuid();
    virtual void save_uuid(const ProtocolC2SHandShake& handshake);
    fs::path exe_path_;
    fs::path cache_dir_;
    std::unique_ptr<CClientPluginMgr> plugin_mgr_;
    std::shared_ptr<HWND> g_main_window_hwnd;
};