#ifndef UARTCOMM_H
#define UARTCOMM_H

#include "Config.h"
#include "SerialComm.h"
#include "EEPROMManager.h"
#include "SerialGuard.h" // Add this include
#include "CRC16.h" // 确保这个是你CRC校验的实现文件
// 定义回调函数类型，参数为解析完成的 UARTCommand
typedef void (*CommandCallback)(UARTCommand);

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


enum class ReceiveState {
    WAIT_FOR_HEADER,
    WAIT_FOR_SOURCE_ADDRESS,
    WAIT_FOR_TARGET_ADDRESS,
    WAIT_FOR_FUNCTION_CODE,
    WAIT_FOR_DATA_ADDRESS,
    WAIT_FOR_DATA, // 如果需要data才进入此状态
    WAIT_FOR_CRC,
    PROCESS_FRAME
};
class UARTComm {
public:
    UARTComm(SerialComm* serialComm, uint32_t baudRate,uint8_t rx_pin,uint8_t tx_pin, bool isCom1Serial = false);
    void init();
    bool update();  // 将返回类型从void改为bool
    void setCommandCallback(CommandCallback cb) { callback = cb; }
    //UARTCommand processIncomingFrame();   
    void sendHEXMessage(uint8_t sourceAddress, uint8_t targetAddress, uint8_t functionCode, uint8_t dataAddress, uint8_t data, bool hasData);
    void sendHEXfunction( uint8_t targetAddress, uint8_t dataAddress);
    void sendHEXheart( uint8_t targetAddress);
    void sendHEXRead( uint8_t targetAddress,  uint8_t dataAddress, uint8_t data);
    void sendHEXWrite( uint8_t targetAddress,  uint8_t dataAddress, uint8_t data);
    
    void sendCharMessage(const char* functionCode);
    void sendscreenCommand( uint8_t dataAddress);
  
   
    private: 
    SerialComm* serialComm;

    CommandCallback callback = nullptr; // 初始化为nullptr

    UARTCommand executeCommand(const CommFrame& frame);
    void sendFrame(const CommFrame& frame);
    UARTCommand handleReadCommand(const CommFrame& frame);
    UARTCommand handleWriteCommand(const CommFrame& frame);
    UARTCommand handleFunctionCommand(const CommFrame& frame);
    UARTCommand handleHeartbeat(const CommFrame& frame);
    uint16_t calculateCRC(const uint8_t* data,uint16_t length);
    bool verifyCRC(const uint8_t* data,uint16_t length, uint16_t receivedCRC);
    void sendResponse(const CommFrame& responseFrame);
    void sendCharFrame(const WindowCommand& cmd);  
    EEPROMManager eeprommanager;  
    uint32_t BaudRate; 
    bool isCom1Serial;  // 用于区分串口类型  
    uint8_t rx_pin;          // RX 引脚
    uint8_t tx_pin;          // TX 引脚 
    void clearSerialBuffer();
    UARTCommand processCommFrame();
    UARTCommand processWindowCommand();
    uint32_t timeout = 50;
    bool waitForBytes(uint16_t numBytes);

    ReceiveState receiveState = ReceiveState::WAIT_FOR_HEADER;
    CommFrame receivedFrame;
    uint16_t bytesReceived = 0;
    uint16_t expectedDataLength = 0;
    uint16_t receivedCRC = 0;
    UARTCommand cmd;
    bool frameRequiresData(uint8_t functionCode);
    uint16_t expectedFrameLength();
       
};

#endif
