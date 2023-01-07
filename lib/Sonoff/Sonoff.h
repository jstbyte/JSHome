#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#include "Helper.h"

/* ::::RULES DEFINED BY AUTHOR::::;
 * External Read & Write as String;
 * RW:   `[PIN INDEX]:[PIN STATE]`;
 * Multiple RW:Semicolon Separated;
 * INDEX 255 is a special opp Code;
 * INDEX 128  is equal to all pins;
 * STATE >1 mean pin flip | toggle;
 */

class Sonoff
{
protected:
    static uint8_t _count;   // Pin Counter;
    static uint8_t _cmask;   // Changed Msk;
    static uint8_t _pins[8]; // Pin Numbers;

public:
    static uint8_t count();                              // Get available pins count;
    static uint8_t cmask();                              // Get  changed masked byte;
    static uint8_t *pins();                              // Get IO pin numbers array;
    static bool begin(String path);                      // Load & Init Pins from FS;
    static bool read(uint8_t index);                     // Read pin state  by index;
    static bool write(uint8_t index, uint8_t state = 3); // Write Pin State By Index;
    static String reads(uint8_t index = 128);            // Retrun Externl RW String;
    static bool writes(char *extrw);                     // External RW  From String;
    static void reset();                                 // Reset  pins changed mask;
    static bool press(uint64_t value);                   // IR remote  key interface;
};