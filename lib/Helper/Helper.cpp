#include "Helper.h"

void mac2str(const uint8_t *macArr, char *macStr)
{
    snprintf(macStr, sizeof(char) * 20, "%02x:%02x:%02x:%02x:%02x:%02x",
             macArr[0], macArr[1], macArr[2], macArr[3], macArr[4], macArr[5]);
}

void str2mac(const char *macStr, uint8_t *macArr)
{
    sscanf(macStr, "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx", &macArr[0], &macArr[1],
           &macArr[2], &macArr[3], &macArr[4], &macArr[5]);
}

String uuid(String prefix)
{
    return (prefix + String(random(0xffff), HEX));
}