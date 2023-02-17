#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#define SPIFFS LittleFS
#include <WiFiClientSecure.h>
#include <ESP8266httpUpdate.h>
#include <CertStoreBearSSL.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <Helper.h>
#include <time.h>
#ifndef MQTT_RETRY_MS
#define MQTT_RETRY_MS 5000
#endif

typedef struct
{
    String identity;
    String wlanSSID;
    String wlanPASS;
    String hostNAME;
    String mqttHOST;
    u32_t mqttPORT;
} wlan_config_t;

class PubSubWiFi : public PubSubClient
{
    /* TOPIC FORMAT: `{SECRAT}/{DEVICE}/{SERVICES}#` */
    /* DEVICE `*` Mean Any Device Under The Same Key */

protected:
    String _pkey;
    u8_t _maxRetry;
    u8_t _retryCount;
    WiFiClient *_wifiClient;
    unsigned long long _timestamp;
    std::function<void(void)> _onRertyExceeds;
    std::function<void(PubSubWiFi *)> _onConnection;

public:
    void eventLoop();
    static wlan_config_t loadWlanConfig(String path);
    wlan_config_t init(String path, const char *pemCert);
    void init(wlan_config_t *config, const char *pemCert);
    void onConnection(std::function<void(PubSubWiFi *)> cb);
    // PubSubWiFi(const char *pemCert) : _pemCert(pemCert){};
    void onRertyExceeds(std::function<void(void)> cb, u8_t maxRetry);

public:
    String parse(char *_topic);
    bool pub(String _topic, String payload);
    bool sub(String _topic, bool parent = false);
    String topic(String _topic, bool parent = false);
    String parse(byte *payload, unsigned int length);
};

class PubSubX : public PubSubWiFi
{ /* Static Class : PubSubWiFi */
private:
    ~PubSubX(){};
    PubSubX() : PubSubWiFi(){};
    PubSubX(const PubSubX &) = delete;
    PubSubX &operator=(const PubSubX &) = delete;

public:
    static PubSubX &Get();
    HTTPUpdateResult update(const char *pemCert, String url, String ver);
};