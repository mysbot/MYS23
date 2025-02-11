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
    
    // 配置任务看门狗
    esp_task_wdt_init(5, true); // 5秒超时，允许发生紧急重启
    esp_task_wdt_add(NULL);
    
    RFParams params;
    for (;;) {
        // 喂狗
        esp_task_wdt_reset();
        
        if (transmitter->readRFParamsFromEEPROM(params)) {
            transmitter->setupRFSender(params);
            if (transmitter->rfSender) {
                Serial.println("Sending RF signal...");
                transmitter->rfSender->send(params.dataLength, params.data);
                Serial.println("RF signal sent successfully");
            }
        }
        
        // 添加适当的延时
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

bool RFTransmitter::readRFParamsFromEEPROM(RFParams& params) {
    // 读取整个RFParams结构体
    if (!EEPROMManager::readData(INIT_DATA_ADDRESS, (uint8_t*)&params, sizeof(RFParams))) {
        Serial.println("Failed to read EEPROM data");
        return false;
    }
    
    // 验证数据有效性
    if (params.dataLength > 32 || params.nBit > 255) {
        Serial.println("Invalid RF parameters too long");
        return false;
    }
    
    // 打印调试信息
    Serial.printf("Read RF Parameters from EEPROM:\n");
    Serial.printf("mod:%d\n", params.encoding);
    Serial.printf("Init seq: %d\n", params.timings[0]);
    // ...其他参数打印...
    Serial.printf("n bits: %d\n", params.nBit);
    Serial.printf("Data (%d bytes): ", params.dataLength);
    for (size_t i = 0; i < params.dataLength; i++) {
        Serial.printf("%02X ", params.data[i]);
    }
    Serial.println();
    
    // 验证数据非空
    bool hasValidData = false;
    for (size_t i = 0; i < params.dataLength; i++) {
        if (params.data[i] != 0xFF) {
            hasValidData = true;
            break;
        }
    }
    
    return hasValidData;
}

void RFTransmitter::setupRFSender(const RFParams& params) {
    if (rfSender) {
        delete rfSender;
        rfSender = nullptr;
    }
    try {
        rfSender = rfsend_builder(
            params.encoding,
            txPin,
            RFSEND_DEFAULT_CONVENTION,
            params.dataLength,
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
        if (!rfSender) {
            Serial.println("Failed to create RF sender");
            return;
        }
        // 打印发送参数
        Serial.printf("send RF Parameters:\n");
        Serial.printf("mod:%d\n", params.encoding);
        Serial.printf("Init seq: %d\n", params.timings[0]);
        Serial.printf("first_low: %d\n", params.timings[1]);
        Serial.printf("first_high: %d\n", params.timings[2]);
        Serial.printf("first_low_ignored: %d\n", params.timings[3]);
        Serial.printf("low_short: %d\n", params.timings[4]);
        Serial.printf("low_long: %d\n", params.timings[5]);
        Serial.printf("high_short: %d\n", params.timings[6]);
        Serial.printf("high_long: %d\n", params.timings[7]);
        Serial.printf("last_low: %d\n", params.timings[8]);
        Serial.printf("seq: %d\n", params.timings[9]);
        Serial.printf("n bits: %d\n", params.nBit);
        Serial.printf("**%d** Data: \n", params.dataLength);
        for (size_t i = 0; i < params.dataLength; i++) {
            Serial.printf("%02X ", params.data[i]);
        }
        Serial.println();
    } 
    catch (...) {
        Serial.println("Exception during RF sender creation");
        if (rfSender) {
            delete rfSender;
            rfSender = nullptr;
        }
    }
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
    // 读取完整的RFParams结构体
    if (!EEPROMManager::readData(INIT_DATA_ADDRESS, (uint8_t*)&lastReceivedParams, sizeof(RFParams))) {
        Serial.println("Failed to read EEPROM data in update");
        return false;
    }
    
    // 验证数据有效性
    bool valid = false;
   
        if (lastReceivedParams.encoding == RfSendEncoding::TRIBIT) {
            valid = true;
            
        }
   
    
    if (valid) {
        Serial.println("Valid data found, adding to queue");
        sendQueue.push(lastReceivedParams);
    }
    
    return valid;
}