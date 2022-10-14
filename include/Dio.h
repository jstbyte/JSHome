#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#ifndef MAX_DIO_PIN_COUNT
#define MAX_DIO_PIN_COUNT 8
#endif

class Dio /* 255 RETURN VALUE = ERROR */
{
private:
    u8 pinCount;
    u8 pins[MAX_DIO_PIN_COUNT];

public:
    Dio() : pinCount(0){};
    Dio(u8 pins[], u8 pinCount)
    {
        memcpy(this->pins, pins, pinCount);
        pinCount = pinCount;
    };

    u8 getLength()
    {
        return pinCount;
    }

    u8 getPinByIndex(u8 idx)
    {
        if (idx < pinCount)
        {
            return pins[idx];
        }

        return 255;
    }

    u8 getIndexByPin(u8 pin)
    {
        /* Return 255 If Not Found */
        for (u8 i = 0; i < pinCount; i++)
        {
            if (pins[i] == pin)
            {
                return i;
            }
        }
        return 255;
    }

    bool add(u8 pin)
    {
        if (pinCount < MAX_DIO_PIN_COUNT)
        {
            pins[pinCount] = pin;
            pinCount++;
            return true;
        }

        return false;
    }

    void setMode(u8 idx, u8 mode)
    {
        if (idx < pinCount)
        {
            pinMode(pins[idx], mode);
        }
    }

    void setMode(u8 mode)
    {
        for (u8 i = 0; i < pinCount; i++)
        {
            pinMode(pins[i], mode);
        }
    }

    void load(u8 pins[], u8 pinCount)
    {
        memcpy(this->pins, pins, pinCount);
        pinCount = pinCount;
    }

    u8 load(String path) /* Returns Loaded Pins Count */
    {
        pinCount = 0;
        File file = LittleFS.open(path, "r");
        StaticJsonDocument<128> dioJson;
        if (deserializeJson(dioJson, file) == DeserializationError::Ok)
        {
            for (JsonVariant v : dioJson.as<JsonArray>())
            {
                if (pinCount < MAX_DIO_PIN_COUNT)
                {
                    pins[pinCount] = v.as<u8>();
                    pinCount++;
                    continue;
                }
                break;
            }
        }

        file.close();
        return pinCount;
    }

    void write(u8 idx, u8 state)
    {
        if (idx < pinCount)
        {
            digitalWrite(pins[idx], state);
        }
    }

    void write(u8 state)
    {
        for (u8 i = 0; i < pinCount; i++)
        {
            digitalWrite(pins[i], state);
        }
    }

    u8 write(char *req)
    {
        const char *validation = "^\\/?dio\\/([0-9]?[0-9])(?:\\/(on|off|flip))?$";
        std::regex pattern(validation);
        std::string searchable(req);
        std::smatch matches;

        if (std::regex_search(searchable, matches, pattern))
        {
            u8 pinIndex = String(matches[1].str().c_str()).toInt() - 1;

            if (pinIndex < pinCount)
            {

                String action = String(matches[2].str().c_str());
                if (!action.isEmpty())
                {
                    if (action == "flip")
                    {
                        digitalWrite(pins[pinIndex], !digitalRead(pins[pinIndex]));
                    }
                    else
                    {
                        digitalWrite(pins[pinIndex], action == "on" ? HIGH : LOW);
                    }
                    return 100 + pinIndex;
                }
                return pinIndex;
            }
        }
        return 255;
    }

    void flip(u8 idx)
    {
        if (idx < pinCount)
        {
            digitalWrite(pins[idx], !digitalRead(pins[idx]));
        }
    }

    void flip()
    {
        for (u8 i = 0; i < pinCount; i++)
        {
            digitalWrite(pins[i], !digitalRead(pins[i]));
        }
    }

    u8 read(u8 idx)
    {
        if (idx < pinCount)
        {
            return digitalRead(pins[idx]);
        }

        return 255;
    }

    String read()
    {
        String res = "";
        for (u8 i = 0; i < pinCount; i++)
        {
            res += String(digitalRead(pins[i]) ? 1 : 0) + ",";
        }
        res.remove(res.length() - 1);
        return res;
    }
};
