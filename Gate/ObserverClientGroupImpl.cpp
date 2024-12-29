#include "pch.h"
#include "ObserverClientGroupImpl.h"
#include <asio2/util/md5.hpp>
 
CObserverClientGroupImpl::CObserverClientGroupImpl()
{

}

CObserverClientGroupImpl::~CObserverClientGroupImpl()
{
    io_.stop();
    {
        std::shared_lock<std::shared_mutex> lck(mtx_);
        group_.clear();
    }
}

std::shared_ptr<CObserverClientImpl> CObserverClientGroupImpl::operator()(const std::string& ip, unsigned short port)
{
    auto address = ip + ":" + std::to_string(port);
    std::shared_ptr<CObserverClientImpl> client;

    std::shared_lock<std::shared_mutex> lck(mtx_);
    auto it = group_.find(address);
    if (it != group_.end())
    {
        client = it->second;
    }
    else
    {
        lck.unlock(); // 释放共享锁，准备获取独占锁

        std::unique_lock<std::shared_mutex> ulck(mtx_);
        it = group_.find(address); // 再次检查，避免竞态条件
        if (it == group_.end())
        {
            client = std::make_shared<CObserverClientImpl>(io_, asio2::md5(ip + ",./;").str());
            group_[address] = client;
        }
        else
        {
            client = it->second;
        }
    }

    return client;
}

void CObserverClientGroupImpl::create_threads(int count)
{
    thread_group_.create_threads([this]() {
        auto guard = asio::make_work_guard(io_);
        io_.run();
    }, count);
}

void CObserverClientGroupImpl::stop()
{
    std::shared_lock<std::shared_mutex> lck(mtx_);
    for (auto[address, client] : group_)
    {
        client->stop();
    }
}

void CObserverClientGroupImpl::register_package_handler(unsigned int package_id, package_handler_t handler)
{
    std::shared_lock<std::shared_mutex> lck(mtx_);
    for (auto[address, client] : group_)
    {
        client->package_mgr().register_handler(package_id, std::bind(handler, client, std::placeholders::_1,
            std::placeholders::_2));
    }
}

void CObserverClientGroupImpl::register_client_package_handler(unsigned int package_id, observer_package_t handler)
{
    std::shared_lock<std::shared_mutex> lck(mtx_);
    for (auto[address, client] : group_)
    {
        client->client_pkg_mgr_.register_handler(package_id, std::bind(handler, client, std::placeholders::_1,
            std::placeholders::_2, std::placeholders::_3));
    }
}