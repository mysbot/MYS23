#ifndef COMM0_H
#define COMM0_H

#include <Arduino.h>
#include "Config.h"
#include "UARTComm.h"
class Comm0 {
public:
    Comm0();
    void init(); 
    bool update();  
   
    // 新增的方法，用于注册回调
    void setCommandCallback(CommandCallback cb);
    void Uart0Write(uint8_t dataAddress, uint8_t data);
    void Uart0Read(uint8_t dataAddress);
    void Uart0Heart();
    void Uart0Function(Command index);
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


#endif // INNERCOM_H