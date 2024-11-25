//
// Created by 31435 on 2024/9/28.
//

#ifndef CHATXT_CHATSERVER_CSERVER_H
#define CHATXT_CHATSERVER_CSERVER_H

#include "../AsioIOServicePool/AsioIOServicePool.h"

#include <boost/asio.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <iostream>

class CSession;
using boost::asio::ip::tcp;

class CServer : public std::enable_shared_from_this<CServer>
{
public:
    CServer(boost::asio::io_context& ioc, unsigned short port);
    ~CServer();
    void ClearSession(std::string);
private:
    void HandleAccept(std::shared_ptr<CSession>, const boost::system::error_code error);
    void StartAccept();
private:
    boost::asio::io_context& m_ioc;
    tcp::acceptor m_acceptor;
    std::map<std::string, std::shared_ptr<CSession>> m_sessions;
    std::mutex m_mutex;
};
#endif //CHATXT_CHATSERVER_CSERVER_H
