using System.Collections;
using System.Collections.Generic;
using System.Numerics;
using TMPro;
using UnityEngine;
using UnityEngine.UI;
using Vector3 = UnityEngine.Vector3;
using Quaternion = UnityEngine.Quaternion;

public class LobbyClass : MonoBehaviour
{
    public Transform XR_OriginPos;
    public Transform InitialPos;
    public RectTransform ButtonsPos;

    public GameObject confirmButtonPrefab;

    public GameObject mahjongPlace; // 麻将桌椅父物体

    public GameObject enterRoomPrefab;
    public GameObject enterRoomObject;

    public GameObject otherPlayerPrefab;
    public UnityTCP unityTcpRef;
    public string roomId;
    
    // 玩家自己的视角下，自己的逻辑
    public void EnterRoomAndSeat(int seat)
    {
        Transform chair = mahjongPlace.transform.GetChild(seat);
        Vector3 destination = chair.position;
        Quaternion rotation = chair.rotation;
        XR_OriginPos.localPosition = destination;
        XR_OriginPos.localRotation = rotation;
        XR_OriginPos.SetParent(chair);       
        gameObject.SetActive(false);
    }

    public void QuitRoomAndReturn()
    {
        gameObject.SetActive(true);
        Vector3 destination = InitialPos.position;
        Quaternion rotation = InitialPos.rotation;

        XR_OriginPos.localPosition = destination;
        XR_OriginPos.localRotation = rotation;
        XR_OriginPos.SetParent(null);
    }

    public void EnterRoomBySearch()
    {
        GameObject searchUI = Instantiate(enterRoomPrefab,ButtonsPos);
        searchUI.transform.SetParent(ButtonsPos);
        Button EnterBtn = searchUI.transform.GetChild(1).GetComponent<Button>();
        if(unityTcpRef == null)
            unityTcpRef = FindObjectOfType<UnityTCP>();
        EnterBtn.onClick.AddListener(unityTcpRef.EnterRoomButton);
        Debug.Log(roomId);
    }

    // 其它玩家的逻辑
    public void OtherPlayerEnterAndSeat(int seat)
    {
        Transform chair = mahjongPlace.transform.GetChild(seat);
        Vector3 destination = chair.position;
        Quaternion rotation = chair.rotation;
        GameObject otherPlayer = Instantiate(otherPlayerPrefab);
        otherPlayer.transform.localPosition = destination;
        otherPlayer.transform.localRotation = rotation;
        otherPlayer.transform.SetParent(chair);
    }

    public void OtherPlayerQuitRoom(int seat)
    {
        Transform chair = mahjongPlace.transform.GetChild(seat);
        Destroy(chair.GetChild(0).gameObject);
    }

    public void ConfirmReady()
    {

    }
}
