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
#include <RTCMemory.h>
#include <Helper.h>
#include <time.h>
#ifndef RTC_DATA_SIZE
#define RTC_DATA_SIZE 8
#endif

typedef struct
{
    uint8_t bootCount;
    uint32_t timeout;
    uint8_t data[7];
} RTCData;

typedef struct
{
    String identity;
    String wlanSSID;
    String wlanPASS;
    String hostNAME;
    String mqttHOST;
    u32_t mqttPORT;
} wlan_config_t;

class BootMan
{
protected:
    static RTCMemory<RTCData> rtcData;

public:
    static RTCData *data();
    static bool recover(void *data = nullptr, uint8_t len = 0);
    static void reboot(uint32_t timeout, void *data = nullptr, uint8_t len = 0);
};

class PubSubWiFi : public PubSubClient
{
protected:
    u32_t _connTimeout;
    const char *_pemCert;
    WiFiClient *_wifiClient;
    unsigned long long _timestamp;
    std::function<void(void)> _onTimeout;
    std::function<void(PubSubWiFi *)> _onConnection;

public:
    void eventLoop();
    void resetTimeout(u32_t time);
    wlan_config_t init(String path);
    void init(wlan_config_t *config);
    static wlan_config_t loadWlanConfig(String path);
    void onConnection(std::function<void(PubSubWiFi *)> cb);
    void onTimeout(std::function<void(void)> cb, u32_t time);
    PubSubWiFi(const char *pemCert) : _pemCert(pemCert){};
};

class PubSubX : public PubSubWiFi
{
    /* TOPIC FORMAT: `{SECRAT}/req|res/${service}/?{hostname}` */

protected:
    static String _pkey;

public:
    wlan_config_t init(String path);
    static String parse(char *topic);
    bool res(String topic, String payload);
    bool sub(String topic, bool host = false);
    static String req(String topic, bool host = false);
    PubSubX(const char *pemCert) : PubSubWiFi(pemCert){};
    static String parse(byte *payload, unsigned int length);
    static HTTPUpdateResult otaUpdate(const char *pemCert, String url);
};
