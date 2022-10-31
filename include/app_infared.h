#pragma once
#include <IRutils.h>
#include <IRrecv.h>

#define IR_RECV_PIN 13 /* Can Be Change */

#define IR_POWER 0x1FE48B7
#define IR_MODE 0x1FE58A7
#define IR_MUTE 0x1FE7887
#define IR_EQ 0x1FE20DF
#define IR_1 0x1FE50AF
#define IR_2 0x1FED827
#define IR_3 0x1FEF807
#define IR_4 0x1FE30CF
#define IR_5 0x1FEB04F
#define IR_6 0x1FE708F
#define IR_7 0x1FE00FF
#define IR_8 0x1FEF00F
#define IR_9 0x1FE9867

extern decode_results ir_result;
extern IRrecv irrecv;
void handleInfared();