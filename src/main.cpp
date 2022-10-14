#include <Arduino.h>
#include "credentials.h"
/* Fixed SPIFFS Error */
#include <FS.h>
#define SPIFFS LittleFS
#include <LittleFS.h>
/* Check ESP Board */
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
/* Include All Other Library */
#include <ESPAsyncWebServer.h>
#include <CertStoreBearSSL.h>
#include <WiFiClientSecure.h>
#include <AsyncElegantOTA.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <IRutils.h>
#include <IRrecv.h>
#include <regex.h>
#include <time.h>
/* My Won Library */
#include "Debouncer.h"
#include "uUtils.h"
#include "Dio.h"

/* WiFi Credentials */
const char *ssid_sta = WIFI_SSID;
const char *password = WIFI_PASS;

/* MQTT Refs */
BearSSL::CertStore certStore;
BearSSL::WiFiClientSecure wifiClint;
PubSubClient mqttClient(wifiClint);

/* IR Refs */
decode_results ir_result;
IRrecv irrecv(IR_RECV_PIN);

/* Pin Refs */
Debouncer eventEmitter;
Dio dio;

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    if (!strcmp(topic, mqtt_topic_dio))
    {
        char req[length + 1];
        req[length] = '\0';
        strncpy(req, (char *)payload, length);
        u8 respCode = dio.write(req);
        if (respCode < 100)
        {
            String topic = String(mqtt_topic_dio);
            topic.replace("/req/", "/res/");
            mqttClient.publish(topic.c_str(), dio.read().c_str());
        }
        else if (respCode < 255)
        {
            eventEmitter.start();
        }
    }
}

void emittEvent()
{
    if (mqttClient.connected())
    {
        String topic = String(mqtt_topic_dio);
        topic.replace("/req/", "/res/");
        mqttClient.publish(topic.c_str(), dio.read().c_str());
    }
}

void setup()
{
    Serial.begin(9600);
    LittleFS.begin();
    dio.load("/dio.json");
    dio.setMode(OUTPUT);
    dio.write(HIGH);

    pinMode(LED_BUILTIN, OUTPUT);
    analogWrite(LED_BUILTIN, 254);

    irrecv.enableIRIn();
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid_sta, password);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);

    eventEmitter.setCallback(emittEvent, 500);
    certStore.initCertStore(LittleFS, "/certs.idx", "/certs.ar");

    wifiClint.setCertStore(&certStore);
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
}

bool ledFlash = false;
void loop()
{
    // Infrared Remote
    if (irrecv.decode(&ir_result))
    {
        bool notify = true;
        switch (ir_result.value)
        {
        case IR_POWER:
            dio.write(HIGH);
            break;
        case IR_EQ:
            dio.flip(LOW);
            break;
        case IR_MODE:
            dio.flip();
            break;
        case IR_1:
            dio.flip(0);
            break;
        case IR_2:
            dio.flip(1);
            break;
        case IR_3:
            dio.flip(2);
            break;
        case IR_4:
            dio.flip(3);
            break;
        default:
            notify = false;
            break;
        }
        irrecv.resume();
        // Notify Mqtt;
        if (notify)
        {
            eventEmitter.start();
        }
    }

    // MQTT Subscribtions;
    if (mqttClient.connected())
    {
        mqttClient.loop();
    }
    else if (WiFi.isConnected())
    {
        configTime(5 * 3600, 0, "pool.ntp.org", "time.nist.gov");
        if (mqttClient.connect(uuid("ESP8266JST-").c_str()))
        {
            mqttClient.subscribe(mqtt_topic_dio);
            analogWrite(LED_BUILTIN, 254);
            Serial.println("Subscribed!");
        }
    }
    else
    {
        analogWrite(LED_BUILTIN, ledFlash ? 254 : 255);
        ledFlash = !ledFlash;
    }

    eventEmitter.loop();
    delay(100);
}
