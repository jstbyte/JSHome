#pragma once
#include <LittleFS.h>
#include <Arduino.h>
#include <IRutils.h>
#include "Helper.h"

class PassMan
{
protected:
    uint8_t _beeppin;
    String _password;
    String _passbuff;

public:
    PassMan(String password, uint8_t pin) : _beeppin(pin), _password(password){};
    String buffer();
    bool loads(String path, String password = "");
    bool press(decode_results key);
    bool press(char key);
    bool space();
    bool clear();
    bool reset();
    bool enter();
};
