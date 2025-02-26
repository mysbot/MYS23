#include "SerialManager.h"

void SerialManager::init() {
    // 初始化串口互斥锁
    SERIAL_MUTEX.init();
    
    // 创建串口包装器
    serial0Wrapper = new HardwareSerialWrapper(Serial);
    serial1Wrapper = new HardwareSerialWrapper(mySerial);
    
    // 创建串口通信实例
    serialComm0 = new SerialComm(serial0Wrapper);
    serialComm1 = new SerialComm(serial1Wrapper);
    
    // 创建UART通信控制器
    uartComm0 = new UARTComm(serialComm0, BAUDRATE, RX0_PIN, TX0_PIN, false);
    uartComm1 = new UARTComm(serialComm1, BAUDRATE, RX1_PIN, TX1_PIN, true);
    
    // 初始化UART通信
    uartComm0->init();
    uartComm1->init();
    
    Serial.println("SerialManager 0 initialized");
    mySerial.println("SerialManager 1 initialized");
}

bool SerialManager::updateAll() {
    bool hadData = false;
    
    if (updateSerial0()) {
        hadData = true;
    }
    
    if (updateSerial1()) {
        hadData = true;
    }
    
    return hadData;
}

bool SerialManager::updateSerial0() {
    static uint32_t lastUpdate = 0;
    if (millis() - lastUpdate < 10) { // 最小10ms更新间隔
        return false;
    }
    lastUpdate = millis();
    
    return uartComm0->update();
}

bool SerialManager::updateSerial1() {
    static uint32_t lastUpdate = 0;
    if (millis() - lastUpdate < 10) { // 最小10ms更新间隔
        return false;
    }
    lastUpdate = millis();
    
    return uartComm1->update();
}

void SerialManager::setSerial0Callback(CommandCallback cb) {
    if (uartComm0) {
        uartComm0->setCommandCallback(cb);
    }
}

void SerialManager::setSerial1Callback(CommandCallback cb) {
    if (uartComm1) {
        uartComm1->setCommandCallback(cb);
    }
}

void SerialManager::serial0Write(uint8_t dataAddress, uint8_t data) {
    if (uartComm0) {
        uartComm0->sendHEXWrite(TARGET_ADDRESS, dataAddress, data);
    }
}

void SerialManager::serial0Read(uint8_t dataAddress) {
    if (uartComm0) {
        uartComm0->sendHEXRead(TARGET_ADDRESS, dataAddress, 0);
    }
}

void SerialManager::serial0Heart() {
    if (uartComm0) {
        uartComm0->sendHEXheart(TARGET_ADDRESS);
    }
}

void SerialManager::serial0Function(Command index) {
    if (uartComm0) {
        uartComm0->sendHEXfunction(TARGET_ADDRESS, static_cast<uint8_t>(index));
    }
}

void SerialManager::serial1SendCommand(Command index) {
    if (uartComm1) {
        if (index <= Command::SCREEN_DOWN) {
            uartComm1->sendscreenCommand(static_cast<uint8_t>(index));
        } else {
            uartComm1->sendCharMessage(String(static_cast<uint8_t>(index), HEX).c_str());
        }
    }
}