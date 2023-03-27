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

void mqttCallback(char *tpk, byte *dta, uint32_t length)
{
    auto topic = mqttClient.parse(tpk);
    auto data = mqttClient.parse(dta, length);

    if (topic.startsWith("req/sonoff"))
        return (void)(Snf::Get()).writes(data);

    if (topic == "req/info")
        return (void)(Snf::Get()).writes("");

    if (topic == "req/update")
        return (void)mqttClient.update(_firebaseRCA, data, version);
}

void onConnection(PubSubWiFi *)
{
    if (!mqttClient.connected())
        return (void)ledTask.enable();

    mqttClient.sub("req/info", true);
    analogWrite(LED_BUILTIN, 254);
    mqttClient.sub("req/sonoff?");
    mqttClient.sub("req/sonoff");
    mqttClient.sub("req/update");
    Snf::Get().fire();
    ledTask.disable();
    return;
}

void setup()
{
    LittleFS.begin();
    irrecv.enableIRIn();
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    ledTask.set(5, TASK_FOREVER, &ledBlink);
    scheduler.addTask(ledTask);
    ledTask.enableDelayed();

    PubSubX &client = PubSubX::Get();
    client.setCallback(mqttCallback);
    client.onConnection(onConnection);
    client.init("/config.json", _emqxRCA);

    (Snf::Get()).begin("/sonoff.txt");
    (Snf::Get()).taskSetup(scheduler, &Snf::fire, 1000, true);
}

void loop()
{
    if (irrecv.decode(&ir_result))
    {
        (Snf::Get()).press(ir_result.value);
        irrecv.resume();
    }

    mqttClient.eventLoop();
    scheduler.execute();
}
#endif
