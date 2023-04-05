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
    _attempt = 0;
}

String PassMan::buffer()
{
    return _passbuff;
}

bool PassMan::loads(String path, String password)
{
    if (password.isEmpty())
    {
        if (!LittleFS.exists(path))
            return false;
        File file = LittleFS.open(path, "r");
        String pass = file.readString();
        file.close();

        if (pass.length() > 3)
        {
            _password = pass;
            return true;
        }
        return false;
    }

    auto file = LittleFS.open(path, "w");
    file.write(password.c_str());
    _password = password;
    file.close();
    return true;
}

bool PassMan::space()
{
    return _passbuff.length() < _password.length();
}

bool PassMan::press(char key)
{
    if (space())
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

    if (PASSMAN_MAX_ATTEMPT > 0 && _attempt >= PASSMAN_MAX_ATTEMPT)
    {
        if ((millis() - _timestamp) < PASSMAN_DELAY_MS)
        {
            _buzzer->beep(millis() - _timestamp);
            return false;
        }

        _timestamp = millis();
    }

    if (_password == _passbuff)
    {
        _buzzer->playMelody(_success_m, _success_d, 5);
        _attempt = 0;
        return true;
    }

    _buzzer->playMelody(_error_mld, _error_dur, 8);
    _attempt += 1;
    return false;
}
