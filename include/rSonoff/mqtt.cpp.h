#include "_defined.h"
#include <CertStoreBearSSL.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <time.h>

String topicSonoff;
String topicDevInfo;
PubSubWiFi *mqttClient;

void emmittMqttEvent()
{
    String topic = String(topicSonoff.c_str());
    topic.replace("/req/", "/res/");
    mqttClient->publish(topic.c_str(), Sonoffe::reads().c_str());
    DEBUG_LOG_LN("MQTT: event emitted.");
}

void onMqttTimeout()
{
    BootMan::reboot(0, Sonoffe::pins(), Sonoffe::count());
}

void onMqttConnection(PubSubWiFi *client)
{
    if (client->connected())
    {
        client->subscribe(topicDevInfo.c_str());
        client->subscribe(topicSonoff.c_str());
        DEBUG_LOG_LN("MQTT: subscribed.");
        emmittMqttEvent();
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
        Sonoffe::writer(index, state);
        return;
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
        mqttClient->publish(topic.c_str(), devInfoJsonDoc.c_str());
        return;
    }
}

void setupMqtt(String path)
{
    auto config = mqttClient->init(path);
    mqttClient->setCallback(mqttCallback);
    mqttClient->onConnection(onMqttConnection);
    mqttClient->onTimeout(onMqttTimeout, BootMan::data()->timeout);
    topicDevInfo = config.identity + "/req/devinfo";
    topicSonoff = config.identity + "/req/sonoff/" + String(ESP.getChipId());
    DEBUG_LOG_LN("MQTT: Topic Ready.");
}
