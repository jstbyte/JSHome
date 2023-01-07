#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#define SPIFFS LittleFS
#include <WiFiClientSecure.h>
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
    WiFiClient *_wifiClient;
    unsigned long long _timestamp;
    std::function<void(void)> _onTimeout;
    std::function<void(PubSubWiFi *)> _onConnection;

    // c4c = check for changes properties;
    u32_t _c4cInterval;
    unsigned long long _c4cTimestamp;
    std::function<void(uint8_t)> _c4cFunc;

public:
    void eventLoop();
    void resetTimeout(u32_t time);
    wlan_config_t init(String path);
    void init(wlan_config_t *config);
    static wlan_config_t loadWlanConfig(String path);
    void onConnection(std::function<void(PubSubWiFi *)> cb);
    void onTimeout(std::function<void(void)> cb, u32_t time);
    void setC4C(std::function<void(uint8_t)> cb, u32_t time);
};

class PubSubService
{
public:
    virtual bool set(char *identity) = 0;
    virtual bool req(char *payload) = 0;
    virtual bool res() = 0;
};