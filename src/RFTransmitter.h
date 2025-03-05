#ifndef RF_TRANSMITTER_H
#define RF_TRANSMITTER_H

#include <Arduino.h>
#include "RF433send.h"
// #include "RFReceiver.h"
#include "Config.h"
#include "setRFparameter.h"
#include "RFStorageManager.h"

#define QUEUE_SIZE 4 // Define the size of the queue

class RFTransmitter
{
public:
    RFTransmitter(uint16_t outputPin, address_Manager &ADDmanager);
    void setup();
    void sendCode(Command index, uint8_t *data, uint8_t group);
    void update(); // 新增：用于处理非阻塞发送
    void rfsend_build(uint8_t rfWorkMode);

private:
    uint16_t outputPin;
    address_Manager AddManager;
    RFStorageManager rfStorageManager = RFStorageManager(FIRST_ADDRESS_FOR_RF_SIGNAL);
    RfSend *tx_whatever;
    void prepareSendDataHans(uint8_t *send_data, uint8_t *data, uint16_t temp);
    void prepareSendDataHopo(uint8_t *send_data, uint8_t *data, uint16_t temp);
    void prepareSendDataGu(uint8_t *send_data, uint8_t *data, uint16_t temp);
    // Add other necessary member variables and methods
    // 发送状态管理
    bool isSending = false;
    uint32_t lastSendTime = 0;
    uint32_t sendInterval = 500; // 1 秒间隔
    uint8_t currentSendData[RF_NUM_DEFAULT];
    uint16_t sendStep = 0;              // 0: 第一次发送, 1: 第二次发送
    RFCommand commandQueue[QUEUE_SIZE]; // Command queue
    uint16_t queueHead = 0;
    uint16_t queueTail = 0;
    bool isQueueEmpty() { return queueHead == queueTail; }
    bool isQueueFull() { return (queueTail + 1) % QUEUE_SIZE == queueHead; }
    void enqueueCommand(RFCommand command);
    RFCommand dequeueCommand();
};
extern RFTransmitter transmitter;
#endif
