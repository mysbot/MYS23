#include "RFTransmitterTask.h"

RFTransmitter::RFTransmitter(uint8_t txPin)
    : txPin(txPin), rfTrack(nullptr),
      rfTaskHandle(nullptr)
{
}

void RFTransmitter::begin()
{

    pinMode(txPin, OUTPUT);
}

// 发送任务，运行在 core2
void RFTransmitter::RFTransmitterTask(void *parameter)
{
    RFTransmitter *transmitter = static_cast<RFTransmitter *>(parameter);

    // 配置任务看门狗
    esp_task_wdt_init(5, true); // 5秒超时，允许发生紧急重启
    esp_task_wdt_add(NULL);

    RFParams params;
    const TickType_t sendInterval = pdMS_TO_TICKS(2000); // 2秒发送间隔
    TickType_t lastSendTime = xTaskGetTickCount();

    for (;;)
    {
        // 喂狗
        esp_task_wdt_reset();

        TickType_t currentTime = xTaskGetTickCount();
        if ((currentTime - lastSendTime) >= sendInterval)
        {
            if (transmitter->readRFParamsFromEEPROM(params))
            {
                transmitter->setupRFSender(params);
                if (transmitter->rfSender)
                {
                    Serial.println("Sending RF signal...");
                    transmitter->rfSender->send(params.dataLength, params.data);
                    Serial.println("RF signal sent successfully");
                    lastSendTime = currentTime; // 更新上次发送时间
                }
            }
        }

        // 较短的任务延时，保持系统响应性
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

bool RFTransmitter::readRFParamsFromEEPROM(RFParams &params)
{
    // 读取整个RFParams结构体
    if (!EEPROMManager::readData(FIRST_ADDRESS_FOR_RF_SIGNAL, (uint8_t *)&params, sizeof(RFParams)))
    {
        Serial.println("Failed to read EEPROM data");
        return false;
    }
    if ((RfSendEncoding)params.encoding >= RfSendEncoding::TRIBIT && (RfSendEncoding)params.encoding <= RfSendEncoding::MANCHESTER)
    {
        Serial.printf("RF encode mode is %d, adding to queue", params.encoding);
        // sendQueue.push(lastReceivedParams);
    }
    else
    {
        Serial.println("Invalid RF encode mode");
        return false;
    }
    // 验证数据有效性
    if (params.dataLength > 32 || params.nBit > 255)
    {
        Serial.println("Invalid RF parameters too long");
        return false;
    }
   
    // 验证数据非空
    bool hasValidData = false;
    for (size_t i = 0; i < params.dataLength; i++)
    {
        if (params.data[i] != 0xFF)
        {
            hasValidData = true;
            break;
        }
    }

    return hasValidData;
}

void RFTransmitter::setupRFSender(const RFParams &params)
{
    if (rfSender)
    {
        delete rfSender;
        rfSender = nullptr;
    }
    try
    {
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
            params.nBit);
        if (!rfSender)
        {
            Serial.println("Failed to create RF sender");
            return;
        }
        
        // 打印发送参数
        Serial.println();
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
    catch (...)
    {
        Serial.println("Exception during RF sender creation");
        if (rfSender)
        {
            delete rfSender;
            rfSender = nullptr;
        }
    }
}

void RFTransmitter::sendStoredSignal()
{
    if (sendQueue.empty())
        return;

    RFParams params = sendQueue.front();
    sendQueue.pop();

    setupRFSender(params);
    if (rfSender)
    {
        Serial.println("Sending stored RF signal...");
        rfSender->send(params.dataLength, params.data);
        Serial.println("RF signal sent.");
    }
}
