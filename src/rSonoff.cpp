#ifdef rSONOFF
#include <Arduino.h>
#include <LittleFS.h>
#define SPIFFS LittleFS
#include <CertStoreBearSSL.h>
#include <WiFiClientSecure.h>
#include <TaskScheduler.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <PubSubWiFi.h>
#include <IRutils.h>
#include <IRrecv.h>
#include "Helper.h"
#include "Sonoff.h"
#include <time.h>
#include "spac.h"

String tSonoff;
String tDevSync;
String tDevInfo;
PubSubWiFi mqttClient;

decode_results ir_result;
IRrecv irrecv(13); // D7;
Scheduler scheduler;
Task ledTask;

void sonoffire()
{
    auto tpk = req2res(tSonoff);
    uint8_t sonoffi = Sonoff::cmask() ? 255 : 128;
    mqttClient.publish(tpk.c_str(), Sonoff::reads(sonoffi).c_str());
    DEBUG_LOG_LN("MQTT: Invocked sonoffire.");
}

void onConnection(PubSubWiFi *client)
{
    if (client->connected())
    {
        client->subscribe(tDevInfo.c_str());
        client->subscribe(tDevSync.c_str());
        client->subscribe(tSonoff.c_str());
        DEBUG_LOG_LN("MQTT: Subscribed.");
        analogWrite(LED_BUILTIN, 254);
        ledTask.disable();
        sonoffire();
        return;
    }

    ledTask.enable();
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    char data[length + 1];
    data[length] = '\0';
    strncpy(data, (char *)payload, length);

    DEBUG_LOG("MQTT: topic recived : ");
    DEBUG_LOG(topic);
    DEBUG_LOG(" payload:: ")
    DEBUG_LOG_LN(data);

    if (tSonoff == topic)
        return (void)Sonoff::writes(data);

    if (tDevSync == topic)
    {
        sonoffire();
    }

    if (tDevInfo == topic)
    {
        String devInfoJsonDoc;
        StaticJsonDocument<256> doc;
        doc["mac"] = WiFi.macAddress();
        doc["name"] = WiFi.getHostname();

        JsonObject services = doc["services"].createNestedObject();
        services["name"] = "sonoff";
        services["data"] = Sonoff::count();

        serializeJson(doc, devInfoJsonDoc);
        String topic = String(tDevInfo.c_str()) + "/" + WiFi.getHostname();
        topic.replace("/req/", "/res/");
        mqttClient.publish(topic.c_str(), devInfoJsonDoc.c_str());
        return;
    }
}

void ledTaskRunner()
{
    uint8_t state = !digitalRead(LED_BUILTIN);
    digitalWrite(LED_BUILTIN, state);
    if (state)
    {
        ledTask.delay(1995);
    }
}

void setup()
{
    LittleFS.begin();
    irrecv.enableIRIn();
    Serial.begin(115200);
    Sonoff::begin("/sonoff.txt");

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    ledTask.set(5, TASK_FOREVER, &ledTaskRunner);
    scheduler.addTask(ledTask);
    ledTask.enable();

    Sonoff::taskSetup(scheduler, &sonoffire, 1000, true);
    auto config = mqttClient.init("/config.json");
    mqttClient.onConnection(onConnection);
    mqttClient.setCallback(mqttCallback);

    /* Prepare Topics */
    tDevInfo = config.identity + "/req/devinfo"; // Use to Pair Device;
    tDevSync = config.identity + "/req/devsync/" + config.hostNAME;
    tSonoff = config.identity + "/req/sonoff/" + config.hostNAME;
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
