using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public enum CardAssort
{
    WAN,    // ��
    TONG,   // Ͳ
    TIAO,   // ��
    ZI      // ����
}

public enum ZICard
{
    DONG,   // ��
    NAN,    // ��
    XI,     // ��
    BEI,    // ��
    ZHONG,  // ��
    FA,     // ��
    BAI     // ��
}

public class MahjongTile
{
    public short Id { get; set; }
    public CardAssort Suit { get; set; }
    public short Value { get; set; } // �����������Ϊ 0
    public ZICard? ZiCard { get; set; } // ���ƾ������ͣ�������Ϊ null

    // ���캯������Ͳ����
    public MahjongTile(short id, CardAssort suit, short value)
    {
        Id = id;
        Suit = suit;
        Value = value;
        ZiCard = null;
    }

    // ���캯��������
    public MahjongTile(short id, ZICard ziCard)
    {
        Id = id;
        Suit = CardAssort.ZI;
        Value = 0;
        ZiCard = ziCard;
    }

    // ���� ToString �����������ӡ����Ϣ
    public override string ToString()
    {
        if (Suit == CardAssort.ZI)
        {
            return $"{Id}: {Suit} - {ZiCard}";
        }
        return $"{Id}: {Suit} - {Value}";
    }
}