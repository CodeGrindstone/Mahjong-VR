#include "TCPManager.h"
#include <cstring>

uint16_t TCPManager::hostToNetworkShort(uint16_t value) {
    return htons(value); // 转换为网络字节序
}

uint16_t TCPManager::networkToHostShort(uint16_t value) {
    return ntohs(value); // 转换为主机字节序
}

void TCPManager::connect(const std::string& host, uint16_t port) {
    asio::ip::tcp::resolver resolver(context_);
    auto endpoints = resolver.resolve(host, std::to_string(port));

    asio::async_connect(socket_, endpoints,
        [this](std::error_code ec, asio::ip::tcp::endpoint) {
            if (!ec) {
                std::cout << "Connected to server.\n";
                startReceive();
            } else {
                std::cerr << "Failed to connect: " << ec.message() << '\n';
            }
        });
}

void TCPManager::startReceive() {
    AsyncReadHead(HEAD_TOTAL_LEN);
}

void TCPManager::handleMessage(short msgId) {
    auto it = messageHandlers_.find(msgId);
    if (it != messageHandlers_.end()) {
        it->second(msgId, _recv_msg_node->_data);
    } else {
        std::cerr << "No handler for message ID: " << msgId << '\n';
    }
}

void TCPManager::sendMessage(uint16_t msgId, const std::vector<char>& message) {
    uint16_t msgLen = static_cast<uint16_t>(message.size());
    uint16_t networkMsgId = hostToNetworkShort(msgId);
    uint16_t networkMsgLen = hostToNetworkShort(msgLen);

    std::vector<char> data(4 + msgLen);
    std::memcpy(&data[0], &networkMsgId, 2);
    std::memcpy(&data[2], &networkMsgLen, 2);
    std::copy(message.begin(), message.end(), data.begin() + 4);

    asio::async_write(socket_, asio::buffer(data),
        [](std::error_code ec, std::size_t /*length*/) {
            if (ec) {
                std::cerr << "Failed to send message: " << ec.message() << '\n';
            }
        });
}

void TCPManager::registerMessageHandler(short msgId, MessageHandler handler) {
    messageHandlers_[msgId] = handler;
}

void TCPManager::AsyncReadBody(int total_len)
{
    auto self = shared_from_this();
	asyncReadFull(total_len, [self, this, total_len](const boost::system::error_code& ec, std::size_t bytes_transfered) {
		try {
			if (ec) {
				std::cout << "handle read failed, error is " << ec.message() << std::endl;
                socket_.close();
				return;
			}

			if (bytes_transfered < total_len) {
				std::cout << "read length not match, read [" << bytes_transfered << "] , total ["
					<< total_len<<"]" << std::endl;
                socket_.close();
				return;
			}

			memcpy(_recv_msg_node->_data , _data , bytes_transfered);
			_recv_msg_node->_cur_len += bytes_transfered;
			_recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
            handleMessage(msgId);
            // 根据msgid处理消息
			AsyncReadHead(HEAD_TOTAL_LEN);
		}
		catch (std::exception& e) {
			std::cout << "Exception code is " << e.what() << std::endl;
		}
		});
}

//读取完整长度
void TCPManager::asyncReadFull(std::size_t maxLength, std::function<void(const boost::system::error_code&, std::size_t)> handler )
{
	::memset(_data, 0, MAX_LENGTH);
	asyncReadLen(0, maxLength, handler);
}

//读取指定字节数
void TCPManager::asyncReadLen(std::size_t read_len, std::size_t total_len, 
	std::function<void(const boost::system::error_code&, std::size_t)> handler)
{
    auto self = shared_from_this();
	socket_.async_read_some(boost::asio::buffer(_data + read_len, total_len-read_len),
		[self, read_len, total_len, handler](const boost::system::error_code& ec, std::size_t  bytesTransfered) {
			if (ec) {
				// 出现错误，调用回调函数
				handler(ec, read_len + bytesTransfered);
				return;
			}

			if (read_len + bytesTransfered >= total_len) {
				//长度够了就调用回调函数
				handler(ec, read_len + bytesTransfered);
				return;
			}

			// 没有错误，且长度不足则继续读取
			self->asyncReadLen(read_len + bytesTransfered, total_len, handler);
	});
}

void TCPManager::AsyncReadHead(int total_len)
{
	auto self = shared_from_this();
	asyncReadFull(HEAD_TOTAL_LEN, [self, this](const boost::system::error_code& ec, std::size_t bytes_transfered) {
		try {
			if (ec) {
				std::cout << "handle read failed, error is " << ec.message() << std::endl;
                socket_.close();
				return;
			}

			if (bytes_transfered < HEAD_TOTAL_LEN) {
				std::cout << "read length not match, read [" << bytes_transfered << "] , total ["
					<< HEAD_TOTAL_LEN << "]" << endl;
                socket_.close();
				return;
			}

			_recv_head_node->Clear();
			memcpy(_recv_head_node->_data, _data, bytes_transfered);

			//获取头部MSGID数据
			short msg_id = 0;
			memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);
			//网络字节序转化为本地字节序
			msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
			std::cout << "msg_id is " << msg_id << endl;
            msgId = msg_id;
			//id非法
			if (msg_id > MAX_LENGTH) {
				std::cout << "invalid msg_id is " << msg_id << endl;
				return;
			}
			short msg_len = 0;
			memcpy(&msg_len, _recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);
			//网络字节序转化为本地字节序
			msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
			std::cout << "msg_len is " << msg_len << endl;

			//id非法
			if (msg_len > MAX_LENGTH) {
				std::cout << "invalid data length is " << msg_len << std::endl;
				return;
			}

			_recv_msg_node = make_shared<RecvNode>(msg_len, msg_id);
			AsyncReadBody(msg_len);
		}
		catch (std::exception& e) {
			std::cout << "Exception code is " << e.what() << std::endl;
		}
		});
}