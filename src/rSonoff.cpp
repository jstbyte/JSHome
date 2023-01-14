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
#include <PubSubWiFi.h>
#include <IRutils.h>
#include <IRrecv.h>
#include "Helper.h"
#include "Sonoff.h"
#include "certs.h"
#include <time.h>
#include "spac.h"
#define D7 13

const char version[] = "3.0.1";
PubSubX mqttClient(_emqxRCA);
decode_results ir_result;
Scheduler scheduler;
IRrecv irrecv(D7);
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
    uint8_t sonoffi = Sonoff::cmask() ? 255 : 128;
    mqttClient.res("sonoff", Sonoff::reads(sonoffi).c_str());
    DEBUG_LOG_LN("MQTT: Invocked sonoffire.");
}

void mqttDevInfo(String topic, String data)
{
    if (topic.startsWith("devinfo"))
    {
        if (data == "sync")
            return sonoffire();

        String devInfoJsonDoc;
        StaticJsonDocument<256> doc;
        doc["mac"] = WiFi.macAddress();
        doc["name"] = WiFi.getHostname();

        JsonObject services = doc["services"].createNestedObject();
        services["name"] = "sonoff";
        services["data"] = Sonoff::count();

        serializeJson(doc, devInfoJsonDoc);
        mqttClient.res("devinfo", devInfoJsonDoc.c_str());
        return;
    }

    if (data && topic.startsWith("update"))
    {
        if (data.isEmpty())
            return (void)mqttClient.res("update", version);

        mqttClient.disconnect();
        BearSSL::WiFiClientSecure client;
        BearSSL::X509List x509(_firebaseRCA);
        client.setTrustAnchors(&x509);

        if (client.probeMaxFragmentLength("server", 443, 1024))
            client.setBufferSizes(1024, 1024);

        ESPhttpUpdate.setLedPin(LED_BUILTIN);
        ESPhttpUpdate.update(client, data);
        return;
    }
}

void mqttCallback(char *tpk, byte *dta, uint32_t length)
{
    auto topic = PubSubX::parse(tpk);
    auto data = PubSubX::parse(dta, length);

    DEBUG_LOG("MQTT: topic recived : `");
    DEBUG_LOG(topic);
    DEBUG_LOG("` payload:: ")
    DEBUG_LOG_LN(data);

    if (topic.startsWith("sonoff"))
        return (void)Sonoff::writes((char *)data.c_str());

    mqttDevInfo(topic, data); /* System Info & Update */
}

void onConnection(PubSubWiFi *client)
{
    if (client->connected())
    {
        ((PubSubX *)client)->sub("devinfo", true);
        ((PubSubX *)client)->sub("sonoff", true);
        ((PubSubX *)client)->sub("update", true);
        ((PubSubX *)client)->sub("devinfo");
        ((PubSubX *)client)->sub("update");
        DEBUG_LOG_LN("MQTT: Subscribed.");
        analogWrite(LED_BUILTIN, 254);
        ledTask.disable();
        sonoffire();
        return;
    }

    ledTask.enable();
}

void setup()
{
    LittleFS.begin();
    irrecv.enableIRIn();
    Serial.begin(115200);
    Sonoff::begin("/sonoff.txt");

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    ledTask.set(5, TASK_FOREVER, &blinkLed);
    scheduler.addTask(ledTask);
    ledTask.enable();

    Sonoff::taskSetup(scheduler, &sonoffire, 1000, true);
    auto config = mqttClient.init("/config.json");
    mqttClient.onConnection(onConnection);
    mqttClient.setCallback(mqttCallback);

    DEBUG_LOG_LN("(((Device Setup Completed)))");
}

void loop()
{
    if (irrecv.decode(&ir_result))
    {
        DEBUG_LOG("INFARED RECIVED : ");
        Sonoff::press(ir_result.value);
        DEBUG_LOG_LN(ir_result.value);
        irrecv.resume();
    }

    mqttClient.eventLoop();
    scheduler.execute();
}
#endif
