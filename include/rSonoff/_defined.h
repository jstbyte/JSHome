#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#define SPIFFS LittleFS
#include <ESP8266WiFi.h>
#include <PubSubWiFi.h>
#include <IRutils.h>
#include <IRrecv.h>
#include "Helper.h"
#include "Sonoff.h"
#include "spac.h"

#define WIFI_RETRY_TIMEOUT 30000 /* WiFi Only: 1 */
#define IR_RECV_PIN 13
