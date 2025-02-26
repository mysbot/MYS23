#ifndef HARDWARE_SERIAL_WRAPPER_H
#define HARDWARE_SERIAL_WRAPPER_H

#include "ISerial.h"
#include <HardwareSerial.h>  // 确保包含正确的库

class HardwareSerialWrapper : public ISerial {
private:
    HardwareSerial& serial;  // 引用硬件串口实例
    int8_t rxPin;  // RX 引脚编号
    int8_t txPin;  // TX 引脚编号
public:
    HardwareSerialWrapper(HardwareSerial& serialPort) : serial(serialPort), rxPin(-1), txPin(-1) {}

   void begin(uint32_t baudRate, uint8_t rxPin = -1, uint8_t txPin = -1) override {
        this->rxPin = rxPin;
        this->txPin = txPin;

        if (rxPin != -1 && txPin != -1) {
            // 如果指定了引脚，则使用指定的 RX 和 TX 引脚进行初始化
            serial.begin(baudRate, SERIAL_8N1, rxPin, txPin,false,256);
        } else {
            // 如果没有指定引脚，则使用默认引脚
            serial.begin(baudRate);
        }
    }

    size_t write(uint8_t byte) override {
        return serial.write(byte);
    }
    size_t write(const uint8_t* buffer, size_t size) override {  // 实现写入数组的函数
        return serial.write(buffer, size);
    }
    uint16_t available() override {
        return serial.available();
    }

    uint16_t read() override {
        return serial.read();
    }

    void print(const String &s) override {
        serial.print(s);
    }

    void println(const String &s) override {
        serial.println(s);
    }

    uint16_t peek() override {
        return serial.peek();
    }
   
};

#endif
