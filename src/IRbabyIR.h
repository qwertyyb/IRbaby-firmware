#ifndef IRBABY_IR_H
#define IRBABY_IR_H

#include "IRbabyGlobal.h"

void enableRecvIR();
void disableRecvIR();
void toggleRecvIR(const bool enable);
bool isRecvIREnable();
void recvIR();

bool sendIR(JsonDoc *payload);
bool sendIR(String name);

bool saveIRTemp(volatile uint16_t *buf, uint16_t len);
bool saveIR(JsonDoc *payload);
String listSavedIR();

#endif
