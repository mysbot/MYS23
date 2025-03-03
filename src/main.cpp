#include "RFReceiverTask.h"
#include "SerialManager.h"
#include "Config.h"
#include "EEPROMManager.h"
#include "WindowControl.h"

HardwareSerial mySerial(2);
uint8_t TARGET_ADDRESS = 0XFF;
uint8_t RF_buffer[NUM_GROUPS][RF_NUM_DEFAULT] = {0};
address_Manager ADDmanager;

RFReceiver rfReceiver(RF_RECEIVER_PIN);
EEPROMManager eeprommanager;
WindowControl windowcontrol;



void startTasks() {
    // 创建串口更新任务，由 SerialManager 模块内部处理 updateAll()
    xTaskCreatePinnedToCore(
        [](void* parameter) { SERIAL_MANAGER.serialManagerTask(); },          // 由 SerialManager.cpp 提供
        "SerialManager_Task",
        4096,                       // 合适的堆栈大小
        &SERIAL_MANAGER,            // 传递 SerialManager 实例
        3,                          // 任务优先级
        NULL,
        0                         // 指定运行核心
    );
    
    // 创建RF接收任务
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
    
    // 发送任务
    
    
}
void processRFCommand(RFCommand cmd) {
    if (cmd.index == Command::RAIN_SIGNAL && ADDmanager.rainSignal_value == static_cast<uint8_t>(rainSignalMode::WIRELESS))
    SERIAL_MANAGER.serial0Function(Command::RAIN_SIGNAL);
//;
else
{
    windowcontrol.controlBasedOnWindowType(ControlType::RELAY_CONTROL, cmd.index);
    windowcontrol.controlBasedOnWindowType(ControlType::COMM1, cmd.index);
}
    
}
void processCommand(UARTCommand command, bool isComm1)
{
    // 添加调试输出
    // Serial.print(isComm1 ? "COM1" : "COM0");
    // Serial.print(" rec com: functioncode=");
    // Serial.print(static_cast<int>(command.responseCode));
    // Serial.print(" dataaddress=0x");
    // Serial.println(command.dataAddress, HEX);

    switch (command.responseCode)
    {
    case FunctionCode::READ:
        // These cases can be handled as needed
        break;
    case FunctionCode::WRITE:
        // These cases can be handled as needed
        break;
    case FunctionCode::CONFIRM:
        // These cases can be handled as needed
        break;
    case FunctionCode::SUCCESEE:
        // These cases can be handled as needed
        break;
    case FunctionCode::FAILED:
        // These cases can be handled as needed
        break;
    case FunctionCode::HEART:
        // These cases can be handled as needed
        break;

    case FunctionCode::FUNCTION:
    {
        Command tempCommand = static_cast<Command>(command.dataAddress);
       
        // 区分处理来源
        // if (isComm1) {
        //     // COM1的处理 - 添加打印确认
        //     mySerial.print("COM1 recv: 0x");
        //     mySerial.println(static_cast<uint8_t>(tempCommand), HEX);
        // }
        
        // 所有功能命令都传递给COM0
        SERIAL_MANAGER.serial0Function(tempCommand);
    }
    break;

    default:
        // Handle unexpected response codes if needed
        break;
    }
}
void processComm1Command(UARTCommand command)
{
    processCommand(command, true);
}

void processComm0Command(UARTCommand command)
{
    processCommand(command, false);
}
void onCommandFromComm0(UARTCommand cmd)
{
    // 在这里处理comm0过来的命令
    processComm0Command(cmd);
}

void onCommandFromComm1(UARTCommand cmd)
{
    // 在这里处理comm1过来的命令
    processComm1Command(cmd);
}

void setup() {
    SERIAL_MANAGER.init();
    SERIAL_MANAGER.setSerial0Callback(onCommandFromComm0);
    SERIAL_MANAGER.setSerial1Callback(onCommandFromComm1);

    // 配置CPU频率
    setCpuFrequencyMhz(240);
    
    // 初始化看门狗，延长超时时间
    esp_task_wdt_init(10, true);
    
    // 初始化 EEPROM
    eeprommanager.EEPROMInitialize();
    //EEPROMManager::EEPROMInitialize();
    
    // 初始化RF管理器
    rfReceiver.begin();
    rfReceiver.setCommandCallback(processRFCommand);
    windowcontrol.setup();

    
    // 启动任务
    startTasks();
    
    // 禁用loop任务的看门狗
    esp_task_wdt_delete(NULL);
    
    Serial.println("System initialized");
}

void loop() {
    // 主循环无需处理串口更新任务
    vTaskDelay(pdMS_TO_TICKS(10));
    
    // 可以在这里添加系统状态监控
    static uint32_t lastStatusTime = 0;
    if(millis() - lastStatusTime > 5000) {
        // 每5秒打印一次系统状态
       // Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
        lastStatusTime = millis();
    }
}
