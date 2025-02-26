#ifndef CRC16_H
#define CRC16_H

#include <Arduino.h>

class CRC16 {
public:
    static uint16_t crc16_modbus(const uint8_t* data, size_t length);
    static bool crc_check(const uint8_t* data, size_t length);
    static bool validateChecksum(const uint8_t* data, size_t length);
    static uint16_t lrc_sum(const uint8_t* data, size_t length);
};

#endif // CRC16_H
