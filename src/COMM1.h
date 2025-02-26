#ifndef COMM1_H
#define COMM1_H

#include <Arduino.h>
#include "Config.h"
#include "UARTComm.h"
class Comm1 {
public:
    Comm1();
    void init(); 
    bool update();    
    
    void sendUart1Data(Command index);
     // 同样添加setCommandCallback方法
    void setCommandCallback(CommandCallback cb);
    //UARTCommand checkUart1Data();
    //void sendUart1Data(Command index);
private:
    //bool validateChecksum(const uint8_t* data, uint16_t length);
    //bool validateModbusCRC16(const uint8_t* data, uint16_t length);
    //void clearSerialBuffer();
    //EEPROMManager eeprommanager; 
    //uint16_t crc16_modbus(const uint8_t* data, size_t length);
    //uint16_t lrc_sum(const uint8_t* data, size_t length);
    //WindowCommand createWindowCommand(const char* function, char data);
    //ScreenCommand createScreenCommand(uint8_t targetaddress,uint8_t command);
    //void sendScreenCommand(const ScreenCommand& command);
    //void sendWindowCommand(const WindowCommand& command);
};

#endif // RS485_H
