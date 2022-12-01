#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Debouncer.h>
#include <LittleFS.h>
#ifndef MAX_SONOFF_PIN_COUNT
#define MAX_SONOFF_PIN_COUNT 8
#endif

class Sonoff /* 255 RETURN VALUE = ERROR */
{
protected:
    static uint8_t _count;
    static uint8_t _pins[MAX_SONOFF_PIN_COUNT];

public:
    static uint8_t count();
    static uint8_t *pins();
    static uint8_t pin(uint8_t index);
    static uint8_t indexOf(uint8_t pin);
    static bool add(uint8_t pin); // Add A Pin
    static void load(uint8_t pins[], uint8_t len);
    static uint8_t load(String path, bool states = false); // Returns Loaded Pins Count From FS;
    static void write(uint8_t index, uint8_t state);       // 0 | 1 | >1
    static void write(uint8_t index);
    static void writes();
    static void writes(uint8_t state);
    static void write_(uint8_t *states); // Args Eg. [1,0,1,0]
    static void writes(String states);   // Args Eg. [1,0,1,0]
    static uint8_t read(uint8_t index);
    static uint8_t read();              // Return Combined Result > 0 | 1 | >3 :Pins Are Not Synced;
    static void reads(uint8_t *states); // Args Eg. [1,0,1,0]
    static String reads();              // Return Eg. [1,0,1,0]
};

class Sonoffe : public Sonoff
{

public:
    /*
    state:::::::::::::::::::::::::::::::::
        <128 = standard;
        +128 = Silent Mode, standard + 128;
         255 = Emmit Event Without Writing;
    */
    static void writer(uint8_t index, uint8_t state);
    static Debouncer event;
};