#include "PassMan.h"

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
        _passbuff += key;
        return true;
    }
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
        _passbuff.remove(_passbuff.length() - 1, 1);
        return true;
    }
    return false;
}

bool PassMan::reset()
{
    if (_passbuff.isEmpty())
        return false;

    _passbuff.clear();
    return true;
}

bool PassMan::enter()
{
    return (_password == _passbuff);
}
