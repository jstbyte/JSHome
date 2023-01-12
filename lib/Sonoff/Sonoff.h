#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#include "Helper.h"
#define ENABLE_SONOFF_EVENT
#ifdef ENABLE_SONOFF_EVENT // Check;
#include <TaskSchedulerDeclarations.h>
#endif

/* ::::RULES DEFINED BY AUTHOR::::;
 * External Read & Write as String;
 * RW:   `[PIN INDEX]:[PIN STATE]`;
 * Multiple RW:Semicolon Separated;
 * INDEX 255 is a special opp Code;
 * INDEX 128  is equal to all pins;
 * STATE >1 mean pin flip | toggle;
 */

class Sonoff
{ /* Enable Event  :  ENABLE_SONOFF_EVENT */
protected:
    static uint8_t _count;    // Pin Counter;
    static uint8_t _cmask;    // Changed Msk;
    static uint8_t _pnums[8]; // Pin Numbers;

public:
    static uint8_t count();                                 // Get available pins count;
    static uint8_t cmask();                                 // Get  changed masked byte;
    static uint8_t *pins();                                 // Get IO pin numbers array;
    static uint8_t begin(String path);                      // Load & Init Pins from FS;
    static uint8_t read(uint8_t index);                     // Read pin state  by index;
    static uint8_t write(uint8_t index, uint8_t state = 3); // Write Pin State By Index;
    static String reads(uint8_t index = 128);               // Retrun Externl RW String;
    static uint8_t writes(char *extrw);                     // External RW  From String;
    static uint8_t press(uint64_t value);                   // IR remote  key interface;
    static void reset();                                    // Reset  pins changed mask;

#ifdef ENABLE_SONOFF_EVENT
    static Task task;
    static uint32_t delay;
    static void taskSetup(Scheduler &ts, TaskCallback cb,
                          uint32_t delay, bool check = false);
#endif
};