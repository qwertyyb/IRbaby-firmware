#ifndef IRBABY_MQTT_H
#define IRBABY_MQTT_H
#include <WString.h>
#include <ArduinoJson.h>
#include "IRbabyGlobal.h"

/* MQTT 初始化 */
void mqttInit();

/* MQTT 重连 */
bool mqttReconnect();

/* MQTT 断开连接 */
void mqttDisconnect();

/* MQTT 请求连接 */
bool mqttConnected();

/* MQTT 接收循环 */
void mqttLoop();

/* MQTT 信息发送 */
void mqttPublish(const char *topic, JsonDoc *json);

/* MQTT 信息发送 */
void mqttPublishRetained(const char *topic, JsonDoc *json);

/* MQTT 连接检查 */
void mqttCheck();

/* 生成MQTT TOPIC */
String createTopic(const char * topic);

String createTopic(String topic);
#endif // IRBABY_MQTT_H