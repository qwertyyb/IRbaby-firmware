#include <IRrecv.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>
#include <LittleFS.h>
#include "defines.h"
#include "IRbabyMQTT.h"
#include "IRbabyGlobal.h"
#include "IRbabySerial.h"
#include "IRbabyIR.h"

// #define SAVE_A_PATH "/bin/";

// As this program is a special purpose capture/resender, let's use a larger
// than expected buffer so we can handle very large IR messages.
const uint16_t kCaptureBufferSize = 1024;  // 1024 == ~511 bits

// kTimeout is the Nr. of milli-Seconds of no-more-data before we consider a
// message ended.
const uint8_t kTimeout = 50;  // Milli-Seconds

// kFrequency is the modulation frequency all UNKNOWN messages will be sent at.
const uint16_t kFrequency = 38000;  // in Hz. e.g. 38kHz.

bool recvIREnabled = false;

// The IR receiver.
IRrecv irrecv(R_IR, kCaptureBufferSize, kTimeout, false);
IRsend irsend(T_IR);
// Somewhere to store the captured message.
decode_results results;

void enableRecvIR() {
  INFOLN("enableRecvIR");
  irrecv.enableIRIn();  // Start up the IR receiver.
  recvIREnabled = true;
}

void disableRecvIR() {
  INFOLN("disableRecvIR")
  irrecv.disableIRIn(); // stop the IR receiver.
  recvIREnabled = false;
}

void toggleRecvIR(const bool enable) {
  if (enable) return enableRecvIR();
  return disableRecvIR();
}

bool isRecvIREnable() {
  return recvIREnabled;
}

String dumpRawData(decode_results *results) {
  String output = "";
  // Dump data
  for (uint16_t i = 1; i < results->rawlen; i++) {
    uint32_t usecs;
    for (usecs = results->rawbuf[i] * kRawTick; usecs > UINT16_MAX;
         usecs -= UINT16_MAX) {
      output += uint64ToString(UINT16_MAX);
      if (i % 2)
        output += F(", 0,  ");
      else
        output += F(",  0, ");
    }
    output += uint64ToString(usecs, 10);
    if (i < results->rawlen - 1)
      output += kCommaSpaceStr;            // ',' not needed on the last one
    if (i % 2 == 0) output += "";  // Extra if it was even.
  }
  return output;
}

void recvIR() {
   // Check if an IR message has been received.
  if (!irrecv.decode(&results)) return;
  // We have captured something.
  String output = resultToHumanReadableBasic(&results) + "\n";
  String description = IRAcUtils::resultAcToString(&results);
  output += (D_STR_MESGDESC ": " + description + "\n");
  output += resultToSourceCode(&results);
  if (LOG_INFO) {
    // Display the basic output of what we found.
    Serial.println(output);
    yield();
  }
  send_msg_doc.clear();
  // protocol
  send_msg_doc["ptl"] = typeToString(results.decode_type, results.repeat);
  send_msg_doc["raw"] = dumpRawData(&results);
  send_msg_doc["len"] = getCorrectedRawLength(&results);
  send_msg_doc["val"] = "0x" + uint64ToString(results.value, 16);
  saveIRTemp(results.rawbuf, results.rawlen);
  irrecv.resume();
  disableRecvIR();
  mqttPublish("send/event/ir-received", &send_msg_doc);
}

uint16_t* getRaw(String str, uint16_t len) {
  uint16_t* raw = new uint16_t[len];
  Serial.println(str);
  int count = 0;
  String substr = "";
  for(unsigned int i = 0; i < str.length(); i++) {
    if (str[i] == ',') {
      Serial.printf("%d: %s\n", count, substr.c_str());
      raw[count++] = atol(substr.c_str());
      substr = "";
    } else {
      substr += str[i];
    }
  }
  Serial.printf("%d: %s\n", count, substr.c_str());
  raw[count++] = atol(substr.c_str());
  return raw;
};
bool sendIR(JsonDoc *payload) {
  disableRecvIR();
  DEBUGF("sendIR with json payload\n");
  if (payload->containsKey("raw")) {
    uint16_t len = (*payload)["len"].as<uint16_t>();
    uint16_t* raw = getRaw((*payload)["raw"], len);
    irsend.begin();
    irsend.sendRaw(raw, len, kFrequency);
    return true;
    delete [] raw;
  } else if (payload->containsKey("name")) {
    return sendIR((*payload)["name"].as<String>());
  }
  return false;
}
bool sendIR(String name) {
  DEBUGF("sendIR: %s\n", name.c_str());
  String save_path = "/bin/";
  save_path += name;
  if (!LittleFS.exists(save_path)) {
    return false;
  }
  File cache = LittleFS.open(save_path, "r");
  if (!cache) return false;
  uint16 content_size = cache.size();
  DEBUGF("content size = %d\n", content_size);
  if (content_size == 0) {
    cache.close();
    return false;
  }
  uint8 *content = (uint8 *)malloc(content_size * sizeof(uint8));
  cache.seek(0L, fs::SeekSet);
  cache.readBytes((char *)content, content_size);
  uint16_t *buf = (uint16 *)malloc(1024 * sizeof(uint16_t));
  disableRecvIR();
  for(int i = 0; i < content_size; i+=2) {
    buf[i / 2] = ((uint16_t)(content[i+1] << 8)) + content[i];
  }
  irsend.sendRaw(buf, content_size / 2, 38);
  free(content);
  free(buf);
  cache.close();
  return true;
}
bool saveIRTemp(volatile uint16_t *buf, uint16_t len) {
  DEBUGF("saveIRTemp: %d\n", len);
  String save_path = "/bin/";
  save_path += "temp";
  DEBUGF("save raw data as %s\n", save_path.c_str());
  File cache = LittleFS.open(save_path, "w");
  if (!cache)
  {
      ERRORLN("Failed to create file");
      return false;
  }
  for (size_t i = 0; i < len; i++)
      *(buf + i) = *(buf + i) * kRawTick;
  cache.write((char *)(buf + 1), (len - 1) * 2);
  cache.close();
  return true;
}

bool saveIR(String name) {
  String save_path = "/bin/";
  save_path += name;
  return LittleFS.rename("/bin/temp", save_path);
}

bool saveIR(JsonDoc *payload) {
  if (payload->containsKey("raw")) {
    uint16_t* raw = getRaw((*payload)["raw"], (*payload)["len"]);
    saveIRTemp(raw, (*payload)["len"]);
  }
  return saveIR((*payload)["name"].as<String>());
}

String listSavedIR() {
  const char *dirname = "/bin";
  DEBUGF("listSavedIR dirname: %s\n", dirname);

  Dir root = LittleFS.openDir(dirname);

  String output = "";
  while (root.next()) {
    File file = root.openFile("r");
    output += ("f:" + root.fileName());
    output += ("\ns:" + String(root.fileSize()));
    output += "\n";
    DEBUG("  FILE: ");
    DEBUG(root.fileName());
    DEBUG("  SIZE: ");
    DEBUG(file.size());
    time_t cr = file.getCreationTime();
    time_t lw = file.getLastWrite();
    file.close();
    struct tm *tmstruct = localtime(&cr);
    DEBUGF("    CREATION: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
    tmstruct = localtime(&lw);
    DEBUGF("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
  }

  DEBUGF("listSavedIR output: %s\n", output.c_str());
  return output;
}