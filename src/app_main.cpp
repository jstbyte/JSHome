#ifdef ESP_RCTRL
#include "app_common.h"
#include "app_mqtt.h"
#include "app_espnow.h"
#include "app_infared.h"

RTCMemory<RTCState> rtcMemory;
DebounceDigiOut digiOut;

void reBoot(u32_t retryTimeout) /* Set States to RTCMemory & Restart */
{
    auto *rtcData = rtcMemory.getData();
    rtcData->bootCount++;
    rtcData->wifiRetryTimeout = retryTimeout;
    for (u8_t i = 0; i < digiOut.count(); i++)
    {
        rtcData->pinStates[i] = digiOut.read(i);
    }
    rtcMemory.save();
    DEBUG_LOG_LN("Rebooting...")
    ESP.restart();
}

void recoverReboot() /* Retrive & Set States From RTC Memory */
{
    if (rtcMemory.begin())
    {
        RTCState *rtcData = rtcMemory.getData();
        wifiRetryTimeout = rtcData->wifiRetryTimeout;
        for (u8 i = 0; i < digiOut.count(); i++)
        {
            digiOut.write(i, rtcData->pinStates[i]);
        }
        DEBUG_LOG("\nRTC Memory Found : ")
        DEBUG_LOG_LN(digiOut.reads())
    }
    else
    {
        RTCState *rtcData = rtcMemory.getData();
        rtcData->wifiRetryTimeout = wifiRetryTimeout;
        digiOut.load("/config/digio_stat.json", true);
        for (u8 i = 0; i < digiOut.count(); i++)
        {
            rtcData->pinStates[i] = digiOut.read(i);
        }
        rtcMemory.save();
    }
}

void setup()
{
    LittleFS.begin();
    Serial.begin(9600);
    irrecv.enableIRIn();
    digiOut.load("/config/digio_pins.json");
    recoverReboot();

    DEBUG_LOG("WiFi Retry Timeout : ")
    DEBUG_LOG_LN(wifiRetryTimeout)
    DEBUG_LOG("Device Mac Address : ");
    DEBUG_LOG_LN(WiFi.macAddress());

    if (wifiRetryTimeout)
    {
        auto wlanConf = loadWlanConfig("/config/wlan_conf.json");
        digiOut.setCallback(emmittMqttEvent, 500);
        setupMqtt(&wlanConf);
    }
    else
    {
        setupEspNow();
        digiOut.setCallback(emmittEspNowEvent, 500);
    }
}

void loop()
{
    handleMqtt();
    handleInfared();
    digiOut.loop();
    delay(100);
}
#endif
