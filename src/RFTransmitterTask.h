#ifndef RF_TRANSMITTERTASK_H
#define RF_TRANSMITTERTASK_H
#pragma once
#include "RF433any.h"
#include "RF433send.h"
#include "RFReceiverTask.h"
#include "EEPROMManager.h"
#include <Arduino.h>
#include <queue>
#include "esp_task_wdt.h"
class RFTransmitter {
public:
    RFTransmitter(uint8_t txPin);
    void begin();
    static void RFTransmitterTask(void *parameter);  // 改为静态方法
    void sendStoredSignal();   
    bool readRFParamsFromEEPROM(RFParams& params);  // 新增方法
    
private:
    uint8_t txPin;
    RfSend* rfSender;
    Track* rfTrack;
    TaskHandle_t rfTaskHandle;
    RFParams lastReceivedParams;
    std::queue<RFParams> sendQueue;    
    
    void setupRFSender(const RFParams& params);
};
#endif