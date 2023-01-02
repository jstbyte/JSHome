#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Debouncer.h>
#include <LittleFS.h>
#include "Helper.h"
#define SONOFF_OVERFLOW 255
#ifndef MAX_SONOFF_PIN_COUNT
#define MAX_SONOFF_PIN_COUNT 8
#endif

class Sonoff /* 255 RETURN VALUE = ERROR */
{
protected:
    static uint8_t _count;
    static uint8_t _pins[MAX_SONOFF_PIN_COUNT];

public:
    static uint8_t count();                                // pin counts;
    static uint8_t *pins();                                // get pins array;
    static uint8_t pin(uint8_t index);                     // get pin number by index;
    static uint8_t indexOf(uint8_t pin);                   // get index by pin number;
    static bool add(uint8_t pin);                          // add a pin to pins array;
    static void load(uint8_t pins[], uint8_t len);         // bulk load pins from array;
    static uint8_t load(String path, bool states = false); // load pins from FS & return count;
    static bool write(uint8_t index, uint8_t state);       // write pin by index, (state: 0 | 1 | >1);
    static bool write(uint8_t index);                      // flip pin state by index;
    static void writes();                                  // flip all pin states;
    static void writes(uint8_t state);                     // set all pin state at once;
    static bool writes(uint8_t *states, uint8_t len);      // bulk state change from array | skip 255;
    static bool writes(String states);                     // bulk state change | arg. Eg. (`[0,2,1,0]`)-1;
    static uint8_t read(uint8_t index);                    // read pin state by index;
    static uint8_t read();                                 // combined read: 0 | 1 | (>3 :Pins Are Not Synced);
    static void reads(uint8_t *states);                    // read states to array;
    static String reads();                                 // read state | return Eg. `[1,0,1,0]`;
};

class Sonoffe : public Sonoff
{

public:
    static void writer(String states); // arg eg. `[0,0,0,1]`;
    static void press(uint64_t value); // IR Code;
    static std::function<void(uint8_t)> trigger;
    static void writer(uint8_t index, uint8_t state);
    static void setTrigger(std::function<void(uint8_t)> tgr);
};