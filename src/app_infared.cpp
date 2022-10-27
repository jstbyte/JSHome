#include <app_common.h>
#include "app_infared.h"

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
        case IR_POWER:
            digiOut.writes(HIGH);
            break;
        case IR_EQ:
            digiOut.writes(LOW);
            break;
        case IR_MODE:
            digiOut.writes();
            break;
        case IR_1:
            digiOut.write(0);
            break;
        case IR_2:
            digiOut.write(1);
            break;
        case IR_3:
            digiOut.write(2);
            break;
        case IR_4:
            digiOut.write(3);
            break;
        default:
            notify = false;
            break;
        }
        irrecv.resume();
        // Notify Mqtt;
        if (notify)
        {
            digiOut.start();
        }
    }
}