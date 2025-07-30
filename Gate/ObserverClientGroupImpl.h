#pragma once
#include "ObserverClientImpl.h"
#include <asio/io_context.hpp>
class CObserverClientGroupImpl
{
    using package_handler_t = std::function<void(std::shared_ptr<CObserverClientImpl> client, const RawProtocolImpl& package, const msgpack::v1::object_handle&)>;
    using observer_package_t = std::function<void(std::shared_ptr<CObserverClientImpl> client, unsigned int session_id, const RawProtocolImpl& package, const msgpack::v1::object_handle&)>;
public:
    CObserverClientGroupImpl();
    ~CObserverClientGroupImpl();
    std::shared_ptr<CObserverClientImpl> operator ()(const std::string& ip, unsigned short port);
    void create_threads(int count = 1);
    void stop();
    void register_package_handler(unsigned int package_id, package_handler_t handler);
    void register_client_package_handler(unsigned int package_id, observer_package_t handler);
    void foreach(std::function<void(std::shared_ptr<CObserverClientImpl>)> cb)
    {
        std::shared_lock<std::shared_mutex> lck(mtx_);
        for (auto [address, client] : group_)
        {
            cb(client);
        }
    }
private:
    asio::io_service io_;
    std::shared_mutex mtx_;
    asio::executor_work_guard<asio::io_context::executor_type> work_guard_;
    std::unordered_map<std::string, std::shared_ptr<CObserverClientImpl>> group_;
    asio::detail::thread_group thread_group_;
public:
    void clear_group() {
        std::unique_lock<std::shared_mutex> lck(mtx_);
        // 清除掉未启动或未认证的连接
        for (auto it = group_.begin(); it != group_.end(); ) {
            if (!it->second->is_started()) {
                it->second->stop();
                it = group_.erase(it);
            }
            else {
                ++it;
            }
        }
    }
    template <typename T> 
    void send(T* req)
    {
        std::shared_lock<std::shared_mutex> lck(mtx_);
        for (auto[address, client] : group_)
        {
            if (!client->is_started() || !client->is_auth()) continue;
            client->send(req);
        }
    }

};


