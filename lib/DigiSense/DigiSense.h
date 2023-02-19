#pragma once
#include <Arduino.h>
#include <PubSub.h>
#include <TaskSchedulerDeclarations.h>

class DigiSense : public Task
{
private:
    static uint8_t _pin;

public:
    uint8_t pin();
    static void handler();
    DigiSense(uint8_t pin);
    void begin(Scheduler &scheduler);
};