#ifdef rPOWER
#include <Arduino.h>
#include <LittleFS.h>
#define SPIFFS LittleFS
#include <TaskScheduler.h>
#include <PubSub.h>
#include "Helper.h"
#include "certs.h"
#include <time.h>
#include "spac.h"
#define DS 13

PubSubX &mqttClient = PubSubX::Get();
const char version[] = "v0.1.0";
volatile uint8_t state = 0;
bool synced = false;

void fire()
{
    if (mqttClient.state() != MQTT_CONNECTED)
        return;

    uint8_t _state = digitalRead(DS);
    String payload = (_state ? "1:" : "0:");
    payload += String(time(nullptr));

    String topic = mqttClient.topic("power", true);
    if (mqttClient.publish(topic.c_str(), payload.c_str(), true))
        state = _state;
    synced = true;
}

void mqttCallback(char *tpk, byte *dta, uint32_t length)
{
    auto topic = mqttClient.parse(tpk);
    auto data = mqttClient.parse(dta, length);

    if (topic == "req/update")
        return (void)mqttClient.update(_firebaseRCA, data, version);
}

void onConnection(PubSubWiFi *)
{
    mqttClient.sub("req/update");
    if (!synced)
        fire();
}

void setup()
{
    LittleFS.begin();
    Serial.begin(115200);
    pinMode(DS, INPUT_PULLUP);

    PubSubX &client = PubSubX::Get();
    client.setCallback(mqttCallback);
    client.onConnection(onConnection);
    client.init("/config.json", _emqxRCA);
}

void loop()
{
    mqttClient.eventLoop(); // Keep Alive;
    if (digitalRead(DS) != state)
        fire();
}
#endif
