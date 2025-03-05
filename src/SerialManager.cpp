#include "SerialManager.h"

void SerialManager::begin()
{
    // 初始化串口互斥锁
    SERIAL_MUTEX.init();

    // 创建串口包装器
    serial0Wrapper = new HardwareSerialWrapper(Serial);
    serial1Wrapper = new HardwareSerialWrapper(mySerial);

    // 创建串口通信实例
    serialComm0 = new SerialComm(serial0Wrapper);
    serialComm1 = new SerialComm(serial1Wrapper);

    // 创建UART通信控制器
    uartComm0 = new UARTComm(serialComm0, BAUDRATE, RX0_PIN, TX0_PIN, 0, ADDmanager);
    uartComm1 = new UARTComm(serialComm1, BAUDRATE, RX1_PIN, TX1_PIN, 1, ADDmanager);

    // 初始化UART通信
    uartComm0->init();
    uartComm1->init();

    // 添加更详细的初始化信息
    Serial.println("SerialManager: COM0 init");
    mySerial.println("SerialManager: COM1 init");

    // 测试COM1连接是否正常
    if (uartComm1)
    {

        // 发送测试心跳，验证COM1是否正常
        uartComm1->sendHEXheart(0xaa);
        mySerial.println("SerialManager: COM1 send test");
    }
}

bool SerialManager::updateAll()
{
    bool hadData = false;

    // 使用非阻塞方式分别更新两个串口
    bool serial0Data = updateSerial0();
    bool serial1Data = updateSerial1();

    // 如果任一串口有数据，则返回true
    return (serial0Data || serial1Data);
}

bool SerialManager::updateSerial0()
{

    static uint32_t lastUpdate = 0;
    const uint32_t minUpdateInterval = 5; // 减少最小更新间隔

    uint32_t currentTime = millis();
    if (currentTime - lastUpdate < minUpdateInterval)
    {
        return false;
    }
    lastUpdate = currentTime;

    if (uartComm0)
    {
        return uartComm0->update();
    }
    return false;
}

bool SerialManager::updateSerial1()
{

    static uint32_t lastUpdate = 0;
    const uint32_t minUpdateInterval = 5; // 减少最小更新间隔

    uint32_t currentTime = millis();
    if (currentTime - lastUpdate < minUpdateInterval)
    {
        return false;
    }
    lastUpdate = currentTime;

    if (uartComm1)
    {
        return uartComm1->update();
    }
    return false;
}

void SerialManager::setSerial0Callback(CommandCallback cb)
{
    if (uartComm0)
    {
        uartComm0->setCommandCallback(cb);
    }
}

void SerialManager::setSerial1Callback(CommandCallback cb)
{
    if (uartComm1)
    {
        uartComm1->setCommandCallback(cb);
    }
}

void SerialManager::serial0Write(uint8_t dataAddress, uint8_t data)
{
    if (uartComm0)
    {
        uartComm0->sendHEXWrite(TARGET_ADDRESS, dataAddress, data);
    }
}

void SerialManager::serial0Read(uint8_t dataAddress)
{
    if (uartComm0)
    {
        uartComm0->sendHEXRead(TARGET_ADDRESS, dataAddress, 0);
    }
}

void SerialManager::serial0Heart()
{
    if (uartComm0)
    {
        uartComm0->sendHEXheart(TARGET_ADDRESS);
    }
}

void SerialManager::serial0Function(Command index)
{
    if (uartComm0)
    {
        uartComm0->sendHEXfunction(TARGET_ADDRESS, static_cast<uint8_t>(index));
    }
}

void SerialManager::serial1SendCommand(Command index)
{
    if (uartComm1)
    {
        if (index <= Command::SCREEN_DOWN)
        {
            uartComm1->sendscreenCommand(static_cast<uint8_t>(index));
        }
        else
        {
            uartComm1->sendCharMessage(String(static_cast<uint8_t>(index), HEX).c_str());
        }
    }
}

// 新增：串口管理任务函数，封装串口更新循环
void SerialManager::serialManagerTask()
{
    const TickType_t delayTime = pdMS_TO_TICKS(5);
    while (true)
    {
        updateAll();
        vTaskDelay(delayTime);
    }
}