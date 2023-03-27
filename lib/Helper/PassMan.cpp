#include "PassMan.h"

void playSciFiSuccessSound(uint8_t pin)
{
    tone(pin, 800, 50);   // First tone
    delay(100);           // Pause between tones
    tone(pin, 1200, 75);  // Second tone
    delay(100);           // Pause between tones
    tone(pin, 1600, 100); // Third tone
    delay(100);           // Pause between tones
}

void playSciFiErrorSound(uint8_t pin)
{
    tone(pin, 700, 100); // First tone
    delay(100);          // Pause between tones
    tone(pin, 500, 125); // Second tone
    delay(100);          // Pause between tones
    tone(pin, 300, 150); // Third tone
    delay(100);          // Pause between tones
    tone(pin, 100, 175); // Fourth tone
    delay(100);          // Pause between tones
}

String PassMan::buffer()
{
    return _passbuff;
}

bool PassMan::space()
{
    return _passbuff.length() < _password.length();
}

bool PassMan::press(char key)
{
    if (space())
    {
        tone(_beeppin, 2000, 50);
        _passbuff += key;
        return true;
    }
    tone(_beeppin, 1000, 30);
    return false;
}

bool PassMan::press(decode_results key)
{
    switch (key.value)
    {
    case IR_USD:
        clear();
        return true;
    case IR_RPT:
        reset();
        return true;
    case IR_0:
        return press('0');
    case IR_1:
        return press('1');
    case IR_2:
        return press('2');
    case IR_3:
        return press('3');
    case IR_4:
        return press('4');
    case IR_5:
        return press('5');
    case IR_6:
        return press('6');
    case IR_7:
        return press('7');
    case IR_8:
        return press('8');
    case IR_9:
        return press('9');
    default:
        return false;
    }
}

bool PassMan::clear()
{
    if (_passbuff.length() > 0)
    {
        tone(_beeppin, 2000, 50);
        _passbuff.remove(_passbuff.length() - 1, 1);
        return true;
    }
    tone(_beeppin, 1000, 30);
    return false;
}

bool PassMan::reset()
{
    tone(_beeppin, 2000, 50);
    if (_passbuff.isEmpty())
        return false;

    _passbuff.clear();
    return true;
}

bool PassMan::enter()
{
    if (_password == _passbuff)
    {
        playSciFiSuccessSound(_beeppin);
        return true;
    }
    playSciFiErrorSound(_beeppin);
    return false;
}
