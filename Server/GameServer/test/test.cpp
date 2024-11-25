#include <iostream>
#include <boost/asio.hpp>
#include <string>
#include "../global.h"
#include "../MsgNode/MsgNode.h"
#include <unistd.h>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include <functional>
using namespace boost;

#include <functional>
#include <queue>
#include <memory>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/uuid/uuid.hpp>            // for boost::uuids::uuid
#include <boost/uuid/uuid_generators.hpp> // for boost::uuids::random_generator
#include <boost/uuid/uuid_io.hpp>         // for outputting UUID as a string

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

void test(asio::ip::tcp::socket socket)
{
    while(1)
    {
        Json::Value root;
        short msgid;
        int field_num;
        std::cout << "MSG ID: ";
        std::cin >> msgid;
        std::cout << "field num: ";
        std::cin >> field_num;
        for(int i = 0; i < field_num; i++){
            std::string field;
            std::string value;
            std::cout << "field: ";
            std::cin >> field;
            std::cout << "value: ";
            std::cin >> value; 

            root[field] = value;
        }

        std::string message = root.toStyledString();

        SendNode sendnode(message.c_str(), message.length(), msgid);
        
        std::string message_send(sendnode._data, sendnode._total_len);

        asio::write(socket, asio::buffer(message, message.size()));
    }
}

int main() {
    try {

        // 创建 io_service 对象
        asio::io_context io_context;

        // 服务器的 IP 地址和端口
        std::string server_ip = "127.0.0.1";
        int server_port = 8090;

        // 创建 TCP socket
        asio::ip::tcp::socket socket(io_context);

        // 创建 endpoint（端点）
        asio::ip::tcp::endpoint endpoint(asio::ip::make_address(server_ip), server_port);

        // 连接到服务器
        socket.connect(endpoint);


        // test(socket);

    while(1)
    {
        Json::Value root;
        short msgid;
        int field_num;
        std::cout << "MSG ID: ";
        std::cin >> msgid;
        if(msgid == MSG_CHAT_LOGIN){
            root["uid"] = 4;
            root["token"] = "123-123-456-789";
        }else if(msgid == MSG_CREATE_ROOM){
            root["uid"] = 4;
            root["token"] = "123-123-456-789";
        }else if(msgid == MSG_ENTER_ROOM){
            root["uid"] = 4;
            root["token"] = "123-123-456-789";
            root["roomid"] = 0;
        }else if(msgid == MSG_QUIT_ROOM){
            root["uid"] = 4;
            root["token"] = "123-123-456-789";
            root["roomid"] = 0;
        }else if(msgid == MSG_CHANGE_READY){
            root["uid"] = 4;
            root["token"] = "123-123-456-789";
        }else if(msgid == MSG_START_GAME){
            root["uid"] = 4;
            root["token"] = "123-123-456-789"; 
            root["roomid"] = 0;
        }

        std::string message = root.toStyledString();
        std::cout << message << std::endl;
        
        SendNode sendnode(message.c_str(), message.length(), msgid);
        
        std::string message_send(sendnode._data, sendnode._total_len);

        asio::write(socket, asio::buffer(message_send, message_send.size()));
    }


        socket.close();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
