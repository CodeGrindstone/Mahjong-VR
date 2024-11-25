using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Player : MonoBehaviour
{
    // 玩家基本信息
    public string Name { get; set; }
    public int Uid { get; set; }
    public string Token { get; set; }
    public int? CurrentRoomId { get; set; }
    public int RoomPostion { get; set; }
    // 当前房间的其他玩家信息 (包含 uid、姓名和入座位置)
    public List<(int Uid, string Name, int SeatPosition)> RoomPlayers { get; set; }
    // 房主信息 (uid 和姓名)
    public (int Uid, string Name) RoomHost { get; set; }

    // 玩家的牌区
    public List<MahjongTile> HandTiles { get; set; } // 当前手牌
    public List<MahjongTile> DiscardPile { get; set; } // 弃牌区
    public List<MahjongTile> PengGangPile { get; set; } // 碰杠区

    // 构造函数
    public Player(string name, int uid, string token, int? currentRoomId = null)
    {
        Name = name;
        Uid = uid;
        Token = token;
        CurrentRoomId = currentRoomId;
        RoomPostion = -1;
        HandTiles = new List<MahjongTile>();
        DiscardPile = new List<MahjongTile>();
        PengGangPile = new List<MahjongTile>();
        RoomPlayers = new List<(int Uid, string Name, int SeatPosition)>();
        RoomHost = (0, ""); // 默认房主信息为空
    }

    public void CreateRoom(int RoomId)
    {
        CurrentRoomId = RoomId;
        RoomPostion = 0;
        RoomPlayers.Add((Uid, Name, RoomPostion));
        SetRoomHost(Uid, Name);
    }


    // 添加手牌
    public void AddToHand(MahjongTile tile)
    {
        HandTiles.Add(tile);
    }

    // 从手牌移到弃牌区
    public void DiscardTile(MahjongTile tile)
    {
        if (HandTiles.Remove(tile))
        {
            DiscardPile.Add(tile);
        }
        else
        {
            throw new InvalidOperationException("Tile not found in hand.");
        }
    }

    // 添加到碰杠区
    public void AddToPengGang(MahjongTile tile)
    {
        PengGangPile.Add(tile);
    }
    
    // 设置房主信息
    public void SetRoomHost(int uid, string name) 
    { 
        RoomHost = (uid, name); 
    }

    // 添加其他房间玩家
    public void AddRoomPlayer(int uid, string name, int seatPosition)
    { 
        if (RoomPlayers.Exists(player => player.Uid == uid)) 
        { 
            throw new InvalidOperationException("Player already exists in the room."); 
        } 
        RoomPlayers.Add((uid, name, seatPosition)); 
    } 

    // 移除房间玩家
    public void RemoveRoomPlayer(int uid) 
    { 
        var player = RoomPlayers.Find(p => p.Uid == uid); 
        if (player != default) { 
            RoomPlayers.Remove(player);
        } 
        else { 
            throw new InvalidOperationException("Player not found in the room."); 
        } 
    }

    // 重载 ToString 方法，显示玩家当前状态
    public override string ToString()
    {
        return $"Name: {Name}, Uid: {Uid}, Token: {Token}, CurrentRoomId: {CurrentRoomId?.ToString() ?? "None"}\n" +
               $"HandTiles: {string.Join(", ", HandTiles)}\n" +
               $"DiscardPile: {string.Join(", ", DiscardPile)}\n" +
               $"PengGangPile: {string.Join(", ", PengGangPile)}";
    }
}
