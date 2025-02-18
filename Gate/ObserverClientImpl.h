#pragma once
#include "Service/ObserverClient.h"
#include <filesystem>

class CObserverClientImpl : public CObserverClient
{
    using super = CObserverClient;
public:
    CObserverClientImpl(asio::io_service& io_);
    virtual void log(int type, LPCTSTR format, ...) override;
    void OpenDocument(const std::wstring& path);
    using observer_package_type = std::function<void(std::size_t session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle&)>;
    NetUtils::EventMgr<observer_package_type> client_pkg_mgr_;
    void set_user_count(size_t count) { user_count_ = count; }
    size_t get_user_count() { return user_count_; }
    std::set<std::size_t>& session_ids() { return session_ids_; }
protected:
    std::set<std::size_t> session_ids_;
    std::filesystem::path cache_dir_;
    size_t user_count_;
};
