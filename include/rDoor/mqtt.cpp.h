#include "shared.h"
#include <CertStoreBearSSL.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <time.h>

String topicDevInfo;
String topicDoorLock;
PubSubWiFi *mqttClient;

void mqttDoorChanged()
{
    auto _topic = String(topicDoorLock.c_str());
    _topic.replace("/req/", "/res/");
    door.write(SERVO_DOOR_LOCK);
    mqttClient->publish(_topic.c_str(), "1");
}

void onMqttTimeout()
{
    ConnMan::reboot(0);
}

void onMqttConnection(PubSubWiFi *client)
{
    if (client->connected())
    {
        client->subscribe(topicDoorLock.c_str());
        client->subscribe(topicDevInfo.c_str());
        DEBUG_LOG_LN("MQTT: subscribed.");

        auto _topic = String(topicDoorLock.c_str());
        _topic.replace("/req/", "/res/");
        mqttClient->publish(_topic.c_str(), "255");
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

    if (topicDoorLock == topic)
    {
        unsigned int _time;
        sscanf(data, "%u", &_time);
        auto _topic = String(topicDoorLock.c_str());
        _topic.replace("/req/", "/res/");

        if (length == 0)
        {
            mqttClient->publish(_topic.c_str(), String(door.isLocked).c_str());
            return;
        }

        if (_time == 0)
        {
            door.timer.stop();
            door.write(SERVO_DOOR_OPEN);
            mqttClient->publish(_topic.c_str(), "0");
            return;
        }
        if (_time == 1)
        {
            door.timer.stop();
            door.write(SERVO_DOOR_LOCK);
            mqttClient->publish(_topic.c_str(), "1");
            return;
        }
        if (_time == DOOR_STATE_NULL)
        {
            door.timer.stop();
            if (door.isLocked)
                door.write(SERVO_DOOR_OPEN);
            else
                door.write(SERVO_DOOR_LOCK);
            mqttClient->publish(_topic.c_str(), String(door.isLocked).c_str());
            return;
        }

        door.write(SERVO_DOOR_OPEN);
        mqttClient->publish(_topic.c_str(), data);
        door.timer.setTimeout(_time * 1000);
        door.timer.start();
        return;
    }

    if (topicDevInfo == topic)
    {
        String devInfoJsonDoc;
        StaticJsonDocument<256> doc;
        doc["uid"] = ESP.getChipId();
        doc["mac"] = WiFi.macAddress();

        JsonObject services = doc["services"].createNestedObject();
        services["name"] = "door";
        services["data"] = 1;

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
    mqttClient->onTimeout(onMqttTimeout, ConnMan::data()->timeout);
    topicDevInfo = config.identity + "/req/devinfo";
    topicDoorLock = config.identity + "/req/door/" + String(ESP.getChipId());
    DEBUG_LOG_LN("MQTT: Topic Ready.");
}
