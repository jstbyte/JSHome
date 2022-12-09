#ifdef rSONOFF
#include "rSonoff/espn.cpp.h"
#include "rSonoff/mqtt.cpp.h"

decode_results ir_result;
IRrecv irrecv(IR_RECV_PIN);

void setup()
{
    LittleFS.begin();
    irrecv.enableIRIn();
    Serial.begin(115200);
    Sonoffe::load("/rsonoff.json");
    if (ConnMan::recover(Sonoffe::pins(), Sonoffe::count()))
    {
        auto connMan = ConnMan::data();
        connMan->timeout = WIFI_RETRY_TIMEOUT;
        Sonoffe::writes(HIGH);
    }

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    DEBUG_LOG("\nMQTT: & WiFi Timeout : ");
    DEBUG_LOG_LN(ConnMan::data()->timeout);
    DEBUG_LOG("Device Mac Address : ");
    DEBUG_LOG_LN(WiFi.macAddress());

    if (ConnMan::data()->timeout)
    {
        ConnMan::data()->timeout--;
        mqttClient = new PubSubWiFi;
        Sonoffe::event.setCallback(emmittMqttEvent, 500);
        setupMqtt("/wconfig.json");
    }
    else
    {
        mqttClient = nullptr;
        setupEspNow();
        Sonoffe::event.setCallback(emmittEspNowEvent, 500);
    }
}

bool ledState = true;
void loop()
{
    if (irrecv.decode(&ir_result))
    {
        DEBUG_LOG("INFARED RECIVED : ");
        Sonoffe::press(ir_result.value);
        DEBUG_LOG_LN(ir_result.value);
        irrecv.resume();
    }

    if (mqttClient)
    {
        mqttClient->eventLoop();
        if (ledState)
            analogWrite(LED_BUILTIN, 255);
        else
            analogWrite(LED_BUILTIN, 254);
        ledState = !ledState;
    }
    else
    {
        analogWrite(LED_BUILTIN, 254);
    }

    MsgPacketizer::parse();
    Sonoffe::event.loop();
    delay(100); // Balance CPU Loads;
}
#endif
