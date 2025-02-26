// UARTComm.cpp
#include "UARTComm.h"
#include <cstdlib> // For std::atoi

UARTComm::UARTComm(SerialComm* serialCommInstance, uint32_t baudRate, uint8_t rx_pin, uint8_t tx_pin, bool isCom1Serial) 
    : serialComm(serialCommInstance), BaudRate(baudRate), rx_pin(rx_pin), tx_pin(tx_pin), isCom1Serial(isCom1Serial) {}

void UARTComm::init() {
    //SerialGuard guard;
    serialComm->begin(BaudRate, rx_pin, tx_pin);
    delay(50);  // 给串口初始化一些时间
    eeprommanager.EEPROMInitialize();
    Serial.println("UARTComm initialized");
}

void UARTComm::clearSerialBuffer() {
    while (serialComm->available()) {
        serialComm->read();
    }
}

bool UARTComm::update() {
    static unsigned long lastUpdateTime = 0;
    const unsigned long updateInterval = 50; // 50ms最小间隔
    bool dataProcessed = false;
    
    unsigned long currentTime = millis();
    if (currentTime - lastUpdateTime < updateInterval) {
        return false;
    }
    lastUpdateTime = currentTime;
    
    if (!serialComm->available()) {
        return false;
    }
    
    // 限制单次处理的数据量
    uint8_t processedBytes = 0;
    const uint8_t maxBytesToProcess = 32;
    
    while (serialComm->available() && processedBytes < maxBytesToProcess) {
        uint8_t byte = serialComm->read();
        processedBytes++;
        dataProcessed = true;  // 标记已处理数据
        //Serial.print("Received byte: ");
        //Serial.println(byte, HEX);

        switch (receiveState) {
            case ReceiveState::WAIT_FOR_HEADER:
                if (byte == FIRSTBYTE) {
                    receivedFrame.header = byte;
                    bytesReceived = 1;
                    receiveState = ReceiveState::WAIT_FOR_SOURCE_ADDRESS;
                } else {
                    Serial.println("Invalid header byte");
                    receiveState = ReceiveState::WAIT_FOR_HEADER;
                    cmd = {FunctionCode::C_DEFAULT, INIT_DATA, INIT_DATA};
                }
                break;

            case ReceiveState::WAIT_FOR_SOURCE_ADDRESS:
                receivedFrame.sourceAddress = byte;
                if (receivedFrame.sourceAddress == THE_THIRD_PART || receivedFrame.sourceAddress == TARGET_ADDRESS) {
                    bytesReceived++;
                    receiveState = ReceiveState::WAIT_FOR_TARGET_ADDRESS;
                } else {
                    Serial.println("Invalid source address byte");
                    receiveState = ReceiveState::WAIT_FOR_HEADER;
                    cmd = {FunctionCode::C_DEFAULT, INIT_DATA, INIT_DATA};
                }
                break;

            case ReceiveState::WAIT_FOR_TARGET_ADDRESS:
                receivedFrame.targetAddress = byte;
                if (receivedFrame.targetAddress == ADDmanager.localadd_value) {
                    bytesReceived++;
                    receiveState = ReceiveState::WAIT_FOR_FUNCTION_CODE;
                } else {
                    Serial.println("Invalid target address byte");
                    receiveState = ReceiveState::WAIT_FOR_HEADER;
                    cmd = {FunctionCode::C_DEFAULT, INIT_DATA, INIT_DATA};
                }
                break;

            case ReceiveState::WAIT_FOR_FUNCTION_CODE:
                receivedFrame.functionCode = byte;
                bytesReceived++;
                expectedDataLength = frameRequiresData(receivedFrame.functionCode) ? 1 : 0;
                receiveState = ReceiveState::WAIT_FOR_DATA_ADDRESS;
                break;

            case ReceiveState::WAIT_FOR_DATA_ADDRESS:
                receivedFrame.dataAddress = byte;
                bytesReceived++;
                receiveState = (expectedDataLength > 0) ? ReceiveState::WAIT_FOR_DATA : ReceiveState::WAIT_FOR_CRC;
                break;

            case ReceiveState::WAIT_FOR_DATA:
                receivedFrame.data = byte;
                bytesReceived++;
                receiveState = ReceiveState::WAIT_FOR_CRC;
                break;

            case ReceiveState::WAIT_FOR_CRC:
                if (bytesReceived < sizeof(CommFrame) - 1) {
                    receivedCRC = (receivedCRC << 8) | byte;
                    bytesReceived++;
                    if (bytesReceived >= expectedFrameLength()) {
                        receivedCRC = (receivedCRC >> 8) | (receivedCRC << 8);
                        if (verifyCRC(reinterpret_cast<uint8_t*>(&receivedFrame), bytesReceived - 2, receivedCRC)) {
                            UARTCommand newCmd = executeCommand(receivedFrame);
                            if (callback) {
                                callback(newCmd);
                            }
                        } else {
                            Serial.println("CRC verification failed");
                        }
                        receiveState = ReceiveState::WAIT_FOR_HEADER;
                        receivedCRC = 0;
                        bytesReceived = 0;
                    }
                }
                break;

            default:
                receiveState = ReceiveState::WAIT_FOR_HEADER;
                bytesReceived = 0;
                receivedCRC = 0;
                cmd = {FunctionCode::C_DEFAULT, INIT_DATA, INIT_DATA};
                break;
        }
    }
    
    return dataProcessed;  // 返回是否处理了数据
}

bool UARTComm::frameRequiresData(uint8_t functionCode) {
    // 根据 functionCode 决定是否需要数据
    switch (functionCode) {
        case static_cast<uint8_t>(FunctionCode::READ):
        case static_cast<uint8_t>(FunctionCode::WRITE):
        case static_cast<uint8_t>(FunctionCode::SUCCESEE):
        case static_cast<uint8_t>(FunctionCode::FAILED):
        case static_cast<uint8_t>(FunctionCode::HEART):
            return true;
        default:
            return false;
    }
}

uint16_t UARTComm::expectedFrameLength() {
    // 根据协议返回期望的帧长度
    // 例如，header + source + target + function + dataaddress +data+ crc
    return 5 + expectedDataLength + 2;
}

UARTCommand UARTComm::executeCommand(const CommFrame& frame) {
    switch (frame.functionCode) {
      // 处理硬件串口的逻辑
  
        case static_cast<uint8_t>(FunctionCode::READ):
           return handleReadCommand(frame);           
            break;
        case static_cast<uint8_t>(FunctionCode::WRITE):
            return handleWriteCommand(frame);            
            break;
        case static_cast<uint8_t>(FunctionCode::HEART):
            return handleHeartbeat(frame);             
            break;
        case static_cast<uint8_t>(FunctionCode::SUCCESEE):
            //serialComm->println("Operation success confirmation");
            return  {FunctionCode::SUCCESEE,frame.dataAddress,frame.data};
            break;
        case static_cast<uint8_t>(FunctionCode::FAILED):
            //serialComm->println("Operation exception confirmation");
            return  {FunctionCode::FAILED,frame.dataAddress,frame.data};
            break;
        case static_cast<uint8_t>(FunctionCode::CONFIRM):
            //serialComm->println("Function receive confirmation");
            return  {FunctionCode::CONFIRM,frame.dataAddress,INIT_DATA};
            break;
      
        case static_cast<uint8_t>(FunctionCode::FUNCTION):
            // Handle custom function           
            return handleFunctionCommand(frame); 
            break;
      
        default:
            //serialComm->println("Undefined function code");
            return  {FunctionCode::C_DEFAULT,INIT_DATA,INIT_DATA};
            break;
    }
    
}

void UARTComm::sendFrame(const CommFrame& frame) {
    static unsigned long lastSendTime = 0;
    const unsigned long sendInterval = 20; // 20ms最小发送间隔
    
    while (millis() - lastSendTime < sendInterval) {
        vTaskDelay(1);
    }
    lastSendTime = millis();
    
    //SerialGuard guard;
    CommFrame frameCopy = frame;
    uint8_t* framePtr = (uint8_t*)&frameCopy;
    frameCopy.crc = calculateCRC(framePtr, frame.hasData ? sizeof(CommFrame) - 4 : sizeof(CommFrame) - 5);
    
    if (frame.hasData) {
        serialComm->writeByte(framePtr, sizeof(CommFrame) - 4);
    } else {
        serialComm->writeByte(framePtr, sizeof(CommFrame) - 5);
    }
    //vTaskDelay(xDelay); // 短暂延时确保数据发送
    serialComm->writeByte((uint8_t*)&frameCopy.crc, 1);
    serialComm->writeByte((uint8_t*)&frameCopy.crc + 1, 1);
}

void UARTComm::sendHEXMessage(uint8_t sourceAddress, uint8_t targetAddress, uint8_t functionCode, uint8_t dataAddress, uint8_t data, bool hasData) {
     CommFrame frame(FIRSTBYTE, sourceAddress, targetAddress, functionCode, dataAddress, data, 0, hasData);
    sendFrame(frame);
}
void UARTComm::sendHEXRead( uint8_t targetAddress,  uint8_t dataAddress, uint8_t data) {
    CommFrame frame = {
        FIRSTBYTE,
        ADDmanager.localadd_value,
        targetAddress,
        static_cast<uint8_t>(FunctionCode::READ),
        dataAddress,
        data,
        0,
        true
    };
    sendFrame(frame);
}
void UARTComm::sendHEXWrite( uint8_t targetAddress,  uint8_t dataAddress, uint8_t data) {
    CommFrame frame = {
        FIRSTBYTE,
        ADDmanager.localadd_value,
        targetAddress,
        static_cast<uint8_t>(FunctionCode::WRITE),
        dataAddress,
        data,
        0,
        true
    };
    sendFrame(frame);
}
void UARTComm::sendHEXheart( uint8_t targetAddress) {
    CommFrame frame = {
        FIRSTBYTE,
        ADDmanager.localadd_value,
        targetAddress,
        static_cast<uint8_t>(FunctionCode::HEART),
        ADDmanager.windowType_value,
        VERSION,
        0,
        true
    };
    sendFrame(frame);
}

void UARTComm::sendscreenCommand( uint8_t dataAddress) {
    CommFrame frame = {
        FIRSTBYTE,
        0,
        0,
        static_cast<uint8_t>(FunctionCode::FUNCTION),
        dataAddress,
        0,
        0,
        false
    };
    sendFrame(frame);
}
void UARTComm::sendHEXfunction( uint8_t targetAddress, uint8_t dataAddress) {
    CommFrame frame = {
        FIRSTBYTE,
        ADDmanager.localadd_value,
        targetAddress,
        static_cast<uint8_t>(FunctionCode::FUNCTION),
        dataAddress,
        0,
        0,
        false
    };
    sendFrame(frame);
}
void UARTComm::sendCharMessage(const char* functionCode) {
    WindowCommand cmd;
    strncpy(cmd.function, functionCode, sizeof(cmd.function) - 1);
    cmd.function[sizeof(cmd.function) - 1] = '\0';
    char temp[10];
    snprintf(temp, sizeof(temp), "%s%s%s%s", cmd.address, cmd.function, cmd.length, cmd.data);

    uint16_t checksum = CRC16::lrc_sum(reinterpret_cast<const uint8_t*>(temp), strlen(temp));
    snprintf(cmd.lrc, sizeof(cmd.lrc), "%03d", checksum);

    
    sendCharFrame(cmd);
   
}

void UARTComm::sendCharFrame(const WindowCommand& cmd) {
    SerialGuard guard;
    //delay(300);
    serialComm->write(cmd.startByte);
    serialComm->writeChar(cmd.address);
    serialComm->writeChar(cmd.function);
    serialComm->writeChar(cmd.length);
    serialComm->writeChar(cmd.data);
    serialComm->writeChar(cmd.lrc);
    serialComm->write(cmd.endByte);
    //delay(120);
}
UARTCommand UARTComm::handleReadCommand(const CommFrame& frame) {
    uint8_t data;
    bool readSuccess=eeprommanager.readData(frame.dataAddress, &data, 1);
     // Assuming EEPROM.read returns a boolean indicating success

    FunctionCode responseCode = readSuccess ? FunctionCode::SUCCESEE : FunctionCode::FAILED;

    CommFrame responseFrame = {
        FIRSTBYTE,
        ADDmanager.localadd_value,
        frame.sourceAddress,
        static_cast<uint8_t>(responseCode),
        frame.dataAddress,
        data,
        0,
        true
    };
    sendResponse(responseFrame);
    return  {responseCode,frame.dataAddress,data};


}

UARTCommand UARTComm::handleWriteCommand(const CommFrame& frame) { 
    uint8_t data=frame.data,olddata;
    bool readSuccess;
    eeprommanager.readData(frame.dataAddress, &olddata, 1);

    if(data!=olddata)
    {
    readSuccess=eeprommanager.writeData(frame.dataAddress,&data,1);
    }
    else
    readSuccess=1;

    FunctionCode responseCode = readSuccess ? FunctionCode::SUCCESEE : FunctionCode::FAILED;

    CommFrame responseFrame = {
        FIRSTBYTE,
        ADDmanager.localadd_value,
        frame.sourceAddress,
        static_cast<uint8_t>(responseCode),
        frame.dataAddress,
        frame.data,
        0,
        true
    };
    sendResponse(responseFrame);
    return {responseCode,frame.dataAddress,frame.data};
}
UARTCommand UARTComm::handleFunctionCommand(const CommFrame& frame) { 
    
    CommFrame responseFrame = {
        FIRSTBYTE,
        ADDmanager.localadd_value,
        frame.sourceAddress,
        static_cast<uint8_t>(FunctionCode::CONFIRM),
        frame.dataAddress,
        frame.data,
        0,
        false
    };
    if(!isCom1Serial)
    sendResponse(responseFrame);

    return {FunctionCode::FUNCTION,frame.dataAddress,INIT_DATA};
}
UARTCommand UARTComm::handleHeartbeat(const CommFrame& frame) {
    CommFrame responseFrame = {
        FIRSTBYTE,
        ADDmanager.localadd_value,
        frame.sourceAddress,
        static_cast<uint8_t>(FunctionCode::HEART),
        ADDmanager.windowType_value,
        VERSION,
        0,
        true
    };
    //sendResponse(responseFrame);
    return {FunctionCode::HEART,ADDmanager.windowType_value,VERSION};
}

uint16_t UARTComm::calculateCRC(const uint8_t* data,uint16_t length) {
    return CRC16::crc16_modbus(data, length);
}

bool UARTComm::verifyCRC(const uint8_t* data,uint16_t length, uint16_t receivedCRC) {
    uint16_t calculatedCRC = calculateCRC(data, length);
    return receivedCRC == calculatedCRC;
}

void UARTComm::sendResponse(const CommFrame& responseFrame) {
    sendFrame(responseFrame);
}
