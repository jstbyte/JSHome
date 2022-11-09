#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "Debouncer.h"
#ifndef MAX_SONOFF_PIN_COUNT
#define MAX_SONOFF_PIN_COUNT 8
#endif

class Sonoff /* 255 RETURN VALUE = ERROR */
{
protected:
    uint8_t pinCount;
    uint8_t pins[MAX_SONOFF_PIN_COUNT];

public:
    Sonoff();
    Sonoff(uint8_t pins[], uint8_t pinCount);
    uint8_t count();
    uint8_t *getPins();
    uint8_t getPinByIndex(uint8_t idx);
    uint8_t getIndexByPin(uint8_t pin);
    bool add(uint8_t pin); // Add A Pin
    void load(uint8_t pins[], uint8_t pinCount);
    uint8_t load(String path, bool states = false); // Returns Loaded Pins Count From FS;
    void write(uint8_t idx, uint8_t state);         // 0 | 1 | >1
    void write(uint8_t idx);
    void writes();
    void writes(uint8_t state);
    void write_(uint8_t *states); // Args Eg. [1,0,1,0]
    void writes(String states);   // Args Eg. [1,0,1,0]
    uint8_t read(uint8_t idx);
    uint8_t read();              // Return Combined Result > 0 | 1 | >3 :Pins Are Not Synced;
    void reads(uint8_t *states); // Args Eg. [1,0,1,0]
    String reads();              // Return Eg. [1,0,1,0]
};

class Sonoffe : public Debouncer, public Sonoff
{
public:
    /*
    state:::::::::::::::::::::::::::::::::
        <128 = standard;
        +128 = Silent Mode, standard + 128;
         255 = Emmit Event Without Writing;
    */
    void writer(uint8_t index, uint8_t state);
};