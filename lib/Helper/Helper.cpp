#include "Helper.h"

Task ledTask;

void ledBlink()
{
    uint8_t state = !digitalRead(LED_BUILTIN);
    digitalWrite(LED_BUILTIN, state);
    if (state)
        ledTask.delay(1995);
}

void mac2str(const uint8_t *macArr, char *macStr)
{
    snprintf(macStr, sizeof(char) * 20, "%02X:%02X:%02X:%02X:%02X:%02X",
             macArr[0], macArr[1], macArr[2], macArr[3], macArr[4], macArr[5]);
}

void str2mac(const char *macStr, uint8_t *macArr)
{
    sscanf(macStr, "%2hhX:%2hhX:%2hhX:%2hhX:%2hhX:%2hhX", &macArr[0], &macArr[1],
           &macArr[2], &macArr[3], &macArr[4], &macArr[5]);
}

String uuid(String prefix)
{
    return (prefix + String(random(0xffff), HEX));
}
