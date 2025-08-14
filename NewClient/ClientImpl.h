#pragma once
#include "Service/AntiCheatClient.h"
#include <filesystem>
namespace fs = std::filesystem;
class CClientImpl : public CAntiCheatClient
{
public:
    using super = CAntiCheatClient;
    CClientImpl();
    virtual void on_recv(unsigned int package_id, const RawProtocolImpl& package, const msgpack::v1::object_handle&);
    virtual void load_uuid();
    virtual void save_uuid(const ProtocolC2SHandShake& handshake);
    void set_role_name_callback(std::function<void(const std::wstring&)> callback);
    void monitor_main_window();
    void window_monitor_thread();
    void init_role_monitor();
    void process_title_messages();
    fs::path exe_path_;
    fs::path cache_dir_;
    // 全局变量存储状态
    std::atomic<HWND> g_main_window_hwnd = nullptr;
    std::mutex g_windowMutex;
    DWORD g_currentPid = 0;
    // 角色名变更回调函数
    std::function<void(const std::wstring&)> g_roleNameCallback;
};