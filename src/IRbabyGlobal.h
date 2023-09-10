/*
 * @Author: Caffreyfans
 * @Date: 2021-07-06 20:59:02
 * @LastEditTime: 2021-07-12 23:04:51
 * @Description: 
 */
#ifndef IRBABY_GLOBAL_H
#define IRBABY_GLOBAL_H

#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <WiFiClient.h>
#include <ir_Coolix.h>
#include <IRrecv.h>
#ifdef USE_LED
#include "jled.h"
#endif // USE_LED

#define JsonDoc StaticJsonDocument<2048>

/* goable json variable */
extern JsonDoc recv_msg_doc;
extern JsonDoc send_msg_doc;
extern JsonDoc udp_msg_doc;
extern JsonDoc mqtt_msg_doc;

extern WiFiManager wifi_manager;
extern WiFiClient wifi_client;

extern IRCoolixAC mideaAC;

extern uint8_t ir_send_pin;
extern uint8_t ir_receive_pin;
#ifdef USE_RF
extern uint8_t rf315_send_pin;
extern uint8_t rf315_receive_pin;
extern uint8_t rf433_send_pin;
extern uint8_t rf433_receive_pin;
#endif
#ifdef USE_LED
extern JLed led;
#endif // USE_LED
#endif // IRBABY_GLOBAL_H