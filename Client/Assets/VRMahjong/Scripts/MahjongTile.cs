using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public enum CardAssort
{
    WAN,    // 万
    TONG,   // 筒
    TIAO,   // 条
    ZI      // 字牌
}

public enum ZICard
{
    DONG,   // 东
    NAN,    // 南
    XI,     // 西
    BEI,    // 北
    ZHONG,  // 中
    FA,     // 发
    BAI     // 白
}

public class MahjongTile
{
    public short Id { get; set; }
    public CardAssort Suit { get; set; }
    public short Value { get; set; } // 如果是字牌则为 0
    public ZICard? ZiCard { get; set; } // 字牌具体类型，非字牌为 null

    // 构造函数：万、筒、条
    public MahjongTile(short id, CardAssort suit, short value)
    {
        Id = id;
        Suit = suit;
        Value = value;
        ZiCard = null;
    }

    // 构造函数：字牌
    public MahjongTile(short id, ZICard ziCard)
    {
        Id = id;
        Suit = CardAssort.ZI;
        Value = 0;
        ZiCard = ziCard;
    }

    // 重载 ToString 方法，方便打印牌信息
    public override string ToString()
    {
        if (Suit == CardAssort.ZI)
        {
            return $"{Id}: {Suit} - {ZiCard}";
        }
        return $"{Id}: {Suit} - {Value}";
    }
}