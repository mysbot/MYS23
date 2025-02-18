#include "RFReceiverTask.h"
#include "RFTransmitterTask.h"




RFReceiver rfReceiver(RF_RECEIVER_PIN);
RFTransmitter rfTransmitter(RF_TRANSMITTER_PIN);

void startTasks() {
    // 接收任务
    xTaskCreatePinnedToCore(
        [](void* parameter) { 
            static_cast<RFReceiver*>(parameter)->RFReceiverTask(parameter); 
        },
        "RF_Receiver_Task",
        8192,
        &rfReceiver,
        5,  // 提高优先级到5
        NULL,
        1
    );

    // 发送任务
    xTaskCreatePinnedToCore(
        RFTransmitter::RFTransmitterTask,
        "RF_Transmitter_Task",
        8192,
        &rfTransmitter,
        4,  // 发送任务优先级4
        NULL,
        0
    );
}

void setup() {
    Serial.begin(9600);  // 提高波特率
    
    // 配置CPU频率
    setCpuFrequencyMhz(240);
    
    // 初始化看门狗，延长超时时间
    esp_task_wdt_init(10, true);
    
    // 初始化 EEPROM
    EEPROMManager::EEPROMInitialize();
    
    // 初始化RF管理器
    rfTransmitter.begin();
    rfReceiver.begin();
    
    // 启动任务
    startTasks();
    
    // 禁用loop任务的看门狗
    esp_task_wdt_delete(NULL);
    
    Serial.println("System initialized");
}

void loop() {
    
    
    yield();  // 让出CPU时间
    vTaskDelay(pdMS_TO_TICKS(10));  // 短暂延时，避免看门狗复位
    
    // 可以在这里添加系统状态监控
    static uint32_t lastStatusTime = 0;
    if(millis() - lastStatusTime > 5000) {
        // 每5秒打印一次系统状态
       // Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
        lastStatusTime = millis();
    }
    
}