using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Player : MonoBehaviour
{
    // ��һ�����Ϣ
    public string Name { get; set; }
    public int Uid { get; set; }
    public string Token { get; set; }
    public int? CurrentRoomId { get; set; }
    public int RoomPostion { get; set; }
    // ��ǰ��������������Ϣ (���� uid������������λ��)
    public List<(int Uid, string Name, int SeatPosition)> RoomPlayers { get; set; }
    // ������Ϣ (uid ������)
    public (int Uid, string Name) RoomHost { get; set; }

    // ��ҵ�����
    public List<MahjongTile> HandTiles { get; set; } // ��ǰ����
    public List<MahjongTile> DiscardPile { get; set; } // ������
    public List<MahjongTile> PengGangPile { get; set; } // ������

    // ���캯��
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
        RoomHost = (0, ""); // Ĭ�Ϸ�����ϢΪ��
    }

    public void CreateRoom(int RoomId)
    {
        CurrentRoomId = RoomId;
        RoomPostion = 0;
        RoomPlayers.Add((Uid, Name, RoomPostion));
        SetRoomHost(Uid, Name);
    }


    // �������
    public void AddToHand(MahjongTile tile)
    {
        HandTiles.Add(tile);
    }

    // �������Ƶ�������
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

    // ��ӵ�������
    public void AddToPengGang(MahjongTile tile)
    {
        PengGangPile.Add(tile);
    }
    
    // ���÷�����Ϣ
    public void SetRoomHost(int uid, string name) 
    { 
        RoomHost = (uid, name); 
    }

    // ��������������
    public void AddRoomPlayer(int uid, string name, int seatPosition)
    { 
        if (RoomPlayers.Exists(player => player.Uid == uid)) 
        { 
            throw new InvalidOperationException("Player already exists in the room."); 
        } 
        RoomPlayers.Add((uid, name, seatPosition)); 
    } 

    // �Ƴ��������
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

    // ���� ToString ��������ʾ��ҵ�ǰ״̬
    public override string ToString()
    {
        return $"Name: {Name}, Uid: {Uid}, Token: {Token}, CurrentRoomId: {CurrentRoomId?.ToString() ?? "None"}\n" +
               $"HandTiles: {string.Join(", ", HandTiles)}\n" +
               $"DiscardPile: {string.Join(", ", DiscardPile)}\n" +
               $"PengGangPile: {string.Join(", ", PengGangPile)}";
    }
}
