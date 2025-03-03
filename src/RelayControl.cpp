#include "RelayControl.h"

RelayControl::RelayControl(uint8_t relayPin1, uint8_t relayPin2)
    : _relayPin1(relayPin1), _relayPin2(relayPin2), _startMillis1(0), _startMillis2(0), _delayDuration1(0), _delayDuration2(0), _currentCommand1(Command::SCREEN_STOP), _currentCommand2(Command::SCREEN_STOP), _relayActive1(false), _relayActive2(false) {
    pinMode(_relayPin1, OUTPUT);
    pinMode(_relayPin2, OUTPUT);
    digitalWrite(_relayPin1, LOW);
    digitalWrite(_relayPin2, LOW);
}

void RelayControl::controlRelay(Command command) {
    switch (command) {
        case Command::CASEMENT_STOP:
        case Command::CASEMENT_STOP_ALT:
           {
            _currentCommand1 = command;
            _relayActive1 = true;
            _startMillis1 = millis();
            digitalWrite(_relayPin1, HIGH);
            _delayDuration1 = OPEN_DELAY_TIME;

             _currentCommand2 = command;
            _relayActive2 = true;
            _startMillis2 = millis();
            digitalWrite(_relayPin2, HIGH);
            _delayDuration2 = OPEN_DELAY_TIME;
           }
            break;

        case Command::SCREEN_UP:
            _currentCommand1 = command;
            _relayActive1 = true;
            _startMillis1 = millis();
            digitalWrite(_relayPin1, HIGH);
            _delayDuration1 = OPEN_DELAY_TIME;
            break;
        case Command::SCREEN_DOWN:
            _currentCommand1 = command;
            digitalWrite(_relayPin1, HIGH);
            _delayDuration1 = 0;
            _relayActive1 = false;
            break;
        case Command::SCREEN_STOP:
            _currentCommand1 = command;
            digitalWrite(_relayPin1, LOW);
            _delayDuration1 = 0;
            _relayActive1 = false;
            break;
        case Command::WINDOW_UP:
            _currentCommand2 = command;
            _relayActive2 = true;
            _startMillis2 = millis();
            digitalWrite(_relayPin2, HIGH);
            _delayDuration2 = OPEN_DELAY_TIME;
            break;
        case Command::WINDOW_DOWN:
            _currentCommand2 = command;
            digitalWrite(_relayPin2, HIGH);
            _delayDuration2 = 0;
            _relayActive2 = false;
            break;
        case Command::WINDOW_STOP:
            _currentCommand2 = command;
            digitalWrite(_relayPin2, LOW);
            _delayDuration2 = 0;
            _relayActive2 = false;
            break;
        default:
            _relayActive1 = false;
            _relayActive2 = false;
            
            break;
    }
}

void RelayControl::update() {
    uint32_t currentMillis = millis();

    // 更新引脚1
    if (_relayActive1 && (currentMillis - _startMillis1 >= _delayDuration1)) {
        if (_currentCommand1 == Command::SCREEN_UP ||
            _currentCommand1 == Command::CASEMENT_STOP ||
            _currentCommand1 == Command::CASEMENT_STOP_ALT) {
            digitalWrite(_relayPin1, LOW);
        }
        _relayActive1 = false;
    }

    // 更新引脚2
    if (_relayActive2 && (currentMillis - _startMillis2 >= _delayDuration2)) {
        if (_currentCommand2 == Command::WINDOW_UP ||
            _currentCommand2 == Command::CASEMENT_STOP ||
            _currentCommand2 == Command::CASEMENT_STOP_ALT) {
            digitalWrite(_relayPin2, LOW);
        }
        _relayActive2 = false;
    }
}
