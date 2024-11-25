using System;
using System.Collections;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using UnityEngine;
using UnityEngine.XR;
using Newtonsoft.Json;

public enum ErrorCodes
{
    Success = 0,
    Error_Json = 1001, //Json��������
    RPCFailed = 1002,  //RPC�������
    VerifyExpired = 1003,  //��֤�����
    VerifyCodeErr = 1004,  //��֤�����
    UserExist = 1005, //�û��Ѿ�����
    PasswdErr = 1006,  //�������
    EmailNotMatch = 1007,  //���䲻ƥ��
    PasswdUpFailed = 1008, //��������ʧ��
    PasswdInvalid = 1009,  //�������ʧ��
    TokenInvalid = 1010,//TokenʧЧ
    UidInvalid = 1011, //uid��Ч
    CreateRoomFailed = 1012,//��������ʧ��
    NotFoundRoom = 1013, // ���䲻����
    CantEnterRoom = 1014,  // ��������
    ChangeReadyError = 1015, // ׼���¼�����
    RoomStartError = 1016,// ���俪ʼ����
    CARDNOTFOUND = 1017, // �Ʋ�����
    CANTDRAWTILE = 1018
};

public enum MSG_IDS
{
    MSG_CHAT_LOGIN = 1005,      //�û���½
    MSG_CHAT_LOGIN_RSP = 1006,
    MSG_CREATE_ROOM = 1007,     // �����·���
    MSG_CREATE_ROOM_RSP = 1008,
    MSG_ENTER_ROOM = 1009,      // ���뷿��
    MSG_ENTER_ROOM_RSP = 1010,
    MSG_ENTER_ROOM_BROADC = 1011,
    MSG_QUIT_ROOM = 1012,       // �˳�����
    MSG_QUIT_ROOM_RSP = 1013,
    MSG_QUIT_ROOM_BROADC = 1014,
    MSG_CHANGE_READY = 1015,    // ���׼�� 
    MSG_CHANGE_READY_RSP = 1016,
    MSG_CHANGE_READY_BROADC = 1017,
    MSG_START_GAME = 1018,      // ��ʼ��Ϸ
    MSG_START_GAME_RSP = 1019,
    MSG_START_GAME_BROADC = 1020,
    MSG_DWAR_TITLE = 1021,      // ����
    MSG_DWAR_TITLE_RSP = 1022,
    MSG_DWAR_TITLE_BROADC = 1023,
    MSG_DISCARD = 1024,         // ����
    MSG_DISCARD_RSP = 1025,
    MSG_DISCARD_BROADC = 1025
};

public class TCPClientManager : MonoBehaviour
{
    public delegate void MessageHandler(short msgId, string msgData);

    private TcpClient client;
    private NetworkStream stream;
    private Dictionary<short, MessageHandler> messageHandlers = new Dictionary<short, MessageHandler>();
    private const int HEAD_TOTAL_LEN = 4; // 2 bytes for MsgId + 2 bytes for MsgLen
    private const int MAX_LENGTH = 4096;

    private byte[] headerBuffer = new byte[HEAD_TOTAL_LEN];
    private byte[] bodyBuffer;
    private short msgId;

    public TCPClientManager()
    {
        client = new TcpClient();
    }
    public async Task ConnectAsync(string host, int port)
    {
        try
        {
            await client.ConnectAsync(host, port);
            stream = client.GetStream();
            Console.WriteLine("Connected to server.");
            StartReceive();
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Failed to connect: {ex.Message}");
        }
    }

    private async void StartReceive()
    {
        await AsyncReadHeader();
    }

    public void RegisterMessageHandler(short msgId, MessageHandler handler)
    {
        messageHandlers[msgId] = handler;
    }

    public async Task SendMessageAsync(short msgId, byte[] message)
    {
        try
        {
            short msgLen = (short)message.Length;
            byte[] data = new byte[HEAD_TOTAL_LEN + msgLen];
            BitConverter.GetBytes(IPAddress.HostToNetworkOrder(msgId)).CopyTo(data, 0);
            BitConverter.GetBytes(IPAddress.HostToNetworkOrder(msgLen)).CopyTo(data, 2);
            Array.Copy(message, 0, data, HEAD_TOTAL_LEN, msgLen);

            await stream.WriteAsync(data, 0, data.Length);
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Failed to send message: {ex.Message}");
        }
    }


    private async Task AsyncReadHeader()
    {
        try
        {
            int bytesRead = await ReadAsync(headerBuffer, HEAD_TOTAL_LEN);
            if (bytesRead < HEAD_TOTAL_LEN)
            {
                Console.WriteLine("Failed to read message header.");
                return;
            }

            msgId = IPAddress.NetworkToHostOrder(BitConverter.ToInt16(headerBuffer, 0));
            short msgLen = IPAddress.NetworkToHostOrder(BitConverter.ToInt16(headerBuffer, 2));

            if (msgLen > MAX_LENGTH)
            {
                Console.WriteLine($"Invalid message length: {msgLen}");
                return;
            }

            bodyBuffer = new byte[msgLen];
            await AsyncReadBody(msgLen);
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error reading header: {ex.Message}");
        }
    }

    private async Task AsyncReadBody(int msgLen)
    {
        try
        {
            int bytesRead = await ReadAsync(bodyBuffer, msgLen);
            if (bytesRead < msgLen)
            {
                Console.WriteLine("Failed to read message body.");
                return;
            }

            HandleMessage(msgId, Encoding.UTF8.GetString(bodyBuffer));
            await AsyncReadHeader();
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error reading body: {ex.Message}");
        }
    }

    private void HandleMessage(short msgId, string message)
    {
        if (messageHandlers.TryGetValue(msgId, out var handler))
        {
            handler(msgId, message);
        }
        else
        {
            Console.WriteLine($"No handler for message ID: {msgId}");
        }
    }

    private async Task<int> ReadAsync(byte[] buffer, int length)
    {
        int totalBytesRead = 0;

        while (totalBytesRead < length)
        {
            int bytesRead = await stream.ReadAsync(buffer, totalBytesRead, length - totalBytesRead);
            if (bytesRead == 0) throw new Exception("Connection closed by remote host.");
            totalBytesRead += bytesRead;
        }

        return totalBytesRead;
    }
}
