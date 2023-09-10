#ifndef IRBABY_UDP_H
#define IRBABY_UDP_H
#include "ArduinoJson.h"
#include "ESP8266WiFi.h"
#include "IRbabyGlobal.h"

#define UDP_PORT 4210
#define UDP_PACKET_SIZE 255

void udpInit();
char* udpRecive();
uint32_t sendUDP(JsonDoc* doc, IPAddress ip);
uint32_t returnUDP(JsonDoc* doc);

extern IPAddress remote_ip;
#endif // IRBABY_UDP_H