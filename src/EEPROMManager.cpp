#include "EEPROMManager.h"

void  EEPROMManager::EEPROMInitialize(){
  EEPROM.begin(eepromSize);
}
bool EEPROMManager::readData(uint16_t startAddress, uint8_t* buffer, uint16_t length) {
  
     // 检查地址范围：startAddress + length 应该不超过 eepromSize
    if (startAddress + length > eepromSize) {
        Serial.println(F("Address out of range"));
        return false; // 超出范围，返回失败
    }
    bool allFF = true; // Start assuming all are FF
    for (uint16_t i = 0; i < length; i++) {   
    buffer[i] = EEPROM.read(startAddress + i);   
       // mySerial.print(buffer[i], HEX);
        //mySerial.print(" "); 
        
        if (buffer[i] != INIT_DATA) {
            allFF = false; // Found a non-FF uint8_t, hence not all are FF
            //ledController.TurnOn();            
        }
    }
   // mySerial.println(" "); 
    if (allFF) {
        //ledController.TurnDown(); // Turn down the LED only if all bytes are FF 
        //mySerial.println(F(" data is load failed,please retry ."));
    }else{        
        //mySerial.println(F(" data load succesed."));        
        //ledController.flash(100, 3);
    }

    return !allFF; // Return true if not all bytes were FF (data is valid), false otherwise
}

bool EEPROMManager::writeData(uint16_t startAddress, uint8_t* data, uint16_t length) {
  // 检查地址范围
    if ( startAddress + length > eepromSize) {
        return false; // 超出范围，返回失败
    }
    //mySerial.println(F(" save debug 1."));

    //noInterrupts();

    //mySerial.println(F(" save debug 2."));
    for (uint16_t i = 0; i < length; i++) {       
        EEPROM.write(startAddress + i, data[i]);
    }
    //mySerial.println(F(" save debug 3."));    
       
     // 确保 EEPROM 被正确提交
    bool result = EEPROM.commit();
    if (result) {
       // mySerial.println(F("Data save successful."));
    } else {
        //mySerial.println(F("Data save failed, change the EEPROM address."));
    }

    //interrupts();

    return result; 

    
}

