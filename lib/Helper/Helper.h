#pragma once
#include <Arduino.h>
#ifdef SERIAL_DEBUG_LOG
#define DEBUG_LOG(arg) (Serial.print(arg));
#define DEBUG_LOG_LN(arg) (Serial.println(arg));
#else
#define DEBUG_LOG(arg)
#define DEBUG_LOG_LN(arg)
#endif

String uuid(String prefix);
// uint8_t mac[6] -> "8d-75-92-6a-40-e6";
void mac2str(const uint8_t *macArr, char *macStr);
// "8d-75-92-6a-40-e6" -> uint8_t mac[6];
void str2mac(const char *macStr, uint8_t *macArr);