#include <app_common.h>
#include "app_infared.h"
#include "app_mqtt.h"

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
            if (wifiRetryTimeout == 0 && WIFI_RETRY_TIMEOUT > 1)
                reBoot(WIFI_RETRY_TIMEOUT);
            break;
        case IR_POWER:
            if (digiOut.read() == HIGH)
            {
                notify = false;
                break;
            }
            digiOut.writes(HIGH);
            break;
        case IR_MUTE:
            if (digiOut.read() == LOW)
            {
                notify = false;
                break;
            }
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