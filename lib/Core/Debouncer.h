#pragma once
#include <Arduino.h>

class Debouncer
{
private:
    bool asap;
    u32 timeout;
    bool enabled;
    u64 timestamp;
    void (*callback)(void);

public:
    void loop();
    void stop();
    void start(bool _asap = false);
    void setTimeout(u32 time);
    void setCallback(void (*_callback)(void), u32 _timeout);
};