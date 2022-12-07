#pragma once
#include <Arduino.h>
#include <IRutils.h>
#include "Helper.h"

class PassMan
{
protected:
    String _password;
    String _passbuff;

public:
    PassMan(String password) : _password(password){};
    String buffer();
    bool press(decode_results key);
    bool press(char key);
    bool space();
    bool clear();
    bool reset();
    bool enter();
};
