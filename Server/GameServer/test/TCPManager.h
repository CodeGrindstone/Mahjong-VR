#ifndef TCP_MANAGER_H
#define TCP_MANAGER_H

#include <boost/asio.hpp>
#include <functional>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <queue>
#include <memory>
#include "../MsgNode/MsgNode.h"

using namespace boost;

class TCPManager : public std::enable_shared_from_this<TCPManager> 
{
public:
    using MessageHandler = std::function<void(short, const std::string& msg_data)>;

    TCPManager(asio::io_context& context)
        : socket_(context), context_(context) {
            _recv_head_node = make_shared<MsgNode>(HEAD_TOTAL_LEN);
        }

    // 连接到服务器
    void connect(const std::string& host, uint16_t port);
    void startReceive();
    // 发送消息
    void sendMessage(uint16_t msgId, const std::vector<char>& message);

    // 注册消息处理器
    void registerMessageHandler(short, MessageHandler handler);

    // 读取消息头
    void AsyncReadBody(int total_len);
    void AsyncReadHead(int total_len);
    //读取完整长度
    void asyncReadFull(std::size_t maxLength, std::function<void(const boost::system::error_code&, std::size_t)> handler);

    //读取指定字节数
    void asyncReadLen(std::size_t read_len, std::size_t total_len, 
	    std::function<void(const boost::system::error_code&, std::size_t)> handler);

private:
    // 处理接收到的消息
    void handleMessage(short);

    // 处理发送队列
    void processSendQueue();

    // 转换主机字节序到网络字节序
    uint16_t hostToNetworkShort(uint16_t value);

    // 转换网络字节序到主机字节序
    uint16_t networkToHostShort(uint16_t value);

    asio::ip::tcp::socket socket_;
    asio::io_context& context_;

    // 消息处理器
    std::unordered_map<short, MessageHandler> messageHandlers_;
    
    char _data[MAX_LENGTH];
    //收到的消息结构
	std::shared_ptr<RecvNode> _recv_msg_node;
    short msgId;
	bool _b_head_parse;
	//收到的头部结构
	std::shared_ptr<MsgNode> _recv_head_node;

    // 消息头缓冲区
    std::vector<char> headerBuffer_ = std::vector<char>(4); // 2字节消息ID + 2字节消息长度
    std::vector<char> bodyBuffer_; // 消息体缓冲区

    // 发送队列
    std::queue<std::vector<char>> sendQueue_;
    bool sending_ = false;
};

#endif // TCP_MANAGER_H
