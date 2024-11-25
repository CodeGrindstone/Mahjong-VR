#include "LogicSystem.h"
#include "../StatusGrpcClient/StatusGrpcClient.h"
#include "../MysqlDao/MysqlMgr.h"

void LogicSystem::PostToQue(std::shared_ptr<LogicNode> msg_node)
{
    std::lock_guard<std::mutex> mutex(m_mutex);
    m_queue.push(msg_node);

    m_con.notify_all();
}

LogicSystem::LogicSystem() : _b_stop(false)
{
    RegisterCallBack();
    m_work_thread = std::thread(&LogicSystem::DealMsg, this);
}

void LogicSystem::RegisterCallBack() {

    m_Fun_Callbks[MSG_QUIT] = std::bind(&LogicSystem::QuitLogin, this, 
    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

    m_Fun_Callbks[MSG_CHAT_LOGIN] = std::bind(&LogicSystem::LoginHandler, this, 
    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    
    m_Fun_Callbks[MSG_CREATE_ROOM] = std::bind(&LogicSystem::CreateRoom, this, 
    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

    m_Fun_Callbks[MSG_ENTER_ROOM] = std::bind(&LogicSystem::EnterRoom, this, 
    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

    m_Fun_Callbks[MSG_QUIT_ROOM] = std::bind(&LogicSystem::QuitRoom, this, 
    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    
    m_Fun_Callbks[MSG_CHANGE_READY] = std::bind(&LogicSystem::ChangeReady, this, 
    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

    m_Fun_Callbks[MSG_START_GAME] = std::bind(&LogicSystem::StartGame, this, 
    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    
    m_Fun_Callbks[MSG_DWAR_TITLE] = std::bind(&LogicSystem::DrawTitle, this, 
    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

    m_Fun_Callbks[MSG_DISCARD] = std::bind(&LogicSystem::DisTitle, this, 
    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}

void LogicSystem::QuitLogin(std::shared_ptr<CSession> session, const short msg_id, const string & msg_data)
{
    std::cout << "-------QuitLogin-------" << std::endl;
    auto uid = session->GetUserId();
    // 删除与uid有关的变量
    auto iter = m_users.find(uid);
    if(iter == m_users.end()){
        // 无法找到该玩家
    }

    Json::Value ReturnBroadC;
    ErrorCodes error;
    auto player = iter->second;
    std::shared_ptr<Room> RoomPtr = player->RoomPtr; 
    if(RoomPtr == nullptr){
        // 当前无加入房间
        m_users.erase(iter);
        return;
    }
    // 退出房间逻辑
    bool IsTrans = false;
    auto seat = RoomPtr->SeatPosition[uid];
    error = RoomManager::GetInstance()->QuitRoom(player, IsTrans);
    if(error == ErrorCodes::Success){
        // 广播消息
        // 1. 推出玩家
        ReturnBroadC["error"] = ErrorCodes::Success;
        Json::Value player_json(Json::objectValue);
        player_json["uid"] = player->PlayerId; 
        player_json["name"] = player->PlayerName; 
        player_json["seat"] = seat;
        ReturnBroadC["player"] = player_json;
        if(IsTrans){
            // 说明推出的是房主
            Json::Value player_json(Json::objectValue);
            auto host = RoomPtr->RoomHost;
            auto _uid = host->PlayerId;
            player_json["uid"] = _uid;
            player_json["name"] = host->PlayerName;
            player_json["seat"] = RoomPtr->SeatPosition[_uid];
            ReturnBroadC["is_trans"] = true;
            ReturnBroadC["new_host"] = player_json;
        }else{
            ReturnBroadC["is_trans"] = false;
        }
        BroadCast(RoomPtr, uid, ReturnBroadC, MSG_QUIT_ROOM_BROADC);
    }
    auto s = RoomPtr->m_sockets.find(uid);
    if(s != RoomPtr->m_sockets.end())   {
        RoomPtr->m_sockets.erase(s);
    }
    m_users.erase(iter);
}

void LogicSystem::LoginHandler(std::shared_ptr<CSession> session,
                               short const msg_id, string const &msg_data) {
    std::cout << "--------Login---------" << std::endl;
    Json::Reader reader;
    Json::Value root;
    reader.parse(msg_data, root);
    auto uid = root["uid"].asInt();
    std::cout << "receive msg: " << std::endl;
    std::cout << msg_data << std::endl;
#ifndef DEBUG
    auto reply = StatusGrpcClient::GetInstance()->Login(root["uid"].asInt(), root["token"].asString());
#endif
    Json::Value ReturnValue;
    Defer defer([this,&ReturnValue, session](){
        std::string return_str = ReturnValue.toStyledString();
    #ifdef DEBUG
        std::cout << return_str << std::endl;
    #endif  
        session->Send(return_str, MSG_CHAT_LOGIN_RSP); 
    });
#ifndef DEBUG
    ReturnValue["error"] = reply.error()m_sockets;
    if(reply.error() != ErrorCodes::Success){
        return;
    }
#endif
    // 内存中查找寻找信息
    auto find_iter = m_users.find(uid);
    std::shared_ptr<Player> player_info = nullptr;
    if(find_iter == m_users.end()){
        // 查询数据库
       player_info = MysqlMgr::GetInstance()->GetUser(uid); 
       if(player_info == nullptr){
            ReturnValue["error"] = ErrorCodes::UidInvalid;
            return;
       }
       m_users[uid] = player_info;
    }else{
        player_info = find_iter->second;
    }
    ReturnValue["uid"] = player_info->PlayerId;
#ifndef DEBUG
    ReturnValue["token"] = reply.token();
#else
    ReturnValue["token"] = root["token"];
#endif

    ReturnValue["error"] = ErrorCodes::Success;
    ReturnValue["name"] = player_info->PlayerName; 

    session->SetUserId(uid);
}

void LogicSystem::CreateRoom(std::shared_ptr<CSession> session, const short msg_id, const string & msg_data)
{
#ifdef DEBUG
    std::cout << "--------CreateRoom---------" << std::endl;
#endif
    Json::Reader reader;
    Json::Value root;
    reader.parse(msg_data, root);
    auto uid = root["uid"].asInt();
    std::cout << "receive msg: " << std::endl;
    std::cout << msg_data << std::endl;
#ifndef DEBUG
    auto reply = StatusGrpcClient::GetInstance()->Login(root["uid"].asInt(), root["token"].asString());
#endif
    Json::Value ReturnValue;
    Defer defer([this,&ReturnValue, session](){
        std::string return_str = ReturnValue.toStyledString();
    #ifdef DEBUG
        std::cout << "return_str msg:" << std::endl;
        std::cout << return_str << std::endl;
    #endif    
        session->Send(return_str, MSG_CREATE_ROOM_RSP); 
    });
#ifndef DEBUG
    ReturnValue["error"] = reply.error();
    if(reply.error() != ErrorCodes::Success){
        return;
    }
#endif

    // 根据客户端传递来的uid查找对应的Player
    std::shared_ptr<Player> player = FindPlayer(uid); 

    if(nullptr == player){
        ReturnValue["error"] = ErrorCodes::UidInvalid;
        return;
    }
    
    // 创建房间
    ReturnValue["error"] = RoomManager::GetInstance()->CreateRoom(player, session);
    if(ReturnValue["error"] = ErrorCodes::Success)
    {
        ReturnValue["roomid"] = player->RoomPtr->RoomId;
    }
}

void LogicSystem::EnterRoom(std::shared_ptr<CSession> session, const short msg_id, const string & msg_data)
{    
#ifdef DEBUG
    std::cout << "--------EnterRoom---------" << std::endl;
#endif
    Json::Reader reader;
    Json::Value root;
    reader.parse(msg_data, root);
    auto uid = root["uid"].asInt();
    auto RoomId = root["roomid"].asInt();
    std::cout << "receive msg: " << std::endl;
    std::cout << msg_data << std::endl;
#ifndef DEBUG
    auto reply = StatusGrpcClient::GetInstance()->Login(root["uid"].asInt(), root["token"].asString());
#endif
    Json::Value ReturnValue;
    Json::Value ReturnBroadC;
    ErrorCodes error;
    Defer defer([this,&ReturnValue, session](){
        std::string return_str = ReturnValue.toStyledString();
    #ifdef DEBUG
        std::cout << "return_str msg:" << std::endl;
        std::cout << return_str << std::endl;
    #endif    
        session->Send(return_str, MSG_ENTER_ROOM_RSP); 
    });
#ifndef DEBUG
    ReturnValue["error"] = reply.error();
    if(reply.error() != ErrorCodes::Success){
        return;
    }
#endif
   // 根据客户端传递来的uid查找对应的Player
    std::shared_ptr<Player> player = FindPlayer(uid); 

    if(nullptr == player){
        ReturnValue["error"] = ErrorCodes::UidInvalid;
        return;
    }
    
    // 进入房间逻辑
    error = RoomManager::GetInstance()->EnterRoom(RoomId, player, session);
    ReturnValue["error"] = error;
    if(ErrorCodes::Success == error){
        // 将当前房间内的玩家名字和房主发送给即将进入房间的玩家
        // 1. 当前房间玩家数量（算上自己）
        // 2. 当前房间房主
        // 3. 当前房间玩家（算上自己）
        std::shared_ptr<Room>& RoomPtr = player->RoomPtr;
        short playernum = RoomPtr->Players.size();  
        ReturnValue["playernum"] = playernum;
        ReturnValue["hostUid"] = RoomPtr->RoomHost->PlayerId;
        Json::Value PlayerList(Json::arrayValue);
        for(int i = 0; i < playernum; i++){
            Json::Value player(Json::objectValue);
            auto _uid = RoomPtr->Players[i]->PlayerId;
            player["uid"] = _uid; 
            player["name"] = RoomPtr->Players[i]->PlayerName;
            short seat = RoomPtr->SeatPosition[_uid];
            player["seat"] = seat;
            player["is_ready"] = RoomPtr->Players[i]->IsReady;
            PlayerList.append(player);
        }
        ReturnValue["members"] = PlayerList;

        // 将即将进入房间的玩家名字广播给房间内全体玩家（除了即将进入的玩家）
        ReturnBroadC["error"] = ErrorCodes::Success;
        Json::Value player_json(Json::objectValue);
        player_json["uid"] = player->PlayerId; 
        player_json["name"] = player->PlayerName; 
        player_json["seat"] = player->RoomPtr->SeatPosition[player->PlayerId];
        ReturnBroadC["player"] = player_json;
        BroadCast(RoomPtr, uid, ReturnBroadC, MSG_ENTER_ROOM_BROADC);
    }
}

void LogicSystem::QuitRoom(std::shared_ptr<CSession> session,
                           short const msg_id, string const &msg_data) 
{
#ifdef DEBUG
    std::cout << "--------QuitRoom---------" << std::endl;
#endif 
    Json::Reader reader;
    Json::Value root;
    reader.parse(msg_data, root);
    auto uid = root["uid"].asInt();
    auto RoomId = root["roomid"].asInt();
    std::cout << "receive msg: " << std::endl;
    std::cout << msg_data << std::endl;
#ifndef DEBUG
    auto reply = StatusGrpcClient::GetInstance()->Login(root["uid"].asInt(), root["token"].asString());
#endif
    Json::Value ReturnValue;
    Json::Value ReturnBroadC;
    ErrorCodes error;
    Defer defer([this,&ReturnValue, session](){
        std::string return_str = ReturnValue.toStyledString();
    #ifdef DEBUG
        std::cout << "return_str msg:" << std::endl;
        std::cout << return_str << std::endl;
    #endif  
        session->Send(return_str, MSG_QUIT_ROOM_RSP); 
    });
#ifndef DEBUG
    ReturnValue["error"] = reply.error();
    if(reply.error() != ErrorCodes::Success){
        return;
    }
#endif
    // 根据客户端传递来的uid查找对应的Player
    std::shared_ptr<Player> player = FindPlayer(uid); 

    if(nullptr == player){
        ReturnValue["error"] = ErrorCodes::UidInvalid;
        return;
    }
    std::shared_ptr<Room> RoomPtr = player->RoomPtr; 
    // 退出房间逻辑
    bool IsTrans = false; 
    auto seat = RoomPtr->SeatPosition[uid];
    error = RoomManager::GetInstance()->QuitRoom(player, IsTrans);
    ReturnValue["error"] = error;
    if(error == ErrorCodes::Success){
        // 广播消息
        // 1. 推出玩家
        ReturnBroadC["error"] = ErrorCodes::Success;
        Json::Value player_json(Json::objectValue);
        player_json["uid"] = player->PlayerId; 
        player_json["name"] = player->PlayerName; 
        player_json["seat"] = seat;
        ReturnBroadC["player"] = player_json;
        if(IsTrans){
            // 说明推出的是房主
            Json::Value player_json(Json::objectValue);
            auto host = RoomPtr->RoomHost;
            auto _uid = host->PlayerId;
            player_json["uid"] = _uid;
            player_json["name"] = host->PlayerName; 
            player_json["seat"] = RoomPtr->SeatPosition[_uid];
            ReturnBroadC["is_trans"] = true;
            ReturnBroadC["new_host"] = player_json;
        }else{
            ReturnBroadC["is_trans"] = false;
        }
        BroadCast(RoomPtr, uid, ReturnBroadC, MSG_QUIT_ROOM_BROADC);
    }
}

void LogicSystem::ChangeReady(std::shared_ptr<CSession> session,
                              short const msg_id, string const &msg_data)
{
#ifdef DEBUG
    std::cout << "--------ChangeReady----------" << std::endl;
#endif
    Json::Reader reader;
    Json::Value root;
    reader.parse(msg_data, root);
    auto uid = root["uid"].asInt();
    auto RoomId = root["roomid"].asInt();
    std::cout << "receive msg: " << std::endl;
    std::cout << msg_data << std::endl;
#ifndef DEBUG
    auto reply = StatusGrpcClient::GetInstance()->Login(root["uid"].asInt(), root["token"].asString());
#endif
    Json::Value ReturnValue;
    Json::Value ReturnBroadC;
    ErrorCodes error;
    Defer defer([this,&ReturnValue, session](){
        std::string return_str = ReturnValue.toStyledString();
    #ifdef DEBUG
        std::cout << "return_str msg: " << std::endl;
        std::cout << return_str << std::endl;
    #endif  
        session->Send(return_str, MSG_CHANGE_READY_RSP); 
    });
#ifndef DEBUG
    ReturnValue["error"] = reply.error();
    if(reply.error() != ErrorCodes::Success){
        return;
    }
#endif
    // 根据客户端传递来的uid查找对应的Player
    std::shared_ptr<Player> player = FindPlayer(uid); 

    if(nullptr == player){
        ReturnValue["error"] = ErrorCodes::UidInvalid;
        return;
    }
    
    // 改变准备状态逻辑
    error = RoomManager::GetInstance()->ChangeReady(player);
    ReturnValue["error"] = error;
    if(error == ErrorCodes::Success){
        // 广播消息
        // 1.准备好的玩家
        ReturnBroadC["error"] = error;
        ReturnBroadC["player"] = player->PlayerName;
        ReturnBroadC["seat"] = player->RoomPtr->SeatPosition[player->PlayerId];
        ReturnBroadC["isready"] = player->IsReady;
        BroadCast(player->RoomPtr, uid, ReturnBroadC, MSG_CHANGE_READY_BROADC);  
    }
}

void LogicSystem::StartGame(std::shared_ptr<CSession> session,
                            short const msg_id, string const &msg_data)
{
#ifdef DEBUG
    std::cout << "--------------StartGame----------------" << std::endl;
#endif
    Json::Reader reader;
    Json::Value root;
    reader.parse(msg_data, root);
    auto uid = root["uid"].asInt();
    auto RoomId = root["roomid"].asInt();
    std::cout << "receive msg: " << std::endl;
    std::cout << msg_data << std::endl;
#ifndef DEBUG
    auto reply = StatusGrpcClient::GetInstance()->Login(root["uid"].asInt(), root["token"].asString());
#endif
    Json::Value ReturnValue;
    ErrorCodes error;
#ifndef DEBUG
    ReturnValue["error"] = reply.error();
    if(reply.error() != ErrorCodes::Success){
        return;
    }
#endif
    // 根据客户端传递来的uid查找对应的Player
    std::shared_ptr<Player> player = FindPlayer(uid); 

    if(nullptr == player){
        ReturnValue["error"] = ErrorCodes::UidInvalid;
        return;
    }
    
    // 开始游戏
    // 改变房间内玩家状态
    error = RoomManager::GetInstance()->StartGame(RoomId);
    ReturnValue["error"] = error;
    if(error == ErrorCodes::Success){
        std::shared_ptr<Room>& room = player->RoomPtr;
        std::shared_ptr<GameRound> gamerd = std::make_shared<GameRound>(room);
        m_GameRounds[room->RoomId] = gamerd;
        // 在服务端初始化牌堆和各玩家手牌
        gamerd->start();

        // 广播游戏开始
        // 将各玩家手牌发送出去
        BroadCastTiles(gamerd);
    }else{
        // 错误将单独向房主发送回包
        std::string return_str = ReturnValue.toStyledString();
    #ifdef DEBUG
        std::cout << return_str << std::endl;
    #endif  
        session->Send(return_str, MSG_START_GAME_RSP); 
    }
}

void LogicSystem::DealMsg() {
    try{
        for(;;){

        std::unique_lock<std::mutex> mutex(m_mutex);

        m_con.wait(mutex, [this](){ return _b_stop || !m_queue.empty();});

        if(_b_stop){
            // 停止
            while(!m_queue.empty()){ 
                auto msg_node = m_queue.front();
                m_queue.pop();
                // 逻辑处理
                std::cout << "recv_msg id is " << msg_node->m_recvnode->_msg_id << std::endl;
                // 根据不同id进行不同操作
                m_Fun_Callbks[msg_node->m_recvnode->_msg_id](msg_node->m_session, msg_node->m_recvnode->_msg_id, msg_node->m_recvnode->_data);
                // 将CSession中的消息节点设置为已读
			    msg_node->m_recvnode->Reset();
            }
            break;
        } 

        auto msg_node = m_queue.front();
        m_queue.pop();
        // 逻辑处理
        std::cout << "recv_msg id is " << msg_node->m_recvnode->_msg_id << std::endl;
        // 根据不同id进行不同操作
        m_Fun_Callbks[msg_node->m_recvnode->_msg_id](msg_node->m_session, msg_node->m_recvnode->_msg_id, msg_node->m_recvnode->_data);
        // 将CSession中的消息节点设置为已读
        msg_node->m_recvnode->Reset();
    }
    }catch (std::exception& e) {
			std::cout << "Exception code is " << e.what() << std::endl;
	}
}

void LogicSystem::DrawTitle(std::shared_ptr<CSession> session, const short msg_id, const string & msg_data)
{
#ifdef DEBUG
    std::cout << "------------DrawTitle----------" << std::endl;
#endif
    Json::Reader reader;
    Json::Value root;
    reader.parse(msg_data, root);
    auto uid = root["uid"].asInt();
    auto RoomId = root["roomid"].asInt();
    std::cout << "receive msg: " << std::endl;
    std::cout << msg_data << std::endl;
#ifndef DEBUG
    auto reply = StatusGrpcClient::GetInstance()->Login(root["uid"].asInt(), root["token"].asString());
#endif
    Json::Value ReturnValue;
    Json::Value ReturnBroadC;
    ErrorCodes error;
    // 超时检测是否有碰、杠操作
    
    Defer defer([this,&ReturnValue, session](){
        std::string return_str = ReturnValue.toStyledString();
    #ifdef DEBUG
        std::cout << "return_str msg: " << std::endl;
        std::cout << return_str << std::endl;
    #endif  
        session->Send(return_str, MSG_DWAR_TITLE_RSP); 
    });
#ifndef DEBUG
    ReturnValue["error"] = reply.error();
    if(reply.error() != ErrorCodes::Success){
        return;
    }
#endif
    // 判断是否有碰杠操作
    // 根据客户端传递来的uid查找对应的Player
    std::shared_ptr<Player> player = FindPlayer(uid); 
    if(nullptr == player){
        ReturnValue["error"] = ErrorCodes::UidInvalid;
        return;
    }
    // 查找GameRound
    std::shared_ptr<GameRound> gamerd = FindGameRound(RoomId);
    if(nullptr == gamerd){
        ReturnValue["error"] = ErrorCodes::NotFoundRoom;
        return;
    }

    // 判断是否有碰杠操作
    if(!WaitForMessageID(gamerd->RoomPtr->m_sockets, uid)){
        // 有人碰杠
        ReturnValue["error"] = ErrorCodes::CANTDRAWTILE;
        return;
    }
    // 实现摸牌
    MahjongTile tile = gamerd->DrawTitle(uid);
    error = ErrorCodes::Success;
    ReturnValue["error"] = error;
    if(error == ErrorCodes::Success){
        // 广播消息
        // 1.操作玩家ID
        // 2.实现动作 
        ReturnBroadC["error"] = error;
        ReturnBroadC["player"] = player->PlayerName;
        ReturnBroadC["action"] = Action::Draw;
        BroadCast(player->RoomPtr, uid, ReturnBroadC, MSG_DWAR_TITLE_BROADC);  
        // 返回状态
        // 1.麻将类型
        ReturnValue["id"] = tile.id;
        ReturnValue["value"] = tile.value;
        if(tile.value == 0){
            ReturnValue["ZiCard"] = tile.ziCard;
        }else{
            ReturnValue["suit"] = tile.suit;
        }
    }
}

void LogicSystem::DisTitle(std::shared_ptr<CSession> session,
                           short const msg_id, string const &msg_data)
{
#ifdef DEBUG
    std::cout << "------------DisTitle----------" << std::endl;
#endif
    Json::Reader reader;
    Json::Value root;
    reader.parse(msg_data, root);
    auto uid = root["uid"].asInt();
    auto RoomId = root["roomid"].asInt();
    auto CardId = root["id"].asInt();
    std::cout << "receive msg: " << std::endl;
    std::cout << msg_data << std::endl;
#ifndef DEBUG
    auto reply = StatusGrpcClient::GetInstance()->Login(root["uid"].asInt(), root["token"].asString());
#endif
    Json::Value ReturnValue;
    Json::Value ReturnBroadC;
    ErrorCodes error;
    Defer defer([this,&ReturnValue, session](){
        std::string return_str = ReturnValue.toStyledString();
    #ifdef DEBUG
        std::cout << "return_str msg: " << std::endl;
        std::cout << return_str << std::endl;
    #endif  
        session->Send(return_str, MSG_DISCARD_RSP); 
    });
#ifndef DEBUG
    ReturnValue["error"] = reply.error();
    if(reply.error() != ErrorCodes::Success){
        return;
    }
#endif
    // 根据客户端传递来的uid查找对应的Player
    std::shared_ptr<Player> player = FindPlayer(uid); 
    if(nullptr == player){
        ReturnValue["error"] = ErrorCodes::UidInvalid;
        return;
    }
    // 查找GameRound
    std::shared_ptr<GameRound> gamerd = FindGameRound(RoomId);
    if(nullptr == gamerd){
        ReturnValue["error"] = ErrorCodes::NotFoundRoom;
        return;
    }

    // 实现弃牌
    MahjongTile DisTile;
    error = gamerd->DisCard(uid, CardId, DisTile);
    ReturnValue["error"] = error;
    if(error == ErrorCodes::Success){
        // 广播消息
        // 1.操作玩家ID
        // 2.实现动作 
        // 3.弃牌的类型
        ReturnBroadC["error"] = error;
        ReturnBroadC["player"] = player->PlayerName;
        ReturnBroadC["action"] = Action::DisCard;
        Json::Value Tile(Json::objectValue);
        Tile["id"] = DisTile.id;
        Tile["value"] = DisTile.value;
        if(DisTile.value == 0){
            Tile["ZiCard"] = DisTile.ziCard;
        }else{
            Tile["suit"] = DisTile.suit;
        }
        ReturnBroadC["card"] = Tile;
        BroadCast(player->RoomPtr, uid, ReturnBroadC, MSG_DWAR_TITLE_BROADC);  
        // 返回状态
        // 1.错误类型
    }
}

void LogicSystem::BroadCast(std::shared_ptr<Room> RoomPtr, int uid, Json::Value & root, MSG_IDS msg_id)
{
    // 向房间的其他玩家发送广播
    for(const auto& it : RoomPtr->m_sockets){
        if(uid == it.first) continue;
        std::string msg = root.toStyledString();
    #ifdef DEBUG
        std::cout << "broad cast msg: " << std::endl;
        std::cout << msg << std::endl;
    #endif
        it.second->Send(msg, msg_id);
    }
}

void LogicSystem::BroadCastTiles(std::shared_ptr<GameRound> gamerd) 
{
    // 向游戏内的所有玩家发送麻将牌
    for(int i = 0; i < gamerd->players_uid.size(); i++){
        Json::Value ReturnMsg;
        ReturnMsg["error"] = ErrorCodes::Success;
        int uid = gamerd->players_uid[i]; //玩家UId
        Json::Value TilesList(Json::arrayValue);
        std::list<MahjongTile>& HandTiles = gamerd->HandTiles[uid]; // 玩家的手牌
        auto iter = HandTiles.begin();
        for(;iter != HandTiles.end(); iter++){
            Json::Value Tile(Json::objectValue);
            Tile["id"] = iter->id;
            Tile["value"] = iter->value;
            if(iter->value == 0){
                Tile["ZiCard"] = iter->ziCard;
            }else{
                Tile["suit"] = iter->suit;
            }
            TilesList.append(Tile);
        }
        ReturnMsg["uid"] = uid;
        ReturnMsg["tilenum"] = HandTiles.size();
        ReturnMsg["HandTiles"] = TilesList; 
        ReturnMsg["winner"] = gamerd->winner;

        auto SessionIter = gamerd->RoomPtr->m_sockets.find(uid);
        if(SessionIter == gamerd->RoomPtr->m_sockets.end()){
            // 有玩家失去连接
            continue;
        }
        std::string msg = ReturnMsg.toStyledString();
    #ifdef DEBUG
        std::cout << "broad cast msg: " << std::endl;
        std::cout << msg << std::endl;
    #endif
        SessionIter->second->Send(msg, MSG_START_GAME_BROADC);
    }
}

bool LogicSystem::WaitForMessageID(std::map<int, std::shared_ptr<CSession>> sessions, int uid)
{
    auto iter = sessions.begin();
    for(; iter != sessions.end(); iter++){
        if(uid == iter->first)  continue;
        std::shared_ptr<RecvNode>& recvnode = iter->second->ReturnRecvNode();
        if(recvnode->_msg_id == MSG_PUNG || recvnode->_msg_id == MSG_GUNG){
            return false;
        }
    }
    return true;
}

std::shared_ptr<Player> LogicSystem::FindPlayer(int uid)
{
    auto iter = m_users.find(uid); 
    if(iter == m_users.end()){
        return nullptr;
    }
    return iter->second;
}

std::shared_ptr<GameRound> LogicSystem::FindGameRound(int roomid)
{
    auto iter = m_GameRounds.find(roomid); 
    if(iter == m_GameRounds.end()){
        return nullptr;
    }
    return iter->second;
}