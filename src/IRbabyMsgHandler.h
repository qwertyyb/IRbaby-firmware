#ifndef IREASY_MSG_HANDLE_H
#define IREASY_MSG_HANDLE_H

#include <ArduinoJson.h>
#include "IRbabyGlobal.h"

void publishSystemInfo();

void publishSystemRunning();

void publishACState();

void requestHandler(String request);

void controlHandler(String control, JsonDoc *payload);

void msgHandler(String topic, JsonDoc *payload);

typedef enum msgtype
{
    mqtt,
    udp
} MsgType;

bool msgHandle(JsonDoc *p_recv_msg_doc, MsgType msg_type);

#endif