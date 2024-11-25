#pragma once

#include "Singleton.h"
#include "../CSession/CSession.h"
#include "../Player/Player.h"
#include "../Room/Room.h"
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include <memory>
#include <iostream>
#include <queue>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <map>
#include <thread>

typedef std::function<void (std::shared_ptr<CSession> session, short msg_id, const std::string& msg_data)> FunCallBack;

class LogicSystem : public Singleton<LogicSystem>
{
    friend class Singleton;
public:
    ~LogicSystem(){};
    void PostToQue(std::shared_ptr<LogicNode> msg_node);
    
private:
    LogicSystem();
    void RegisterCallBack();
    void QuitLogin(std::shared_ptr<CSession> session, const short msg_id, const string& msg_data);
    void LoginHandler(std::shared_ptr<CSession> session, const short msg_id, const string& msg_data);
    void CreateRoom(std::shared_ptr<CSession> session, const short msg_id, const string& msg_data);
    void EnterRoom(std::shared_ptr<CSession> session, const short msg_id, const string& msg_data);
    void QuitRoom(std::shared_ptr<CSession> session, const short msg_id, const string& msg_data);
    void ChangeReady(std::shared_ptr<CSession> session, const short msg_id, const string& msg_data);
    void StartGame(std::shared_ptr<CSession> session, const short msg_id, const string& msg_data);
    void DrawTitle(std::shared_ptr<CSession> session, const short msg_id, const string& msg_data);
    void DisTitle(std::shared_ptr<CSession> session, const short msg_id, const string& msg_data);

    void DealMsg();
    void BroadCast(std::shared_ptr<Room> RoomPtr, int uid, Json::Value& root, MSG_IDS msg_id);
    void BroadCastTiles(std::shared_ptr<GameRound> gamerd);
    bool WaitForMessageID(std::map<int, std::shared_ptr<CSession>> sessions, int timeoutSeconds);
    std::shared_ptr<Player> FindPlayer(int uid);
    std::shared_ptr<GameRound> FindGameRound(int roomid);
private:
    bool _b_stop;
    std::map<short, FunCallBack> m_Fun_Callbks;
    std::queue<std::shared_ptr<LogicNode>> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_con;
    std::thread m_work_thread;
    std::map<int, std::shared_ptr<Player>> m_users;
    std::map<int, std::shared_ptr<GameRound>> m_GameRounds;
};