#ifndef EEPROMMANAGER_H
#define EEPROMMANAGER_H

#include <Config.h>
#include <EEPROM.h>
#include <Arduino.h>



class EEPROMManager {
public:
    static void begin();    
    static bool readData(uint16_t startAddress, uint8_t* buffer, uint16_t length);
    static bool writeData(uint16_t startAddress, uint8_t* data, uint16_t length);
    
private: 
    static const uint16_t eepromSize = 4096; // 最大支持4096字节
};

#endif
