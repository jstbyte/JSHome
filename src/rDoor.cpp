#ifdef rDOOR
#include "rDoor/mqtt.cpp.h"
#include "rDoor/infared.cpp.h"

Door door;

void setup()
{
    LittleFS.begin();
    irrecv.enableIRIn();
    Serial.begin(115200);
    pinMode(12, OUTPUT);
    digitalWrite(12, HIGH);

    if (ConnMan::recover(&door.isLocked, 1))
    {
        ConnMan::data()->timeout = WIFI_RETRY_TIMEOUT;
        door.isLocked = DOOR_STATE_NULL;
    }

    if (door.isLocked != DOOR_STATE_NULL)
        door.attach(DOOR_LOCK_SERVO_PIN, 500, 2400);
    door.timer.setCallback(mqttDoorChanged, 0);

    if (ConnMan::data()->timeout)
    {
        ConnMan::data()->timeout--;
        mqttClient = new PubSubWiFi;
        setupMqtt("/wconfig.json");
    }
    else
    {
        mqttClient = nullptr;
        Serial.println("espnow mode");
    }
}

void loop()
{
    handleInfared();
    door.timer.loop();
    if (mqttClient)
        mqttClient->eventLoop();
    delay(100);
}

/****************************/
void Door::write(int value)
{
    if (isLocked == DOOR_STATE_NULL)
    {
        if (value == DOOR_STATE_NULL)
            return;
        attach(DOOR_LOCK_SERVO_PIN, 500, 2400);
    }
    Servo::write(value);
    if (value == SERVO_DOOR_OPEN)
        isLocked = 0;
    else if (value == SERVO_DOOR_LOCK)
        isLocked = 1;
}
#endif