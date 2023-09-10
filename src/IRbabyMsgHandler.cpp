#include "IRbabyMsgHandler.h"
#include "IRbabySerial.h"
#include "IRbabyUserSettings.h"
#include "IRbabyMQTT.h"
#include "IRbabyUDP.h"
#include "ESP8266WiFi.h"
#include "IRbabyOTA.h"
#include "defines.h"
#include <LittleFS.h>
#include "IRbabyRF.h"
#include "IRbabyGlobal.h"
#include "IRbabyIR.h"

void publishSystemInfo()
{
    String mac = WiFi.macAddress();
    send_msg_doc.clear();
    send_msg_doc["MAC"] = mac;
    send_msg_doc["IP"] = WiFi.localIP().toString();
    mqttPublishRetained("send/system/info", &send_msg_doc);
}

void publishSystemRunning() {
    String free_mem = String(ESP.getFreeHeap() / 1024) + "KB";
    String chip_id = String(ESP.getChipId(), HEX);
    chip_id.toUpperCase();
    String cpu_freq = String(ESP.getCpuFreqMHz()) + "MHz";
    String flash_speed = String(ESP.getFlashChipSpeed() / 1000000);
    String flash_size = String(ESP.getFlashChipSize() / 1024) + "KB";
    String sketch_size = String(ESP.getSketchSize() / 1024) + "KB";
    String reset_reason = ESP.getResetReason();
    String sketch_space = String(ESP.getFreeSketchSpace() / 1024) + "KB";
    FSInfo fsinfo;
    LittleFS.info(fsinfo);
    String fs_total_bytes = String(fsinfo.totalBytes / 1024) + "KB";
    String fs_used_bytes = String(fsinfo.usedBytes / 1024) + "KB";
    send_msg_doc.clear();
    send_msg_doc["free_mem"] = free_mem;
    send_msg_doc["chip_id"] = chip_id;
    send_msg_doc["cpu_freq"] = cpu_freq;
    send_msg_doc["flash_speed"] = flash_speed;
    send_msg_doc["flash_size"] = flash_size;
    send_msg_doc["reset_reason"] = reset_reason;
    send_msg_doc["sketch_space"] = sketch_space;
    send_msg_doc["fs_total_bytes"] = fs_total_bytes;
    send_msg_doc["fs_used_bytes"] = fs_used_bytes;
    send_msg_doc["version_name"] = FIRMWARE_VERSION;
    send_msg_doc["version_code"] = VERSION_CODE;
    send_msg_doc["ip"] = WiFi.localIP().toString();
    mqttPublishRetained("send/system/running", &send_msg_doc);
}

void publishACState()
{
    send_msg_doc.clear();
    // IR Receiver state
    send_msg_doc["recvingIR"] = isRecvIREnable();
    send_msg_doc["savedIRList"] = listSavedIR();

    send_msg_doc["power"] = mideaAC.getPower();
    send_msg_doc["temperature"] = mideaAC.getTemp();
    send_msg_doc["mode"] = (int)mideaAC.toCommonMode(mideaAC.getMode());
    send_msg_doc["fanspeed"] = (int)mideaAC.toCommonFanSpeed(mideaAC.getFan());
    send_msg_doc["swingv"] = mideaAC.getSwingVStep();
    send_msg_doc["swing"] = mideaAC.getSwing();
    send_msg_doc["light"] = mideaAC.getLed();
    // send_msg_doc["eco"] = mideaAC.getEconoToggle();
    // send_msg_doc["quiet"] = mideaAC.getQuiet();
    mqttPublishRetained("send/state/$all", &send_msg_doc);
}

void requestHandler(String request)
{
    if (request.equals("system/info")) {
        return publishSystemInfo();
    }
    if (request.equals("system/running")) {
        return publishSystemRunning();
    }
    if (request.equals("state/$all")) {
        return publishACState();
    }
    if (request.equals("state/savedIRList")) {
        send_msg_doc.clear();
        send_msg_doc["value"] = listSavedIR();
        mqttPublish("send/state/savedIRList", &send_msg_doc);
    }
}

void controlHandler(String control, JsonDoc *msg)
{
    JsonObject payload = msg->as<JsonObject>();
    INFOF("controlHandler control: %s\n", control.c_str());
    if (!control.startsWith("state/")) return;
    control.replace("state/", "");
    if (control.equals("recvingIR")) {
        toggleRecvIR(payload["value"].as<bool>());
        publishACState();
        return;
    }
    if (!control.equals("$all")) {
        send_msg_doc[control] = payload["value"];
    }
    if (payload.containsKey("power")) {
        INFOF("setPower: %d\n", payload["power"].as<bool>());
        mideaAC.setPower(payload["power"].as<bool>());
    }
    if (payload.containsKey("mode")) {
        mideaAC.setMode(mideaAC.convertMode((stdAc::opmode_t)payload["mode"].as<int8_t>()));
    }
    if (payload.containsKey("temperature")) {
        mideaAC.setTemp(payload["temperature"].as<int>());
    }
    if (payload.containsKey("light")) {
        mideaAC.setLed();
    }
    if (payload.containsKey("swingv")) {
        mideaAC.setSwing();
    }
    if (payload.containsKey("fanspeed")) {
        mideaAC.setFan(mideaAC.convertFan((stdAc::fanspeed_t)payload["fanspeed"].as<int8_t>()));
    }
    if (payload.containsKey("eco")) {
        // mideaAC.setEconoToggle(payload["eco"].as<bool>());
    }
    if (payload.containsKey("quiet")) {
        // mideaAC.setQuiet(payload["quiet"].as<bool>());
    }
    mideaAC.send();
    publishACState();
}

void commandHandler(String topic, JsonDoc *payload)
{
    if (topic.equals("sendIR")) {
        sendIR(payload);
        return;
    }
    if (topic.equals("saveIR")) {
        saveIR(payload);
        publishACState();
        return;
    }
}

void msgHandler(String topic, JsonDoc *payload) {
    String receiveTopicPrefix = createTopic("receive/");
    topic.replace(receiveTopicPrefix, "");
    if (topic.startsWith("$request/")) {
        topic.replace("$request/", "");
        return requestHandler(topic);
    }
    if (topic.startsWith("control/")) {
        topic.replace("control/", "");
        return controlHandler(topic, payload);
    }
    if (topic.startsWith("command/")) {
        topic.replace("command/", "");
        return commandHandler(topic, payload);
    }
}
