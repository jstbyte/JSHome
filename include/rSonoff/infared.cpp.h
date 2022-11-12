#include "shared.h"
#include <IRutils.h>
#include <IRrecv.h>

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
            if (ConnMan::data()->timeout == 0 && WIFI_RETRY_TIMEOUT > 1)
                ConnMan::reboot(WIFI_RETRY_TIMEOUT, Sonoffe::pins(), Sonoffe::count());
            notify = false;
            break;
        case IR_POWER:
            if (Sonoffe::read() == HIGH)
            {
                notify = false;
                break;
            }
            Sonoffe::writes(HIGH);
            break;
        case IR_MUTE:
            if (Sonoffe::read() == LOW)
            {
                notify = false;
                break;
            }
            Sonoffe::writes(LOW);
            break;
        case IR_MODE:
            Sonoffe::writes();
            break;
        case IR_1:
            Sonoffe::write(0);
            break;
        case IR_2:
            Sonoffe::write(1);
            break;
        case IR_3:
            Sonoffe::write(2);
            break;
        case IR_4:
            Sonoffe::write(3);
            break;
        default:
            notify = false;
            break;
        }
        irrecv.resume();
        // Notify Mqtt;
        if (notify)
        {
            Sonoffe::event.start();
        }
    }
}