cmake_minimum_required(VERSION 3.5)

add_definitions(-DDEBUG)

project(Chatxt_chatserver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${Protobuf_VERSION}")

set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)


# Find gRPC installation
# Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")                                                                      

set(_GRPC_GRPCPP gRPC::grpc++)


set(AsioIOServicePool
AsioIOServicePool/AsioIOServicePool.cpp
AsioIOServicePool/AsioIOServicePool.h
)

set(ConfigMgr
ConfigMgr/ConfigMgr.h
ConfigMgr/ConfigMgr.cpp
)

set(CServer
CServer/CServer.h
CServer/CServer.cpp
)

set(CSession
CSession/CSession.h
CSession/CSession.cpp)

set(LogicSystem
LogicSystem/LogicSystem.cpp)

set(MsgNode
MsgNode/MsgNode.h
MsgNode/MsgNode.cpp)

set(MysqlDao
MysqlDao/MysqlDao.cpp
MysqlDao/MysqlMgr.cpp)

set(protoc
protoc/message.grpc.pb.cc
protoc/message.pb.cc)

set(StatusGrpcClient
StatusGrpcClient/StatusGrpcClient.cpp)

set(Room
Room/Room.cpp)

add_executable(GameServer
        main.cpp
        ${ConfigMgr}
        ${AsioIOServicePool}
        ${CServer}
        ${CSession}
        ${MsgNode}
        ${LogicSystem}
        ${MysqlDao}
        ${protoc}
        ${RedisMgr}
        ${StatusGrpcClient}
        ${Room}
)

add_executable(TEST
        test/test_main.cpp
        test/TCPManager.cpp
        ${MsgNode}
)

target_link_libraries(TEST
        jsoncpp                                                                                                   
)
add_executable(TEST2
        test/test2.cpp
        ${MsgNode}
        test/TCPManager.cpp
)

target_link_libraries(TEST2
        jsoncpp                                                                                                   
)
add_executable(TEST3
        test/test3.cpp
        ${MsgNode}
        test/TCPManager.cpp
)

target_link_libraries(TEST3
        jsoncpp                                                                                                   
)
add_executable(TEST4
test/test4.cpp
${MsgNode}
test/TCPManager.cpp
)

target_link_libraries(TEST4
jsoncpp                                                                                                   
)

add_executable(TEST5
test/test5.cpp
${MsgNode}
test/TCPManager.cpp
)

target_link_libraries(TEST5
jsoncpp                                                                                                   
)

SET(EXECUTABLE_OUTPUT_PATH ../)

target_link_libraries(GameServer
        jsoncpp                                                                                                   
        boost_filesystem
        ${_GRPC_GRPCPP}
        ${_PROTOBUF_LIBPROTOBUF}
        mysqlcppconn
)
