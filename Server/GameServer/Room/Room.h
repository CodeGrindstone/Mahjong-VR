#pragma one
#include "../global.h"
#include "../LogicSystem/Singleton.h"
#include <memory>
#include "../Player/Player.h"
#include <mutex>
#include <map>
#include <list>


class Player;
class GameRound;
class CSession;
class Room
{
    friend class LogicSystem;
    friend class RoomManager;
    friend class GameRound;
    int RoomId;         // 房间ID
    std::shared_ptr<Player> RoomHost;//房主
    std::vector<std::shared_ptr<Player>> Players;// 房间内非房主列表
    std::map<int, short> SeatPosition;    // uid -- 位置
    std::vector<bool> IsSeat;           // 当前位置是否被占用
    short CurrentRound;         // 当前局数
    std::map<int, std::shared_ptr<CSession>> m_sockets;
public:
    Room(int roomid, std::shared_ptr<Player>);
    short FindPostion();
    ~Room();
};

class RoomManager : public Singleton<RoomManager>
{
    friend class Singleton;
private:
    std::map<int, std::shared_ptr<Room>> RoomMap;
    int m_RoomNum;
    std::mutex m_mutex;

private:
    RoomManager();

public:
    ~RoomManager(){}
    ErrorCodes CreateRoom(std::shared_ptr<Player> host, std::shared_ptr<CSession> session);
    ErrorCodes EnterRoom(int RoomId, std::shared_ptr<Player> player, std::shared_ptr<CSession> session);
    ErrorCodes QuitRoom(std::shared_ptr<Player> player, bool&);
    ErrorCodes ChangeReady(std::shared_ptr<Player> player);
    ErrorCodes StartGame(int RoomId);
    bool DestructRoom(int RoomId);
};

class CSession;
class GameRound
{
    friend class LogicSystem;
private:
    int RoomId;     // 房间ID
    std::vector<MahjongTile> RemainTiles;// 桌面剩余牌
    std::shared_ptr<Room> RoomPtr;  // 房间指针
    std::vector<int> players_uid;   // 玩家id
    std::map<int, int> players_socre;// 玩家分数
    std::map<int, std::list<MahjongTile>> HandTiles;  // 玩家手牌区
    std::map<int, std::vector<MahjongTile>> DisTiles;   // 玩家弃牌区
    std::map<int, std::vector<MahjongTile>> OperatTiles; // 玩家碰杠区
    std::string winner;// 一局游戏胜者
public:
    GameRound(std::shared_ptr<Room> room);
    ~GameRound(){}
    void start();
    MahjongTile DrawTitle(int uid);
    ErrorCodes DisCard(int uid, int mid, MahjongTile&);
private:
    void shuffle_remaint();
    void shuffle_player_title();
    void initiallize_playerhandtitles(int uid);
};
