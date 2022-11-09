#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#define SPIFFS LittleFS
#include <RTCMemory.h>
#include "Debouncer.h"
#include "Helper.h"
#include "Sonoff.h"
#include "spac.h"

#define WIFI_RETRY_TIMEOUT 30000 /* WiFi Only: 1 */

typedef struct
{
    u8_t channel;
    u32_t timeout;
    String gateway;
} espnow_config_t;

typedef struct
{
    u8_t bootCount;
    u8_t pinStates[7];
    u32_t wifiRetryTimeout; // 0 = ESPNOW_MODE;
} RTCState;

namespace Global
{
    extern Sonoffe digiOut;
    extern u32_t wifiRetryTimeout;
    extern u8_t bootCount;
}

void reBoot(u32_t wifiRetryTimeout);