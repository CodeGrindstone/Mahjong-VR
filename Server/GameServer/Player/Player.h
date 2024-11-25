#pragma once
#include "../global.h"
#include <memory>
class Room;

struct Player
{
    int PlayerId;               // 玩家唯一ID
    std::string PlayerName;     // 玩家名称
    std::shared_ptr<Room> RoomPtr;// 房间共享指针
    bool IsHost;                // 是否为房主
    bool IsReady;               // 是否准备好
    UserGameState State;        // 玩家游戏状态

    Player() : IsHost(false), 
    IsReady(false), State(UserGameState::Waiting){}
};