#include "shared.h"
#include <IRrecv.h>
#include <IRutils.h>
#include <PassMan.h>

#define IR_RECV_PIN 13 /* Can Be Change */

IRrecv irrecv(IR_RECV_PIN);
decode_results ir_result;
PassMan passMan("0147");

void handleInfared()
{
    digitalWrite(12, HIGH);
    if (irrecv.decode(&ir_result))
    {
        if (ir_result.value == IR_POWER)
        {
            door.write(SERVO_DOOR_LOCK);
            irrecv.resume();
            return;
        }

        if (ir_result.value == IR_EQ && passMan.enter())
        {
            door.write(SERVO_DOOR_OPEN);
            irrecv.resume();
            return;
        }

        if (passMan.press(ir_result))
        {
            digitalWrite(12, LOW);
        }
        irrecv.resume();
    }
}