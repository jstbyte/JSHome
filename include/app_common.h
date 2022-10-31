#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#define SPIFFS LittleFS
#include <RTCMemory.h>
#include "Debouncer.h"
#include "DigiOut.h"
#include "Helper.h"

#define WIFI_RETRY_TIMEOUT 30000 /* WiFi Only: 1 */

typedef struct
{
    u8_t bootCount;
    bool pinStates[7];
    u32_t wifiRetryTimeout; // 0 = ESPNOW_MODE;
} RTCState;

extern DebounceDigiOut digiOut;
extern RTCMemory<RTCState> rtcMemory;

void reBoot(u32_t wifiRetryTimeout);