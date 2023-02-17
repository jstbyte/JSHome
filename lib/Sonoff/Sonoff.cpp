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
    client.pub("res/sonoff", sonoff.reads(sonoffi));
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

    bool hasChanged = false;
    for (uint8_t i = 0; i < _count; i++)
    {
        hasChanged = write(i, state) || hasChanged;
    }

    return hasChanged;
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