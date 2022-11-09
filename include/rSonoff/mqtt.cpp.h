#include "shared.h"
#include <ESP8266WiFi.h>
#include <CertStoreBearSSL.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <PubSubWiFi.h>
#include <time.h>

String topicDevInfo;
String topicDigiOut;
PubSubWiFi *mqttClient;

void onMqttTimeout()
{
    reBoot(0);
}

void onMqttConnection(PubSubWiFi *client)
{
    if (client->connected())
    {
        client->subscribe(topicDigiOut.c_str());
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

    if (topicDigiOut == topic)
    {
        unsigned short index = 255;
        unsigned short state = 255;
        sscanf(data, "%hu:%hu", &index, &state);
        Global::digiOut.writer(index, state);
        return;
    }

    if (topicDevInfo == topic)
    {
        String devInfoJsonDoc;
        StaticJsonDocument<128> doc;
        doc["id"] = ESP.getChipId();
        doc["mac"] = WiFi.macAddress();
        doc["digioutCount"] = Global::digiOut.count();
        serializeJson(doc, devInfoJsonDoc);
        String topic = String(topicDevInfo.c_str()) + "/" + ESP.getChipId();
        topic.replace("/req/", "/res/");
        mqttClient->publish(topic.c_str(), devInfoJsonDoc.c_str());
        return;
    }
}

void emmittMqttEvent()
{
    String topic = String(topicDigiOut.c_str());
    topic.replace("/req/", "/res/");
    mqttClient->publish(topic.c_str(), Global::digiOut.reads().c_str());
    DEBUG_LOG_LN("MQTT: event emitted.");
}

void setupMqtt(String path)
{
    auto config = mqttClient->init(path);
    mqttClient->setCallback(mqttCallback);
    mqttClient->onConnection(onMqttConnection);
    mqttClient->onTimeout(onMqttTimeout, Global::wifiRetryTimeout);
    topicDevInfo = config.identity + "/req/devinfo";
    topicDigiOut = config.identity + "/req/digiout/" + String(ESP.getChipId());
    DEBUG_LOG_LN("MQTT: Topic Ready.");
}
