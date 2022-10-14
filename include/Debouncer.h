#pragma once
#include <Arduino.h>

class Debouncer
{
private:
    u32 timeout;
    bool enabled;
    u64 timestamp;
    void (*callback)(void);

public:
    void setCallback(void (*_callback)(void), u32 _timeout)
    {
        callback = _callback;
        timeout = _timeout;
        enabled = false;
    }

    void loop()
    {
        if (enabled && (millis() - timestamp) > timeout)
        {
            callback();
            enabled = false;
        }
    }

    void start()
    {
        enabled = true;
        timestamp = millis();
    }

    void stop()
    {
        enabled = false;
    }
};