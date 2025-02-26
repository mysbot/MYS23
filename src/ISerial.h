#ifndef ISERIAL_H
#define ISERIAL_H

class ISerial {
public:
    virtual void begin(uint32_t baudRate, uint8_t rxPin, uint8_t txPin) = 0;
    virtual size_t write(uint8_t byte) = 0;
    virtual size_t write(const uint8_t* buffer, size_t size) = 0;  // 新增写入数组的函数   
    virtual uint16_t available() = 0;
    virtual uint16_t read() = 0;
    virtual void print(const String &s) = 0;
    virtual void println(const String &s) = 0;
    virtual uint16_t peek() = 0;
    
    virtual ~ISerial() {}
};

#endif
