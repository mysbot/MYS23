#include "COMM1.h"

Comm1::Comm1() {}
// 创建串口封装器
HardwareSerialWrapper serialWrapper1(mySerial);     // 使用硬件串口
//SoftwareSerialWrapper SerialWrapper(mySerial); // 使用软件串口
// 创建 SerialComm 和 UARTComm 实例
SerialComm serialComm1(&serialWrapper1);
//SerialComm serialComm1(&SerialWrapper);
//UARTComm uartComm1(&serialComm1, BAUDRATE,0);  // 使用硬件串口的UART通信
UARTComm uartComm1(&serialComm1, BAUDRATE,RX1_PIN,TX1_PIN,1);  // 使用软件串口的UART通信

void Comm1::init() {
    //uartComm1.init();  // 初始化使用硬件串口的 UART 通信
    uartComm1.init();  // 初始化使用软件串口的 UART 通信
    mySerial.println("mySerial is OK");
   
}

bool Comm1::update() {
    static uint32_t lastUpdate = 0;
    if (millis() - lastUpdate < 10) { // 最小10ms更新间隔
        return false;
    }
    lastUpdate = millis();
    
    bool hadData = false;
    if (uartComm1.update()) {
        hadData = true;
    }
    return hadData;
}

void Comm1::setCommandCallback(CommandCallback cb) {
    uartComm1.setCommandCallback(cb);
}
void Comm1::sendUart1Data(Command index) {
switch (index) {
        case Command::SCREEN_STOP: // Send screen stop command            
     
        case Command::SCREEN_UP: // Send screen up command            
      
        case Command::SCREEN_DOWN: // Send screen down command
         uartComm1.sendscreenCommand(static_cast<uint8_t>(index));  // 直接使用命令值发送
            break;
        case Command::WINDOW_STOP: // Send window stop command
        
        case Command::WINDOW_UP: // Send window up command
         
        case Command::WINDOW_DOWN: // Send window down command

        case Command::LIGHT_ON: // Send window down command
        
        case Command::LIGHT_OFF: // Send window down command
          uartComm1.sendCharMessage(String(static_cast<uint8_t>(index), HEX).c_str());  // 将命令值转换为字符串发送   
            break;
        
        default:
           // mySerial.println("Unknown command index");
            break;
    }

};


