#include "Room.h"
#include <iostream>
#include <random>

Room::Room(int roomid, std::shared_ptr<Player> host) : 
RoomId(roomid), RoomHost(host),CurrentRound(0)
{
    SeatPosition[host->PlayerId] = 0;
    IsSeat.reserve(4);
    for(int i = 0; i < 4; i++)  IsSeat[i] = false; 
    IsSeat[0] = true;
}

short Room::FindPostion()
{
    for(int i = 0; i < 4; i++)
    {
        if(!IsSeat[i])  return i;
    }
    return -1;
}

Room::~Room()
{
    std::cout << "Room destructed!" << std::endl;
}


RoomManager::RoomManager() : m_RoomNum(0){}

ErrorCodes RoomManager::CreateRoom(std::shared_ptr<Player> host, std::shared_ptr<CSession> session) 
{
    std::lock_guard<std::mutex> mutex(m_mutex);
    std::shared_ptr<Room> room = std::make_shared<Room>(m_RoomNum, host);
    RoomMap.insert(std::pair<int, std::shared_ptr<Room>>(m_RoomNum, room)); 
    m_RoomNum++;
    
    room->RoomHost = host;
    room->Players.push_back(host);
    host->IsHost = true;
    host->RoomPtr = room;
    room->m_sockets[host->PlayerId] = session;
    return ErrorCodes::Success;
}

ErrorCodes RoomManager::EnterRoom(int RoomId, std::shared_ptr<Player> player, std::shared_ptr<CSession> session)
{
    std::lock_guard<std::mutex> mutex(m_mutex);
    auto iter = RoomMap.find(RoomId);
    if (iter == RoomMap.end()){
        std::cout << "房间ID为" << RoomId << "不存在！" << std::endl;
        return ErrorCodes::NotFoundRoom;
    }else{
        if(iter->second->Players.size() < 4){
            bool flag = true;
            for(auto iter_ = iter->second->Players.begin(); iter_ != iter->second->Players.end(); iter_++){
                if(iter_->get()->PlayerId == player->PlayerId){
                    flag = false;
                    break;
                }
            }
            if(!flag){
                std::cout << "Player id 已经加入房间！" << std::endl;
                return ErrorCodes::AlreadyJoinRoom;
            }


            std::cout << "Player id " << player->PlayerId << "进入房间！" << std::endl;
            player->RoomPtr = iter->second;
            iter->second->Players.push_back(player);
            player->RoomPtr->m_sockets[player->PlayerId] = session; 
            // 找到座位
            short seat = player->RoomPtr->FindPostion();
            if(-1 == seat)  return ErrorCodes::CantEnterRoom;
            player->RoomPtr->IsSeat[seat] = true;
            player->RoomPtr->SeatPosition[player->PlayerId] = seat;
            return ErrorCodes::Success;
        }
        else{
            std::cout << "房间已满！" << std::endl;
            return ErrorCodes::CantEnterRoom;
        }
    }
}

ErrorCodes RoomManager::QuitRoom(std::shared_ptr<Player> player, bool& Istrans)
{
    std::lock_guard<std::mutex> mutex(m_mutex);
    // 房间指针
    std::shared_ptr<Room> RoomPtr = player->RoomPtr;
    // 判断是否为最后一个用户在房间
    if(RoomPtr->Players.size() == 1){
        // 直接销毁房间
        player->IsHost = false;
        player->RoomPtr = nullptr;
        DestructRoom(RoomPtr->RoomId);
    }
    else
    {
        // 需要将位置让出来
        auto iter = player->RoomPtr->SeatPosition.find(player->PlayerId);
        short seat = iter->second; 
        player->RoomPtr->IsSeat[seat] = false;
        player->RoomPtr->SeatPosition.erase(iter);
        // 判断是否为房主，若为房主则需要将房主转移
        if(player->IsHost){
            for(auto iter = RoomPtr->Players.begin(); iter != RoomPtr->Players.end(); iter++){
                if(iter->get()->PlayerId == player->PlayerId){
                    RoomPtr->Players.erase(iter);
                    break;
                }
            } 
            player->IsHost = false;
            player->RoomPtr = nullptr;

            // 选择新房主，默认为idx为0的玩家
            RoomPtr->RoomHost = RoomPtr->Players[0];
            RoomPtr->Players[0]->IsHost = true;
            Istrans = true;
        }
        else{
            for(auto iter = RoomPtr->Players.begin(); iter != RoomPtr->Players.end(); iter++){
                if(iter->get()->PlayerId == player->PlayerId){
                    RoomPtr->Players.erase(iter);
                    break;
                }
            }
            player->IsHost = false;
            player->RoomPtr = nullptr;
        }
    }
    return ErrorCodes::Success;
}

ErrorCodes RoomManager::ChangeReady(std::shared_ptr<Player> player)
{
    auto RoomPtr = player->RoomPtr;
    if(RoomPtr == nullptr)  return ErrorCodes::ChangeReadyError;
    player->IsReady = !player->IsReady;
    std::cout << "player ID " << player->PlayerId << " IsReady: " << player->IsReady << std::endl;
    return ErrorCodes::Success;
}

ErrorCodes RoomManager::StartGame(int RoomId)
{
    auto iter = RoomMap.find(RoomId);
    if (iter == RoomMap.end()){
        std::cout << "房间ID为" << RoomId << "不存在！" << std::endl;
        return ErrorCodes::RoomStartError;
    }
    // 房间是否满人和准备好 都由客户端判断
    // 这里仅做改变房间和玩家状态
    // 遍历房间内玩家，改变其状态
    for(auto player : iter->second->Players){
        player->State = UserGameState::playing;
    }
    return ErrorCodes::Success;
}

bool RoomManager::DestructRoom(int RoomId) 
{
    auto iter = RoomMap.find(RoomId);
    if (iter == RoomMap.end()){
        std::cout << "房间ID为" << RoomId << "不存在！" << std::endl;
        return false;
    }else{
        RoomMap.erase(iter);
        return true;
    }
}

GameRound::GameRound(std::shared_ptr<Room> room) :
RoomId(room->RoomId), RoomPtr(room), winner(room->RoomHost->PlayerName) 
{
    // 初始化玩家uid和分数
    for(auto player : RoomPtr->Players){
        int uid = player->PlayerId;
        players_uid.push_back(uid);
        players_socre[uid] = 0;
    }
}

void GameRound::start() 
{
    shuffle_remaint();
    shuffle_player_title();

}

MahjongTile GameRound::DrawTitle(int uid)
{
    MahjongTile title = RemainTiles.back();
    RemainTiles.pop_back();
    HandTiles[uid].push_back(title);
    return title;
}

ErrorCodes GameRound::DisCard(int uid, int mid, MahjongTile& m)
{
    std::list<MahjongTile>& handtile = HandTiles[uid];
    auto iter = handtile.begin();
    for(; iter != handtile.end(); iter++){
        if(iter->id == mid){
            // 弃牌
            m = *iter;
            handtile.erase(iter);
            DisTiles[uid].push_back(m); 
            return ErrorCodes::Success;
        }
    }
    
    // 找不到牌    
    return ErrorCodes::CARDNOTFOUND;
}

void GameRound::shuffle_remaint()
{
    short id = 1;
    // 普通花色: 万、筒、条 (1-9，每种4张)
    for (int suit = WAN; suit <= TIAO; ++suit) {
        for (int value = 1; value <= 9; ++value) {
            for (int i = 0; i < 4; ++i) { // 每种牌有4张
                RemainTiles.push_back({id++, static_cast<CardAssort>(suit), value}); // 字牌默认值
            }
        }
    }

    // 风牌: 东南西北
    // 字牌: 东、南、西、北、中、发、白 (每种4张)
    for (int zi = DONG_; zi <= BAI_; ++zi) {
        for (int i = 0; i < 4; ++i) {
            RemainTiles.push_back({id++, static_cast<ZICard>(zi)});
        }
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(RemainTiles.begin(), RemainTiles.end(), g);

#ifdef DEBUG
    for(int i = 0; i < RemainTiles.size(); i++){
        std::cout << "id: " << RemainTiles[i].id << " suit: " << RemainTiles[i].suit << " Zi: " << RemainTiles[i].ziCard << " Value: " << RemainTiles[i].value << std::endl;
    }
#endif
}

void GameRound::shuffle_player_title()
{
    for(int i = 0; i < players_uid.size(); i++){
        int uid = players_uid[i];
        HandTiles[uid].clear();
        // 初始化玩家手牌
        initiallize_playerhandtitles(uid);
        
        DisTiles[uid].clear();
        DisTiles[uid].reserve(16);
        DisTiles[uid].clear();
        OperatTiles[uid].reserve(8);
    }
}

void GameRound::initiallize_playerhandtitles(int uid)
{
    for(int j = 0; j < 13; j++){
        HandTiles[uid].push_back(RemainTiles.back());
        RemainTiles.pop_back();
    }
}
