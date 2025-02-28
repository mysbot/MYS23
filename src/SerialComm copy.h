#ifndef SERIALCOMM_H
#define SERIALCOMM_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "Config.h"

// 线程安全控制
class SerialMutex {
public:
    static SerialMutex& getInstance() {
        static SerialMutex instance;
        return instance;
    }

    void init() {
        mutex = xSemaphoreCreateMutex();
    }

    bool lock(TickType_t timeout = pdMS_TO_TICKS(10)) {
        return xSemaphoreTake(mutex, timeout) == pdTRUE;
    }

    void unlock() {
        if (mutex != NULL) {
            xSemaphoreGive(mutex);
        }
    }

    bool isLocked() {
        return uxSemaphoreGetCount(mutex) == 0;
    }

private:
    SerialMutex() : mutex(NULL) {}
    SemaphoreHandle_t mutex;
};

#define SERIAL_MUTEX SerialMutex::getInstance()

// 串口互斥锁包装
class SerialGuard {
public:
    SerialGuard() {
        SERIAL_MUTEX.lock();
    }
    
    ~SerialGuard() {
        SERIAL_MUTEX.unlock();
    }
};

// 串口接口定义
class ISerial {
public:
    virtual void begin(uint32_t baudRate, uint8_t rxPin, uint8_t txPin) = 0;
    virtual size_t write(uint8_t byte) = 0;
    virtual size_t write(const uint8_t* buffer, size_t size) = 0;
    virtual uint16_t available() = 0;
    virtual uint16_t read() = 0;
    virtual void print(const String &s) = 0;
    virtual void println(const String &s) = 0;
    virtual uint16_t peek() = 0;
    virtual ~ISerial() {}
};

// 硬件串口包装器
class HardwareSerialWrapper : public ISerial {
private:
    HardwareSerial& serial;
    int8_t rxPin;
    int8_t txPin;
public:
    HardwareSerialWrapper(HardwareSerial& serialPort) : serial(serialPort), rxPin(-1), txPin(-1) {}

    void begin(uint32_t baudRate, uint8_t rxPin = -1, uint8_t txPin = -1) override {
        this->rxPin = rxPin;
        this->txPin = txPin;

        if (rxPin != -1 && txPin != -1) {
            serial.begin(baudRate, SERIAL_8N1, rxPin, txPin, false, 256);
        } else {
            serial.begin(baudRate);
        }
    }

    size_t write(uint8_t byte) override {
        return serial.write(byte);
    }
    
    size_t write(const uint8_t* buffer, size_t size) override {
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

// 统一的串口通信类
class SerialComm {
private:
    ISerial* serial;
    
public:
    SerialComm(ISerial* serialInterface) : serial(serialInterface) {}
    
    void begin(uint32_t baudRate, uint8_t rxPin = -1, uint8_t txPin = -1) {
        SerialGuard guard;
        if (serial) {
            serial->begin(baudRate, rxPin, txPin);
        }
    }
    
    size_t write(uint8_t byte) {
        SerialGuard guard;
        return serial ? serial->write(byte) : 0;
    }
    
    size_t writeByte(const uint8_t* buffer, size_t size) {
        SerialGuard guard;
        return serial ? serial->write(buffer, size) : 0;
    }
    
    void writeChar(const char* str) {
        SerialGuard guard;
        if (serial) {
            while (*str) {
                serial->write(*str++);
            }
        }
    }
    
    uint16_t available() {
        return serial ? serial->available() : 0;
    }
    
    uint16_t read() {
        return serial ? serial->read() : 0;
    }
    
    void print(const String &s) {
        SerialGuard guard;
        if (serial) {
            serial->print(s);
        }
    }
    
    void println(const String &s) {
        SerialGuard guard;
        if (serial) {
            serial->println(s);
        }
    }
    
    uint16_t peek() {
        return serial ? serial->peek() : 0;
    }
};

#endif // SERIALCOMM_H
