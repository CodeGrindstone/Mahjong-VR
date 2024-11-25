//
// Created by 31435 on 2024/9/28.
//

#include "ConfigMgr/ConfigMgr.h"
#include "AsioIOServicePool/AsioIOServicePool.h"
#include "CServer/CServer.h"

#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>

int main()
{
    try{
        auto& cfg = ConfigMgr::getInstance();
        auto pool = AsioIOServicePool::GetInstance();
        boost::asio::io_context ioContext;
        boost::asio::signal_set signals(ioContext, SIGINT, SIGTERM);
        signals.async_wait([&ioContext, pool](auto, auto){
            ioContext.stop();
            pool->stop();
        });
        auto port_str = cfg["SelfServer"]["Port"];
        CServer s(ioContext, atoi(port_str.c_str()));
        ioContext.run();

    }catch(std::exception& e){
        std::cerr << "Exception: " << e.what() << std::endl;

    }
}
