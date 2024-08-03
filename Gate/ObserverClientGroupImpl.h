#pragma once
#include "ObserverClientImpl.h"
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
    std::shared_mutex mtx_;
    std::unordered_map<std::string, std::shared_ptr<CObserverClientImpl>> group_;
    asio::io_service io_;
    asio::detail::thread_group thread_group_;
public:

    template <typename T> 
    void send(T* req)
    {
        std::shared_lock<std::shared_mutex> lck(mtx_);
        for (auto[address, client] : group_)
        {
            client->send(req);
        }
    }

};


