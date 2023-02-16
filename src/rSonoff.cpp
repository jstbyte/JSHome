#ifdef rSONOFF
#include <Arduino.h>
#include <LittleFS.h>
#define SPIFFS LittleFS
#include <CertStoreBearSSL.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClientSecure.h>
#include <TaskScheduler.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <IRutils.h>
#include <PubSub.h>
#include <IRrecv.h>
#include "Helper.h"
#include "Sonoff.h"
#include "certs.h"
#include <time.h>
#include "spac.h"
#define IR 13

const char version[] = "v3.1.0";
PubSubX mqttClient(_emqxRCA);
decode_results ir_result;
Scheduler scheduler;
IRrecv irrecv(IR);
Task ledTask;

void blinkLed()
{
    uint8_t state = !digitalRead(LED_BUILTIN);
    digitalWrite(LED_BUILTIN, state);
    if (state)
        ledTask.delay(1995);
}

void sonoffire()
{
    Snf &sonoff = Snf::Get();
    uint8_t sonoffi = sonoff.cmask() ? 255 : 128;
    mqttClient.pub("res/sonoff", sonoff.reads(sonoffi));
    DEBUG_LOG_LN("MQTT: Invocked sonoffire.");
}

void mqttCallback(char *tpk, byte *dta, uint32_t length)
{
    auto topic = PubSubX::parse(tpk);
    auto data = PubSubX::parse(dta, length);

    DEBUG_LOG("MQTT: topic recived : `");
    DEBUG_LOG(topic);
    DEBUG_LOG("` payload:: ")
    DEBUG_LOG_LN(data);

    if (topic == "req/sonoff")
        return (void)(Snf::Get()).writes(data);

    if (topic == "req/update")
        return (void)mqttClient.otaUpdate(_firebaseRCA, data, version);
}

void onConnection(PubSubWiFi *client)
{
    if (!client->connected())
        return (void)ledTask.enable();

    if (Snf::enabled())
        ((PubSubX *)client)->sub("req/sonoff");
    ((PubSubX *)client)->sub("req/update");
    DEBUG_LOG_LN("MQTT: Subscribed.");
    analogWrite(LED_BUILTIN, 254);
    ledTask.disable();
    sonoffire();
    return;
}

void setup()
{
    LittleFS.begin();
    irrecv.enableIRIn();
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    ledTask.set(5, TASK_FOREVER, &blinkLed);
    scheduler.addTask(ledTask);
    ledTask.enable();

    auto config = mqttClient.init("/config.json");
    mqttClient.onConnection(onConnection);
    mqttClient.setCallback(mqttCallback);

    if (LittleFS.exists("/sonoff.txt"))
    {
        Snf &sonoff = Snf::Get();
        sonoff.begin("/sonoff.txt");
        DEBUG_LOG_LN("SONOFF: pins found.");
        sonoff.taskSetup(scheduler, &sonoffire, 1000, true);
    }

    DEBUG_LOG_LN("(((Device Setup Completed)))");
}

void loop()
{
    if (irrecv.decode(&ir_result))
    {
        if (Snf::enabled())
            (Snf::Get()).press(ir_result.value);
        DEBUG_LOG("INFARED RECIVED : ");
        DEBUG_LOG_LN(ir_result.value);
        irrecv.resume();
    }

    mqttClient.eventLoop();
    scheduler.execute();
}
#endif
