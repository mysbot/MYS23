
#include "Config.h"

void setup()
{
    // 配置CPU频率
    setCpuFrequencyMhz(240);

    // 初始化看门狗，延长超时时间
    esp_task_wdt_init(10, true);

    initializeComponents();

    // 禁用loop任务的看门狗
    esp_task_wdt_delete(NULL);

    // Serial.println("System initialized");
}

void loop()
{
    // 主循环无需处理串口更新任务
    vTaskDelay(pdMS_TO_TICKS(10));
    updateComponents();
    // 可以在这里添加系统状态监控
    static uint32_t lastStatusTime = 0;
    if (millis() - lastStatusTime > 5000)
    {
        // 每5秒打印一次系统状态
        // Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
        lastStatusTime = millis();
    }
}
