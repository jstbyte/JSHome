#ifdef rSONOFF
#include <Arduino.h>
#include <LittleFS.h>
#define SPIFFS LittleFS
#include <CertStoreBearSSL.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClientSecure.h>
#include <TaskScheduler.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <IRutils.h>
#include <PubSub.h>
#include <IRrecv.h>
#include "Helper.h"
#include "Sonoff.h"
#include "certs.h"
#include <time.h>
#include "spac.h"
#define IR 13

PubSubX &mqttClient = PubSubX::Get();
uint8_t manual_sw_pins[2] = {4, 5};
const char version[] = "v3.2.0";
decode_results ir_result;
Task manual_sw_task;
Scheduler scheduler;
IRrecv irrecv(IR);

void IRAM_ATTR manual_sw_isr()
{
    manual_sw_task.restartDelayed(100);
}

void manual_sw_task_callback()
{
    (Snf::Get()).write(0, digitalRead(manual_sw_pins[0]));
    (Snf::Get()).write(1, digitalRead(manual_sw_pins[1]));
}

void mqttCallback(char *tpk, byte *dta, uint32_t length)
{
    auto topic = mqttClient.parse(tpk);
    auto data = mqttClient.parse(dta, length);

    DEBUG_LOG("new message recived from:")
    DEBUG_LOG_LN(topic)

    if (topic.startsWith("req/sonoff"))
        return (void)(Snf::Get()).writes(data);

    if (topic == "req/info")
        return (void)(Snf::Get()).writes("");

    if (topic == "req/update")
        return (void)mqttClient.update(_firebaseRCA, data, version);
}

void onConnection(PubSubWiFi *)
{
    if (!mqttClient.connected())
        return (void)ledTask.enable();

    mqttClient.sub("req/info", true);
    analogWrite(LED_BUILTIN, 254);
    mqttClient.sub("req/sonoff?");
    mqttClient.sub("req/sonoff");
    mqttClient.sub("req/update");
    Snf::Get().fire();
    ledTask.disable();
    return;
}

void setup()
{
    LittleFS.begin();
    irrecv.enableIRIn();
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    ledTask.set(5, TASK_FOREVER, &ledBlink);
    scheduler.addTask(ledTask);
    ledTask.enableDelayed();

    PubSubX &client = PubSubX::Get();
    client.setCallback(mqttCallback);
    client.onConnection(onConnection);
    client.init("/config.json", _emqxRCA);

    // (Snf::Get()).begin("/sonoff.txt");
    (Snf::Get()).init("12:1;14:1"); // Two Pins Only;
    (Snf::Get()).taskSetup(scheduler, &Snf::fire, 1000, true);

    if ((Snf::Get()).count() < 3) // Ignore if>2;
    {
        // Set: Sonoff Output as Manual SW Input;
        pinMode(manual_sw_pins[0], INPUT_PULLUP);
        pinMode(manual_sw_pins[1], INPUT_PULLUP);
        (Snf::Get()).write(0, digitalRead(manual_sw_pins[0]));
        (Snf::Get()).write(1, digitalRead(manual_sw_pins[1]));
        uint8_t i0 = digitalPinToInterrupt(manual_sw_pins[0]);
        uint8_t i1 = digitalPinToInterrupt(manual_sw_pins[1]);
        manual_sw_task.set(0, 1, manual_sw_task_callback);
        attachInterrupt(i0, manual_sw_isr, CHANGE);
        attachInterrupt(i1, manual_sw_isr, CHANGE);
        scheduler.addTask(manual_sw_task);
    }
}

void loop()
{
    if (irrecv.decode(&ir_result))
    {
        (Snf::Get()).press(ir_result.value);
        irrecv.resume();
    }

    mqttClient.eventLoop();
    scheduler.execute();
}
#endif
