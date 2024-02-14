#ifdef SLOCK
#include <Arduino.h>
#include <LittleFS.h>
#define SPIFFS LittleFS
#include <TaskScheduler.h>
#include <ezBuzzer.h>
#include <EZServo.h>
#include "PassMan.h"
#include <PubSub.h>
#include <IRrecv.h>
#include "Helper.h"
#include "certs.h"
#include <time.h>
#include "spac.h"
#define SP 2

/* file required: data/_password.txt
   chng password: [key/dev]/req/chpass */

#define MAX_PASSWORD_ATTEMPTS 7
#define ATTEMPTS_BASE_DELAY 30000
unsigned int attempts_delay = ATTEMPTS_BASE_DELAY;
unsigned long attempts_timestamp = 0;
PubSubX &mqttClient = PubSubX::Get();
const char version[] = "v1.1.1";
ezBuzzer buzzer(14); // Piezo Bz;
PassMan passman("0000", &buzzer);
decode_results ir_result;
Scheduler scheduler;
EZServo servo(SP);
IRrecv irrecv(13);

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

    if (topic == "req/chpass")
    {
        bool res = passman.load("/_password.txt", data);
        return (void)mqttClient.pub("res/chpass", res ? "OK" : "ERROR");
    }

    if (topic == "req/info")
        return (void)mqttClient.pub("res/servo", String(servo.current()));

    if (topic == "req/update")
        return (void)mqttClient.update(_firebaseRCA, data, version);
}

void onConnection(PubSubWiFi *)
{
    mqttClient.sub("req/update");
    mqttClient.sub("req/servo");
    mqttClient.sub("req/chpass");
    mqttClient.sub("req/info", true);
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
    passman.load("/_password.txt", "0000");
    servo.taskSetup(scheduler, 10000);
    irrecv.enableIRIn();
}

void handleIRKeypad()
{

    if (attempts_timestamp > 0)
    {
        if (millis() - attempts_timestamp < attempts_delay)
        {
            return;
        }

        attempts_delay = ATTEMPTS_BASE_DELAY;
        attempts_timestamp = 0;
    }

    if (!passman.press(ir_result))
    {
        if (ir_result.value == IR_POWER)
        {
            if (passman.isEmpty())
            {
                buzzer.beep(50);
                if (servo.current() != 180)
                    servo.write(180);
            }
            else if (passman.enter())
            {
                servo.write(0);
            }
            else if (passman.attempts() >= MAX_PASSWORD_ATTEMPTS)
            {
                buzzer.beep(attempts_delay);
                attempts_timestamp = millis();
            }
            passman.reset();
        }
    }
}

void loop()
{
    mqttClient.eventLoop(); // Keep Alive;
    scheduler.execute();
    buzzer.loop();

    if (irrecv.decode(&ir_result))
    {
        handleIRKeypad();
        irrecv.resume();
    }
}
#endif
