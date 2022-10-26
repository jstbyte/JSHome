#include "app_mqtt.h"

u32_t wifiRetryTimeout = WIFI_RETRY_TIMEOUT;
unsigned long long wifiRetryTimeStamp = 0;

const char *mqtt_topic_digi_out = MQTT_TOPIC_DIGI_OUT;
const char *regx_topic_digi_out = REGX_TOPIC_DIGI_OUT;

BearSSL::X509List cert(mqtt_cert);
BearSSL::WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);

void handleMqtt()
{
    if (wifiRetryTimeout)
    {

        if (mqttClient.loop())
        {
            return;
        }

        if (WiFi.isConnected())
        {
            configTime(5 * 3600, 0, "pool.ntp.org", "time.nist.gov");
            if (mqttClient.connect(uuid("ESP8266JST-").c_str()))
            {
                mqttClient.subscribe(mqtt_topic_digi_out);
                DEBUG_LOG_LN("Subscribed!");
                wifiRetryTimeStamp = 0; // Give A Change To Retry When Disconnect;
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
            DEBUG_LOG_LN("MQTT Trying to connect...");
        }
    }
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    char data[length + 1];
    data[length] = '\0';
    strncpy(data, (char *)payload, length);

    std::regex pattern(regx_topic_digi_out);
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
    String topic = String(mqtt_topic_digi_out);
    topic.replace("/req/", "/res/");
    topic.replace("/+", "");
    mqttClient.publish(topic.c_str(), digiOut.reads().c_str());
    DEBUG_LOG_LN("MQTT Event Emitted");
}

wlan_config_t loadWlanConfig(String path)
{
    wlan_config_t config;
    StaticJsonDocument<256> wConfigDoc;
    File configFile = LittleFS.open(path, "r");
    if (deserializeJson(wConfigDoc, configFile) == DeserializationError::Ok)
    {
        config.ssid = wConfigDoc["wlanSSID"].as<String>();
        config.pass = wConfigDoc["wlanPASS"].as<String>();
        config.host = wConfigDoc["hostName"].as<String>();
        configFile.close();
        return config;
    }
    DEBUG_LOG_LN("WLAN Config DeserializationError");

    reBoot(0);
    return config;
}

void setupMqtt(wlan_config_t *config)
{
    WiFi.mode(WIFI_STA);
    WiFi.persistent(false);
    WiFi.begin(config->ssid, config->pass);
    WiFi.setAutoReconnect(true);
    WiFi.setHostname(config->host.c_str());

    wifiClient.setTrustAnchors(&cert);
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
    DEBUG_LOG_LN("WIFI & MQTT Setup Complate")
}