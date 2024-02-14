#pragma once
#include <LittleFS.h>
#include <ezBuzzer.h>
#include <Arduino.h>
#include <IRutils.h>
#include "Helper.h"

class PassMan
{
protected:
    String _password;
    String _passbuff;
    // SecureMode ON;
    uint8_t _attempts;
    ezBuzzer *_buzzer;

public:
    bool isEmpty();
    String buffer();
    uint8_t attempts();
    PassMan(String password, ezBuzzer *buzzer);
    bool load(String path, String password);
    void dump(String path, String password);
    bool press(decode_results key);
    bool press(char key);
    bool hasSpace();
    bool clear();
    bool reset();
    bool enter();
};
