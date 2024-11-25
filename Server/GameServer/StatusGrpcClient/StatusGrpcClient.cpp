//
// Created by 31435 on 2024/9/24.
//

#include "StatusGrpcClient.h"

GetChatServerRsp StatusGrpcClient::GetChatServer(int uid)
{
    ClientContext context;
    GetChatServerRsp reply;
    GetChatServerReq request;
    request.set_uid(uid);
    auto stub = pool_->getConnection();
    Status status = stub->GetChatServer(&context, request, &reply);

    Defer defer([&stub, this]() {
        pool_->returnConnection(std::move(stub));
    });

    if (status.ok()) {
        return reply;
    }
    else {
        reply.set_error(ErrorCodes::RPCFailed);
        // 获取错误码
        grpc::StatusCode errorCode = status.error_code();
        // 获取错误消息
        std::string errorMessage = status.error_message();
        // 获取错误详情
        std::string errorDetails = status.error_details();

        std::cout << "RPC failed with error code: " << errorCode << std::endl;
        std::cout << "Error message: " << errorMessage << std::endl;

        // 如果有错误详情，输出详细信息
        if (!errorDetails.empty()) {
            std::cout << "Error details: " << errorDetails << std::endl;
        } 
        return reply;
    }
}

LoginRsp StatusGrpcClient::Login(int uid, std::string token)
{
    ClientContext context;
    LoginReq request;
    LoginRsp reply;
    request.set_uid(uid);
    request.set_token(token);
    auto stub = pool_->getConnection();
    Status status = stub->Login(&context, request, &reply);

    Defer defer([&stub, this]() {
        pool_->returnConnection(std::move(stub));
    });

    if (status.ok()) {
        return reply;
    }else {
        reply.set_error(ErrorCodes::RPCFailed);
        // 获取错误码
        grpc::StatusCode errorCode = status.error_code();
        // 获取错误消息
        std::string errorMessage = status.error_message();
        // 获取错误详情
        std::string errorDetails = status.error_details();

        std::cout << "RPC failed with error code: " << errorCode << std::endl;
        std::cout << "Error message: " << errorMessage << std::endl;

        // 如果有错误详情，输出详细信息
        if (!errorDetails.empty()) {
            std::cout << "Error details: " << errorDetails << std::endl;
        } 
        return reply;
    }
}
StatusGrpcClient::StatusGrpcClient()
{
    auto& gCfgMgr = ConfigMgr::getInstance();
    std::string host = gCfgMgr["StatusServer"]["Host"];
    std::string port = gCfgMgr["StatusServer"]["Port"];
#if DEBUG
    std::cout << "host: " << host << "\tport: " << port << std::endl;
#endif
    pool_.reset(new StatusConPool(5, host, port));
}