#include <BootMan.h>

RTCMemory<RTCData> BootMan::rtcData;

RTCData *BootMan::data()
{
    return BootMan::rtcData.getData();
}

bool BootMan::recover(void *data, uint8_t len)
{
    bool hasData = BootMan::rtcData.begin();
    if (hasData)
    {
        memcpy(data, BootMan::data()->data, len);
    }
    return !hasData;
}

void BootMan::reboot(uint32_t timeout, void *data, uint8_t len)
{
    auto _data = BootMan::data();
    memcpy(_data->data, data, len);
    _data->timeout = timeout;
    _data->bootCount++;
    BootMan::rtcData.save();
    ESP.restart();
}