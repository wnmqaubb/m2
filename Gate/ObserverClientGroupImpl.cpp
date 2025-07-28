#include "pch.h"
#include "ObserverClientGroupImpl.h"
#include <asio2/util/md5.hpp>


 
CObserverClientGroupImpl::CObserverClientGroupImpl() : work_guard_(asio::make_work_guard(io_))
{

}

CObserverClientGroupImpl::~CObserverClientGroupImpl()
{
    work_guard_.reset(); // 先释放work_guard
    io_.stop();
    thread_group_.join();
}

std::shared_ptr<CObserverClientImpl> CObserverClientGroupImpl::operator()(const std::string& ip, unsigned short port)
{
    auto address = ip + ":" + std::to_string(port);
    {
        std::shared_lock<std::shared_mutex> lck(mtx_);
        if (auto it = group_.find(address); it != group_.end()) {
            return it->second;
        }
    }
    {
        std::unique_lock lock(mtx_);
        auto client = std::make_shared<CObserverClientImpl>(io_, asio2::md5(ip + ",./;").str());
        group_.emplace(address, client);
        return client;
    }
}


void CObserverClientGroupImpl::create_threads(int count)
{
    // 确保不会重复创建线程
    if (!thread_group_.empty()) return;

    for (int i = 0; i < count; ++i) {
        thread_group_.create_thread([this] {
            while (true) {
                try {
                    io_.run();
                    break; // 正常退出
                }
                catch (const std::exception& e) {
                    // 异常处理
                }
            }
        });
    }
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