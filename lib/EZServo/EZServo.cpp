#include "EZServo.h"

uint8_t EZServo::current()
{
    return _state;
}

#ifdef SERVO_AUTO_SLEEP
EZServo *_ezservo = nullptr;

void ezservo_callback()
{
    _ezservo->detach();
}

void EZServo::taskSetup(Scheduler &ts, uint32_t delay)
{
    _ezservo = this;
    _task.set(0, 1, ezservo_callback);
    ts.addTask(_task);
    _delay = delay;
}

#endif

void EZServo::write(uint8_t angle)
{
    attach(_pin, 400, 2600);
    Servo::write(angle);
    _state = angle;

#ifdef SERVO_AUTO_SLEEP
    if (_ezservo != nullptr)
        _task.restartDelayed(_delay);
#endif
}
