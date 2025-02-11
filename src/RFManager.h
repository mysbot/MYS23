#pragma once

#include "RF433any.h"
#include "RF433send.h"
#include <Arduino.h>
#include <queue>

class RFManager {
public:
    RFManager(uint8_t rxPin, uint8_t txPin);
    void begin();
    void startTasks();
    void setMonitorMode(bool enable) { monitorMode = enable; }
    bool isMonitoring() const { return monitorMode; }

private:
    const uint8_t rxPin;
    const uint8_t txPin;
    RfSend* rfSender;
    Track* rfTrack;
    TaskHandle_t rfTaskHandle;
    volatile bool monitorMode = false;
   
    // 存储接收到的RF参数
    struct RFParams {
        RfSendEncoding encoding;
        uint16_t timings[11]; // 存储RF时序参数
        uint8_t data[32];     // 存储接收到的数据
        uint8_t nBit;
        size_t dataLength;
    };
    
    RFParams lastReceivedParams;
    std::queue<RFParams> sendQueue;
    
    static void rfTask(void* parameter);
    void processReceivedData(Decoder* pdec);
    void setupRFSender(const RFParams& params);
    void sendStoredSignal();
    void storeSignalParams(Decoder* pdec);
    bool sendData(uint8_t* data, size_t length);
    const char *id_letter_to_encoding_name(char c);
    void serial_printf(const char* msg, ...);
    uint8_t stringToHex(const char* str, uint8_t* out);
    RfSendEncoding getEncodingFromName(const char* name);
};
