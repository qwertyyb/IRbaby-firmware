/**
 *
 * Copyright (c) 2020 IRbaby
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <Ticker.h>

#include "IRbabyGlobal.h"
#include "IRbabyMQTT.h"
#include "IRbabyMsgHandler.h"
#include "IRbabyOTA.h"
#include "IRbabyRF.h"
#include "IRbabyUDP.h"
#include "IRbabyUserSettings.h"
#include "OneButton.h"
#include "defines.h"
#include "IRbabyIR.h"

#ifdef USE_SENSOR
#include "IRbabyBinarySensor.h"
Ticker sensor_ticker;  // binary sensor ticker
#endif                 // USE_SENSOR

void uploadIP();    // device info upload to devicehive
Ticker mqtt_check;  // MQTT check timer
Ticker disable_ir;  // disable IR receive
Ticker disable_rf;  // disable RF receive
Ticker save_data;   // save data

OneButton button(RESET_PIN, true);
void setup() {
  if (LOG_DEBUG || LOG_ERROR || LOG_INFO) Serial.begin(BAUD_RATE);
  pinMode(RESET_PIN, INPUT_PULLUP);
  INFOLN();
  INFOLN("8888888 8888888b.  888               888               ");
  INFOLN("  888   888   Y88b 888               888               ");
  INFOLN("  888   888    888 888               888               ");
  INFOLN("  888   888   d88P 88888b.   8888b.  88888b.  888  888 ");
  INFOLN("  888   8888888P   888  88b      88b 888  88b 888  888 ");
  INFOLN("  888   888 T88b   888  888 .d888888 888  888 888  888 ");
  INFOLN("  888   888  T88b  888 d88P 888  888 888 d88P Y88b 888 ");
  INFOLN("8888888 888   T88b 88888P    Y888888 88888P     Y88888 ");
  INFOLN("                                              Y8b d88P ");
  INFOLN("                                                 Y88P  ");
#ifdef USE_LED
  led.Off();
#endif  // USE_LED
  wifi_manager.autoConnect();
#ifdef USE_LED
  led.On();
#endif  // USE_LED

  settingsLoad();  // load user settings form fs
  delay(5);
  udpInit();   // udp init
  mqttInit();  // mqtt init
  mideaAC.begin();

#ifdef USE_RF
  initRF();  // RF init
#endif

  // loadIRPin(ConfigData["pin"]["ir_send"], ConfigData["pin"]["ir_receive"]);

#ifdef USE_INFO_UPLOAD
  uploadIP();
#endif

  mqtt_check.attach_scheduled(MQTT_CHECK_INTERVALS, mqttCheck);
  disable_rf.attach_scheduled(DISABLE_SIGNAL_INTERVALS, disableRF);
  save_data.attach_scheduled(SAVE_DATA_INTERVALS, settingsSave);

#ifdef USE_SENSOR
  binary_sensor_init();
  sensor_ticker.attach_scheduled(SENSOR_UPLOAD_INTERVAL, binary_sensor_loop);
#endif  // USE_SENSOR

  button.setPressTicks(3000);
  button.attachLongPressStart([]() { settingsClear(); });
#ifdef USE_LED
  led.Blink(200, 200).Repeat(10);
#endif  // USE_LED
}

void loop() {
  recvIR();
#ifdef USE_RF
  /* RF receive */
  recvRF();
#endif
  /* UDP receive and handle */
  char *msg = udpRecive();
  if (msg) {
    udp_msg_doc.clear();
    DeserializationError error = deserializeJson(udp_msg_doc, msg);
    if (error) ERRORLN("Failed to parse udp message");
    // msgHandle(&udp_msg_doc, MsgType::udp);
  }

  /* mqtt loop */
  mqttLoop();
  yield();
}

// only upload chip id
void uploadIP() {
  HTTPClient http;
  StaticJsonDocument<128> body_json;
  String chip_id = String(ESP.getChipId(), HEX);
  chip_id.toUpperCase();
  String head = "http://playground.devicehive.com/api/rest/device/";
  head += chip_id;
  http.begin(wifi_client, head);
  http.addHeader("Content-Type", "application/json");
  http.addHeader(
      "Authorization",
      "Bearer "
      "eyJhbGciOiJIUzI1NiJ9."
      "eyJwYXlsb2FkIjp7ImEiOlsyLDMsNCw1LDYsNyw4LDksMTAsMTEsMTIsMTUsMTYsMTddLCJl"
      "IjoxNzQzNDM2ODAwMDAwLCJ0IjoxLCJ1Ijo2NjM1LCJuIjpbIjY1NDIiXSwiZHQiOlsiKiJd"
      "fX0.WyyxNr2OD5pvBSxMq84NZh6TkNnFZe_PXenkrUkRSiw");
  body_json["name"] = chip_id;
  body_json["networkId"] = "6542";
  String body = body_json.as<String>();
  INFOF("update %s to devicehive\n", body.c_str());
  http.PUT(body);
  http.end();
}




// /*
//  * IRremoteESP8266: SmartIRRepeater.ino - Record and playback IR codes.
//  * Copyright 2019 David Conran (crankyoldgit)
//  *
//  * This program will try to capture incoming IR messages and tries to
//  * intelligently replay them back.
//  * It uses the advanced detection features of the library, and the custom
//  * sending routines. Thus it will try to use the correct frequencies,
//  * duty cycles, and repeats as it thinks is required.
//  * Anything it doesn't understand, it will try to replay back as best it can,
//  * but at 38kHz.
//  * Note:
//  *   That might NOT be the frequency of the incoming message, so some not
//  *   recogised messages that are replayed may not work. The frequency & duty
//  *   cycle of unknown incoming messages is lost at the point of the Hardware IR
//  *   demodulator. The ESP can't see it.
//  *
//  *                               W A R N I N G
//  *   This code is just for educational/example use only. No help will be given
//  *   to you to make it do something else, or to make it work with some
//  *   weird device or circuit, or to make it more usable or practical.
//  *   If it works for you. Great. If not, Congratulations on changing/fixing it.
//  *
//  * An IR detector/demodulator must be connected to the input, kRecvPin.
//  * An IR LED circuit must be connected to the output, kIrLedPin.
//  *
//  * Example circuit diagrams (both are needed):
//  *  https://github.com/crankyoldgit/IRremoteESP8266/wiki#ir-receiving
//  *  https://github.com/crankyoldgit/IRremoteESP8266/wiki#ir-sending
//  *
//  * Common mistakes & tips:
//  *   * Don't just connect the IR LED directly to the pin, it won't
//  *     have enough current to drive the IR LED effectively.
//  *   * Make sure you have the IR LED polarity correct.
//  *     See: https://learn.sparkfun.com/tutorials/polarity/diode-and-led-polarity
//  *   * Some digital camera/phones can be used to see if the IR LED is flashed.
//  *     Replace the IR LED with a normal LED if you don't have a digital camera
//  *     when debugging.
//  *   * Avoid using the following pins unless you really know what you are doing:
//  *     * Pin 0/D3: Can interfere with the boot/program mode & support circuits.
//  *     * Pin 1/TX/TXD0: Any serial transmissions from the ESP will interfere.
//  *     * Pin 3/RX/RXD0: Any serial transmissions to the ESP will interfere.
//  *     * Pin 16/D0: Has no interrupts on the ESP8266, so can't be used for IR
//  *       receiving with this library.
//  *   * ESP-01 modules are tricky. We suggest you use a module with more GPIOs
//  *     for your first time. e.g. ESP-12 etc.
//  *
//  * Changes:
//  *   Version 1.0: June, 2019
//  *     - Initial version.
//  */

// #include <Arduino.h>
// #include <IRsend.h>
// #include <IRrecv.h>
// #include <IRremoteESP8266.h>
// #include <IRac.h>
// #include <IRtext.h>
// #include <IRutils.h>

// // ==================== start of TUNEABLE PARAMETERS ====================

// // The GPIO an IR detector/demodulator is connected to. Recommended: 14 (D5)
// // Note: GPIO 16 won't work on the ESP8266 as it does not have interrupts.
// // Note: GPIO 14 won't work on the ESP32-C3 as it causes the board to reboot.
// #ifdef ARDUINO_ESP32C3_DEV
// const uint16_t kRecvPin = 10;  // 14 on a ESP32-C3 causes a boot loop.
// #else  // ARDUINO_ESP32C3_DEV
// const uint16_t kRecvPin = 5;
// #endif  // ARDUINO_ESP32C3_DEV

// // GPIO to use to control the IR LED circuit. Recommended: 4 (D2).
// const uint16_t kIrLedPin = 14;

// // The Serial connection baud rate.
// // NOTE: Make sure you set your Serial Monitor to the same speed.
// const uint32_t kBaudRate = 115200;

// // As this program is a special purpose capture/resender, let's use a larger
// // than expected buffer so we can handle very large IR messages.
// const uint16_t kCaptureBufferSize = 1024;  // 1024 == ~511 bits

// // kTimeout is the Nr. of milli-Seconds of no-more-data before we consider a
// // message ended.
// const uint8_t kTimeout = 50;  // Milli-Seconds

// // kFrequency is the modulation frequency all UNKNOWN messages will be sent at.
// const uint16_t kFrequency = 38000;  // in Hz. e.g. 38kHz.

// // ==================== end of TUNEABLE PARAMETERS ====================

// // The IR transmitter.
// IRsend irsend(kIrLedPin);
// // The IR receiver.
// IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, false);
// // Somewhere to store the captured message.
// decode_results results;

// // This section of code runs only once at start-up.
// void setup() {
//   irrecv.enableIRIn();  // Start up the IR receiver.
//   irsend.begin();       // Start up the IR sender.

//   Serial.begin(kBaudRate, SERIAL_8N1);
//   while (!Serial)  // Wait for the serial connection to be establised.
//     delay(50);
//   Serial.println();

//   Serial.print("SmartIRRepeater is now running and waiting for IR input "
//                "on Pin ");
//   Serial.println(kRecvPin);
//   Serial.print("and will retransmit it on Pin ");
//   Serial.println(kIrLedPin);
// }

// // The repeating section of the code
// void loop() {
//   // Check if an IR message has been received.
//   if (irrecv.decode(&results)) {  // We have captured something.
//     // The capture has stopped at this point.
//     decode_type_t protocol = results.decode_type;
//     uint16_t size = results.bits;
//     // Display the basic output of what we found.
//     Serial.print(resultToHumanReadableBasic(&results));
//     // Display any extra A/C info if we have it.
//     String description = IRAcUtils::resultAcToString(&results);
//     if (description.length()) Serial.println(D_STR_MESGDESC ": " + description);
//     yield();  // Feed the WDT as the text output can take a while to print.
//     Serial.println(resultToSourceCode(&results));
//     Serial.println();    // Blank line between entries
//     yield();
//     bool success = true;
//     // Is it a protocol we don't understand?
//     if (protocol == decode_type_t::UNKNOWN || true) {  // Yes.
//       // Convert the results into an array suitable for sendRaw().
//       // resultToRawArray() allocates the memory we need for the array.
//       uint16_t *raw_array = resultToRawArray(&results);
//       // Find out how many elements are in the array.
//       size = getCorrectedRawLength(&results);
//       // Send it out via the IR LED circuit.
//       irsend.sendRaw(raw_array, size, kFrequency);
//       // Deallocate the memory allocated by resultToRawArray().
//       delete [] raw_array;
//     } else if (hasACState(protocol)) {  // Does the message require a state[]?
//       // It does, so send with bytes instead.
//       success = irsend.send(protocol, results.state, size / 8);
//     } else {  // Anything else must be a simple message protocol. ie. <= 64 bits
//       success = irsend.send(protocol, results.value, size);
//     }
//     // Resume capturing IR messages. It was not restarted until after we sent
//     // the message so we didn't capture our own message.
//     irrecv.resume();

//     // Display a crude timestamp & notification.
//     uint32_t now = millis();
//     Serial.printf(
//         "%06u.%03u: A %d-bit %s message was %ssuccessfully retransmitted.\n",
//         now / 1000, now % 1000, size, typeToString(protocol).c_str(),
//         success ? "" : "un");
//   }
//   yield();  // Or delay(milliseconds); This ensures the ESP doesn't WDT reset.
// }