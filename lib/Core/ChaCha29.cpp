#include "ChaCha29.h"

size_t EspNowChaCha::ivLen()
{
    return _ivLen;
}

void EspNowChaCha::genIV(byte *iv)
{
    for (size_t i = 0; i < _ivLen; i++)
    {
        iv[i] = (byte)random(0, 255);
    }
}

void EspNowChaCha::ivLen(size_t len)
{
    _ivLen = len;
}

void EspNowChaCha::encrypt(byte *output, byte *input, size_t len)
{
    genIV(output);
    setIV(output, _ivLen);
    ChaCha::encrypt(output + _ivLen, input, len);
}

void EspNowChaCha::decrypt(byte *output, byte *input, size_t len)
{
    setIV(input, _ivLen);
    ChaCha::decrypt(output, input + _ivLen, len);
}