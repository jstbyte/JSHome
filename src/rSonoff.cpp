#ifdef rSONOFF
#include "rSonoff/shared.h"
#include "rSonoff/espnow.cpp.h"
#include "rSonoff/infared.cpp.h"
#include "rSonoff/mqtt.cpp.h"

u8_t Global::bootCount = 0;
Sonoffe Global::digiOut;
u32_t Global::wifiRetryTimeout = WIFI_RETRY_TIMEOUT;

void reBoot(u32_t retryTimeout) /* Set States to RTCMemory & Restart */
{
    RTCMemory<RTCState> rtcMemory;
    rtcMemory.begin();

    auto *rtcData = rtcMemory.getData();
    rtcData->bootCount = ++Global::bootCount;
    rtcData->wifiRetryTimeout = retryTimeout;
    Global::digiOut.write_(rtcData->pinStates);
    DEBUG_LOG_LN("Rebooting...")
    rtcMemory.save();
    ESP.restart();
}

void recoverReboot() /* Retrive & Set States From RTC Memory */
{
    RTCMemory<RTCState> rtcMemory;
    if (rtcMemory.begin())
    {
        RTCState *rtcData = rtcMemory.getData();
        Global::wifiRetryTimeout = rtcData->wifiRetryTimeout;
        Global::bootCount = rtcData->bootCount;
        Global::digiOut.reads(rtcData->pinStates);
        DEBUG_LOG("\nRTC Memory Found : ")
        DEBUG_LOG(Global::digiOut.reads())
    }
    else
    {
        Global::digiOut.load("/config/digio_stat.json", true);
    }
}

void setup()
{
    LittleFS.begin();
    irrecv.enableIRIn();
    Serial.begin(115200);
    Global::digiOut.load("/config/digio_pins.json");
    recoverReboot();

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    DEBUG_LOG("\nWiFi Retry Timeout : ")
    DEBUG_LOG_LN(Global::wifiRetryTimeout)
    DEBUG_LOG("Device Mac Address : ");
    DEBUG_LOG_LN(WiFi.macAddress());

    if (Global::wifiRetryTimeout)
    {
        mqttClient = new PubSubWiFi;
        Global::digiOut.setCallback(emmittMqttEvent, 500);
        setupMqtt("/config/wlan_conf.json");
    }
    else
    {
        mqttClient = nullptr;
        setupEspNow("/config/espn_conf.json");
        Global::digiOut.setCallback(emmittEspNowEvent, 500);
    }
}

void loop()
{
    if (mqttClient)
    {
        mqttClient->eventLoop();
    }
    Global::digiOut.loop();
    MsgPacketizer::parse();
    handleInfared();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(100);
}
#endif
