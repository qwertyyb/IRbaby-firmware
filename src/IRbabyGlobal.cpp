/*
 * @Author: Caffreyfans
 * @Date: 2021-07-06 20:59:02
 * @LastEditTime: 2021-07-12 23:05:04
 * @Description: 
 */
#include "IRbabyGlobal.h"
#include "defines.h"
#include <ir_Coolix.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>

JsonDoc recv_msg_doc;
JsonDoc send_msg_doc;
JsonDoc udp_msg_doc;
JsonDoc mqtt_msg_doc;

WiFiManager wifi_manager;
WiFiClient wifi_client;

const uint16_t kCaptureBufferSize = 1024;

uint8_t ir_send_pin = T_IR;
uint8_t ir_receive_pin = R_IR;

IRCoolixAC mideaAC(T_IR, true);

#ifdef USE_RF
uint8_t rf315_send_pin = T_315;
uint8_t rf315_receive_pin = R_315;
uint8_t rf433_send_pin = T_433;
uint8_t rf433_receive_pin = R_433;
#endif
#ifdef USE_LED
JLed led = JLed(LED_PIN);
#endif // USE_LED