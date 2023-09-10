#include "IRbabyMQTT.h"
#include "PubSubClient.h"
#include "IRbabySerial.h"
#include "ArduinoJson.h"
#include "IRbabyUserSettings.h"
#include "IRbabyMsgHandler.h"
#include "IRbabyGlobal.h"
#include <ArduinoJson.h>

PubSubClient mqtt_client(wifi_client);

void callback(char *topic, byte *payload, unsigned int length);

void mqttInit()
{
    INFOLN("MQTT Init");
    mqttReconnect();
    mqtt_client.setCallback(callback);
}

/**************************************
 * sub example: /IRbaby/chip_id/set/#
 **************************************/
bool mqttReconnect()
{
    bool flag = false;
    INFOF("MQTT client is connected: %d\n", mqttConnected());
    if (!mqttConnected())
    {
        const char *host = "192.168.31.31"; // mqtt_obj["host"];
        int port = 1883; // mqtt_obj["port"];
        const char *user = "user"; // mqtt_obj["user"];
        const char *password = "password"; //mqtt_obj["password"];
        if (host && port)
        {
            mqtt_client.setServer(host, port);
            String chip_id = String(ESP.getChipId(), HEX);
            chip_id.toUpperCase();
            DEBUGF("Trying to connect %s:%d\n", host, port);
            if (mqtt_client.connect(chip_id.c_str(), user,
                                    password))
            {
                String sub_topic = createTopic("receive/#");
                DEBUGF("MQTT subscribe %s\n", sub_topic.c_str());
                publishSystemInfo();
                publishSystemRunning();
                mqtt_client.subscribe(sub_topic.c_str());
                flag = true;
            }
        }
        INFOF("MQTT state rc = %d\n", mqtt_client.state());
    }
    delay(1000);
    return flag;
}

void mqttDisconnect()
{
    mqtt_client.disconnect();
}

void callback(char *topic, byte *payload, unsigned int length)
{
    mqtt_msg_doc.clear();

    String payload_str = "";
    for (uint32_t i = 0; i < length; i++)
        payload_str += (char)payload[i];
    String topic_str(topic);
    if (LOG_DEBUG) {
        Serial.println(topic_str);
        Serial.println(payload_str);
        Serial.println();
    }
    deserializeJson(recv_msg_doc, payload_str.c_str());
    return msgHandler(String(topic), &recv_msg_doc);
    uint8_t index = 0;
    String option;
    String func;
    do
    {
        int divsion = topic_str.lastIndexOf("/");
        String tmp = topic_str.substring(divsion + 1, -1);
        topic_str = topic_str.substring(0, divsion);
        switch (index++)
        {
        case 0:
            func = tmp;
            break;
        case 1:
            mqtt_msg_doc["params"]["file"] = tmp;
            break;
        case 2:
            mqtt_msg_doc["params"]["type"] = tmp;
            option = tmp;
            break;
        case 3:
            mqtt_msg_doc["params"]["signal"] = tmp;
            break;
        case 4:
            mqtt_msg_doc["cmd"] = tmp;
        default:
            break;
        }
    } while (topic_str.lastIndexOf("/") > 0);
    mqtt_msg_doc["params"][option][func] = payload_str;
    msgHandle(&mqtt_msg_doc, MsgType::mqtt);
}

bool mqttConnected()
{
    return mqtt_client.connected();
}

void mqttLoop()
{
    mqtt_client.loop();
}

void mqttPublish(const char *topic, JsonDoc *json)
{
    char str[2048];
    serializeJson(*json, str);
    Serial.printf("mqttPublish: %s\n", str);
    mqtt_client.publish(createTopic(topic).c_str(), str);
}

void mqttPublishRetained(const char *topic, JsonDoc *json)
{
    char str[1024];
    serializeJson(*json, str);
    mqtt_client.publish(createTopic(topic).c_str(), str, true);
}

void mqttCheck()
{
    if (!mqttConnected())
    {
        DEBUGLN("MQTT disconnect, try to reconnect");
        mqtt_client.disconnect();
        mqttReconnect();
#ifdef USE_LED
        led.Blink(500, 500);
#endif // USE_LED
    } else {
#ifdef USE_LED
        led.On();
#endif // USE_LED
    }
}

String createTopic(const char* topic) {
    return String("device/") + WiFi.macAddress() + "/" + String(topic);
}
String createTopic(String topic) {
    return createTopic(topic.c_str());
}