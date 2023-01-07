#ifdef rSONOFF
#include <Arduino.h>
#include <LittleFS.h>
#define SPIFFS LittleFS
#include <CertStoreBearSSL.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <PubSubWiFi.h>
#include <IRutils.h>
#include <IRrecv.h>
#include "Helper.h"
#include "Sonoff.h"
#include <time.h>
#include "spac.h"
#include "Debouncer.h"

String topicSonoff;
String topicDevSync;
String topicDevInfo;
PubSubWiFi mqttClient;

decode_results ir_result;
IRrecv irrecv(13); // D7;
bool led = true;

void devSync(uint8_t index = 128)
{
    if (Sonoff::cmask() || index < 255)
    {
        String topic = String(topicSonoff.c_str());
        topic.replace("/req/", "/res/");
        mqttClient.publish(topic.c_str(), Sonoff::reads(index).c_str());
        DEBUG_LOG_LN("MQTT: Sonoff event emitted.");
    }

    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void onConnection(PubSubWiFi *client)
{
    if (client->connected())
    {
        client->subscribe(topicDevInfo.c_str());
        client->subscribe(topicDevSync.c_str());
        client->subscribe(topicSonoff.c_str());
        DEBUG_LOG_LN("MQTT: subscribed.");
        devSync();
        return;
    }

    digitalWrite(LED_BUILTIN, LOW);
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

    if (topicSonoff == topic)
        return (void)Sonoff::writes(data);

    if (topicDevSync == topic)
        return devSync();

    if (topicDevInfo == topic)
    {
        String devInfoJsonDoc;
        StaticJsonDocument<256> doc;
        doc["mac"] = WiFi.macAddress();
        doc["name"] = WiFi.getHostname();

        JsonObject services = doc["services"].createNestedObject();
        services["name"] = "sonoff";
        services["data"] = Sonoff::count();

        serializeJson(doc, devInfoJsonDoc);
        String topic = String(topicDevInfo.c_str()) + "/" + WiFi.getHostname();
        topic.replace("/req/", "/res/");
        mqttClient.publish(topic.c_str(), devInfoJsonDoc.c_str());
        return;
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

    auto config = mqttClient.init("/config.json");
    mqttClient.onConnection(onConnection);
    mqttClient.setCallback(mqttCallback);
    mqttClient.setC4C(devSync, 2000);

    /* Prepare Topics */
    topicDevInfo = config.identity + "/req/devinfo"; // Use to Pair Device;
    topicDevSync = config.identity + "/req/devsync/" + config.hostNAME;
    topicSonoff = config.identity + "/req/sonoff/" + config.hostNAME;
    DEBUG_LOG_LN("MQTT: Topics Ready.");
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
    delay(100); // Balance CPU Loads;
}
#endif
