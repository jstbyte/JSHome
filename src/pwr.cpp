#ifdef PWR
#include <Arduino.h>
#include <LittleFS.h>
#define SPIFFS LittleFS
#include <TaskScheduler.h>
#include <PubSub.h>
#include "Helper.h"
#include "certs.h"
#include <time.h>
#define DS 13

PubSubX &mqttClient = PubSubX::Get();
const char version[] = "v1.0.0";
volatile uint8_t state = 0;
uint64_t timeStamp = 0;
bool sent = false;

void fire()
{
    uint8_t _state = digitalRead(DS); // Current State;
    String payload = String(_state ? "" : "-");
    payload += timeStamp;

    String topic = mqttClient.topic("res/power", true);
    if (mqttClient.publish(topic.c_str(), payload.c_str()))
    {
        state = _state;
        sent = true;
    }
}

void mqttCallback(char *tpk, byte *dta, uint32_t length)
{
    auto topic = mqttClient.parse(tpk);
    auto data = mqttClient.parse(dta, length);

    if (topic.endsWith("req/power"))
        fire();

    if (topic == "req/info")
        fire();

    if (topic == "req/update")
        return (void)mqttClient.update(_firebaseRCA, data, version);
}

void onConnection(PubSubWiFi *)
{
    mqttClient.sub("req/power");
    mqttClient.sub("req/update");
    mqttClient.sub("req/info", true);
    mqttClient.sub("req/power", true);
    if (mqttClient.state() == MQTT_CONNECTED)
    {
        if (timeStamp == 0)
            timeStamp = time(nullptr);
        fire(); // Runs Every Time Connected;
    }
}

void setup()
{
    LittleFS.begin();
    // Serial.begin(115200);
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
    {
        timeStamp = sent ? time(nullptr) : timeStamp;
        if (mqttClient.state() == MQTT_CONNECTED)
            fire();
    }
    else
    {
        sent = true;
    }
}
#endif
