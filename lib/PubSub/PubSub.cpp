#include "PubSub.h"

void PubSubWiFi::eventLoop()
{
    if (!loop())
    {
        if (_timestamp < 2)
        {
            DEBUG_LOG_LN("MQTT: Disconnected.");
            if (_timestamp == 1 && _onConnection)
                _onConnection(this);
            _timestamp = millis();
            return;
        }

        if ((u32_t)(millis() - _timestamp) > MQTT_RETRY_MS)
        {
            if (WiFi.isConnected())
            {
                DEBUG_LOG("MQTT: {");
                DEBUG_LOG(_retryCount);
                DEBUG_LOG_LN("} Connecting...");
                configTime(5 * 3600, 0, "pool.ntp.org", "time.nist.gov");
                if (connect(uuid("ESP8266JST-").c_str()))
                {
                    DEBUG_LOG_LN("MQTT: Connected.");
                    if (_onConnection)
                        _onConnection(this);
                    _retryCount = 0;
                    _timestamp = 1;
                    return;
                }
            }

            _retryCount++;
            _timestamp = millis();
            if (_retryCount == _maxRetry && _retryCount > 0)
            {
                DEBUG_LOG_LN("MQTT: Maximum Retry Exceeds.");
                _onRertyExceeds();
            }
        }
    }
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
        auto caCert = new BearSSL::X509List(_pemCert);
        ((WiFiClientSecure *)_wifiClient)->setTrustAnchors(caCert);
    }
    else
    {
        DEBUG_LOG_LN("::Insecure.");
        _wifiClient = new WiFiClient();
    }

    _maxRetry = 0;
    _timestamp = 0;
    _retryCount = 0;
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

void PubSubWiFi::onRertyExceeds(std::function<void(void)> cb, u8_t maxRetry)
{
    _maxRetry = maxRetry;
    _onRertyExceeds = cb;
}

/*******************************************************************/

String PubSubX::_pkey;
String PubSubX::_host;

wlan_config_t PubSubX::init(String path)
{
    auto config = PubSubWiFi::init(path);
    PubSubX::_pkey = config.identity;
    PubSubX::_host = config.hostNAME;
    return config;
}

bool PubSubX::pub(String topic, String payload)
{
    return publish(PubSubX::topic(topic).c_str(), payload.c_str());
}

bool PubSubX::sub(String topic, bool parent)
{
    return subscribe(PubSubX::topic(topic, parent).c_str());
}

String PubSubX::topic(String topic, bool parent)
{
    String tpk = PubSubX::_pkey + "/";
    tpk += parent ? "*" : PubSubX::_host;
    return topic.isEmpty() ? tpk : (tpk + "/" + topic);
}

String PubSubX::parse(char *topic)
{
    char *tpk = topic + PubSubX::_pkey.length() + 1;
    tpk += (tpk[0] == '*') ? 1 : PubSubX::_host.length();
    return (tpk[0] == '/') ? tpk + 1 : tpk;
}

String PubSubX::parse(byte *payload, unsigned int length)
{
    if (length)
    {
        char data[length + 1];
        data[length] = '\0';
        strncpy(data, (char *)payload, length);
        return String(data);
    }
    return String();
}

HTTPUpdateResult PubSubX::otaUpdate(const char *pemCert, String url, String ver)
{
    if (url.isEmpty())
    {
        pub("res/update", ver);
        return HTTP_UPDATE_FAILED;
    }

    disconnect();
    return PubSubX::otaUpdate(pemCert, url);
}

HTTPUpdateResult PubSubX::otaUpdate(const char *pemCert, String url)
{
    BearSSL::WiFiClientSecure client;
    BearSSL::X509List x509(pemCert);
    client.setTrustAnchors(&x509);

    if (client.probeMaxFragmentLength("server", 443, 1024))
        client.setBufferSizes(1024, 1024);

    ESPhttpUpdate.setLedPin(LED_BUILTIN);
    return ESPhttpUpdate.update(client, url);
}