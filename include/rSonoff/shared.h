#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#define SPIFFS LittleFS
#include <ESP8266WiFi.h>
#include <ConnMan.h>
#include "helper.h"
#include "Sonoff.h"
#include "spac.h"

#define WIFI_RETRY_TIMEOUT 30000 /* WiFi Only: 1 */
