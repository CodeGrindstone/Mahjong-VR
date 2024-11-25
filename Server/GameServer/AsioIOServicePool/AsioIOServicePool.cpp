//
// Created by 31435 on 2024/9/12.
//
#include <iostream>
#include "AsioIOServicePool.h"

AsioIOServicePool::~AsioIOServicePool()
{
    stop();
    std::cout << "AsioIOServicePool destruct" << std::endl;
}

boost::asio::io_service& AsioIOServicePool::getIOService()
{
    auto& service = m_io_services[_nextIOSerice++];
    if(_nextIOSerice == m_io_services.size())
    {
        _nextIOSerice = 0;
    }
    return service;
}

void AsioIOServicePool::stop()
{
    for(auto& work : m_works)
    {
        work->get_io_context().stop();
        work.reset();
    }

    for(auto& t:m_threads)
    {
        t.join();
    }
}

AsioIOServicePool::AsioIOServicePool(std::size_t pool_size) : m_io_services(pool_size), m_works(pool_size), _nextIOSerice(0)
{
    for(int i = 0; i < pool_size; i++) 
    { 
        m_works[i] = std::unique_ptr<Work>(new Work(m_io_services[i])); 
    }
    for(int i = 0; i < pool_size; i++)
    {
        m_threads.emplace_back([this, i]()
        {
            m_io_services[i].run();
        });
    }
}
