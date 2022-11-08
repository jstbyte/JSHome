#include <ESP8266WiFi.h>
#include <CertStoreBearSSL.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "shared.h"
#include <time.h>

/* Mqtt SSL/TLS CA Root Certificate */
const char mqtt_cert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD
QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB
CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97
nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt
43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P
T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4
gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO
BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR
TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw
DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr
hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg
06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF
PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls
YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk
CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=
-----END CERTIFICATE-----
)EOF";

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
