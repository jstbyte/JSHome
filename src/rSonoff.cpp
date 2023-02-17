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

PubSubX &mqttClient = PubSubX::Get();
const char version[] = "v3.1.0";
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
}

void mqttCallback(char *tpk, byte *dta, uint32_t length)
{
    auto topic = mqttClient.parse(tpk);
    auto data = mqttClient.parse(dta, length);

    if (topic == "req/sonoff")
        return (void)(Snf::Get()).writes(data);

    if (topic == "req/update")
        return (void)mqttClient.update(_firebaseRCA, data, version);
}

void onConnection(PubSubWiFi *)
{
    if (!mqttClient.connected())
        return (void)ledTask.enable();

    if (Snf::enabled())
        mqttClient.sub("req/sonoff");
    mqttClient.sub("req/update");
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

    PubSubX &client = PubSubX::Get();
    client.setCallback(mqttCallback);
    client.onConnection(onConnection);
    client.init("/config.json", _emqxRCA);

    if (LittleFS.exists("/sonoff.txt"))
    {
        Snf &sonoff = Snf::Get();
        sonoff.begin("/sonoff.txt");
        DEBUG_LOG_LN("SONOFF: pins found.");
        sonoff.taskSetup(scheduler, &sonoffire, 1000, true);
    }
}

void loop()
{
    if (irrecv.decode(&ir_result))
    {
        if (Snf::enabled())
            (Snf::Get()).press(ir_result.value);
        irrecv.resume();
    }

    mqttClient.eventLoop();
    scheduler.execute();
}
#endif
