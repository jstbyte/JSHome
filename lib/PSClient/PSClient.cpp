#include "PSClient.h"

void PSClient::eventLoop()
{
    if (loop())
    {
        return;
    }

    if (WiFi.isConnected())
    {
        configTime(5 * 3600, 0, "pool.ntp.org", "time.nist.gov");
        if (connect(uuid("ESP8266JST-").c_str()))
        {
            _onConnection(this);
            timestamp = 1;
            return;
        }
    }

    if (timestamp < 2)
    {
        if (timestamp == 1)
            _onConnection(this);
        timestamp = millis();
        return;
    }

    if (wifiTimeout && ((u32_t)(millis() - timestamp) > wifiTimeout))
    {
        _onTimeout();
        DEBUG_LOG(wifiTimeout - (millis() - timestamp));
        DEBUG_LOG_LN("ms Remaining, MQTT Connecting...");
    }
}

wlan_config_t PSClient::begin(String path)
{
    auto config = loadWlanConfig(path);
    begin(&config);
    return config;
}

void PSClient::begin(wlan_config_t *config)
{
    WiFi.mode(WIFI_STA);
    WiFi.persistent(false);
    WiFi.setAutoReconnect(true);
    WiFi.setHostname(config->hostNAME.c_str());
    WiFi.begin(config->wlanSSID, config->wlanPASS);

    char *mqttHost = new char[config->mqttHOST.length() + 1];
    strncpy(mqttHost, config->mqttHOST.c_str(), config->mqttHOST.length());

    if (config->mqttPORT == 8883)
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

    timestamp = 0;
    setClient(*wifiClient);
    setServer(mqttHost, config->mqttPORT);
    DEBUG_LOG("WIFI & MQTT Setup Complate\n\n")
}

wlan_config_t PSClient::loadWlanConfig(String path)
{
    wlan_config_t config;
    StaticJsonDocument<256> wConfigDoc;
    File configFile = LittleFS.open(path, "r");
    if (deserializeJson(wConfigDoc, configFile))
    {
        DEBUG_LOG_LN("WLAN Config Error.");
        configFile.close();
        return config;
    }

    config.identity = wConfigDoc["identity"].as<String>();
    config.wlanSSID = wConfigDoc["wlanSSID"].as<String>();
    config.wlanPASS = wConfigDoc["wlanPASS"].as<String>();
    config.hostNAME = wConfigDoc["hostNAME"].as<String>();
    config.mqttHOST = wConfigDoc["mqttHOST"].as<String>();
    config.mqttPORT = wConfigDoc["mqttPORT"].as<u32_t>();
    configFile.close();
    return config;
}

void PSClient::onTimeout(std::function<void(void)> cb)
{
    _onTimeout = cb;
}

void PSClient::onConnection(std::function<void(PSClient *)> cb)
{
    _onConnection = cb;
}