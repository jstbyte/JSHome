#pragma once
#include <Arduino.h>
#include "IRutils.h"

#define IR_POWER 0x1FE48B7
#define IR_MODE 0x1FE58A7
#define IR_MUTE 0x1FE7887
#define IR_USD 0x1FE906F
#define IR_RPT 0x1FE10EF
#define IR_EQ 0x1FE20DF
#define IR_0 0x1FEE01F
#define IR_1 0x1FE50AF
#define IR_2 0x1FED827
#define IR_3 0x1FEF807
#define IR_4 0x1FE30CF
#define IR_5 0x1FEB04F
#define IR_6 0x1FE708F
#define IR_7 0x1FE00FF
#define IR_8 0x1FEF00F
#define IR_9 0x1FE9867

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
