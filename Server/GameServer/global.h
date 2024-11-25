#pragma once

#include <functional>

#define MAX_LENGTH  1024*2
//头部总长度
#define HEAD_TOTAL_LEN 4
//头部id长度
#define HEAD_ID_LEN 2
//头部数据长度
#define HEAD_DATA_LEN 2
#define MAX_RECVQUE  10000
#define MAX_SENDQUE 1000

enum ErrorCodes {
    Success = 0,
    Error_Json = 1001,      //Json解析错误
    RPCFailed = 1002,       //RPC请求错误
    VerifyExpired = 1003,   //验证码过期
    VerifyCodeErr = 1004,   //验证码错误
    UserExist = 1005,       //用户已经存在
    PasswdErr = 1006,       //密码错误
    EmailNotMatch = 1007,   //邮箱不匹配
    PasswdUpFailed = 1008,  //更新密码失败
    PasswdInvalid = 1009,   //密码更新失败
    TokenInvalid = 1010,    //Token失效
    UidInvalid = 1011,      //uid无效
    CreateRoomFailed = 1012,//创建房间失败
    NotFoundRoom = 1013,    // 房间不存在
    CantEnterRoom = 1014,   // 房间已满
    AlreadyJoinRoom = 1015, // 已加入房间
    ChangeReadyError = 1016, // 准备事件错误
    RoomStartError = 1017,  // 房间开始错误
    CARDNOTFOUND = 1018,    // 牌不存在
    CANTDRAWTILE = 1019,    // 摸牌失败
};

enum MSG_IDS { 
    MSG_QUIT = 1000,            // 退出或断线
    MSG_CHAT_LOGIN = 1005,      //用户登陆
    MSG_CHAT_LOGIN_RSP = 1006, 
    MSG_CREATE_ROOM = 1007,     // 创建新房间
    MSG_CREATE_ROOM_RSP = 1008, 
    MSG_ENTER_ROOM = 1009,      // 进入房间
    MSG_ENTER_ROOM_RSP = 1010,  
    MSG_ENTER_ROOM_BROADC = 1011,
    MSG_QUIT_ROOM = 1012,       // 退出房间
    MSG_QUIT_ROOM_RSP = 1013,
    MSG_QUIT_ROOM_BROADC = 1014,
    MSG_CHANGE_READY = 1015,    // 点击准备 
    MSG_CHANGE_READY_RSP = 1016,
    MSG_CHANGE_READY_BROADC = 1017, 
    MSG_START_GAME = 1018,      // 开始游戏
    MSG_START_GAME_RSP = 1019,
    MSG_START_GAME_BROADC = 1020,   
    MSG_DWAR_TITLE = 1021,      // 摸牌
    MSG_DWAR_TITLE_RSP = 1022 , 
    MSG_DWAR_TITLE_BROADC = 1023,
    MSG_DISCARD = 1024,         // 弃牌
    MSG_DISCARD_RSP = 1025,
    MSG_DISCARD_BROADC = 1025,
    MSG_PUNG = 1026,            // 碰
    MSG_PUNG_RSP = 1027,        
    MSG_PUNG_BROADC = 1028,
    MSG_GUNG = 1029,            // 杠
    MSG_GUNG_RSP = 1030,        
    MSG_GUNG_BROADC = 1031
 };




// Defer类
class Defer {                                                                                                     
private:
    std::function<void()> func_;
public:
    // 接受一个lambda表达式或者函数指针
    Defer(std::function<void()> func) : func_(func) {}
 
    // 析构函数中执行传入的函数
    ~Defer() {
        func_();
    }
};

// 牌的分类
enum CardAssort {
    WAN,    // 万
    TONG,   // 筒
    TIAO,   // 条
    ZI
};

enum ZICard {
    DONG_,   // 东
    NAN_,    // 南
    XI_,     // 西
    BEI_,    // 北
    ZHONG_,  // 中
    FA_,     // 发
    BAI_     // 白
};

struct MahjongTile {
    short id;
    CardAssort suit;  // 花色
    short value;      // 通过值来判断具体类型，0为字牌
    ZICard ziCard;    // 如果是字牌则指定具体类型

    MahjongTile(short _id, CardAssort _suit, int _value) : id(_id), suit(_suit), value(_value){}
    MahjongTile(short _id, ZICard _ziCard) : id(_id), suit(CardAssort::ZI),value(0), ziCard(_ziCard){}
    // 默认构造和赋值
    MahjongTile() = default;
    MahjongTile(const MahjongTile&) = default;
    MahjongTile(MahjongTile&&) noexcept = default;
    MahjongTile& operator=(const MahjongTile&) = default;
    MahjongTile& operator=(MahjongTile&&) noexcept = default;
};

enum Action{
    Draw,
    DisCard,
    Pung,
    Gung
};

enum UserGameState{
    Waiting,
    playing
};

struct ActionRecord {
    Action actionType;        // 操作类型：discard/pong/kong
    short playerId;          // 执行操作的玩家
    MahjongTile tile;        // 涉及的牌
    int affectedPlayerId;    // 受影响的玩家（如果有）
};



