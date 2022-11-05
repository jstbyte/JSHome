#include <app_common.h>
#include "app_mqtt.h"

String topicDevInfo;
String topicDigiOut;
WiFiClient *wifiClient;
PubSubClient *mqttClient;
unsigned long long wifiRetryTimeStamp = 0;

void handleMqtt()
{
    if (Global::wifiRetryTimeout)
    {

        if (mqttClient->loop())
        {
            return;
        }

        if (WiFi.isConnected())
        {
            configTime(5 * 3600, 0, "pool.ntp.org", "time.nist.gov");
            if (mqttClient->connect(uuid("ESP8266JST-").c_str()))
            {
                mqttClient->subscribe(topicDigiOut.c_str());
                mqttClient->subscribe(topicDevInfo.c_str());
                DEBUG_LOG_LN("Subscribed!");
                wifiRetryTimeStamp = 0; // Give A Change To Retry When Disconnect;
                Global::digiOut.start();
                return;
            }
        }

        if (Global::wifiRetryTimeout != 1)
        {
            if (wifiRetryTimeStamp == 0) // If TimeStamp Not Availble Then -
            {
                wifiRetryTimeStamp = millis(); // Take A TimeStamp for Timeout;
            }
            else if ((u32_t)(millis() - wifiRetryTimeStamp) > Global::wifiRetryTimeout)
            {
                reBoot(0);
            }
            DEBUG_LOG(Global::wifiRetryTimeout - (u32_t)(millis() - wifiRetryTimeStamp));
            DEBUG_LOG_LN("ms Remaining, MQTT Connecting...");
        }
    }
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    char data[length + 1];
    data[length] = '\0';
    strncpy(data, (char *)payload, length);

    DEBUG_LOG("MQTT Topic Recived : ");
    DEBUG_LOG_LN(topic);
    DEBUG_LOG("Payload:: ")
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
    DEBUG_LOG_LN("MQTT Event Emitted");
}

wlan_config_t loadWlanConfig(String path)
{
    wlan_config_t config;
    StaticJsonDocument<256> wConfigDoc;
    File configFile = LittleFS.open(path, "r");
    if (deserializeJson(wConfigDoc, configFile) == DeserializationError::Ok)
    {
        config.identity = wConfigDoc["identity"].as<String>();
        config.wlanSSID = wConfigDoc["wlanSSID"].as<String>();
        config.wlanPASS = wConfigDoc["wlanPASS"].as<String>();
        config.hostNAME = wConfigDoc["hostNAME"].as<String>();
        config.mqttHOST = wConfigDoc["mqttHOST"].as<String>();
        config.mqttPORT = wConfigDoc["mqttPORT"].as<u32_t>();
        configFile.close();
        return config;
    }
    DEBUG_LOG_LN("WLAN Config DeserializationError");

    reBoot(0);
    return config;
}

void setupMqtt(String path)
{
    auto config = loadWlanConfig(path);

    WiFi.mode(WIFI_STA);
    WiFi.persistent(false);
    WiFi.begin(config.wlanSSID, config.wlanPASS);
    WiFi.setAutoReconnect(true);
    WiFi.setHostname(config.hostNAME.c_str());

    char *mqttHost = new char[config.mqttHOST.length() + 1];
    strncpy(mqttHost, config.mqttHOST.c_str(), config.mqttHOST.length());

    mqttClient = new PubSubClient;
    if (config.mqttPORT == 8883)
    {
        wifiClient = new BearSSL::WiFiClientSecure;
        auto caCert = new BearSSL::X509List(mqtt_cert);
        ((WiFiClientSecure *)wifiClient)->setTrustAnchors(caCert);
        DEBUG_LOG("Secure:: ");
    }
    else
    {
        wifiClient = new WiFiClient;
        DEBUG_LOG("Insecure:: ");
    }

    mqttClient->setClient(*wifiClient);
    mqttClient->setCallback(mqttCallback);
    mqttClient->setServer(mqttHost, config.mqttPORT);
    topicDevInfo = config.identity + "/req/devinfo";
    topicDigiOut = config.identity + "/req/digiout/" + String(ESP.getChipId());
    DEBUG_LOG("WIFI & MQTT Setup Complate\n\n")
}