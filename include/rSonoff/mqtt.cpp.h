#include "shared.h"
#include <ESP8266WiFi.h>
#include <CertStoreBearSSL.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <PubSubWiFi.h>
#include <time.h>

String topicSonoff;
String topicDevInfo;
PubSubWiFi *mqttClient;

void onMqttTimeout()
{
    reBoot(0);
}

void onMqttConnection(PubSubWiFi *client)
{
    if (client->connected())
    {
        client->subscribe(topicSonoff.c_str());
        client->subscribe(topicDevInfo.c_str());
        DEBUG_LOG_LN("MQTT: subscribed.");
        return;
    }
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
        unsigned short index = 255;
        unsigned short state = 255;
        sscanf(data, "%hu:%hu", &index, &state);
        Global::sonoff.writer(index, state);
        return;
    }

    if (topicDevInfo == topic)
    {
        String devInfoJsonDoc;
        StaticJsonDocument<128> doc;
        doc["dev"] = "sonoff";
        doc["uid"] = ESP.getChipId();
        doc["mac"] = WiFi.macAddress();
        doc["pin"] = Global::sonoff.count();
        serializeJson(doc, devInfoJsonDoc);
        String topic = String(topicDevInfo.c_str()) + "/" + ESP.getChipId();
        topic.replace("/req/", "/res/");
        mqttClient->publish(topic.c_str(), devInfoJsonDoc.c_str());
        return;
    }
}

void emmittMqttEvent()
{
    String topic = String(topicSonoff.c_str());
    topic.replace("/req/", "/res/");
    mqttClient->publish(topic.c_str(), Global::sonoff.reads().c_str());
    DEBUG_LOG_LN("MQTT: event emitted.");
}

void setupMqtt(String path)
{
    auto config = mqttClient->init(path);
    mqttClient->setCallback(mqttCallback);
    mqttClient->onConnection(onMqttConnection);
    mqttClient->onTimeout(onMqttTimeout, Global::wifiRetryTimeout);
    topicDevInfo = config.identity + "/req/devinfo";
    topicSonoff = config.identity + "/req/sonoff/" + String(ESP.getChipId());
    DEBUG_LOG_LN("MQTT: Topic Ready.");
}
