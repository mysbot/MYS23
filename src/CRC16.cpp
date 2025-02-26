#include "CRC16.h"

uint16_t CRC16::crc16_modbus(const uint8_t* data, size_t length) {
   uint16_t crc = 0xFFFF;

    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }

    return crc;
}

bool CRC16::crc_check(const uint8_t* data, size_t length) {
    if (length < 2) return false; // Not enough data for CRC check
    
    // Calculate CRC on all but the last 2 bytes
    uint16_t calculated_crc = crc16_modbus(data, length - 2);
    
    // Extract the provided CRC from the last 2 bytes
    uint16_t provided_crc = (data[length - 1] << 8) | data[length - 2];
    
    return provided_crc == calculated_crc;
}
bool CRC16::validateChecksum(const uint8_t* data, size_t length) {
    if (length < 2) return false; // Not enough data
    uint16_t provided_lrc = (data[length - 4]-'0')*100+(data[length - 3]-'0')*10+(data[length - 2]-'0');   
    uint16_t calculated_lrc=lrc_sum(data, length-4);   
    return provided_lrc == calculated_lrc;
}
uint16_t CRC16::lrc_sum(const uint8_t* data, size_t length) {
   uint16_t checksum = 0;
    for (uint16_t i = 1; i < length; ++i) {
        checksum += data[i]-'0';
    }
    checksum %= 100;
    //mySerial.print(" checksum:");
    //mySerial.println(checksum);
    return checksum;
}