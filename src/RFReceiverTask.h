#pragma once
#include "RF433any.h"
#include "RF433send.h"
#include "EEPROMManager.h"
#include <cstring>
#include <cstdarg>
#include <Arduino.h>
#include <queue>
extern uint16_t INIT_DATA_ADDRESS;

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
    RfSendEncoding getEncodingFromName(const char* name);
    // ← 新增公开方法，避免直接访问私有成员
  
    
private:
    const uint8_t rxPin;    
    RfSend* rfSender;
    Track* rfTrack;
    TaskHandle_t rfTaskHandle;
    volatile bool monitorMode = false;
   
    
    
    RFParams lastReceivedParams;
    std::queue<RFParams> sendQueue;
    void storeSignalParams(Decoder* pdec);
    void processReceivedData(Decoder* pdec);
    void setupRFSender(const RFParams& params);
};
