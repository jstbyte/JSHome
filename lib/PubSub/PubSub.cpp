#include "PubSub.h"

PubSubX &PubSubX::Get()
{
    static PubSubX instance;
    return instance;
}

void PubSubWiFi::eventLoop()
{
    if (state() == MQTT_CONNECTED)
    {
        return (void)loop();
    }

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

wlan_config_t PubSubWiFi::init(String path, const char *pemCert)
{
    auto config = PubSubWiFi::loadWlanConfig(path);
    init(&config, pemCert);
    return config;
}

void PubSubWiFi::init(wlan_config_t *config, const char *pemCert)
{
    WiFi.mode(WIFI_STA);
    WiFi.persistent(false);
    WiFi.begin(config->wlanSSID, config->wlanPASS);
    WiFi.setAutoReconnect(true);
    WiFi.setHostname(config->hostNAME.c_str());

    char *mqttHost = new char[config->mqttHOST.length() + 1];
    config->mqttHOST.toCharArray(mqttHost, config->mqttHOST.length() + 1);

    DEBUG_LOG("MQTT: & WiFi Initialized");
    if (config->mqttPORT == 8883)
    {
        DEBUG_LOG_LN("::Secure.");
        auto caCert = new BearSSL::X509List(pemCert);
        _wifiClient = new BearSSL::WiFiClientSecure();
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
    _pkey = config->identity;
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

bool PubSubWiFi::pub(String _topic, String payload)
{
    return publish(topic(_topic).c_str(), payload.c_str());
}

bool PubSubWiFi::sub(String _topic, bool parent)
{
    return subscribe(topic(_topic, parent).c_str());
}

String PubSubWiFi::topic(String _topic, bool parent)
{
    String tpk = _pkey + "/";
    tpk += parent ? "*" : String(WiFi.getHostname());
    return _topic.isEmpty() ? tpk : (tpk + "/" + _topic);
}

String PubSubWiFi::parse(char *_topic)
{
    char *tpk = _topic + _pkey.length() + 1;
    tpk += (tpk[0] == '*') ? 1 : String(WiFi.getHostname()).length();
    return (tpk[0] == '/') ? tpk + 1 : tpk;
}

String PubSubWiFi::parse(byte *payload, unsigned int length)
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

/**************************************************************************/

HTTPUpdateResult PubSubX::update(const char *pemCert, String url, String ver)
{
    if (url.isEmpty())
    {
        pub("res/update", ver);
        return HTTP_UPDATE_FAILED;
    }

    this->disconnect();

    BearSSL::WiFiClientSecure client;
    BearSSL::X509List x509(pemCert);
    client.setTrustAnchors(&x509);

    if (client.probeMaxFragmentLength("server", 443, 1024))
        client.setBufferSizes(1024, 1024);

    ESPhttpUpdate.setLedPin(LED_BUILTIN);
    return ESPhttpUpdate.update(client, url);
}
