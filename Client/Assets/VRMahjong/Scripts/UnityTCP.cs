using Newtonsoft.Json.Linq;
using Newtonsoft.Json;
using System.Collections;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;
using UnityEngine;
using UnityEditor.PackageManager;
using TMPro;
using UnityEngine.UI;
using System.Linq;
using JetBrains.Annotations;

public class member
{
    public bool is_ready { get; set; }
    public string name { get; set; }
    public int seat { get; set; }
    public int uid { get; set; }
}

public class UnityTCP : MonoBehaviour
{
    public GameObject PlayerNameRender;
    public GameObject PlayerPrepareRender;
    public LobbyClass Lobby; // ���������ʵ��
    private TCPClientManager manager;
    private Player player;

    public void OnEnable()
    {
        Lobby = FindObjectOfType<LobbyClass>();
    }

    public bool verifyjson(ErrorCodes error)
    {
        if (ErrorCodes.Success != error)
        {
            if (ErrorCodes.Error_Json == error)
            {
                Debug.Log("json�������������ԣ�\n");
                return false;
            }
            if (ErrorCodes.UidInvalid == error)
            {
                Debug.Log("�Ƿ���uid\n");
                return false;
            }
            return false;
        }
        else
        {
            return true;
        }
    }


    public void judgeIsReady()
    {
        if (player.Uid == player.RoomHost.Uid)
        {
            // ����Ϊ����
            if (player.RoomPlayers.Count == 4)
            {
                bool flag = true;
                for (int i = 0; i < 4; i++)
                {
                    if (!PlayerPrepareRender.transform.GetChild(i).GetComponent<Toggle>().isOn)
                    {
                        flag = false;
                        break;
                    }
                }
                if (flag == true)
                {
                    //��Ⱦһ������ʼ��Ϸ
                    PlayerNameRender.transform.GetChild(0).GetComponent<TMP_Text>().text = "�뷿�����¿ո����ʼ��Ϸ";

                }
            }
        }
    }

    public void RegisterHandlers()
    {
        // ע����Ϣ������
        manager.RegisterMessageHandler((short)MSG_IDS.MSG_CHAT_LOGIN_RSP, (msgId, message) =>
        {
            Debug.Log("----------MSG_CHAT_LOGIN_RSP----------");
            Debug.Log(message + "\n");
            // ����
            var json = JObject.Parse(message);
            var error = json["error"].ToObject<ErrorCodes>();
            if (!verifyjson(error)) return; 

            int uid = json["uid"].ToObject<int>();
            string name = json["name"].ToObject<string>();
            string token = json["token"].ToObject<string>();
            player = new Player(name, uid, token);
            Debug.Log("-------��½�ɹ�--------\n");
        });

        manager.RegisterMessageHandler((short)MSG_IDS.MSG_CREATE_ROOM_RSP, (msgId, message) =>
        {
            Debug.Log("----------MSG_CREATE_ROOM_RSP----------");
            // ����
            var json = JObject.Parse(message);
            var error = json["error"].ToObject<ErrorCodes>();
            if (!verifyjson(error)) return;
            if(error == ErrorCodes.CreateRoomFailed)
            {
                Debug.Log("��������ʧ��\n");
                return;
            }
            int roomId = json["roomid"].ToObject<int>();
            player.CreateRoom(roomId);
            // ������Ⱦ.....
           PlayerNameRender.transform.GetChild(0).GetComponent<TMP_Text>().text = "����ID��"+roomId;
           PlayerNameRender.transform.GetChild(1).GetComponent<TMP_Text>().text = "������"+player.RoomHost.Name;
            Lobby.EnterRoomAndSeat(0);

            Debug.Log("--------�ɹ���������--------\n");
            return;
        });

        manager.RegisterMessageHandler((short)MSG_IDS.MSG_ENTER_ROOM_RSP, (msgId, message) =>
        {
            Debug.Log("----------MSG_ENTER_ROOM_RSP----------");


            var json = JObject.Parse(message);
            var error = json["error"].ToObject<ErrorCodes>();
            if (!verifyjson(error)) return;

            var hostuid = json["hostUid"].ToObject<int>();          
            JArray members_json = (JArray)json["members"];
            
            List<member> members = members_json.ToObject<List<member>>();
            int host_seat;
            foreach(var member in members)
            {
                if (hostuid == member.uid) host_seat = member.seat;
                Debug.Log($"name : {member.name}, seat : {member.seat}, uid : {member.uid}");
            }


            // ��Ⱦ
            // ÿ����ҵ���λ
            // ����
          foreach(var member in members)
            {
                if (hostuid == member.uid)
                {
                    PlayerNameRender.transform.GetChild(member.seat + 1).GetComponent<TMP_Text>().text = "������" + member.name;                   
                }
                else
                    PlayerNameRender.transform.GetChild(member.seat+1).GetComponent<TMP_Text>().text = member.name;
                PlayerPrepareRender.transform.GetChild(member.seat).GetComponent<Toggle>().isOn = member.is_ready;
                Lobby.OtherPlayerEnterAndSeat(member.seat);
            }
          // ����Լ�������Ӧ����λ
          // Lobby.EnterRoomAndSeat(my_seat);
        });

        manager.RegisterMessageHandler((short)MSG_IDS.MSG_ENTER_ROOM_BROADC, (msgId, message) =>
        {
            Debug.Log("----------MSG_ENTER_ROOM_BROADC----------");
            // ����
            var json = JObject.Parse(message);
            var error = json["error"].ToObject<ErrorCodes>();
            if (error != ErrorCodes.Success) return;

            // ���չ㲥��Ϣ
            var new_player_json = json["player"].ToObject<JObject>();
            int uid = new_player_json["uid"].ToObject<int>();
            string name = new_player_json["name"].ToObject<string>();
            int seat = new_player_json["seat"].ToObject<int>();

            player.AddRoomPlayer(uid, name, seat);
            // ��Ⱦ
            // 1. ����������
            PlayerNameRender.transform.GetChild(seat + 1 % 4).GetComponent<TMP_Text>().text = name;
            PlayerPrepareRender.transform.GetChild(seat).GetComponent<Toggle>().isOn = false;
            // 2. ����
            Lobby.OtherPlayerEnterAndSeat(seat);

            Debug.Log("--------�ɹ����뷿��--------\n");
            return;
        });

        manager.RegisterMessageHandler((short)MSG_IDS.MSG_CHANGE_READY_RSP, (msgId, message) =>
        {
            Debug.Log("----------MSG_DISCARD_RSP----------");
            Debug.Log(message + "\n");

            // ����
            var json = JObject.Parse(message);
            var error = json["error"].ToObject<ErrorCodes>();
            if (!verifyjson(error)) return;

            // ��ʾ
            Debug.Log("----------�ı�׼��״̬----------");
        });

        manager.RegisterMessageHandler((short)MSG_IDS.MSG_CHANGE_READY_BROADC, (msgId, message) =>
        {
            Debug.Log("----------MSG_CHANGE_READY_BROADC----------");
            Debug.Log(message + "\n");

            // ����
            var json = JObject.Parse(message);
            var error = json["error"].ToObject<ErrorCodes>();
            if (error != ErrorCodes.Success) return;

            // ���չ㲥��Ϣ
            var player_name = json["player"].ToObject<string>();
            var seat = json["seat"].ToObject<int>();
            var isready = json["isready"].ToObject<bool>();
            Debug.Log(isready);

            // ��Ⱦ
            PlayerPrepareRender.transform.GetChild(seat).GetComponent<Toggle>().isOn = isready;
           

           // �ж��Ƿ���Կ�ʼ��Ϸ
           judgeIsReady();

            return;
        });

        manager.RegisterMessageHandler((short)MSG_IDS.MSG_QUIT_ROOM_RSP, (msgId, message) =>
        {
            Debug.Log("----------MSG_QUIT_ROOM_RSP----------");
            Debug.Log(message + "\n");

            player.CurrentRoomId = -1;
            player.RoomPostion = -1;
            player.RoomPlayers = null;
            player.RoomHost = (-1, "");

            // ��Ⱦ
            // 1. ��շ����ڵ����
            Lobby.QuitRoomAndReturn();
            PlayerNameRender.transform.GetChild(0).GetComponent<TMP_Text>().text = "-1";
            for (int i = 1; i < PlayerNameRender.transform.childCount; i++)
                PlayerNameRender.transform.GetChild(i).GetComponent<TMP_Text>().text = "��";
            for(int i = 0;i<PlayerPrepareRender.transform.childCount;i++)
                PlayerPrepareRender.transform.GetChild(i).GetComponent<Toggle>().isOn = false;

            return;
        });

        manager.RegisterMessageHandler((short)MSG_IDS.MSG_QUIT_ROOM_BROADC, (msgId, message) =>
        {
            Debug.Log("----------MSG_QUIT_ROOM_BROADC----------");
            Debug.Log(message + "\n");

            // ����
            var json = JObject.Parse(message);
            var error = json["error"].ToObject<ErrorCodes>();
            if (error != ErrorCodes.Success) return;


            // �㲥
            var is_trans = json["is_trans"].ToObject<bool>();
            var quit_player = json["player"].ToObject<JObject>();

            var quit_player_name = quit_player["name"].ToObject<string>();
            var quit_player_uid = quit_player["uid"].ToObject<int>();
            var quit_player_seat = quit_player["seat"].ToObject<int>();

            player.RoomPlayers.Remove((quit_player_uid, quit_player_name, quit_player_seat));
            // ��Ⱦ
            // һ������˳�
           TMP_Text quitPlayerIdText = PlayerNameRender.transform.GetChild(quit_player_seat + 1).GetComponent<TMP_Text>();
            Toggle quitPlayerPrepare = PlayerPrepareRender.transform.GetChild(quit_player_seat).GetComponent<Toggle>();
            quitPlayerIdText.text = "��";
            quitPlayerPrepare.isOn = false;
            Lobby.OtherPlayerQuitRoom(quit_player_seat);
            // �жϷ����Ƿ�ı�
            if (is_trans)
            {
                // �����ı�
                var new_host = json["new_host"].ToObject<JObject>();
                var new_host_name = new_host.ToObject<string>();
                var new_host_uid = new_host.ToObject<int>();
                var new_host_seat = new_host.ToObject<short>();

                player.RoomHost = (new_host_uid, new_host_name);
                // ��Ⱦ
                // �µķ���
                TMP_Text hostText = PlayerNameRender.transform.GetChild(new_host_seat + 1).GetComponent<TMP_Text>();
                Toggle hostPrepare = PlayerPrepareRender.transform.GetChild(new_host_seat).GetComponent<Toggle>();
                hostText.text = "������" + new_host_name;
                hostPrepare.isOn = true;
            }
        });

        manager.RegisterMessageHandler((short)MSG_IDS.MSG_DISCARD_RSP, (msgId, message) =>
        {
            Debug.Log("----------MSG_DISCARD_RSP----------");
            Debug.Log(message + "\n");
        });

    }    

    public async void SendMessage(short msgId, string content)
    {
        byte[] message = Encoding.UTF8.GetBytes(content);
        await manager.SendMessageAsync(msgId, message);
    }


    public async void LoginButton()
    {
        await Login();
    }

    public async Task Login()
    {
        manager = new TCPClientManager();

        // ע����Ϣ������
        RegisterHandlers();

        // ���ӵ�������
        await manager.ConnectAsync("192.168.3.111", 8090);

        // �����첽����
        var receiveTask = Task.Run(() => { }); // ������Ϣ������TCPManager �ڲ��Զ�����

        // ����
        // 1. ��¼ -- Login
        // �������ݶ���
        var data = new JObject();
        data["uid"] = 4;
        data["token"] = "123123123";
        string msg = data.ToString();
        SendMessage((short)MSG_IDS.MSG_CHAT_LOGIN, msg);
    }

    public void CreatRoomButton()
    {
        var data = new JObject();
        data["uid"] = player.Uid;
        data["token"] = player.Token;
        string msg = data.ToString();
        SendMessage((short)MSG_IDS.MSG_CREATE_ROOM, msg);
    }

    public void QuitRoomButton()
    {
        var data = new JObject();
        data["uid"] = player.Uid;
        data["token"] = player.Token;
        data["roomid"] = player.CurrentRoomId;
        string msg = data.ToString();     
        SendMessage((short)MSG_IDS.MSG_QUIT_ROOM, msg);
    }

    public void EnterRoomButton()
    {
        if(int.TryParse(Lobby.roomId, out int roomId))
        {
            var data = new JObject();
            data["uid"] = player.Uid;
            data["token"] = player.Token;
            data["roomid"] = roomId;
            string msg = data.ToString();
            SendMessage((short)MSG_IDS.MSG_ENTER_ROOM, msg);
        }
        else
        {
            Debug.Log("conversion failed.");
        }

    }
}

