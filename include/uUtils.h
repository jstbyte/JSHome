#pragma once
#include <Arduino.h>

#pragma once
#define IR_POWER 0x1FE48B7
#define IR_MODE 0x1FE58A7
#define IR_MUTE 0x1FE7887
#define IR_EQ 0x1FE20DF
#define IR_1 0x1FE50AF
#define IR_2 0x1FED827
#define IR_3 0x1FEF807
#define IR_4 0x1FE30CF

// uint8_t mac[6] -> "8d-75-92-6a-40-e6";
void mac2str(const uint8_t *macArr, char *macStr)
{
    snprintf(macStr, sizeof(char) * 20, "%02x-%02x-%02x-%02x-%02x-%02x",
             macArr[0], macArr[1], macArr[2], macArr[3], macArr[4], macArr[5]);
}

// "8d-75-92-6a-40-e6" -> uint8_t mac[6];
void str2mac(const char *macStr, uint8_t *macArr)
{
    sscanf(macStr, "%2hhx-%2hhx-%2hhx-%2hhx-%2hhx-%2hhx", &macArr[0], &macArr[1],
           &macArr[2], &macArr[3], &macArr[4], &macArr[5]);
}

String uuid(String prefix)
{
    return (prefix + String(random(0xffff), HEX));
}