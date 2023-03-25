#include <Servo.h>
#include <TaskSchedulerDeclarations.h>
#define SERVO_AUTO_SLEEP

class EZServo : public Servo
{
private:
    uint8_t _pin;
    uint8_t _state;

public:
    EZServo(uint8_t pin) : _pin(pin), _state(255){};
    uint8_t current();
    void write(uint8_t angle);

#ifdef SERVO_AUTO_SLEEP
    Task _task;
    uint32_t _delay;
    void taskSetup(Scheduler &ts, uint32_t delay);
#endif
};