#include "RFReceiverTask.h"
#include "RFTransmitterTask.h"

// 定义RF引脚
#define RF_RX_PIN 19  // 接收引脚
#define RF_TX_PIN 21  // 发射引脚
uint16_t INIT_DATA_ADDRESS = 0x0100;  // 示例存储地址（可根据实际修改）
RFReceiver rfReceiver(RF_RX_PIN);
RFTransmitter rfTransmitter(RF_TX_PIN);

// 修改 startTasks()，将 RFReceiverTask 固定在 core1，RFTransmitterTask 固定在 core0
void startTasks() {
    // 创建接收任务，运行在 core1
    xTaskCreatePinnedToCore(
        [](void* parameter) { 
            static_cast<RFReceiver*>(parameter)->RFReceiverTask(parameter); 
        },
        "RF_Receiver_Task",
        8192,
        &rfReceiver,
        2,
        NULL,
        1
    );

    // 创建发送任务，运行在 core0
    xTaskCreatePinnedToCore(
        RFTransmitter::RFTransmitterTask,
        "RF_Transmitter_Task",
        8192,
        &rfTransmitter,
        1,
        NULL,
        0
    );
}

void setup() {
    Serial.begin(9600);
    while (!Serial) delay(10);
    
    // 初始化 EEPROM
    EEPROMManager::EEPROMInitialize();
    
    // 初始化RF管理器
    rfTransmitter.begin();
    rfReceiver.begin();
    
    startTasks();
    
    Serial.println("System initialized");
}

void loop() {
    static uint32_t lastToggleTime = 0;
    const uint32_t TOGGLE_INTERVAL = 10000; // 10秒切换一次模式
    
    uint32_t currentTime = millis();
    if (currentTime - lastToggleTime >= TOGGLE_INTERVAL) {
        lastToggleTime = currentTime;
        static bool monitorFlag = 0;
        monitorFlag = !monitorFlag;       
        
        Serial.printf("Switched to %s mode\n", 
            monitorFlag ? "monitor" : "transmit");
    }
    
    delay(100);
}