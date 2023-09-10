#include "IRbabyUserSettings.h"
#include "IRbabySerial.h"
#include "IRbabyMQTT.h"
#include "WiFiManager.h"
#include "IRbabyGlobal.h"
#include "defines.h"
#include <LittleFS.h>
JsonDoc ConfigData;
JsonDoc ACStatus;

bool settingsSave()
{
    DEBUGLN("Save Config");
    File cache = LittleFS.open("/config", "w");
    if (!cache || (serializeJson(ConfigData, cache) == 0)) {
        ERRORLN("Failed to save config file");
        cache.close();
        return false;
    }
    cache.close();
    cache = LittleFS.open("/acstatus", "w");
    if (!cache || (serializeJson(ACStatus, cache) == 0))
    {
        ERRORLN("ERROR: Failed to save acstatus file");
        cache.close();
        return false;        
    }
    cache.close();
    return true;
}

bool settingsLoad()
{
    LittleFS.begin();
    int ret = false;
    FSInfo64 info;
    LittleFS.info64(info);
    DEBUGF("fs total bytes = %llu\n", info.totalBytes);
    if (LittleFS.exists("/config"))
    {
        File cache = LittleFS.open("/config", "r");
        if (!cache)
        {
            ERRORLN("Failed to read config file");
            return ret;
        }
        if (cache.size() > 0)
        {
            DeserializationError error = deserializeJson(ConfigData, cache);
            if (error)
            {
                ERRORLN("Failed to load config settings");
                return ret;
            }
            INFOLN("Load config data:");
            ConfigData["version"] = FIRMWARE_VERSION;
            serializeJsonPretty(ConfigData, Serial);
            Serial.println();
        }
        cache.close();
    } else {
        DEBUGLN("Don't exsit config");
    }

    if (LittleFS.exists("/acstatus")) {
        File cache = LittleFS.open("/acstatus", "r");
        if (!cache) {
            ERRORLN("Failed to read acstatus file");
            return ret;
        }
        if (cache.size() > 0) {
            DeserializationError error = deserializeJson(ACStatus, cache);
            if (error) {
                ERRORLN("Failed to load acstatus settings");
                return ret;
            }
        }
        cache.close();
    }
    ret = true;
    return ret;
}

void settingsClear()
{
    ESP.eraseConfig();
    LittleFS.format();
    ESP.reset();
}

void clearBinFiles() {
    Dir root = LittleFS.openDir(BIN_SAVE_PATH);
    while (root.next()) {
        LittleFS.remove(BIN_SAVE_PATH + root.fileName());
    }
}