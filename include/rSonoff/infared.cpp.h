#include <IRutils.h>
#include <IRrecv.h>
#include "shared.h"

#define IR_RECV_PIN 13 /* Can Be Change */

decode_results ir_result;
IRrecv irrecv(IR_RECV_PIN);

void handleInfared()
{
    if (irrecv.decode(&ir_result))
    {
        DEBUG_LOG("INFARED RECIVED : ");
        DEBUG_LOG_LN(ir_result.value);

        bool notify = true;
        switch (ir_result.value)
        {
        case IR_EQ:
            if (Global::wifiRetryTimeout == 0 && WIFI_RETRY_TIMEOUT > 1)
                reBoot(WIFI_RETRY_TIMEOUT);
            notify = false;
            break;
        case IR_POWER:
            if (Global::sonoff.read() == HIGH)
            {
                notify = false;
                break;
            }
            Global::sonoff.writes(HIGH);
            break;
        case IR_MUTE:
            if (Global::sonoff.read() == LOW)
            {
                notify = false;
                break;
            }
            Global::sonoff.writes(LOW);
            break;
        case IR_MODE:
            Global::sonoff.writes();
            break;
        case IR_1:
            Global::sonoff.write(0);
            break;
        case IR_2:
            Global::sonoff.write(1);
            break;
        case IR_3:
            Global::sonoff.write(2);
            break;
        case IR_4:
            Global::sonoff.write(3);
            break;
        default:
            notify = false;
            break;
        }
        irrecv.resume();
        // Notify Mqtt;
        if (notify)
        {
            Global::sonoff.start();
        }
    }
}