//
// Created by 31435 on 2024/9/12.
//

#ifndef ASIOIOSERVICEPOOL_H
#define ASIOIOSERVICEPOOL_H

#include "../LogicSystem/Singleton.h"
#include <vector>
#include <boost/asio.hpp>
#include <memory>

class AsioIOServicePool : public Singleton<AsioIOServicePool>
{
    friend class Singleton<AsioIOServicePool>;
public:
    using IOServce = boost::asio::io_service;
    using Work = boost::asio::io_service::work;
    using WorkPtr = std::unique_ptr<Work>;

    ~AsioIOServicePool();
    AsioIOServicePool(const AsioIOServicePool&) = delete;
    AsioIOServicePool& operator=(const AsioIOServicePool&) = delete;

    boost::asio::io_service& getIOService();
    void stop();
private:
    AsioIOServicePool(std::size_t pool_size = std::thread::hardware_concurrency());
    std::vector<IOServce> m_io_services;
    std::vector<WorkPtr> m_works; std::vector<std::thread> m_threads;
    std::size_t _nextIOSerice;
};



#endif //ASIOIOSERVICEPOOL_H
