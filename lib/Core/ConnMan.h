#pragma once
#include <Arduino.h>
#include <RTCMemory.h>
#ifndef RTC_DATA_SIZE
#define RTC_DATA_SIZE 8
#endif

typedef struct
{
    uint8_t bootCount;
    uint32_t timeout;
    uint8_t data[7];
} RTCData;

class ConnMan
{
protected:
    static RTCMemory<RTCData> rtcData;

public:
    static RTCData *data();
    static bool recover(void *data = nullptr, uint8_t len = 0);
    static void reboot(uint32_t timeout, void *data = nullptr, uint8_t len = 0);
};
