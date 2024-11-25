using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class MahjongManager : MonoBehaviour
{
   
    public GameObject tilePrefab;                        // �齫��Ԥ�Ƽ�
    public Transform centerPos;
    public Transform cardParent;
   
    public float tileWidth = 0.2f;
    public float tileHeight = 0.1f;                         // ÿ���ƵĿ�Ⱥ͸߶�
    public float wallDistance = 3.0f;                    // ��ǽ���齫�����ĵľ���   
    public int tilesPerSide = 17;                          // ÿ���ƶѵĶ���
    public float rotationOffset = 45.0f;                // �ƶ���ת�Ƕ�

    public int numberOfPlayer = 4;
    public float distanceFromCenter = 2.5f;       // ��Ҿ�������λ�õľ���
    public Transform[] playerCardParents;

    private Vector3 tableCenter;                                                                   // ���ĵ㣨Ĭ��Ϊ��Ϸ����λ�ã�
    private List<Vector3> tilePositions = new List<Vector3>();
    private List<Quaternion> tileRotations = new List<Quaternion>();
    private List<GameObject> cards = new List<GameObject> ();            // �洢�����Ƶ�����
   
    void Start()
    {
        // ��ȡ���ĵ�
        tableCenter = centerPos.position;
        // �����ƶѲ���
        GenerateWallLayout();      
    }

    // �����ƶѲ���
    void GenerateWallLayout()
    {
        // �����ߵ�λ�ú���ת����
        Vector3[] wallOrigins = {
            new Vector3(0, 0, wallDistance),   // ����
            new Vector3(-wallDistance, 0, 0), // �ϱ�
            new Vector3(0, 0, -wallDistance), // ����
            new Vector3(wallDistance, 0, 0)   // ����
        };

        Quaternion[] wallRotations = {
            Quaternion.Euler(0, 180, 0), // �����������ģ�
            Quaternion.Euler(0, 270, 0), // �ϣ��������ģ�
            Quaternion.Euler(0, 90, 0),   // �����������ģ�
            Quaternion.Euler(0, 90, 0)   // �����������ģ�
        };

        // ����б�
        tilePositions.Clear();
        tileRotations.Clear();

       // ����ÿһ��
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
                // ���ݱߵ�ż����ѡ��ƫ����
                float xOffset = (side % 2 == 0) ? i * tileWidth - (tilesPerSide * tileWidth / 2) : 0;
                float zOffset = (side % 2 == 0) ? 0 : i * tileWidth - (tilesPerSide * tileWidth / 2);

                // ��һ��
                tilePosition = startPosition + new Vector3(xOffset, 0, zOffset);
                tilePosition = rotationAngleY * (tilePosition - tableCenter) + tableCenter;
                tilePositions.Add(tilePosition);
                tileRotations.Add(rotation);

                // �ڶ���
                tilePosition = startPosition + new Vector3(xOffset, tileHeight, zOffset);
                tilePosition = rotationAngleY * (tilePosition - tableCenter) + tableCenter;
                tilePositions.Add(tilePosition);
                tileRotations.Add(rotation);
            }
        } 
    }
    // ��Ⱦ�ƶ�
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
            Quaternion.Euler(0, 0, 0), // ��
            Quaternion.Euler(0, 270, 0), // ��
            Quaternion.Euler(0, 180, 0),   // ��
            Quaternion.Euler(0, 90, 0)   // ��
         };

        // Ϊÿ��������� 13 ����
        for (int playerIndex = 0; playerIndex < numberOfPlayer; playerIndex++)
        {           
            for (int j = 0; j < 13; j++)
            {
                if (cards.Count == 0)
                {
                    Debug.LogWarning("û���㹻���������Ÿ����!");
                    break;
                }

                // ���ѡ��һ����
                int randomIndex = UnityEngine.Random.Range(0, cards.Count);
                GameObject selectedTile = cards[randomIndex];
                
                Vector3 playerPosition = GetPlayerPosition(playerIndex);
                Quaternion playerRotation = cardTowards[playerIndex];
                selectedTile.transform.SetPositionAndRotation(playerPosition, playerRotation); 
                cards.RemoveAt(randomIndex);
                float offset = j * tileWidth;
               selectedTile.transform.position += (playerIndex%2 == 0) ? new Vector3(-offset, 0, 0) : new Vector3(0,0,offset); // ÿ���Ƶ�ƫ��
            }
        }

    }
    // ��ȡ���λ�õķ����������������������Ӧ��λ��
    private Vector3 GetPlayerPosition(int playerIndex)
    {       
        switch (playerIndex)
        {
            case 0: // ��� 1 (��)
                return tableCenter + new Vector3(-distanceFromCenter, 0, wallDistance);
            case 1: // ��� 2 (��)
                return tableCenter + new Vector3(-wallDistance, 0, distanceFromCenter);
            case 2: // ��� 3 (��)
                return tableCenter + new Vector3(-distanceFromCenter, 0, -wallDistance);
            case 3: // ��� 4 (��)
                return tableCenter + new Vector3(wallDistance, 0, distanceFromCenter);
            default:
                return tableCenter; // Ĭ��λ��
        }
    }
}
