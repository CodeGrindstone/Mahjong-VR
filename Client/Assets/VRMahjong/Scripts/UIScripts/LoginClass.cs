using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;

public class LoginClass : MonoBehaviour
{
    //进入前变量
    public InputField username, password, confirmPassword;
    public Text reminderText;
    public int errorsNum;
    public Button loginButton;
    public GameObject hallSetUI, loginUI;
    public GameObject SceneManagerRef;
    //进入后变量
    public static string myUsername;

    // 引用 TCPClientManager
    private TCPClientManager tcpClientManager;

    void Start()
    {
        // 获取 TCPClientManager 实例
        tcpClientManager = FindObjectOfType<TCPClientManager>();
        loginButton.onClick.AddListener(() => { FindObjectOfType<UIManager>().SetUI(1); });
    }

    public void Register()
    {
        if (PlayerPrefs.GetString(username.text) == "")
        {
            if (password.text == confirmPassword.text)
            {
                PlayerPrefs.SetString(username.text, username.text);
                PlayerPrefs.SetString(username.text + "password", password.text);
                // 注册成功后发送信息到服务器
                if (tcpClientManager != null)
                    tcpClientManager.SendMessage($"REGISTER:{username.text}:{password.text}");
                else
                    Debug.LogWarning("Client doesn't connect to server!");
                reminderText.text = "注册成功！";
            }
            else
            {
                reminderText.text = "两次密码输入不一致";
            }
        }
        else
        {
            reminderText.text = "用户已存在";
        }

    }
    private void Recovery()
    {
        loginButton.interactable = true;
    }
    public void Login()
    {
        if (PlayerPrefs.GetString(username.text) != "")
        {
            if (PlayerPrefs.GetString(username.text + "password") == password.text)
            {
                reminderText.text = "登录成功";

                myUsername = username.text;
                // 登录成功后发送信息到服务器
                if (tcpClientManager != null)
                    tcpClientManager.SendMessage($"LOGIN:{username.text}:{password.text}");
                else
                    Debug.LogWarning("Client doesn't connect to server!");

                hallSetUI.SetActive(true);
                loginUI.SetActive(false);
            }
            else
            {
                reminderText.text = "密码错误";
                errorsNum++;
                if (errorsNum >= 3)
                {
                    reminderText.text = "连续错误3次，请30秒后再试！";
                    loginButton.interactable = false;
                    Invoke("Recovery", 5);
                    errorsNum = 0;
                }
            }
        }
        else
        {
            reminderText.text = "账号不存在";
        }
    }

}
