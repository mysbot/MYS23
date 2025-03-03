// RelayControl.h
#ifndef RELAYCONTROL_H
#define RELAYCONTROL_H

#include <Arduino.h>
#include "Config.h"

class RelayControl {
public:
    RelayControl(uint8_t relayPin1, uint8_t relayPin2);
    void controlRelay(Command command);
    void update();

private:
    uint8_t _relayPin1;
    uint8_t _relayPin2;
   uint32_t _startMillis2;
   uint32_t _startMillis1;
   uint32_t _delayDuration2;
   uint32_t _delayDuration1;
    Command _currentCommand2;  // Change type to Command
    Command _currentCommand1;  // Change type to Command
    bool _relayActive2;
    bool _relayActive1;
};

#endif // RELAYCONTROL_H
