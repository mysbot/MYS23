#include "COMM0.h"

Comm0::Comm0() {}
// Create serial wrapper
HardwareSerialWrapper serialWrapper0(Serial);     // Use hardware serial
// Create SerialComm and UARTComm instances
SerialComm serialComm0(&serialWrapper0);
UARTComm uartComm0(&serialComm0, BAUDRATE, RX0_PIN, TX0_PIN, 0);  // Use hardware serial for UART communication

void Comm0::init() {
    uartComm0.init();  // Initialize UART communication using hardware serial
    Serial.println("Comm0 initialized");
}
/*
void Comm0::update() {
    uartComm0.update(); 
    Serial.println("Comm0 updated");
}
*/
bool Comm0::update() {
    static uint32_t lastUpdate = 0;
    if (millis() - lastUpdate < 10) { // 最小10ms更新间隔
        return false;
    }
    lastUpdate = millis();
    
    bool hadData = false;
    if (uartComm0.update()) {
        hadData = true;
    }
    return hadData;
}
// Implement setCommandCallback to register callback with internal UARTComm
void Comm0::setCommandCallback(CommandCallback cb) {
    uartComm0.setCommandCallback(cb);
}

void Comm0::Uart0Function(Command index) {
    uartComm0.sendHEXfunction(TARGET_ADDRESS, static_cast<uint8_t>(index));
}

void Comm0::Uart0Write(uint8_t dataAddress, uint8_t data) {
    uartComm0.sendHEXWrite(TARGET_ADDRESS, dataAddress, data);
}

void Comm0::Uart0Read(uint8_t dataAddress) {
    uartComm0.sendHEXRead(TARGET_ADDRESS, dataAddress, 0);
}

void Comm0::Uart0Heart() {
    uartComm0.sendHEXheart(TARGET_ADDRESS);
}
