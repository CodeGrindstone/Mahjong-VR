using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class UIManager : MonoBehaviour
{
    //public GameObject LoginUI;
    //public GameObject LobbyUI;
    //public GameObject MahjongUI;
    public List<GameObject> UIObjects;

    public void SetUI(int UIIndex)
    {
        foreach(var obj in UIObjects)
                obj.SetActive(false);
        UIObjects[UIIndex].SetActive(true);
    }
}
