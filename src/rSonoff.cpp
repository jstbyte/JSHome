#ifdef rSONOFF
#include "rSonoff/shared.h"
#include "rSonoff/espnow.cpp.h"
#include "rSonoff/infared.cpp.h"
#include "rSonoff/mqtt.cpp.h"

u8_t Global::bootCount = 0;
Sonoffe Global::sonoff;
u32_t Global::wifiRetryTimeout = WIFI_RETRY_TIMEOUT;

void reBoot(u32_t retryTimeout) /* Set States to RTCMemory & Restart */
{
    RTCMemory<RTCState> rtcMemory;
    rtcMemory.begin();

    auto *rtcData = rtcMemory.getData();
    rtcData->bootCount = ++Global::bootCount;
    rtcData->wifiRetryTimeout = retryTimeout;
    Global::sonoff.write_(rtcData->pinStates);
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
        Global::sonoff.reads(rtcData->pinStates);
        DEBUG_LOG("\nRTC Memory Found : ")
        DEBUG_LOG(Global::sonoff.reads())
    }
    else
    {
        Global::sonoff.load("/config/sonoff_stat.json", true);
    }
}

void setup()
{
    LittleFS.begin();
    irrecv.enableIRIn();
    Serial.begin(115200);
    Global::sonoff.load("/config/sonoff_pins.json");
    recoverReboot();

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    DEBUG_LOG("\nMQTT: & WiFi Timeout : ")
    DEBUG_LOG_LN(Global::wifiRetryTimeout)
    DEBUG_LOG("Device Mac Address : ");
    DEBUG_LOG_LN(WiFi.macAddress());

    if (Global::wifiRetryTimeout)
    {
        mqttClient = new PubSubWiFi;
        Global::sonoff.setCallback(emmittMqttEvent, 500);
        setupMqtt("/config/wlan_conf.json");
    }
    else
    {
        mqttClient = nullptr;
        setupEspNow("/config/espn_conf.json");
        Global::sonoff.setCallback(emmittEspNowEvent, 500);
    }
}

void loop()
{
    if (mqttClient)
    {
        mqttClient->eventLoop();
    }
    Global::sonoff.loop();
    MsgPacketizer::parse();
    handleInfared();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(100);
}
#endif
