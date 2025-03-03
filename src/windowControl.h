#ifndef WINDOW_CONTROL_H
#define WINDOW_CONTROL_H


#include "Config.h"
#include "serialManager.h"
#include "EEPROMManager.h"
#include "RFTransmitter.h"
#include "RelayControl.h"
//#include "RFReceiver.h"

class WindowControl {
public:
    WindowControl();
    void setup();
    void controlBasedOnWindowType(ControlType controltype,Command command);
    void ControlUpdate();
   

private:
    RelayControl relayControl;    // Relay control object
    RFTransmitter rfTransmitter;  // RF transmitter object   
    EEPROMManager eepromManager;
    bool   mutualActive = 0;
   uint32_t     mutualstartMillis ;
   uint32_t     mutualdelayDuration = OPEN_DELAY_TIME;
    
    bool isWindowCommand(Command command) ;

    void controlRelay(uint16_t windowType, Command command);
    void controlAutoLiftRelay(Command command);
    void controlAutoSlidingDoorRelay(Command command);
    void controlCasementWindowRelay(Command command);
    void controlSkylightWindowRelay(Command command);
    void controlCurtainRelay(Command command);

    void controlTransmitter(uint16_t windowType, Command command);   
    void controlAutoLiftTransmitter(Command command,uint16_t bufferIndex);
    void controlAutoSlidingDoorTransmitter(Command command,uint16_t bufferIndex);
    void controlCasementWindowTransmitter(Command command,uint16_t bufferIndex);
    void controlSkylightWindowTransmitter(Command command,uint16_t bufferIndex);
    void controlCurtainTransmitter(Command command,uint16_t bufferIndex);

    void controlCom(uint16_t windowType, Command command);
    void controlAutoLiftCom(Command command);
    void controlAutoSlidingDoorCom(Command command);
    void controlCasementWindowCom(Command command);
    void controlSkylightWindowCom(Command command);
    void controlCurtainCom(Command command);
    
    void isMutualControl();
    void updateAddress(uint8_t address, uint8_t value);
    
};


#endif