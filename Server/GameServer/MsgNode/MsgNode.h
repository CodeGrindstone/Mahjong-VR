//
// Created by 31435 on 2024/10/10.
//

#ifndef CHATXT_CHATSERVER_MSGNODE_H
#define CHATXT_CHATSERVER_MSGNODE_H


#pragma once
#include <string>
#include "../global.h"
#include <iostream>
#include <boost/asio.hpp>
using namespace std;
using boost::asio::ip::tcp;
class LogicSystem;
class MsgNode
{
public:
	MsgNode(short max_len) :_total_len(max_len), _cur_len(0) {
		_data = new char[_total_len + 1]();
		_data[_total_len] = '\0';
	}

	~MsgNode() {
		std::cout << "destruct MsgNode" << endl;
		delete[] _data;
	}

	void Clear() {
		::memset(_data, 0, _total_len);
		_cur_len = 0;
	}

	short _cur_len;
	short _total_len;
	char* _data;
};

class RecvNode :public MsgNode {
	friend class LogicSystem;
public:
	RecvNode(short max_len, short msg_id);
	void Reset(){ _msg_id = -1; }
	short GetMsgId() {	return _msg_id; }
	void SetMsgId(short _msg_id_) { _msg_id = _msg_id_; }
private:
	short _msg_id;
};

class SendNode:public MsgNode {
	friend class LogicSystem;
public:
	SendNode(const char* msg,short max_len, short msg_id);
private:
	short _msg_id;
};

#endif //CHATXT_CHATSERVER_MSGNODE_H
