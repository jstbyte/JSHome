#include "Sonoff.h"

uint8_t Sonoff::_count = 0;
uint8_t Sonoff::_pins[MAX_SONOFF_PIN_COUNT];

uint8_t Sonoff::count()
{
    return Sonoff::_count;
}

uint8_t *Sonoff::pins()
{
    return Sonoff::_pins;
}

uint8_t Sonoff::pin(uint8_t idx)
{
    if (idx < Sonoff::_count)
    {
        return Sonoff::_pins[idx];
    }

    return 255;
}

uint8_t Sonoff::indexOf(uint8_t pin)
{
    /* Return 255 If Not Found */
    for (uint8_t i = 0; i < Sonoff::_count; i++)
    {
        if (Sonoff::_pins[i] == pin)
        {
            return i;
        }
    }
    return 255;
}

bool Sonoff::add(uint8_t pin) /* Add A Pin */
{
    if (Sonoff::_count < MAX_SONOFF_PIN_COUNT)
    {
        pinMode(pin, OUTPUT);
        Sonoff::_pins[Sonoff::_count] = pin;
        Sonoff::_count++;
        return true;
    }

    return false;
}

void Sonoff::load(uint8_t *pins, uint8_t count)
{
    memcpy(Sonoff::_pins, pins, count);
    Sonoff::_count = count;
    for (uint8_t i = 0; i < Sonoff::_count; i++)
    {
        pinMode(Sonoff::_pins[i], OUTPUT);
    }
}

uint8_t Sonoff::load(String path, bool states) /* Returns Loaded Pins Count */
{
    File file = LittleFS.open(path, "r");
    StaticJsonDocument<128> jdoc;

    if (deserializeJson(jdoc, file) == DeserializationError::Ok)
    {
        if (states) /* Load DigiOut Pins Default State [...numbers] */
        {
            uint8_t index = 0;
            for (JsonVariant v : jdoc.as<JsonArray>())
            {
                if (index <= Sonoff::_count)
                {
                    digitalWrite(Sonoff::_pins[index], v.as<u8>());
                    index++;
                    continue;
                }
                break;
            }
            file.close();
            return index;
        }

        Sonoff::_count = 0; /* Load DigiOut Pins [...numbers] */
        for (JsonVariant v : jdoc.as<JsonArray>())
        {
            if (Sonoff::_count < MAX_SONOFF_PIN_COUNT)
            {
                Sonoff::_pins[Sonoff::_count] = v.as<u8>();
                pinMode(Sonoff::_pins[Sonoff::_count], OUTPUT);
                Sonoff::_count++;
                continue;
            }
            break;
        }
    }

    file.close();
    return Sonoff::_count;
}

void Sonoff::write(uint8_t index, uint8_t state) /* 0 | 1 | >1 */
{
    if (index < Sonoff::_count)
    {
        if (state > 1)
        {
            digitalWrite(Sonoff::_pins[index], !digitalRead(Sonoff::_pins[index]));
        }
        else
        {
            digitalWrite(Sonoff::_pins[index], state);
        }
    }
}

void Sonoff::write(uint8_t index)
{
    if (index < Sonoff::_count)
    {
        digitalWrite(Sonoff::_pins[index], !digitalRead(Sonoff::_pins[index]));
    }
}

void Sonoff::writes(uint8_t state)
{
    for (uint8_t i = 0; i < Sonoff::_count; i++)
    {
        digitalWrite(Sonoff::_pins[i], state);
    }
}

void Sonoff::writes()
{
    for (uint8_t i = 0; i < Sonoff::_count; i++)
    {
        digitalWrite(Sonoff::_pins[i], !digitalRead(Sonoff::_pins[i]));
    }
}

void Sonoff::writes(uint8_t *states, uint8_t len)
{
    if (Sonoff::_count != len)
    {
        return;
    }

    for (uint8_t i = 0; i < Sonoff::_count; i++)
    {
        Sonoff::write(i, states[i]);
    }
}

void Sonoff::writes(String states)
{
    StaticJsonDocument<128> jdoc;
    if (deserializeJson(jdoc, states) == DeserializationError::Ok)
    {
        uint8_t idx = 0;
        for (JsonVariant v : jdoc.as<JsonArray>())
        {
            if (idx < Sonoff::_count)
            {
                Sonoff::write(idx, v.as<u8>());
                idx++;
                continue;
            }
            break;
        }
    }
}

uint8_t Sonoff::read(uint8_t index)
{
    if (index < Sonoff::_count)
    {
        return digitalRead(Sonoff::_pins[index]);
    }

    return 255;
}

uint8_t Sonoff::read()
{
    uint8_t result = digitalRead(_pins[0]);
    for (uint8_t i = 1; i < Sonoff::_count; i++)
    {
        if (digitalRead(Sonoff::_pins[i]) != result)
        {
            return 3;
        }
    }

    return result;
}

void Sonoff::reads(uint8_t *states)
{
    for (uint8_t i = 0; i < Sonoff::_count; i++)
    {
        states[i] = digitalRead(Sonoff::_pins[i]);
    }
}

String Sonoff::reads()
{

    if (Sonoff::_count)
    {
        String res = "[";
        for (uint8_t i = 0; i < Sonoff::_count; i++)
        {
            res += String(digitalRead(Sonoff::_pins[i]) ? 1 : 0) + ",";
        }
        res[res.length() - 1] = ']';
        return res;
    }

    return String("[]");
}

/* Sonoffe Implementation */
std::function<void(uint8_t)> Sonoffe::trigger;

void Sonoffe::setTrigger(std::function<void(uint8_t)> tgr)
{
    Sonoffe::trigger = tgr;
}

void Sonoffe::writer(uint8_t index, uint8_t state)
{
    if (state == 255)
    { /* Event Fire Only */
        Sonoffe::trigger(index);
        return;
    }

    if (index < Sonoff::_count)
    {
        if (state < 128)
        {
            if (digitalRead(Sonoff::_pins[index]) != state)
            {
                Sonoff::write(index, state);
                Sonoffe::trigger(index);
            }
            return;
        }

        state -= 128;
        Sonoff::write(index, state);
    }
}

void Sonoffe::writer(char *csd)
{
    unsigned short index = 255;
    unsigned short state = 255;
    sscanf(csd, "%hu:%hu", &index, &state);
    Sonoffe::writer(index, state);
}

void Sonoffe::press(uint64_t value)
{
    uint8_t index = MULTI_PIN_INDEX;
    switch (value)
    {
    case IR_POWER:
        if (Sonoffe::read() == HIGH)
            return;
        Sonoffe::writes(HIGH);
        break;
    case IR_MUTE:
        if (Sonoffe::read() == LOW)
            return;
        Sonoffe::writes(LOW);
        break;
    case IR_MODE:
        Sonoffe::writes();
        break;
    case IR_1:
        index = 0;
        break;
    case IR_2:
        index = 1;
        break;
    case IR_3:
        index = 2;
        break;
    case IR_4:
        index = 3;
        break;
    default:
        return;
    }
    Sonoffe::write(index);
    Sonoffe::trigger(index);
}