#ifndef SERIAL_MANAGER_H
#define SERIAL_MANAGER_H

#include <Arduino.h>
#include "Config.h"
#include "UARTComm.h"

// 统一的串口管理器类
class SerialManager
{
public:
    SerialManager(address_Manager &AddManager);

    void serialManagerTask();
    void begin();

    // bool updateSerial2();
    //  设置回调
    void setSerial0Callback(CommandCallback cb);
    void setSerial1Callback(CommandCallback cb);
    // void setSerial2Callback(CommandCallback cb);
    //  串口0控制方法 (主控串口)
    void serial0Write(uint8_t dataAddress, uint8_t data);
    void serial0Read(uint8_t dataAddress);
    void serial0Heart();
    void serial0Function(Command index);

    // 串口1控制方法 (外部设备串口)
    void serial1SendCommand(Command index);

    // 串口2控制方法 (备用串口)
    // void serial2SendCommand(Command index);
private:
    address_Manager AddManager;
    // SerialManager() {} // 私有构造函数确保单例
    bool updateAll();
    bool updateSerial0();
    bool updateSerial1();
    // 硬件串口包装器
    HardwareSerialWrapper *serial0Wrapper = nullptr;
    HardwareSerialWrapper *serial1Wrapper = nullptr;
    // HardwareSerialWrapper* serial2Wrapper = nullptr;

    // 串口通信实例
    SerialComm *serialComm0 = nullptr;
    SerialComm *serialComm1 = nullptr;
    // SerialComm* serialComm2 = nullptr;
    //  UART通信控制器
    UARTComm *uartComm0 = nullptr;
    UARTComm *uartComm1 = nullptr;
    // UARTComm* uartComm2 = nullptr;
};

// #define SERIAL_MANAGER SerialManager::getInstance()

#endif // SERIAL_MANAGER_H
