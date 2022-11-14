#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#define SPIFFS LittleFS
#include <ESP8266WiFi.h>
#include <Debouncer.h>
#include <ConnMan.h>
#include "helper.h"
#include <Servo.h>
#include "spac.h"

#define WIFI_RETRY_TIMEOUT 1 /* WiFi Only: 1 */
#define DOOR_LOCK_SERVO_PIN 2
#define DOOR_STATE_NULL 255
#define SERVO_DOOR_LOCK 135
#define SERVO_DOOR_OPEN 0

class Door : public Servo
{
public:
    u8_t isLocked;
    Debouncer timer;
    void write(int value);
};

extern Door door;