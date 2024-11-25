using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class EnterRoomUI : MonoBehaviour
{
    public InputField searchRoomId;
   
    public Button enterButton;
    public Button exitButton;
    public TMP_Text infoText;

    public LobbyClass lobby;

    public void Send()
    {
        if (lobby == null)
            lobby = FindObjectOfType<LobbyClass>();
        lobby.roomId = searchRoomId.text;
    }
   
    public void Close()
    {
        Destroy(gameObject);
    }

}
