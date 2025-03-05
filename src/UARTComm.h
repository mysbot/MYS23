#ifndef UARTCOMM_H
#define UARTCOMM_H

#include "Config.h"
#include "SerialComm.h"
#include "EEPROMManager.h"
#include "CRC16.h"

// 定义回调函数类型，参数为解析完成的 UARTCommand
typedef void (*CommandCallback)(UARTCommand);

// 接收缓冲区配置
#define UART_RX_BUFFER_SIZE 128

// 通信帧结构
struct CommFrame
{
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
enum class ReceiveState
{
    WAIT_FOR_HEADER,
    WAIT_FOR_SOURCE_ADDRESS,
    WAIT_FOR_TARGET_ADDRESS,
    WAIT_FOR_FUNCTION_CODE,
    WAIT_FOR_DATA_ADDRESS,
    WAIT_FOR_DATA,
    WAIT_FOR_CRC,
    PROCESS_FRAME,
    ERROR_RECOVERY // 新增错误恢复状态
};

// 串口收发独立封装
class UARTTransceiver
{
public:
    UARTTransceiver(SerialComm *serialComm);

    // 接收方法
    bool receiveBytes(uint8_t *buffer, uint16_t *bytesRead, uint16_t maxBytes);

    // 发送方法
    bool sendBytes(const uint8_t *buffer, uint16_t length);
    bool sendByte(uint8_t byte);
    void sendString(const char *str);

private:
   
    SerialComm *serialComm;
    uint32_t lastSendTime = 0;
    const unsigned long sendInterval = 10; // 减少发送间隔到10ms
};

// UART通信核心类
class UARTComm
{
public:
    UARTComm(SerialComm *serialComm, uint32_t baudRate, uint8_t rx_pin, uint8_t tx_pin, uint8_t comNumber , address_Manager &AddManager );
    void init();
    bool update();
    void setCommandCallback(CommandCallback cb) { callback = cb; }

    // 发送方法
    void sendHEXMessage(uint8_t sourceAddress, uint8_t targetAddress, uint8_t functionCode, uint8_t dataAddress, uint8_t data, bool hasData);
    void sendHEXfunction(uint8_t targetAddress, uint8_t dataAddress);
    void sendHEXheart(uint8_t targetAddress);
    void sendHEXRead(uint8_t targetAddress, uint8_t dataAddress, uint8_t data);
    void sendHEXWrite(uint8_t targetAddress, uint8_t dataAddress, uint8_t data);
    void sendCharMessage(const char *functionCode);
    void sendscreenCommand(uint8_t dataAddress);

private:
    address_Manager AddManager;
    SerialComm *serialComm;
    CommandCallback callback = nullptr;
    EEPROMManager eeprommanager;
    uint32_t BaudRate;
    uint8_t comNumber;
    uint8_t rx_pin;
    uint8_t tx_pin;
    uint32_t timeout = 30; // 减少超时时间
    unsigned long lastUpdateTime = 0;
    // 接收缓冲区
    uint8_t rxBuffer[UART_RX_BUFFER_SIZE];
    uint16_t rxHead = 0;
    uint16_t rxTail = 0;

    ReceiveState receiveState = ReceiveState::WAIT_FOR_HEADER;
    CommFrame receivedFrame;
    uint16_t bytesReceived = 0;
    uint16_t expectedDataLength = 0;
    uint16_t receivedCRC = 0;
    UARTCommand cmd;

    // 独立的收发器
    UARTTransceiver *transceiver = nullptr;

    // 辅助方法
    UARTCommand executeCommand(const CommFrame &frame);
    void sendFrame(const CommFrame &frame);
    UARTCommand handleReadCommand(const CommFrame &frame);
    UARTCommand handleWriteCommand(const CommFrame &frame);
    UARTCommand handleFunctionCommand(const CommFrame &frame);
    UARTCommand handleHeartbeat(const CommFrame &frame);
    uint16_t calculateCRC(const uint8_t *data, uint16_t length);
    bool verifyCRC(const uint8_t *data, uint16_t length, uint16_t receivedCRC);
    void sendResponse(const CommFrame &responseFrame);
    void sendCharFrame(const WindowCommand &cmd);
    void clearSerialBuffer();
    UARTCommand processCommFrame();
    UARTCommand processWindowCommand();
    bool waitForBytes(uint16_t numBytes);
    bool frameRequiresData(uint8_t functionCode);
    uint16_t expectedFrameLength();

    UARTCommand processWindowCommand(const WindowCommand &cmd);
    UARTCommand processWindowCommandMessage(const char *message, size_t length);
     // 新增：用于接收 WindowCommand 的状态和缓冲区
     static const uint8_t WINDOW_CMD_LEN = windowCommandSize;  // WindowCommand 固定长度
     bool inWindowCommandMode = false;
     char windowBuffer[WINDOW_CMD_LEN];
     uint8_t windowBufferIndex = 0;
    // 新增方法
    void bufferByte(uint8_t byte);
    uint8_t readBufferedByte();
    uint16_t availableBuffered();
    bool findFrameStart();
    void resetReceiveState();
    bool processReceivedByte(uint8_t byte);
    bool validateFrame();
};

#endif
