/*
 * @Author: Caffreyfans
 * @Date: 2021-07-06 20:59:02
 * @LastEditTime: 2021-07-12 21:24:00
 * @Description:
 */
#ifndef IRBABY_USER_SETTINGS_H
#define IRBABY_USER_SETTINGS_H
#include <ArduinoJson.h>
#include "IRbabyGlobal.h"

#define BIN_SAVE_PATH "/bin/"

/* 保存配置信息 */
bool settingsSave();

/* 清除配置信息 */
void settingsClear();

/* 加载配置信息 */
bool settingsLoad();

void clearBinFiles();
extern JsonDoc ConfigData;
extern JsonDoc ACStatus;
#endif  // IRBABY_USER_SETTINGS_H