#include "RFTransmitterTask.h"

RFTransmitter::RFTransmitter( uint8_t txPin) 
    :  txPin(txPin),  rfTrack(nullptr), 
      rfTaskHandle(nullptr) {
}

void RFTransmitter::begin() {
  
    pinMode(txPin, OUTPUT);
   
}

// 发送任务，运行在 core2
void RFTransmitter::RFTransmitterTask(void* parameter) {
    RFTransmitter* transmitter = static_cast<RFTransmitter*>(parameter);
    
    for (;;) {
        if (transmitter->updateRFParamsFromEEPROM(32)) { // 固定32字节数据长度
            transmitter->sendStoredSignal();
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void RFTransmitter::setupRFSender(const RFParams& params) {
    if (rfSender) {
        delete rfSender;
    }
    
    rfSender = rfsend_builder(
        params.encoding,
        //RfSendEncoding::MANCHESTER,
        txPin,
        RFSEND_DEFAULT_CONVENTION,
        params.dataLength,
        //4,
        nullptr,
        params.timings[0],
        params.timings[1],
        params.timings[2],
        params.timings[3],
        params.timings[4],
        params.timings[5],
        params.timings[6],
        params.timings[7],
        params.timings[8],
        params.timings[9],
        params.nBit 
    );
}

void RFTransmitter::sendStoredSignal() {
    if (sendQueue.empty()) return;
    
    RFParams params = sendQueue.front();
    sendQueue.pop();
    
    setupRFSender(params);
    if (rfSender) {
        Serial.println("Sending stored RF signal...");
        rfSender->send(params.dataLength, params.data);
        Serial.println("RF signal sent.");
    }
}

// 新增对 EEPROM 数据更新的实现
bool RFTransmitter::updateRFParamsFromEEPROM(uint16_t length) {
    if (!EEPROMManager::readData(INIT_DATA_ADDRESS, lastReceivedParams.data, length)) {
        Serial.println("Failed to read EEPROM data");
        return false;
    }
    
    // 检查数据有效性
    bool valid = false;
    for (uint16_t i = 0; i < length; i++) {
        if (lastReceivedParams.data[i] != 0xFF) {
            valid = true;
            break;
        }
    }
    
    if (valid) {
        sendQueue.push(lastReceivedParams);
    }
    return valid;
}