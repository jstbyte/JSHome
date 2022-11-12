#include "Debouncer.h"

void Debouncer::setCallback(void (*_callback)(void), u32 _timeout)
{
    callback = _callback;
    timeout = _timeout;
    enabled = false;
    asap = false;
}

void Debouncer::setTimeout(u32 time)
{
    timeout = time;
}

void Debouncer::loop()
{
    if (enabled && (asap || (millis() - timestamp) > timeout))
    {
        callback();
        enabled = false;
        asap = false;
    }
}

void Debouncer::start(bool _asap)
{
    asap = _asap;
    enabled = true;
    timestamp = millis();
}

void Debouncer::stop()
{
    asap = false;
    enabled = false;
}