#pragma once
#include <Arduino.h>
#include <lwip-err-t.h>
#ifdef SERIAL_DEBUG_LOG
#define DEBUG_LOG(arg) (Serial.print(arg));
#define DEBUG_LOG_LN(arg) (Serial.println(arg));
#else
#define DEBUG_LOG(arg)
#define DEBUG_LOG_LN(arg)
#endif

/* USB FM REMOTE CONTROL */
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

String uuid(String prefix);                        // Generate Random UUID With Prefix
void str2mac(const char *macStr, uint8_t *macArr); // "8d-75-92-6a-40-e6"->uint8_t mac[6]
void mac2str(const uint8_t *macArr, char *macStr); // uint8_t mac[6] -> "8d-75-92-6a-40-e6"

#include <TaskSchedulerDeclarations.h>
extern Task ledTask;
void ledBlink();