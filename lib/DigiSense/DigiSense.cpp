#include "DigiSense.h"

uint8_t DigiSense::_pin;

uint8_t DigiSense::pin()
{
    return DigiSense::_pin;
}

void DigiSense::handler()
{
    String stat = String(digitalRead(DigiSense::_pin));
    (PubSubX::Get()).pub("res/dsense", stat);
}

DigiSense::DigiSense(uint8_t pin)
{
    DigiSense::_pin = pin;
}

void DigiSense::begin(Scheduler &scheduler)
{
    set(0, 1, DigiSense::handler);
    scheduler.addTask(*this);
}