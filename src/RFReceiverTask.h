#ifndef RF_RECEIVERTASK_H
#define RF_RECEIVERTASK_H
#pragma once
#include "RF433any.h"
#include "RF433send.h" 
#include "RFStorageManager.h"
#include <cstring>
#include <cstdarg>
#include <Arduino.h>
#include <queue>
#include "Config.h"


struct RFParams {
    RfSendEncoding encoding;
    uint16_t timings[11];
    uint8_t data[32];
    uint8_t nBit;
    size_t dataLength;
};
class RFReceiver{
public:
    
    RFReceiver(uint8_t rxPin);
    void begin();
    void RFReceiverTask(void* parameter);
    void sendStoredSignal();
    Track* getRFTrack() { return rfTrack; }
   
    const char *id_letter_to_encoding_name(char c);
    uint8_t stringToHex(const char* str, uint8_t* out);
   
    // ← 新增公开方法，避免直接访问私有成员
  
    
private:
    const uint8_t rxPin;   
    
    RfSend *rfSend;
    Track* rfTrack;

    TaskHandle_t rfTaskHandle;
    volatile bool monitorMode = false;
    RFStorageManager rfStorageManager = RFStorageManager(FIRST_ADDRESS_FOR_RF_SIGNAL);
    RFCommand receiveCommand; // Command queue
    RfSendEncoding getEncodingFromName(const char *name);
    RFParams lastReceivedParams;
    std::queue<RFParams> sendQueue;
    void processSignalParams(Decoder* pdec);
    void processReceivedData(Decoder* pdec);
    void setupRFSender(const RFParams& params);
    void checkAndExecuteCommand(uint8_t *data);
    uint8_t checkRFLastByte(uint8_t lastByte, uint8_t commandlastByte);
    uint8_t checkRFLastByteHans(uint8_t lastByte, uint8_t commandlastByte);
    uint8_t checkRFLastByteHopo(uint8_t lastByte);
    void executeCommand(uint16_t group, Command screenCmd, Command windowCmd, const char *action);
};
#endif