using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using uPLibrary.Networking.M2Mqtt;
using uPLibrary.Networking.M2Mqtt.Messages;

namespace AGLStudio.Core.Helpers;

public class MQTTClient
{
    public delegate void onDataReceived();
    public delegate void onSettingsReceived();
    public delegate void onCommandResponseReceived();
    public delegate void onPingReceived();
    public delegate void onMiscReceived(string topic, string data);

    public onMiscReceived miscCallback = null;

    private MqttClient client = null;
    private readonly string[] topics = new string[] { "larch/detections", "cs/settings", "cs/command" };
    private readonly List<string> misc_topics = new List<string>();

    public int subscribe(string topic)
    {
        if (!isConnected())
            return 0;

        client.Subscribe(new string[] { topic }, new byte[] { MqttMsgBase.QOS_LEVEL_AT_MOST_ONCE });

        return 1;
    }

    public void unsubscribe_all()
    {
        foreach (var topic in misc_topics)
        {
            client.Unsubscribe(new string[] { topic });
        }

        misc_topics.Clear();
    }

    private string current_ip = "";
    private int current_port = 0;
    private bool current_is_subscribe = false;

    public int connect(string ip, int port, bool is_subscribe)
    {
        var ret = 1;

        current_ip = ip;
        current_port = port;
        current_is_subscribe = is_subscribe;

        try
        {
            client = new MqttClient(ip, port, false, null, null, MqttSslProtocols.None);
            client.MqttMsgPublishReceived += client_MqttMsgPublishReceived;

            var clientId = Guid.NewGuid().ToString();
            client.Connect(clientId);

            if (is_subscribe)
            {
                for (var i = 0; i < topics.Length; i++)
                {
                    client.Subscribe(new string[] { topics[i] }, new byte[] { MqttMsgBase.QOS_LEVEL_AT_MOST_ONCE });
                }
            }
        }
        catch (Exception e)
        {
            ret = 0;
        }

        return ret;
    }

    public int connect(string ip, int port)
    {
        return connect(ip, port, true);
    }

    private int reconnect()
    {
        return connect(current_ip, current_port, current_is_subscribe);
    }

    public bool isConnected()
    {
        return (client != null && client.IsConnected);
    }

    public bool disconnect()
    {
        if (isConnected())
        {
            client.Unsubscribe(topics);
            client.Disconnect();
        }

        return true;
    }

    private void client_MqttMsgPublishReceived(object sender, MqttMsgPublishEventArgs e)
    {
        var topic = e.Topic;
        var json = System.Text.Encoding.Default.GetString(e.Message);

        //Debug.WriteLine(json);

        switch (topic)
        {
            case "kuka/data":
                //client_MqttDataReceived(json);
                break;
            case "kuka/settings":
                //client_MqttSettingsReceived(json);
                break;
            case "kuka/command":
                //client_MqttResponseReceived(json);
                break;
            default:
                if (miscCallback != null)
                    miscCallback(topic, json);
                break;
        }
    }
}
