#ifndef UARTCOMM_H
#define UARTCOMM_H

#include "Config.h"
#include "SerialComm.h"
#include "EEPROMManager.h"
#include "CRC16.h"

// 定义回调函数类型，参数为解析完成的 UARTCommand
typedef void (*CommandCallback)(UARTCommand);

// 通信帧结构
struct CommFrame {
    uint8_t header;
    uint8_t sourceAddress;
    uint8_t targetAddress;
    uint8_t functionCode;
    uint8_t dataAddress;
    uint8_t data;
    uint16_t crc;
    bool hasData;

    CommFrame()
        : header(0), sourceAddress(0), targetAddress(0), functionCode(0),
          dataAddress(0), data(0), crc(0), hasData(false) {}

    CommFrame(uint8_t h, uint8_t s, uint8_t t, uint8_t f, uint8_t dA, uint8_t d, uint16_t c, bool hD)
        : header(h), sourceAddress(s), targetAddress(t), functionCode(f),
          dataAddress(dA), data(d), crc(c), hasData(hD) {}
};

// 接收状态枚举
enum class ReceiveState {
    WAIT_FOR_HEADER,
    WAIT_FOR_SOURCE_ADDRESS,
    WAIT_FOR_TARGET_ADDRESS,
    WAIT_FOR_FUNCTION_CODE,
    WAIT_FOR_DATA_ADDRESS,
    WAIT_FOR_DATA,
    WAIT_FOR_CRC,
    PROCESS_FRAME
};

// UART通信核心类
class UARTComm {
public:
    UARTComm(SerialComm* serialComm, uint32_t baudRate, uint8_t rx_pin, uint8_t tx_pin, bool isCom1Serial = false);
    void init();
    bool update();
    void setCommandCallback(CommandCallback cb) { callback = cb; }
    
    // 发送方法
    void sendHEXMessage(uint8_t sourceAddress, uint8_t targetAddress, uint8_t functionCode, uint8_t dataAddress, uint8_t data, bool hasData);
    void sendHEXfunction(uint8_t targetAddress, uint8_t dataAddress);
    void sendHEXheart(uint8_t targetAddress);
    void sendHEXRead(uint8_t targetAddress, uint8_t dataAddress, uint8_t data);
    void sendHEXWrite(uint8_t targetAddress, uint8_t dataAddress, uint8_t data);
    void sendCharMessage(const char* functionCode);
    void sendscreenCommand(uint8_t dataAddress);
  
private: 
    SerialComm* serialComm;
    CommandCallback callback = nullptr;
    EEPROMManager eeprommanager;
    uint32_t BaudRate;
    bool isCom1Serial;
    uint8_t rx_pin;
    uint8_t tx_pin;
    uint32_t timeout = 50;
    
    ReceiveState receiveState = ReceiveState::WAIT_FOR_HEADER;
    CommFrame receivedFrame;
    uint16_t bytesReceived = 0;
    uint16_t expectedDataLength = 0;
    uint16_t receivedCRC = 0;
    UARTCommand cmd;
    
    // 辅助方法
    UARTCommand executeCommand(const CommFrame& frame);
    void sendFrame(const CommFrame& frame);
    UARTCommand handleReadCommand(const CommFrame& frame);
    UARTCommand handleWriteCommand(const CommFrame& frame);
    UARTCommand handleFunctionCommand(const CommFrame& frame);
    UARTCommand handleHeartbeat(const CommFrame& frame);
    uint16_t calculateCRC(const uint8_t* data, uint16_t length);
    bool verifyCRC(const uint8_t* data, uint16_t length, uint16_t receivedCRC);
    void sendResponse(const CommFrame& responseFrame);
    void sendCharFrame(const WindowCommand& cmd);
    void clearSerialBuffer();
    UARTCommand processCommFrame();
    UARTCommand processWindowCommand();
    bool waitForBytes(uint16_t numBytes);
    bool frameRequiresData(uint8_t functionCode);
    uint16_t expectedFrameLength();
};

#endif
