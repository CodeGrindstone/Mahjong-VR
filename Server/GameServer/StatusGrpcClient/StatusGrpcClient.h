//
// Created by 31435 on 2024/9/24.
//

#ifndef CHATXTSERVER_STATUSGRPCCLIENT_H
#define CHATXTSERVER_STATUSGRPCCLIENT_H


#include "../global.h"
#include "../LogicSystem/Singleton.h"
#include "../ConfigMgr/ConfigMgr.h"
#include "../protoc/message.grpc.pb.h"
#include "../protoc/message.pb.h"

#include <grpcpp/grpcpp.h>
#include <queue>
#include <atomic>
#include <condition_variable>

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;
using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::StatusService;
using message::LoginReq;
using message::LoginRsp;

class StatusConPool {
public:
    StatusConPool(size_t poolsize, std::string host, std::string port) : 
    b_stop_(false), poolSize_(poolsize), host_(host), port_(port){
        std::string server_address(host_ + ":" + port_);
    #if DEBUG
        std::cout << server_address << std::endl;
    #endif
        for(int i = 0; i < poolSize_; i++){
           auto channel = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
           connections_.push(StatusService::NewStub(channel)); 
        }
    }

    ~StatusConPool(){
        std::lock_guard<std::mutex> mutex(mutex_);
        Close();
        while(!connections_.empty()){
            connections_.pop();
        }
    }

    std::unique_ptr<StatusService::Stub> getConnection(){
        std::unique_lock<std::mutex> mutex(mutex_);
        cond_.wait(mutex, [this](){return b_stop_ || !connections_.empty();});

        if(b_stop_) {   return nullptr; }

        auto conn = std::move(connections_.front());
        connections_.pop();
        return conn;
    }

    void returnConnection(std::unique_ptr<StatusService::Stub> conn){
        std::unique_lock<std::mutex> mutex(mutex_);
        if(b_stop_)
            return;
        connections_.push(std::move(conn)); 
        cond_.notify_one();
    }

    void Close(){
        b_stop_ = true;
        cond_.notify_all();
    }
    
private:
    std::atomic<bool> b_stop_;
    size_t poolSize_;
    std::string host_;
    std::string port_;
    std::queue<std::unique_ptr<StatusService::Stub>> connections_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

class StatusGrpcClient :public Singleton<StatusGrpcClient>
{
    friend class Singleton<StatusGrpcClient>;
public:
    ~StatusGrpcClient() {
    }
    GetChatServerRsp GetChatServer(int uid);
    LoginRsp Login(int uid, std::string token);
private:
    StatusGrpcClient();
    std::unique_ptr<StatusConPool> pool_;
};


#endif //CHATXTSERVER_STATUSGRPCCLIENT_H
