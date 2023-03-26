#ifdef rServo
#include <Arduino.h>
#include <LittleFS.h>
#define SPIFFS LittleFS
#include <TaskScheduler.h>
#include <EZServo.h>
#include <PubSub.h>
#include "Helper.h"
#include "certs.h"
#include <time.h>
#include "spac.h"
#define SP 2

PubSubX &mqttClient = PubSubX::Get();
const char version[] = "v1.0.0";
Scheduler scheduler;
EZServo servo(2);

void mqttCallback(char *tpk, byte *dta, uint32_t length)
{
    auto topic = mqttClient.parse(tpk);
    auto data = mqttClient.parse(dta, length);

    if (topic == "req/servo")
    {
        if (!data.isEmpty())
            servo.write(data.toInt());
        return (void)mqttClient.pub("res/servo", String(servo.current()));
    }

    if (topic == "req/update")
        return (void)mqttClient.update(_firebaseRCA, data, version);
}

void onConnection(PubSubWiFi *)
{
    mqttClient.sub("req/update");
    mqttClient.sub("req/servo");
    mqttClient.pub("res/servo", String(servo.current()));
}

void setup()
{
    LittleFS.begin();
    Serial.begin(115200);
    PubSubX &client = PubSubX::Get();
    client.setCallback(mqttCallback);
    client.onConnection(onConnection);
    client.init("/config.json", _emqxRCA);
    servo.taskSetup(scheduler, 10000);
}

void loop()
{
    mqttClient.eventLoop(); // Keep Alive;
    scheduler.execute();
}
#endif
