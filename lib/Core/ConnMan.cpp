#include "ConnMan.h"

RTCMemory<RTCData> ConnMan::rtcData;

RTCData *ConnMan::data()
{
    return ConnMan::rtcData.getData();
}

bool ConnMan::recover(void *data, uint8_t len)
{
    bool hasData = ConnMan::rtcData.begin();
    if (hasData)
    {
        memcpy(data, ConnMan::data()->data, len);
    }
    return !hasData;
}

void ConnMan::reboot(uint32_t timeout, void *data, uint8_t len)
{
    auto _data = ConnMan::data();
    memcpy(_data->data, data, len);
    _data->timeout = timeout;
    _data->bootCount++;
    ConnMan::rtcData.save();
    ESP.restart();
}