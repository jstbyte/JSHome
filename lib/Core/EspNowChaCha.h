#include <Arduino.h>
#include <ChaCha.h>
#include <espnow.h>
#define DEFAULT_IV_SIZE 7

class EspNowChaCha : public ChaCha
{
protected:
    size_t _ivLen;

public:
    size_t ivLen();
    void genIV(byte *iv);
    void ivLen(size_t len);
    void encrypt(byte *output, byte *input, size_t len);
    void decrypt(byte *output, byte *input, size_t len);
};