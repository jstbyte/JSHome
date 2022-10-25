#pragma once
#include "credentials.h"
#include <Arduino.h>
#include <LittleFS.h>
#define SPIFFS LittleFS
#include <ESP8266WiFi.h>
#include <MsgPacketizer.h>
#include <RTCMemory.h>
#include "Helper.h"
#include "DigiOut.h"
#include "Debouncer.h"

typedef struct
{
    u8_t bootCount;
    bool pinStates[7];
    int wifiRetryTimeout; // 0 = ESPNOW_MODE;
} RTCState;

extern DebounceDigiOut digiOut;
extern RTCMemory<RTCState> rtcMemory;

void recoverReboot();
void reBoot(int wifiRetryTimeout);