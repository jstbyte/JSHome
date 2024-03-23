#include "Sonoff.h"
#include <PubSub.h>

bool Snf::_enabled = false;

Snf &Snf::Get()
{
    static Snf instance;
    _enabled = true;
    return instance;
}

void Snf::fire()
{
    Snf &sonoff = Snf::Get();
    PubSubX &client = PubSubX::Get();
    uint8_t sonoffi = sonoff.cmask() ? 255 : 128;
    client.pub("res/sonoff", sonoff.readJson(sonoffi));
}

bool Snf::enabled()
{
    return _enabled;
}

uint8_t Sonoff::count() // Geter
{
    return _count;
}

uint8_t Sonoff::cmask() // Geter
{
    return _cmask;
}

uint8_t *Sonoff::pins() // Geter
{
    return _pnums;
}

void Sonoff::init(String rw)
{
    char *token = strtok((char *)rw.c_str(), ";");
    while (token != NULL)
    {
        unsigned short pin = 255;
        unsigned short state = 255;
        sscanf(token, "%hu:%hu", &pin, &state);

        if (pin != 255 || state != 255)
        {
            pinMode(pin, OUTPUT);
            digitalWrite(pin, state);
            _pnums[_count] = pin;
            _count++;
        }

        token = strtok(NULL, ";");
    }
}

uint8_t Sonoff::begin(String path)
{
    File file = LittleFS.open(path, "r");

    while (file.available())
    {
        unsigned short pin = 255;
        unsigned short state = 255;
        String csd = file.readStringUntil(';');
        sscanf(csd.c_str(), "%hu:%hu", &pin, &state);

        if (pin != 255 || state != 255)
        {
            pinMode(pin, OUTPUT);
            digitalWrite(pin, state);
            _pnums[_count] = pin;
            _count++;
        }
    }

    file.close();
    return _count;
};

uint8_t Sonoff::read(uint8_t index)
{
    if (index >= _count)
        return 0;
    return digitalRead(_pnums[index]);
}

uint8_t Sonoff::write(uint8_t index, uint8_t state, bool mask)
{
    if (index < _count)
    {
        bool currentState = digitalRead(_pnums[index]);
        state = (state > 1) ? !currentState : state;
        if (state == currentState)
            return false; /* No Pin Affected */
        digitalWrite(_pnums[index], !currentState);

        if (mask)
        {
            _cmask |= (1 << index);
#ifdef ENABLE_SONOFF_EVENT
            _task.restartDelayed(_delay);
#endif
        }

        return true;
    }

    if (index == 128)
    {
        bool hasChanged = false;
        for (uint8_t i = 0; i < _count; i++)
        {
            hasChanged = write(i, state) || hasChanged;
        }

        return hasChanged;
    }

    return false; // Invalid Index;
}

String Sonoff::reads(uint8_t index)
{
    if (index < _count)
    {
        char csd[8];
        sprintf(csd, "%hu:%hu", index, read(index));
        return String(csd);
    }

    String result;

    if (index == 255)
    {
        for (uint8_t i = 0; i < _count; i++)
            if (_cmask & (1 << i))
                result += reads(i) + ";";

        result.remove(result.length() - 1);
        _cmask = 0;
        return result;
    }

    for (uint8_t i = 0; i < _count; i++)
    {
        result += reads(i) + ";";
    }

    result.remove(result.length() - 1);
    return result;
}

uint8_t Sonoff::writes(String extrw)
{
    if (extrw.isEmpty())
    {
#ifdef ENABLE_SONOFF_EVENT
        this->reset();
        _task.restart();
#endif
        return false;
    }

    char *token = strtok((char *)extrw.c_str(), ";");
    bool hasChanged = false;
    while (token != NULL)
    {
        unsigned short index = 255;
        unsigned short state = 255;
        sscanf(token, "%hu:%hu", &index, &state);
        hasChanged = write(index, state) || hasChanged;
        token = strtok(NULL, ";");
    }
    return hasChanged;
}

void Sonoff::reset(uint8_t index)
{
    if (index != 255)
    {
        _cmask = 0;
        return;
    }

    Sonoff::_cmask &= ~(1 << index);
}

uint8_t Sonoff::press(uint64_t value)
{
    bool hasChanged = false;
    switch (value)
    {
    case IR_POWER:
        hasChanged = write(128, HIGH);
        break;
    case IR_MUTE:
        hasChanged = write(128, LOW);
        break;
    case IR_MODE:
        hasChanged = write(128);
        break;
    case IR_1:
        hasChanged = write(0);
        break;
    case IR_2:
        hasChanged = write(1);
        break;
    case IR_3:
        hasChanged = write(2);
        break;
    case IR_4:
        hasChanged = write(3);
        break;
    default:
        return false;
    }
    return hasChanged;
}

String Sonoff::readJson(uint8_t index)
{

    JsonDocument doc; // Json Memory;
    String data;      // Json   Data;

    if (index < _count)
    {

        doc[String(index)] = read(index);
        serializeJson(doc, data);
        return data;
    }

    String result;

    if (index == 255)
    {
        for (uint8_t i = 0; i < _count; i++)
            if (_cmask & (1 << i)) // Check;
                doc[String(i)] = read(i);

        _cmask = 0; // Reset Changed Mask;
        serializeJson(doc, data);
        return data;
    }

    for (uint8_t i = 0; i < _count; i++)
    {
        doc[String(i)] = read(i);
    }

    serializeJson(doc, data);
    return data;
}

uint8_t Sonoff::writeJson(String data)
{

    if (data.isEmpty())
    {
#ifdef ENABLE_SONOFF_EVENT
        this->reset();
        _task.restart();
#endif
        return false;
    }

    Serial.println(data);

    JsonDocument doc;
    bool hasChanged = false;

    if (deserializeJson(doc, data))
        return false;

    for (JsonPair kv : doc.as<JsonObject>())
    {
        uint8_t index = String(kv.key().c_str()).toInt();
        uint8_t state = String(kv.value()).toInt();

        hasChanged = write(index, state) || hasChanged;
    }

    return hasChanged;
}

#ifdef ENABLE_SONOFF_EVENT
void Sonoff::taskSetup(Scheduler &ts, TaskCallback cb, uint32_t delay, bool check)
{
    if (check && !_count)
        return;

    _task.set(0, 1, cb);
    ts.addTask(_task);
    _delay = delay;
}
#endif
