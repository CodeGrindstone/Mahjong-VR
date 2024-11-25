using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class MahjongManager : MonoBehaviour
{
   
    public GameObject tilePrefab;                        // 麻将牌预制件
    public Transform centerPos;
    public Transform cardParent;
   
    public float tileWidth = 0.2f;
    public float tileHeight = 0.1f;                         // 每墩牌的宽度和高度
    public float wallDistance = 3.0f;                    // 牌墙与麻将桌中心的距离   
    public int tilesPerSide = 17;                          // 每边牌堆的墩数
    public float rotationOffset = 45.0f;                // 牌堆旋转角度

    public int numberOfPlayer = 4;
    public float distanceFromCenter = 2.5f;       // 玩家距离中心位置的距离
    public Transform[] playerCardParents;

    private Vector3 tableCenter;                                                                   // 中心点（默认为游戏对象位置）
    private List<Vector3> tilePositions = new List<Vector3>();
    private List<Quaternion> tileRotations = new List<Quaternion>();
    private List<GameObject> cards = new List<GameObject> ();            // 存储所有牌的数据
   
    void Start()
    {
        // 获取中心点
        tableCenter = centerPos.position;
        // 生成牌堆布局
        GenerateWallLayout();      
    }

    // 生成牌堆布局
    void GenerateWallLayout()
    {
        // 四条边的位置和旋转设置
        Vector3[] wallOrigins = {
            new Vector3(0, 0, wallDistance),   // 东边
            new Vector3(-wallDistance, 0, 0), // 南边
            new Vector3(0, 0, -wallDistance), // 西边
            new Vector3(wallDistance, 0, 0)   // 北边
        };

        Quaternion[] wallRotations = {
            Quaternion.Euler(0, 180, 0), // 东（朝向中心）
            Quaternion.Euler(0, 270, 0), // 南（朝向中心）
            Quaternion.Euler(0, 90, 0),   // 西（朝向中心）
            Quaternion.Euler(0, 90, 0)   // 北（朝向中心）
        };

        // 清空列表
        tilePositions.Clear();
        tileRotations.Clear();

       // 遍历每一边
       for (int side = 0; side < 4; side++)
        {      
            Vector3 startPosition = tableCenter + wallOrigins[side] * (tilesPerSide * tileWidth / 2); 
            Quaternion rotation = Quaternion.LookRotation(-wallRotations[side].eulerAngles,Vector3.up);
            Quaternion rotationAngleY = Quaternion.Euler(0, rotationOffset, 0);
            Quaternion rotationAngleZ = Quaternion.Euler(0, 0, rotationOffset);
            rotation *= rotationAngleZ;
            startPosition = rotationAngleY * (startPosition - tableCenter) + tableCenter;

            Vector3 tilePosition;

            for (int i = 0; i < tilesPerSide; i++)
            {
                // 根据边的偶奇性选择偏移量
                float xOffset = (side % 2 == 0) ? i * tileWidth - (tilesPerSide * tileWidth / 2) : 0;
                float zOffset = (side % 2 == 0) ? 0 : i * tileWidth - (tilesPerSide * tileWidth / 2);

                // 第一层
                tilePosition = startPosition + new Vector3(xOffset, 0, zOffset);
                tilePosition = rotationAngleY * (tilePosition - tableCenter) + tableCenter;
                tilePositions.Add(tilePosition);
                tileRotations.Add(rotation);

                // 第二层
                tilePosition = startPosition + new Vector3(xOffset, tileHeight, zOffset);
                tilePosition = rotationAngleY * (tilePosition - tableCenter) + tableCenter;
                tilePositions.Add(tilePosition);
                tileRotations.Add(rotation);
            }
        } 
    }
    // 渲染牌堆
    public void RenderWall()
    { 
        if (tilePrefab == null) 
        {
            Debug.LogError("Tile Prefab is not assigned!"); 
            return; 
        } 
        for (int i = 0; i < tilePositions.Count; i++)
        { 
            GameObject card = Instantiate(tilePrefab, tilePositions[i], tileRotations[i]);
            card.transform.SetParent(cardParent);
            cards.Add(card);
        }
    } 

    public void RenderCardPreparation()
    {

        Quaternion[] cardTowards =
        {
            Quaternion.Euler(0, 0, 0), // 东
            Quaternion.Euler(0, 270, 0), // 南
            Quaternion.Euler(0, 180, 0),   // 西
            Quaternion.Euler(0, 90, 0)   // 北
         };

        // 为每个玩家生成 13 张牌
        for (int playerIndex = 0; playerIndex < numberOfPlayer; playerIndex++)
        {           
            for (int j = 0; j < 13; j++)
            {
                if (cards.Count == 0)
                {
                    Debug.LogWarning("没有足够的牌来发放给玩家!");
                    break;
                }

                // 随机选择一张牌
                int randomIndex = UnityEngine.Random.Range(0, cards.Count);
                GameObject selectedTile = cards[randomIndex];
                
                Vector3 playerPosition = GetPlayerPosition(playerIndex);
                Quaternion playerRotation = cardTowards[playerIndex];
                selectedTile.transform.SetPositionAndRotation(playerPosition, playerRotation); 
                cards.RemoveAt(randomIndex);
                float offset = j * tileWidth;
               selectedTile.transform.position += (playerIndex%2 == 0) ? new Vector3(-offset, 0, 0) : new Vector3(0,0,offset); // 每张牌的偏移
            }
        }

    }
    // 获取玩家位置的方法，根据玩家索引返回相应的位置
    private Vector3 GetPlayerPosition(int playerIndex)
    {       
        switch (playerIndex)
        {
            case 0: // 玩家 1 (东)
                return tableCenter + new Vector3(-distanceFromCenter, 0, wallDistance);
            case 1: // 玩家 2 (南)
                return tableCenter + new Vector3(-wallDistance, 0, distanceFromCenter);
            case 2: // 玩家 3 (西)
                return tableCenter + new Vector3(-distanceFromCenter, 0, -wallDistance);
            case 3: // 玩家 4 (北)
                return tableCenter + new Vector3(wallDistance, 0, distanceFromCenter);
            default:
                return tableCenter; // 默认位置
        }
    }
}
