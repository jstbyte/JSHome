#include <app_common.h>
#include "app_mqtt.h"

unsigned long long wifiRetryTimeStamp = 0;

String topicDigiOut = "{SECRAT}/req/digiout/+";
String regexDigiOut = "^{SECRAT}\\/req\\/digiout\\/([1-8])$";

WiFiClient *wifiClient;
PubSubClient *mqttClient;

void handleMqtt()
{
    if (wifiRetryTimeout)
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
                DEBUG_LOG_LN("Subscribed!");
                wifiRetryTimeStamp = 0; // Give A Change To Retry When Disconnect;
                digiOut.start();
                return;
            }
        }

        if (wifiRetryTimeout != 1)
        {
            if (wifiRetryTimeStamp == 0) // If TimeStamp Not Availble Then -
            {
                wifiRetryTimeStamp = millis(); // Take A TimeStamp for Timeout;
            }
            else if ((u32_t)(millis() - wifiRetryTimeStamp) > wifiRetryTimeout)
            {
                reBoot(0);
            }
            DEBUG_LOG(wifiRetryTimeout - (u32_t)(millis() - wifiRetryTimeStamp));
            DEBUG_LOG_LN("ms Remaining, MQTT Connecting...");
        }
    }
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    char data[length + 1];
    data[length] = '\0';
    strncpy(data, (char *)payload, length);

    std::regex pattern(regexDigiOut.c_str());
    std::string searchable(topic);
    std::smatch matches;

    DEBUG_LOG("MQTT Topic Recived : ");
    DEBUG_LOG_LN(topic);

    if (std::regex_search(searchable, matches, pattern))
    {
        u8_t pinIndex = String(matches[1].str().c_str()).toInt() - 1;
        String action = String(data);
        if (action.isEmpty())
        {
            digiOut.start(true);
            return;
        }

        digiOut.write(pinIndex, action.toInt());
        digiOut.start();
    }
}

void emmittMqttEvent()
{
    String topic = String(topicDigiOut.c_str());
    topic.replace("/req/", "/res/");
    topic.replace("/+", "");
    mqttClient->publish(topic.c_str(), digiOut.reads().c_str());
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

    topicDigiOut.replace("{SECRAT}", config.identity);
    regexDigiOut.replace("{SECRAT}", config.identity);

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
        DEBUG_LOG_LN("Secure Mqtt Mode");
        wifiClient = new BearSSL::WiFiClientSecure;
        auto caCert = new BearSSL::X509List(mqtt_cert);
        ((WiFiClientSecure *)wifiClient)->setTrustAnchors(caCert);
    }
    else
    {
        wifiClient = new WiFiClient;
    }

    mqttClient->setClient(*wifiClient);
    mqttClient->setServer(mqttHost, config.mqttPORT);
    mqttClient->setCallback(mqttCallback);
    DEBUG_LOG_LN("WIFI & MQTT Setup Complate")
}