#include <Arduino.h>
#include "RFManager.h"
#include "send.h" // 包含 RFSender 类声明

// 定义RF引脚
#define RF_RX_PIN 19  // 接收引脚
#define RF_TX_PIN 21  // 发射引脚

RFManager rfManager(RF_RX_PIN, RF_TX_PIN);
RFSender rfSender(RF_TX_PIN); // 实例化 RFSender 类
volatile bool monitorFlag = true;

void setup() {
    Serial.begin(9600);
    
    // 初始化RF管理器
    rfManager.begin();
    rfManager.setMonitorMode(true); // 默认启动监听模式
    rfManager.startTasks();
    
    // 初始化RF发送器
    rfSender.begin();
    
    Serial.println("RF Manager initialized in monitor mode");
}

void loop() {
    /*
    static uint32_t lastToggleTime = 0;
    const uint32_t TOGGLE_INTERVAL = 10000; // 10秒切换一次模式
    
    uint32_t currentTime = millis();
    if (currentTime - lastToggleTime >= TOGGLE_INTERVAL) {
        lastToggleTime = currentTime;
        monitorFlag = !monitorFlag;
        rfManager.setMonitorMode(monitorFlag);
        
        Serial.printf("Switched to %s mode\n", 
            monitorFlag ? "monitor" : "transmit");
    }
    
    if (!monitorFlag) {
        rfSender.sendLoop(); // 调用发送循环
    }
    */
    delay(100);
}