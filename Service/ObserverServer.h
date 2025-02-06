#pragma once
#include "AntiCheatClient.h"
#include "AntiCheatServer.h"
#include "LogicClient.h"
#include "ObserverClient.h"
#include "ObServerPackage.h"
#include <asio2/tcp/tcp_server.hpp>
#include <chrono>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

using tcp_session_shared_ptr_t = std::shared_ptr<asio2::tcp_session>;
struct Task {
    unsigned int package_id;
    tcp_session_shared_ptr_t session;
    RawProtocolImpl package;
    msgpack::v1::object_handle raw_msg;

    Task() = default;
    Task(unsigned int id, tcp_session_shared_ptr_t s, const RawProtocolImpl& p, msgpack::v1::object_handle&& msg)
        : package_id(id), session(std::move(s)), package(p), raw_msg(std::move(msg)) {
    }

    // 禁止拷贝
    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    // 允许移动
    Task(Task&&) noexcept = default;
    Task& operator=(Task&&) noexcept = default;
};

class CObserverServer : public CAntiCheatServer {
    using super = CAntiCheatServer;

public:
    CObserverServer();
    static CObserverServer& instance() {
        static CObserverServer instance;
        return instance;
    }

    void connect_to_logic_server(const std::string& ip, unsigned short port);
    void logic_client_stop() { logic_client_->stop(); }
    void stop() { logic_client_->stop(); super::stop(); }

    std::wstring& get_vmp_expire() { return vmp_expire_; }
    void process_task(Task&& task);

    void log_cb(const wchar_t* msg, bool silence, bool gm_show, const std::string& identify, bool punish_flag);

    // 添加获取最后活跃时间的方法
    std::chrono::steady_clock::time_point get_last_active_time(const tcp_session_shared_ptr_t& session) {
        std::lock_guard<std::mutex> lock(session_times_mtx_);
        auto it = session_last_active_times_.find(session);
        if (it != session_last_active_times_.end()) {
            return it->second;
        }
        return std::chrono::steady_clock::now();
    }

    // 更新session最后活跃时间
    void update_last_active_time(const tcp_session_shared_ptr_t& session) {
        std::lock_guard<std::mutex> lock(session_times_mtx_);
        session_last_active_times_[session] = std::chrono::steady_clock::now();
    }
protected:
    bool on_recv(unsigned int package_id, tcp_session_shared_ptr_t& session, const RawProtocolImpl& package, msgpack::v1::object_handle&& raw_msg) override;
    void on_post_disconnect(tcp_session_shared_ptr_t& session) override;
    std::wstring vmp_expire_;
    std::shared_ptr<CLogicClient> logic_client_;
    NetUtils::EventMgr<package_handler_t> ob_pkg_mgr_;
    std::shared_mutex shared_mtx_;
    std::unordered_map<tcp_session_shared_ptr_t, std::chrono::steady_clock::time_point> session_last_active_times_;
    std::shared_timed_mutex task_mtx_;
    std::timed_mutex session_mtx_;  // 用于保护session相关操作
public:
    std::mutex session_times_mtx_;
};