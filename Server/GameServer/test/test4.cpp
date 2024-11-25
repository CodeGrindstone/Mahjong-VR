#include "TCPManager.h"
#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <string>
#include "../global.h"
#include <json/value.h>

int roomid;
std::string parsemsg(MSG_IDS msgid)
{
    Json::Value root;
    if(msgid == MSG_CHAT_LOGIN){
            root["uid"] = 7;
            root["token"] = "123-123-456-789";
        }else if(msgid == MSG_CREATE_ROOM){
            root["uid"] = 7;
            root["token"] = "123-123-456-789";
        }else if(msgid == MSG_ENTER_ROOM){
            root["uid"] = 7;
            root["token"] = "123-123-456-789";
            std::cout << "请输入房间ID: ";
            std::cin >> roomid;
            root["roomid"] = roomid;
        }else if(msgid == MSG_QUIT_ROOM){
            root["uid"] = 7;
            root["token"] = "123-123-456-789";
            root["roomid"] = 0;
        }else if(msgid == MSG_CHANGE_READY){
            root["uid"] = 7;
            root["token"] = "123-123-456-789";
        }else if(msgid == MSG_START_GAME){
            root["uid"] = 7;
            root["token"] = "123-123-456-789"; 
            root["roomid"] = roomid;
        }else if(msgid == MSG_DWAR_TITLE){
            root["uid"] = 7;
            root["token"] = "123-123-456-789"; 
            root["roomid"] = roomid;
        }else if(msgid == MSG_DISCARD) {
            root["uid"] = 7;
            root["token"] = "123-123-456-789";
            root["roomid"] = roomid;
            int discard;
            std::cout << "请输入要扔的牌的id: ";
            std::cin >> discard;
            root["id"] = discard;
        }
    std::string message = root.toStyledString();
    return message;
    
}


void register_(std::shared_ptr<TCPManager> manager)
{
    manager->registerMessageHandler(MSG_CHAT_LOGIN_RSP, [](short msgid, const std::string& message ) {
            std::cout << "----------MSG_CHAT_LOGIN_RSP----------" << std :: endl;
            std::cout << message << std::endl << std::endl;
    });

    manager->registerMessageHandler(MSG_CREATE_ROOM_RSP, [](short msgid, const std::string& message ) {
            std::cout << "----------MSG_CREATE_ROOM_RSP----------" << std :: endl;
            std::cout << message << std::endl << std::endl;
    });

    manager->registerMessageHandler(MSG_ENTER_ROOM_RSP, [](short msgid, const std::string& message ) {
            std::cout << "----------MSG_ENTER_ROOM_RSP----------" << std :: endl;
            std::cout << message << std::endl << std::endl;
    });
    
    manager->registerMessageHandler(MSG_ENTER_ROOM_BROADC, [](short msgid, const std::string& message ) {
            std::cout << "----------MSG_ENTER_ROOM_BROADC----------" << std :: endl;
            std::cout << message << std::endl << std::endl;
    });

    manager->registerMessageHandler(MSG_QUIT_ROOM_RSP, [](short msgid, const std::string& message ) {
            std::cout << "----------MSG_QUIT_ROOM_RSP----------" << std :: endl;
            std::cout << message << std::endl << std::endl;
    });

    manager->registerMessageHandler(MSG_QUIT_ROOM_BROADC, [](short msgid, const std::string& message ) {
            std::cout << "----------MSG_QUIT_ROOM_BROADC----------" << std :: endl;
            std::cout << message << std::endl << std::endl;
    });

    manager->registerMessageHandler(MSG_CHANGE_READY_RSP, [](short msgid, const std::string& message ) {
            std::cout << "----------MSG_CHANGE_READY_RSP----------" << std :: endl;
            std::cout << message << std::endl << std::endl;
    });

    manager->registerMessageHandler(MSG_CHANGE_READY_BROADC, [](short msgid, const std::string& message ) {
            std::cout << "----------MSG_CHANGE_READY_BROADC----------" << std :: endl;
            std::cout << message << std::endl << std::endl;
    });

    manager->registerMessageHandler( MSG_START_GAME_BROADC, [](short msgid, const std::string& message ) {
            std::cout << "----------MSG_START_GAME_BROADC----------" << std :: endl;
            std::cout << message << std::endl << std::endl;
    });

    manager->registerMessageHandler( MSG_DWAR_TITLE_RSP, [](short msgid, const std::string& message ) {
            std::cout << "----------MSG_DWAR_TITLE_RSP----------" << std :: endl;
            std::cout << message << std::endl << std::endl;
    });

    manager->registerMessageHandler(MSG_DWAR_TITLE_BROADC, [](short msgid, const std::string& message ) {
            std::cout << "----------MSG_DWAR_TITLE_BROADC----------" << std :: endl;
            std::cout << message << std::endl << std::endl;
    });
    
    manager->registerMessageHandler(MSG_DISCARD_RSP, [](short msgid, const std::string& message ) {
            std::cout << "----------MSG_DISCARD_RSP----------" << std :: endl;
            std::cout << message << std::endl << std::endl;
    });

    manager->registerMessageHandler(MSG_DISCARD_BROADC, [](short msgid, const std::string& message ) {
            std::cout << "----------MSG_DISCARD_BROADC----------" << std :: endl;
            std::cout << message << std::endl << std::endl;
    });
}

int main() {
    try {
        asio::io_context context;

        std::shared_ptr<TCPManager> manager = std::make_shared<TCPManager>(context);

        register_(manager);

        manager->connect("127.0.0.1", 8090);

        // 启动异步接收和发送逻辑
        std::thread ioThread([&context]() { context.run(); });

        // 主线程用于模拟用户输入
        while (true) {
            short msgid;
            std::cin >> msgid;

            std::string msg = parsemsg(static_cast<MSG_IDS>(msgid));

            manager->sendMessage(msgid, std::vector<char>(msg.begin(), msg.end()));
        }

        context.stop();
        ioThread.join();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << '\n';
    }

    return 0;
}
