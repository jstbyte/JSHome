#include "PassMan.h"

int _invalid_m[] = {1000};
int _invalid_d[] = {60};
int _success_m[] = {2000, 3000, 4000, 5000, 6000};
int _success_d[] = {16, 12, 6, 4, 2};
int _error_mld[] = {NOTE_C5, 0, NOTE_C5, 0, NOTE_C5, 0, NOTE_C5, 0};
int _error_dur[] = {4, 6, 4, 6, 4, 6, 4, 6};

PassMan::PassMan(String password, ezBuzzer *buzzer)
{
    _password = String(password);
    _buzzer = buzzer;
    _attempts = 0;
}

bool PassMan::isEmpty()
{
    return _passbuff.length() == 0;
}

String PassMan::buffer()
{
    return _passbuff;
}

uint8_t PassMan::attempts()
{
    return _attempts;
}

bool PassMan::load(String path, String password)
{

    if (LittleFS.exists(path))
    {
        File file = LittleFS.open(path, "r");
        String pass = file.readString();
        _password = pass;
        file.close();
        return true;
    }

    if (!password.isEmpty())
    {
        dump(path, password);
        return true;
    }

    return false;
}

void PassMan::dump(String path, String password)
{
    File file = LittleFS.open(path, "w");
    _password = password; // Register;
    file.write(password.c_str());
    file.close();
}

bool PassMan::hasSpace()
{
    return _passbuff.length() < _password.length();
}

bool PassMan::press(char key)
{
    if (hasSpace())
    {
        _buzzer->beep(50);
        _passbuff += key;
        return true;
    }
    _buzzer->playMelody(_invalid_m, _invalid_d, 1);
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
        _buzzer->beep(50);
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
        _buzzer->beep(50);
        _passbuff.remove(_passbuff.length() - 1, 1);
        return true;
    }
    _buzzer->playMelody(_invalid_m, _invalid_d, 1);
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
    if (_passbuff.isEmpty())
    {
        _buzzer->playMelody(_invalid_m, _invalid_d, 1);
        return false;
    }

    if (_password == _passbuff)
    {
        _buzzer->playMelody(_success_m, _success_d, 5);
        _attempts = 0;
        return true;
    }

    _buzzer->playMelody(_error_mld, _error_dur, 8);
    _attempts += 1;
    return false;
}
