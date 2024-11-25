//
// Created by 31435 on 2024/9/28.
//

#include "CServer.h"
#include "../CSession/CSession.h"

#include <memory>

void CServer::StartAccept() {
    auto& io_context = AsioIOServicePool::GetInstance()->getIOService();
    std::shared_ptr<CSession> new_conn = std::make_shared<CSession>(io_context, this);
    m_acceptor.async_accept(new_conn->GetSocket(), std::bind(&CServer::HandleAccept, this, new_conn, std::placeholders::_1));
}

CServer::~CServer() {

}

void CServer::ClearSession(std::string uuid) 
{
    if(m_sessions.find(uuid) != m_sessions.end()){
       // .... 
    }
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_sessions.erase(uuid);
    }

}

CServer::CServer(boost::asio::io_context &ioc, unsigned short port) :
m_ioc(ioc),
m_acceptor(ioc, tcp::endpoint(tcp::v4(), port))
{
    std::cout << "Server start success, listen on port : " << port << std::endl;
    StartAccept();
}

void CServer::HandleAccept(std::shared_ptr<CSession> new_session, const boost::system::error_code ec)
{
    if(ec){
        std::cout << "boost error is " << ec.message() << std::endl;
    }
    else{
        new_session->Start();
        std::lock_guard<std::mutex> mutex(m_mutex);
        m_sessions.insert(std::make_pair(new_session->GetSessionId(), new_session));
    }
    StartAccept();
}
