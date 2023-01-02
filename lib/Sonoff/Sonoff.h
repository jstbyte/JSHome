#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Debouncer.h>
#include <LittleFS.h>
#include "Helper.h"
#define MULTI_PIN_INDEX 255
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
    static void write(uint8_t index, uint8_t state);       // write pin by index, (state: 0 | 1 | >1);
    static void write(uint8_t index);                      // flip pin state by index;
    static void writes();                                  // flip all pin states;
    static void writes(uint8_t state);                     // set all pin state at once;
    static void writes(uint8_t *states, uint8_t len);      // bulk state change from array;
    static void writes(String states);                     // bulk state change | args Eg. `[1,0,1,0]`;
    static uint8_t read(uint8_t index);                    // read pin state by index;
    static uint8_t read();                                 // combined read: 0 | 1 | (>3 :Pins Are Not Synced);
    static void reads(uint8_t *states);                    // read states to array;
    static String reads();                                 // read state | return Eg. `[1,0,1,0]`;
};

class Sonoffe : public Sonoff
{

public:
    /*
    colon separated data = `[idx]:[state]`;
    state::::::::::::::::::::::::::::::::::
        <128 = standard;
        +128 = Silent Mode, standard + 128;
         255 = Emmit Event Without Writing;
    */
    static void writer(char *csd);
    static void press(uint64_t value); // IR Code;
    static std::function<void(uint8_t)> trigger;
    static void writer(uint8_t index, uint8_t state);
    static void setTrigger(std::function<void(uint8_t)> tgr);
};