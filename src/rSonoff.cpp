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

String topicSonoff;
String topicDevInfo;
PubSubWiFi mqttClient;

decode_results ir_result;
IRrecv irrecv(13); // D7;

void sonoffire()
{
    String topic = String(topicSonoff.c_str());
    topic.replace("/req/", "/res/");
    mqttClient.publish(topic.c_str(), Sonoffe::reads().c_str());
    DEBUG_LOG_LN("MQTT: event emitted.");
}

void onConnection(PubSubWiFi *client)
{
    if (client->connected())
    {
        client->subscribe(topicDevInfo.c_str());
        client->subscribe(topicSonoff.c_str());
        DEBUG_LOG_LN("MQTT: subscribed.");
        analogWrite(LED_BUILTIN, 254);
        sonoffire();
        return;
    }

    digitalWrite(LED_BUILTIN, HIGH);
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
    {
        return Sonoffe::writer(data);
    }

    if (topicDevInfo == topic)
    {
        String devInfoJsonDoc;
        StaticJsonDocument<256> doc;
        doc["uid"] = ESP.getChipId();
        doc["mac"] = WiFi.macAddress();

        JsonObject services = doc["services"].createNestedObject();
        services["name"] = "sonoff";
        services["data"] = Sonoffe::count();

        serializeJson(doc, devInfoJsonDoc);
        String topic = String(topicDevInfo.c_str()) + "/" + ESP.getChipId();
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
    Sonoffe::load("/rsonoff.json");
    Sonoffe::writes(HIGH);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    auto config = mqttClient.init("/wconfig.json");
    Sonoffe::event.setCallback(sonoffire, 500);
    mqttClient.onConnection(onConnection);
    mqttClient.setCallback(mqttCallback);

    /* Prepare Topics */
    topicDevInfo = config.identity + "/req/devinfo";
    topicSonoff = config.identity + "/req/sonoff/" + String(ESP.getChipId());
    DEBUG_LOG_LN("MQTT: Topics Ready.");
}

bool ledState = true;
void loop()
{
    if (irrecv.decode(&ir_result))
    {
        if (!mqttClient.connected())
            digitalWrite(LED_BUILTIN, LOW);
        DEBUG_LOG("INFARED RECIVED : ");
        Sonoffe::press(ir_result.value);
        DEBUG_LOG_LN(ir_result.value);
        irrecv.resume();
        if (!mqttClient.connected())
            digitalWrite(LED_BUILTIN, HIGH);
    }

    mqttClient.eventLoop();
    MsgPacketizer::parse();
    Sonoffe::event.loop();
    delay(100); // Balance CPU Loads;
}
#endif
