#ifndef SERIAL_COMM_H
#define SERIAL_COMM_H

#include "Config.h"
#include "ISerial.h"

class SerialComm {
public:
    SerialComm(ISerial* serialPort) : serial(serialPort) {}

     void begin(uint32_t baudRate, uint8_t rxPin, uint8_t txPin) {
        serial->begin(baudRate, rxPin, txPin);
    }

    size_t write(uint8_t uint8_t) {
        return serial->write(uint8_t);
    }

   size_t writeChar(const char* str) {
        return serial->write(reinterpret_cast<const uint8_t*>(str), strlen(str));
    }

    uint16_t available() {
        return serial->available();
    }

    uint16_t read() {
        return serial->read();
    }

    void print(const String &s) {
        serial->print(s);
    }

    void println(const String &s) {
        serial->println(s);
    }

    uint16_t peek() {
        return serial->peek();
    }
    
    void writeByte(const uint8_t* buf, size_t len) {
        for (size_t i = 0; i < len; i++) {
            serial->write(buf[i]);
        }
    }

    void readByte(uint8_t* buf, size_t len) {
        for (size_t i = 0; i < len; i++) {
            buf[i] = serial->read();
        }
    }

private:
    ISerial* serial;  // 使用ISerial接口指针来指代具体的串口
};

#endif
