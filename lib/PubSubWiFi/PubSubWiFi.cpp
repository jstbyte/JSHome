#include "PubSubWiFi.h"

RTCMemory<RTCData> ConnMan::rtcData;

RTCData *ConnMan::data()
{
    return ConnMan::rtcData.getData();
}

bool ConnMan::recover(void *data, uint8_t len)
{
    bool hasData = ConnMan::rtcData.begin();
    if (hasData)
    {
        memcpy(data, ConnMan::data()->data, len);
    }
    return !hasData;
}

void ConnMan::reboot(uint32_t timeout, void *data, uint8_t len)
{
    auto _data = ConnMan::data();
    memcpy(_data->data, data, len);
    _data->timeout = timeout;
    _data->bootCount++;
    ConnMan::rtcData.save();
    ESP.restart();
}

/***************************************************************/

void PubSubWiFi::eventLoop()
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
            DEBUG_LOG_LN("MQTT: connected.");
            if (_onConnection)
                _onConnection(this);
            _timestamp = 1;
            return;
        }
    }

    if (_timestamp < 2)
    {
        if (_timestamp == 1)
        {
            DEBUG_LOG_LN("MQTT: disonnected!");
            if (_onConnection)
                _onConnection(this);
        }
        _timestamp = millis();
        return;
    }

    if (_connTimeout)
    {
        if (((u32_t)(millis() - _timestamp) > _connTimeout))
        {
            DEBUG_LOG_LN("MQTT: conn. timeout!");
            _connTimeout = 0;
            if (_onTimeout)
                _onTimeout();
            return;
        }
    }
}

void PubSubWiFi::resetTimeout(u32_t time)
{
    if (_onConnection)
        _connTimeout = time;
}

wlan_config_t PubSubWiFi::init(String path)
{
    auto config = PubSubWiFi::loadWlanConfig(path);
    init(&config);
    return config;
}

void PubSubWiFi::init(wlan_config_t *config)
{
    WiFi.mode(WIFI_STA);
    WiFi.persistent(false);
    WiFi.begin(config->wlanSSID, config->wlanPASS);
    WiFi.setAutoReconnect(true);
    WiFi.setHostname(config->hostNAME.c_str());

    char *mqttHost = new char[config->mqttHOST.length() + 1];
    strncpy(mqttHost, config->mqttHOST.c_str(), config->mqttHOST.length());

    DEBUG_LOG("MQTT: & WiFi Initialized");
    if (config->mqttPORT == 8883)
    {
        DEBUG_LOG_LN("::Secure.");
        _wifiClient = new BearSSL::WiFiClientSecure();
        auto caCert = new BearSSL::X509List(mqtt_cert);
        ((WiFiClientSecure *)_wifiClient)->setTrustAnchors(caCert);
    }
    else
    {
        DEBUG_LOG_LN("::Insecure.");
        _wifiClient = new WiFiClient();
    }

    _timestamp = 0;
    _connTimeout = 0;
    setClient(*_wifiClient);
    setServer(mqttHost, config->mqttPORT);
}

wlan_config_t PubSubWiFi::loadWlanConfig(String path)
{
    wlan_config_t config;
    StaticJsonDocument<256> wConfigDoc;
    File configFile = LittleFS.open(path, "r");
    if (deserializeJson(wConfigDoc, configFile))
    {
        DEBUG_LOG_LN("MQTT: & WiFi config. error!");
        configFile.close();
        return config;
    }

    config.identity = wConfigDoc["identity"].as<String>();
    config.wlanSSID = wConfigDoc["wlanSSID"].as<String>();
    config.wlanPASS = wConfigDoc["wlanPASS"].as<String>();
    config.hostNAME = wConfigDoc["hostNAME"].as<String>();
    config.mqttHOST = wConfigDoc["mqttHOST"].as<String>();
    config.mqttPORT = wConfigDoc["mqttPORT"].as<u32_t>();
    DEBUG_LOG_LN("MQTT: & WiFi config loaded.");
    configFile.close();
    return config;
}

void PubSubWiFi::onConnection(std::function<void(PubSubWiFi *)> cb)
{
    _onConnection = cb;
}

void PubSubWiFi::onTimeout(std::function<void(void)> cb, u32_t time)
{
    _connTimeout = time;
    _onTimeout = cb;
}