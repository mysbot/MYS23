#include "Config.h"
#include "EEPROMManager.h"
#include "APManager.h"
#include "windowControl.h"
#include "SerialManager.h"
#include "RFReceiverTask.h"
#include "RFStorageManager.h"
#include "Button_Operations.h"
// Class Instances
ButtonOperations buttonOperations(BUTTON_MODE, BUTTON_UP, BUTTON_DOWN, BUTTON_STOP);
RFTransmitter rfTransmitter(RF_TRANSMITTER_PIN, ADDmanager);
EEPROMManager eepromManager;
APManager apManager(80, ADDmanager);
WindowControl windowcontrol(ADDmanager);
RFReceiver rfReceiver(RF_RECEIVER_PIN, ADDmanager);
SerialManager serialManager(ADDmanager);

HardwareSerial mySerial(2);
uint8_t TARGET_ADDRESS = 0XFF;
//uint8_t RF_buffer[NUM_GROUPS][RF_NUM_DEFAULT] = {0};
address_Manager ADDmanager;
// Variables
uint32_t pairingStartTime = 0;
uint32_t testModeStartTime = 0;
bool pairingStarted = false;
bool inProductionTestMode = false;

void initializeComponents()
{
    // Initialize EEPROM
    eepromManager.begin();
    // Initialize serial manager
    serialManager.begin();
    serialManager.setSerial0Callback(onCommandFromComm0);
    serialManager.setSerial1Callback(onCommandFromComm1);
    // Initialize RF receiver
    rfReceiver.begin();
    rfReceiver.setCommandCallback(processRFCommand);
    // Initialize window control
    windowcontrol.begin();
    apManager.begin();
    // Initialize button operations
    buttonOperations.begin();
    

    startTasks();
   
}
void updateComponents()
{
    // Update window control
    windowcontrol.ControlUpdate();
    // Update web server
    apManager.update();
    // Update addmanager struct
    updateParameter();
    setRFWorkModeByWindowType(ADDmanager);
    setRFTransmitterModeByworkmode(ADDmanager);
    setRelayByWindowType(ADDmanager);
//     // Update button operations
    setButtonCallbacks();

    //heartBeatTimer();
//     // Update RF paring mode
    managePairingMode();    
    checkPairingTimeout();
//     // Update production test mode
    handleProductionTestMode();
    

}

void loadEEPROMSettings()
{
    ADDmanager.slidingDoorMode_value = TURN_OFF;
    // mySerial.println(F("test1."));
    updateAddress(ADDmanager.slidingDoorModeAddress, ADDmanager.slidingDoorMode_value);
    // mySerial.println(F("test2."));
    ADDmanager.RFpairingMode_value = static_cast<uint8_t>(Pairing::PAIR_OUT_TO_WORK);
    // updateAddress(ADDmanager.RFpairingModeAddress, ADDmanager.RFpairingMode_value);

    ADDmanager.rainSignal_value = static_cast<uint8_t>(rainSignalMode::WIRELESS);
    updateAddress(ADDmanager.rainSignalAddress, ADDmanager.rainSignal_value);

    ADDmanager.Is_mutual_value = !TURN_OFF;
    updateAddress(ADDmanager.mutualAddress, ADDmanager.Is_mutual_value);

    ADDmanager.Is_security_value = static_cast<uint8_t>(antiClampMode::TURNON);
    updateAddress(ADDmanager.securityAddress, ADDmanager.Is_security_value);

    eepromManager.readData(PRODUCTION_TEST_MODE_ADDRESS, &ADDmanager.productionTestModeTriggered, 1);
}

void heartBeatTimer()
{
    static uint32_t heartBeatTime = millis();
    static uint16_t messege = 0;

    if ((millis() - heartBeatTime) > HEART_BEAT_TIME)
    {
        heartBeatTime = millis();
        switch (messege)
        {

        case 0:
            serialManager.serial0Heart();
            // messege++;
            break;

        case 1:
            serialManager.serial0Write(ADDmanager.rainSignalAddress, ADDmanager.rainSignal_value);
            messege++;
            break;

        case 2:
            serialManager.serial0Write(ADDmanager.mutualAddress, ADDmanager.Is_mutual_value);
            messege++;
            break;

        case 3:
            serialManager.serial0Write(ADDmanager.securityAddress, ADDmanager.Is_security_value);
            messege = 0;
            break;

        default:
            break;
        }
    }
}

// 根据windowType_value决定基础RFworkmode的示例函数（在主程序中根据窗型调用）
inline void setRFWorkModeByWindowType(address_Manager &manager)
{
    static WindowType lastWindowType = (WindowType)manager.windowType_value;
    WindowType currentWindowType = (WindowType)manager.windowType_value;
    bool needTransmitter = false;
    // 只在窗型发生变化时才更新RF模式
    if (lastWindowType != currentWindowType)
    {
        uint8_t newRFWorkingMode;

        // 根据窗型选择合适的RF工作模式
        switch (currentWindowType)
        {
        case WindowType::AUTOLIFTWINDOW:
        case WindowType::AUTOSLIDINGDOOR:
        case WindowType::SKYLIGHTWINDOW:
        case WindowType::CURTAIN:
            newRFWorkingMode = (uint8_t)RFworkMode::HANS_RECEIVER;

            break;
        case WindowType::OUTWARDWINDOW:
        case WindowType::TILT_TURNWINDOW:
            newRFWorkingMode = (uint8_t)RFworkMode::HOPO_TRANSMITTER;

            break;
        default:
            newRFWorkingMode = (uint8_t)RFworkMode::HANS_RECEIVER;
            break;
        }

        // 如果RF工作模式需要改变，则安全地更新
        if (manager.RFworkingMode_value != newRFWorkingMode)
        {
            manager.RFworkingMode_value = newRFWorkingMode;
            // 将新的RF工作模式写入EEPROM
            if (eepromManager.writeData(manager.RFworkingModeAddress, &newRFWorkingMode, 1))
            {
            }
        }

        lastWindowType = currentWindowType;
    }
}
// 根据windowType_value决定基础RFworkmode的示例函数（在主程序中根据窗型调用）
inline void setRFTransmitterModeByworkmode(address_Manager &manager)
{
    static RFworkMode lastWorkMode = (RFworkMode)manager.RFworkingMode_value;
    RFworkMode currentWorkMode = (RFworkMode)manager.RFworkingMode_value;

    // 只在窗型发生变化时才更新RF模式
    if (lastWorkMode != currentWorkMode)
    {
        rfTransmitter.rfsend_build(manager.RFworkingMode_value);
    }

    lastWorkMode = currentWorkMode;
}

// 后续在主程序中，根据RFpairingMode_value的改变，决定调用setparameter或清码逻辑

void handleProductionTestMode()
{
    if (inProductionTestMode && (millis() - testModeStartTime >= PAIRING_TIMEOUT / 12))
    {
        endProductionTestMode();
    }
}

// 通用的配码管理函数
void managePairingMode()
{
    // eepromManager.readData(ADDmanager.RFpairingModeAddress, &ADDmanager.RFpairingMode_value, 1);

    switch (ADDmanager.RFpairingMode_value)
    {
    case static_cast<uint8_t>(Pairing::PAIR_CLEAR):
        clearPairing();
        break;
    case static_cast<uint8_t>(Pairing::HANS_1)...(static_cast<uint8_t>(Pairing::PAIR_CLEAR) - 1):
        startPairingMode(ADDmanager.RFpairingMode_value);
        break;
    default:
        stopPairingMode();
        break;
    }

    // updateAddress(ADDmanager.RFpairingModeAddress, ADDmanager.RFpairingMode_value);
}
// 检查配对模式是否超时
void checkPairingTimeout()
{
    if (pairingStarted && (millis() - pairingStartTime > PAIRING_TIMEOUT))
    {
        stopPairingMode();
        // updateAddress(ADDmanager.RFpairingModeAddress, ADDmanager.RFpairingMode_value);
    }
}
void startPairingMode(uint16_t mode)
{
    if (!pairingStarted)
    {
        pairingStartTime = millis();
        pairingStarted = true;
    }
    // 进入配对模式时，强制刷新接收与发射参数

    ADDmanager.RFpairingMode_value = mode;
}

void stopPairingMode()
{
    ADDmanager.RFpairingMode_value = static_cast<uint8_t>(Pairing::PAIR_OUT_TO_WORK);
    pairingStarted = false;
    // rfReceiver.register_Receiver(ADDmanager.RFworkingMode_value);
}

void clearPairing()
{
    RFStorageManager rfStorageManager(FIRST_ADDRESS_FOR_RF_SIGNAL);
    for (size_t i = 0; i < NUM_GROUPS; i++)
    {
        rfStorageManager.initRFData(i,ADDmanager);
    }

    stopPairingMode();
    rfTransmitter.rfsend_build(ADDmanager.RFworkingMode_value);
}

// 通用的地址更新函数
void updateAddress(uint8_t address, uint8_t value)
{
    uint8_t currentValue;
    eepromManager.readData(address, &currentValue, 1);

    // 只有在值变更时才写入EEPROM
    if (currentValue != value)
    {
        eepromManager.writeData(address, &value, 1);
    }
}

void updateParameter()
{
    // Helper lambda for reading and updating data
    auto readAndUpdate = [](uint8_t address, uint8_t &value, uint8_t defaultValue)
    {
        if (!eepromManager.readData(address, &value, 1))
        {
            value = defaultValue;
            updateAddress(address, value);
        }
    };

    // Sync all addresses with corresponding default values
    readAndUpdate(ADDmanager.localAddress, ADDmanager.localadd_value, 0);
    readAndUpdate(ADDmanager.RFworkingModeAddress, ADDmanager.RFworkingMode_value, static_cast<uint8_t>(RFworkMode::HANS_RECEIVER));
    // readAndUpdate(ADDmanager.RFmodeAddress, ADDmanager.RFmode_value, RF_HANS_MODE);
    // readAndUpdate(ADDmanager.RFpairingModeAddress, ADDmanager.RFpairingMode_value, static_cast<uint8_t>(Pairing::PAIR_OUT_TO_WORK));
    readAndUpdate(ADDmanager.controlGroupAddress, ADDmanager.controlGroup_value, static_cast<uint8_t>(ControlGroup::ALL));
    readAndUpdate(ADDmanager.windowTypeAddress, ADDmanager.windowType_value, static_cast<uint8_t>(WindowType::CURTAIN));
    readAndUpdate(ADDmanager.slidingDoorModeAddress, ADDmanager.slidingDoorMode_value, TURN_OFF);

    // Direct reads without updates
    eepromManager.readData(ADDmanager.rainSignalAddress, &ADDmanager.rainSignal_value, 1);
    eepromManager.readData(ADDmanager.mutualAddress, &ADDmanager.Is_mutual_value, 1);
    eepromManager.readData(ADDmanager.securityAddress, &ADDmanager.Is_security_value, 1);
}

void setRelayByWindowType(address_Manager &manager)
{
    static uint16_t previouswindowType = 0;
    if (manager.windowType_value != previouswindowType)
    {
        windowcontrol.controlBasedOnWindowType(ControlType::RELAY_CONTROL, Command::SCREEN_STOP);
        windowcontrol.controlBasedOnWindowType(ControlType::RELAY_CONTROL, Command::WINDOW_STOP);
        previouswindowType = manager.windowType_value;
    }
    switch (static_cast<WindowType>(manager.windowType_value))
    {
    case WindowType::AUTOLIFTWINDOW:
    case WindowType::AUTOSLIDINGDOOR:
        windowcontrol.controlBasedOnWindowType(ControlType::RELAY_CONTROL, Command::C_DEFAULT);
        break;

    default:

        break;
    }
}

void onCommandFromComm0(UARTCommand cmd)
{
    // 在这里处理comm0过来的命令
    processCommand(cmd, 0);
}

void onCommandFromComm1(UARTCommand cmd)
{
    // 在这里处理comm1过来的命令
    processCommand(cmd, 1);
}

void processCommand(UARTCommand command, u_int8_t uartNumber)
{
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
        windowcontrol.controlBasedOnWindowType(ControlType::COMM1, tempCommand);
        windowcontrol.controlBasedOnWindowType(ControlType::RELAY_CONTROL, tempCommand);
        windowcontrol.controlBasedOnWindowType(ControlType::TRANSMITTER, tempCommand);
    }
    break;

    default:
        // Handle unexpected response codes if needed
        break;
    }
}

void setButtonCallbacks()
{
    buttonOperations.onButtonALongPressed([]() {

    });
    buttonOperations.onButtonAPressed([]() {

    });
    buttonOperations.onButtonBPressed([]()
                                      {
                                          if (!inProductionTestMode && ADDmanager.productionTestModeTriggered == INIT_DATA)
                                          {
                                              enterProductionTestMode();
                                              windowcontrol.controlBasedOnWindowType(ControlType::TRANSMITTER, Command::SCREEN_UP);

                                              return;
                                          }
                                          handleButtonPress(Command::SCREEN_UP, Command::WINDOW_UP); });

    buttonOperations.onButtonCPressed([]()
                                      { handleButtonPress(Command::SCREEN_DOWN, Command::WINDOW_DOWN); });

    buttonOperations.onButtonDPressed([]()
                                      { handleButtonPress(Command::SCREEN_STOP, Command::WINDOW_STOP); });
    buttonOperations.onButtonBShortPressed([]()
                                           {
        if(ADDmanager.rainSignal_value==static_cast<uint8_t>(rainSignalMode::WIRED))
          serialManager.serial0Function(Command::RAIN_SIGNAL);
        ; });
    buttonOperations.onButtonDShortPressed([]() {

    });
}

void handleButtonPress(Command screenCommand, Command windowCommand)
{
    if (ADDmanager.controlGroup_value == static_cast<uint8_t>(ControlGroup::GROUP1) || ADDmanager.controlGroup_value == static_cast<uint8_t>(ControlGroup::ALL))
    {
        windowcontrol.controlBasedOnWindowType(ControlType::RELAY_CONTROL, screenCommand);
        windowcontrol.controlBasedOnWindowType(ControlType::TRANSMITTER, screenCommand);
        windowcontrol.controlBasedOnWindowType(ControlType::COMM1, screenCommand);
        ;
    }
    if (ADDmanager.controlGroup_value == static_cast<uint8_t>(ControlGroup::GROUP2) || ADDmanager.controlGroup_value == static_cast<uint8_t>(ControlGroup::ALL))
    {
        windowcontrol.controlBasedOnWindowType(ControlType::RELAY_CONTROL, windowCommand);
        windowcontrol.controlBasedOnWindowType(ControlType::TRANSMITTER, windowCommand);
        windowcontrol.controlBasedOnWindowType(ControlType::COMM1, windowCommand);
        ;
    }
}

void handleProductionRFLoop(Command RF_index)
{

    if (RF_index == Command::SCREEN_UP)
    {
        windowcontrol.controlBasedOnWindowType(ControlType::RELAY_CONTROL, Command::WINDOW_UP);
        windowcontrol.controlBasedOnWindowType(ControlType::TRANSMITTER, Command::WINDOW_UP);
        windowcontrol.controlBasedOnWindowType(ControlType::COMM1, Command::WINDOW_UP);
    }
    else if (RF_index == Command::WINDOW_UP)
    {
        windowcontrol.controlBasedOnWindowType(ControlType::RELAY_CONTROL, Command::SCREEN_UP);
        windowcontrol.controlBasedOnWindowType(ControlType::TRANSMITTER, Command::SCREEN_UP);
        windowcontrol.controlBasedOnWindowType(ControlType::COMM1, Command::SCREEN_UP);
    }
}

void processRFCommand(RFCommand cmd)
{

    if (inProductionTestMode)
    {
        // 在产测模式下，保持发送和接收的循环
        handleProductionRFLoop(cmd.index);
        return;
    }

    if (cmd.index == Command::RAIN_SIGNAL && ADDmanager.rainSignal_value == static_cast<uint8_t>(rainSignalMode::WIRELESS))
        serialManager.serial0Function(Command::RAIN_SIGNAL);
    //;
    else
    {
        windowcontrol.controlBasedOnWindowType(ControlType::RELAY_CONTROL, cmd.index);
        windowcontrol.controlBasedOnWindowType(ControlType::COMM1, cmd.index);
    }
}

// 进入产测模式
void enterProductionTestMode()
{
    // 设置 RF 工作模式为 HANS_BOTH
    ADDmanager.windowType_value = static_cast<uint8_t>(WindowType::CURTAIN);
    updateAddress(ADDmanager.windowTypeAddress, ADDmanager.windowType_value);

    ADDmanager.RFworkingMode_value = static_cast<uint8_t>(RFworkMode::HANS_BOTH);
    updateAddress(ADDmanager.RFworkingModeAddress, ADDmanager.RFworkingMode_value);

    //memcpy(RF_buffer, hansValues, sizeof(RF_buffer));
    RFStorageManager rfStorageManager(FIRST_ADDRESS_FOR_RF_SIGNAL);
    rfStorageManager.initRFData((uint8_t)Pairing::HANS_1-1,ADDmanager);
    rfStorageManager.initRFData((uint8_t)Pairing::HANS_2-1,ADDmanager);
    // 设置产测模式触发标志
    ADDmanager.productionTestModeTriggered = !TURN_OFF;
    updateAddress(PRODUCTION_TEST_MODE_ADDRESS, ADDmanager.productionTestModeTriggered);

    // 启动产测模式
    inProductionTestMode = true;
    testModeStartTime = millis();

    Serial.println("enter product testing,the RS485,relay,RF transmitter and receiver is working.");
}

// 结束产测模式
void endProductionTestMode()
{
    // 恢复 RF 工作模式为正常模式（假设为 0）
    ADDmanager.RFworkingMode_value = static_cast<uint8_t>(RFworkMode::HANS_RECEIVER);
    updateAddress(ADDmanager.RFworkingModeAddress, ADDmanager.RFworkingMode_value);
    // 关闭继电器通道（假设为关闭状态）

    windowcontrol.controlBasedOnWindowType(ControlType::RELAY_CONTROL, Command::SCREEN_STOP);
    windowcontrol.controlBasedOnWindowType(ControlType::RELAY_CONTROL, Command::WINDOW_STOP);
    // 停止产测模式
    inProductionTestMode = false;

    Serial.println("product testing is end,go back to normal");
}

void startTasks()
{
    // 创建串口更新任务，由 SerialManager 模块内部处理 updateAll()
    xTaskCreatePinnedToCore(
        [](void *parameter)
        { serialManager.serialManagerTask(); }, // 由 SerialManager.cpp 提供
        "SerialManager_Task",
        4096,            // 合适的堆栈大小
        &serialManager, // 传递 SerialManager 实例
        3,               // 任务优先级
        NULL,
        0 // 指定运行核心
    );

    // 创建RF接收任务
    xTaskCreatePinnedToCore(
        [](void *parameter)
        {
            static_cast<RFReceiver *>(parameter)->RFReceiverTask(parameter);
        },
        "RF_Receiver_Task",
        8192,
        &rfReceiver,
        2,
        NULL,
        1);

    // 发送任务
}