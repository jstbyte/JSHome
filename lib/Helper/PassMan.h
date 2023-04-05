#pragma once
#include <LittleFS.h>
#include <ezBuzzer.h>
#include <Arduino.h>
#include <IRutils.h>
#include "Helper.h"
#define PASSMAN_DELAY_MS 30000
#define PASSMAN_MAX_ATTEMPT 10

class PassMan
{
protected:
    String _password;
    String _passbuff;
    // SecureMode ON;
    uint8_t _attempt;
    ezBuzzer *_buzzer;
    unsigned long long _timestamp;

public:
    String buffer();
    PassMan(String password, ezBuzzer *buzzer);
    bool loads(String path, String password = "");
    bool press(decode_results key);
    bool press(char key);
    bool space();
    bool clear();
    bool reset();
    bool enter();
};
