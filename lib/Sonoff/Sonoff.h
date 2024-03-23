#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#include "Helper.h"
#define MASK mask = true
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
    uint8_t _count;    // Pin Counter;
    uint8_t _cmask;    // Changed Msk;
    uint8_t _pnums[8]; // Pin Numbers;

#define SNOF_WRITE_ARG uint8_t index, uint8_t state = 3, bool MASK
public:
    uint8_t count();                      // Get available pins count;
    uint8_t cmask();                      // Get  changed masked byte;
    uint8_t *pins();                      // Get IO pin numbers array;
    void init(String rw);                 // Init Pins from RW String;
    uint8_t begin(String path);           // Load & Init Pins from FS;
    uint8_t read(uint8_t index);          // Read pin state  by index;
    uint8_t write(SNOF_WRITE_ARG);        // Write Pin State By Index;
    String reads(uint8_t index = 128);    // Retrun Externl RW String;
    uint8_t writes(String extrw);         // External RW  From String;
    uint8_t press(uint64_t value);        // IR remote  key interface;
    void reset(uint8_t index = 255);      // Reset  pins changed mask;
    Sonoff() : _count(0), _cmask(0){};    // Default constructer call;
    String readJson(uint8_t index = 128); // Returns {pin, state:0|1};
    uint8_t writeJson(String data);       // Data{pin,state:[0|1|>1]};

#ifdef ENABLE_SONOFF_EVENT
    Task _task;
    uint32_t _delay;
    void taskSetup(Scheduler &ts, TaskCallback cb,
                   uint32_t delay, bool check = false);
#endif
};

class Snf : public Sonoff
{
public:
    static Snf &Get();
    static void fire();
    static bool enabled();

protected:
    static bool _enabled;

private:
    ~Snf(){};
    Snf() : Sonoff(){};
    Snf(const Snf &) = delete;
    Snf &operator=(const Snf &) = delete;
};
