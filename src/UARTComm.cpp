// UARTComm.cpp
#include "UARTComm.h"
#include <cstdlib>

// 实现独立的收发器类
UARTTransceiver::UARTTransceiver(SerialComm *serialComm) : serialComm(serialComm) {}

bool UARTTransceiver::receiveBytes(uint8_t *buffer, uint16_t *bytesRead, uint16_t maxBytes)
{
    *bytesRead = 0;
    while (serialComm->available() && *bytesRead < maxBytes)
    {
        buffer[(*bytesRead)++] = serialComm->read();
    }
    return *bytesRead > 0;
}

bool UARTTransceiver::sendBytes(const uint8_t *buffer, uint16_t length)
{
    // 限制发送频率
    unsigned long currentTime = millis();
    if (currentTime - lastSendTime < sendInterval)
    {
        vTaskDelay(1);
    }
    lastSendTime = millis();

    return serialComm->writeByte(buffer, length) == length;
}

bool UARTTransceiver::sendByte(uint8_t byte)
{
    return sendBytes(&byte, 1);
}

void UARTTransceiver::sendString(const char *str)
{
    serialComm->writeChar(str);
}

// UARTComm类方法
UARTComm::UARTComm(SerialComm *serialCommInstance, uint32_t baudRate, uint8_t rx_pin, uint8_t tx_pin, uint8_t comNumber)
    : serialComm(serialCommInstance), BaudRate(baudRate), rx_pin(rx_pin), tx_pin(tx_pin), comNumber(comNumber)
{
    transceiver = new UARTTransceiver(serialCommInstance);
}

void UARTComm::init()
{
    serialComm->begin(BaudRate, rx_pin, tx_pin);
    delay(50); // 给串口初始化一些时间
    eeprommanager.begin();
    Serial.println("UARTComm initialized");
    // 清空缓冲区
    clearSerialBuffer();
    rxHead = 0;
    rxTail = 0;
}

void UARTComm::clearSerialBuffer()
{
    while (serialComm->available())
    {
        serialComm->read();
    }
    // 重置接收状态
    resetReceiveState();
}

void UARTComm::resetReceiveState()
{
    receiveState = ReceiveState::WAIT_FOR_HEADER;
    bytesReceived = 0;
    receivedCRC = 0;
    expectedDataLength = 0;
}

void UARTComm::bufferByte(uint8_t byte)
{
    rxBuffer[rxHead] = byte;
    rxHead = (rxHead + 1) % UART_RX_BUFFER_SIZE;
    // 检查缓冲区溢出
    if (rxHead == rxTail)
    {
        // 缓冲区已满，移动尾指针
        rxTail = (rxTail + 1) % UART_RX_BUFFER_SIZE;
    }
}

uint8_t UARTComm::readBufferedByte()
{
    if (rxHead == rxTail)
    {
        return 0; // 缓冲区为空
    }
    uint8_t byte = rxBuffer[rxTail];
    rxTail = (rxTail + 1) % UART_RX_BUFFER_SIZE;
    return byte;
}

uint16_t UARTComm::availableBuffered()
{
    return (rxHead - rxTail + UART_RX_BUFFER_SIZE) % UART_RX_BUFFER_SIZE;
}

bool UARTComm::update()
{
    const unsigned long updateInterval = 20; // 减少为20ms
    bool dataProcessed = false;

    unsigned long currentTime = millis();
    if (currentTime - lastUpdateTime < updateInterval)
    {
        return false;
    }
    lastUpdateTime = currentTime;

    // 从串口读取数据到缓冲区
    if (serialComm->available())
    {
        uint16_t bytesRead = 0;
        uint8_t tempBuffer[64]; // 临时缓冲区

        // 使用收发器读取数据
        if (transceiver->receiveBytes(tempBuffer, &bytesRead, 64))
        {
            // 将数据存入内部缓冲区
            for (uint16_t i = 0; i < bytesRead; i++)
            {
                bufferByte(tempBuffer[i]);
            }
            dataProcessed = true;
        }
    }

    // 处理缓冲区中的数据
    uint16_t processedBytes = 0;
    const uint16_t maxBytesToProcess = 64; // 增加处理字节数

    while (availableBuffered() > 0 && processedBytes < maxBytesToProcess)
    {
        uint8_t byte = readBufferedByte();
        processedBytes++;

        // 处理接收到的字节
        if (processReceivedByte(byte))
        {
            dataProcessed = true;
        }
    }

    return dataProcessed;
}

UARTCommand UARTComm::processWindowCommandMessage(const char *message, size_t length)
{
    // 检查长度是否正确
    if (length != WINDOW_CMD_LEN)
    {
        Serial.print("WindowCommand: message is wrong, length=");
        Serial.println(length);
        return {FunctionCode::C_DEFAULT, INIT_DATA, INIT_DATA};
    }

    WindowCommand cmd;
    size_t index = 0;

    // 检查 WindowCommand 的起始符是否为 '('，并紧跟 '001'
    if (message[index++] != '(')
    {
        Serial.println("WindowCommand: start byte is wrong");
        return {FunctionCode::C_DEFAULT, INIT_DATA, INIT_DATA};
    }
    // 检查 address 字段是否为 '001'
    if (strncmp(&message[index], "001", 3) != 0)
    {
       // Serial.println("WindowCommand: address is wrong");
        return {FunctionCode::C_DEFAULT, INIT_DATA, INIT_DATA};
    }
    index += 3;

    // 解析 function：复制2个字符并添加结束符
    memcpy(cmd.function, &message[index], 2);
    cmd.function[2] = '\0';
    index += 2;

    // 解析 length：复制2个字符并添加结束符
    memcpy(cmd.length, &message[index], 2);
    cmd.length[2] = '\0';
    index += 2;

    // 解析 data：复制1个字符并添加结束符
    memcpy(cmd.data, &message[index], 1);
    cmd.data[1] = '\0';
    index += 1;

    // 解析 lrc：复制3个字符并添加结束符
    memcpy(cmd.lrc, &message[index], 3);
    cmd.lrc[3] = '\0';
    index += 3;

    cmd.endByte = message[index++];

    return processWindowCommand(cmd);
}

UARTCommand UARTComm::processWindowCommand(const WindowCommand &cmd)
{
    // 检查起始和结束字符
    if (cmd.startByte != '(' || cmd.endByte != ')')
    {
        Serial.println("WindowCommand: the start or end byte is wrong");
        return {FunctionCode::C_DEFAULT, INIT_DATA, INIT_DATA};
    }

    // 拼接 address, function, length, data 字段用于 LRC 校验
    String combined = String(cmd.address) + String(cmd.function) + String(cmd.length) + String(cmd.data);
    uint16_t computedLrc = CRC16::lrc_sum((const uint8_t *)combined.c_str(), combined.length());
    uint16_t providedLrc = atoi(cmd.lrc);

    if (computedLrc != providedLrc)
    {
        Serial.print("WindowCommand:  LRC is wrong, caculate LRC=");
        Serial.print(computedLrc);
        Serial.print(",receive LRC=");
        Serial.println(providedLrc);
        return {FunctionCode::C_DEFAULT, INIT_DATA, INIT_DATA};
    }

    // 校验成功后，从 function 字段（2个字符）提取功能码
    // 这里假定 function 字段以十六进制表示，如 "03" 对应 0x03
    uint8_t funcCode = (uint8_t)strtol(cmd.function, NULL, 16);

    // 可根据需要进一步解析 length 和 data 字段，这里示例仅返回 function 转换结果
    return {FunctionCode::FUNCTION, funcCode, INIT_DATA};
}

bool UARTComm::processReceivedByte(uint8_t byte)
{
    bool frameComplete = false;

    switch (receiveState)
    {
    case ReceiveState::WAIT_FOR_HEADER:
        if (byte == FIRSTBYTE)
        {
            receivedFrame.header = byte;
            bytesReceived = 1;
            // if (comNumber==1) {
            //     mySerial.print("COM1 data: ");
            //     mySerial.print(receivedFrame.header, HEX);
            // }
            receiveState = ReceiveState::WAIT_FOR_SOURCE_ADDRESS;
        }
        break;

    case ReceiveState::WAIT_FOR_SOURCE_ADDRESS:
        receivedFrame.sourceAddress = byte;
        if (receivedFrame.sourceAddress == THE_THIRD_PART || receivedFrame.sourceAddress == TARGET_ADDRESS)
        {
            bytesReceived++;
            receiveState = ReceiveState::WAIT_FOR_TARGET_ADDRESS;
        }
        else
        {
            // 无效的源地址，尝试恢复
            if (byte == FIRSTBYTE)
            {
                receivedFrame.header = byte;
                bytesReceived = 1;
                // 保持在等待源地址状态
            }
            else
            {
                // if (comNumber==1) {
                //     mySerial.print("COM1 source address is wrong: ");
                //     mySerial.print(receivedFrame.sourceAddress, HEX);
                // }
                receiveState = ReceiveState::WAIT_FOR_HEADER;
            }
        }
        break;

    case ReceiveState::WAIT_FOR_TARGET_ADDRESS:
        receivedFrame.targetAddress = byte;

        if (receivedFrame.targetAddress == ADDmanager.localadd_value)
        {
            bytesReceived++;
            receiveState = ReceiveState::WAIT_FOR_FUNCTION_CODE;
        }
        else
        {
            // 无效的目标地址，尝试恢复
            if (byte == FIRSTBYTE)
            {
                receivedFrame.header = byte;
                bytesReceived = 1;
                receiveState = ReceiveState::WAIT_FOR_SOURCE_ADDRESS;
            }
            else
            {
                receiveState = ReceiveState::WAIT_FOR_HEADER;
            }
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
        if (bytesReceived < expectedFrameLength())
        {
            receivedCRC = (receivedCRC << 8) | byte;
            bytesReceived++;
            if (bytesReceived >= expectedFrameLength())
            {
                // 调整CRC字节序
                receivedCRC = (receivedCRC >> 8) | (receivedCRC << 8);

                // 添加调试输出
                // if (comNumber==1) {
                //     mySerial.print("COM1 CRC: ");
                //     mySerial.print(validateFrame() ? "succese" : "failed");
                //     mySerial.print(" functioncode: 0x");
                //     mySerial.print(receivedFrame.functionCode, HEX);
                //     mySerial.print(" dataaddrese: 0x");
                //     mySerial.println(receivedFrame.dataAddress, HEX);
                // }

                // 验证CRC
                if (validateFrame())
                {
                    UARTCommand newCmd = executeCommand(receivedFrame);
                    if (callback)
                    {
                        callback(newCmd);
                    }
                    frameComplete = true;
                }
                else
                {
                    // CRC验证失败，记录错误，但不打印以减少开销
                    receiveState = ReceiveState::ERROR_RECOVERY;
                    return false;
                }

                // 重置接收状态，准备接收下一帧
                resetReceiveState();
            }
        }
        break;

    case ReceiveState::ERROR_RECOVERY:
        // 尝试恢复同步
        if (byte == FIRSTBYTE)
        {
            receivedFrame.header = byte;
            bytesReceived = 1;
            receiveState = ReceiveState::WAIT_FOR_SOURCE_ADDRESS;
        }
        break;

    default:
        resetReceiveState();
        break;
    }
    // 如果处于 WindowCommand 接收状态，则直接将字节存入 windowBuffer
    if (inWindowCommandMode)
    {
        windowBuffer[windowBufferIndex++] = byte;
        // 当 windowBuffer 收满时，解析完整的 WindowCommand 消息
        if (windowBufferIndex >= WINDOW_CMD_LEN)
        {
            UARTCommand cmd = processWindowCommandMessage(windowBuffer, WINDOW_CMD_LEN);
            // 重置 WindowCommand 接收状态
            inWindowCommandMode = false;
            windowBufferIndex = 0;
            // 如果解析有效，则调用回调
            if (cmd.responseCode != FunctionCode::C_DEFAULT && callback)
            {
                callback(cmd);
                frameComplete = true;
            }
            // return true;
        }
        // return true; // 已处理该字节
    }

    // 若接收到的是 WindowCommand 的起始符 '('，进入 WindowCommand 模式
    if (byte == '(')
    {
        // 预读接下来的字节，检查是否是有效的 WindowCommand 地址 '001'
        // if (availableBuffered() >= 3)
        // { // 确保接下来有足够的字节读取 '001'
        //     uint8_t address[3];
        //     address[0] = readBufferedByte();
        //     address[1] = readBufferedByte();
        //     address[2] = readBufferedByte();

        //     // 检查接下来三个字节是否为 '001'
        //     if (address[0] == '0' && address[1] == '0' && address[2] == '1')
        //     {
                // 地址是 '001'，我们认为是 WindowCommand，进入 WindowCommand 模式
                inWindowCommandMode = true;
                windowBufferIndex = 0;
                windowBuffer[windowBufferIndex++] = byte;
                // return true;
        //     }
        //     else
        //     {
        //         // 如果不是有效的 WindowCommand 地址，则跳过
        //         // 恢复状态继续处理其他命令
        //          resetReceiveState();
        //     }
        // }
        // else
        // {
        //     // 数据不足，跳过此字节，继续等待有效数据
        //     // return false;
        // }
    }

    return frameComplete;
}

bool UARTComm::validateFrame()
{
    bool isValid = verifyCRC(reinterpret_cast<uint8_t *>(&receivedFrame), bytesReceived - 2, receivedCRC);

    // 对于COM1串口，记录CRC错误但不中止处理
    // if (!isValid && (comNumber==1)) {
    //     // 只对特定类型的命令允许继续处理 (如Function命令)
    //     if (receivedFrame.functionCode == static_cast<uint8_t>(FunctionCode::FUNCTION)) {
    //         mySerial.println("COM1 CRC is wrong but Functioncode execute continue");
    //         // 有效帧的基本检查 - 帧头和地址检查
    //         return receivedFrame.header == FIRSTBYTE &&
    //                receivedFrame.targetAddress == ADDmanager.localadd_value;
    //     }
    // }

    return isValid;
}

bool UARTComm::frameRequiresData(uint8_t functionCode)
{
    // 根据 functionCode 决定是否需要数据
    switch (functionCode)
    {
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

uint16_t UARTComm::expectedFrameLength()
{
    // 根据协议返回期望的帧长度
    // 例如，header + source + target + function + dataaddress +data+ crc
    return 5 + expectedDataLength + 2;
}

UARTCommand UARTComm::executeCommand(const CommFrame &frame)
{
    switch (frame.functionCode)
    {
    case static_cast<uint8_t>(FunctionCode::READ):
        return handleReadCommand(frame);
    case static_cast<uint8_t>(FunctionCode::WRITE):
        return handleWriteCommand(frame);
    case static_cast<uint8_t>(FunctionCode::HEART):
        return handleHeartbeat(frame);
    case static_cast<uint8_t>(FunctionCode::SUCCESEE):
        return {FunctionCode::SUCCESEE, frame.dataAddress, frame.data};
    case static_cast<uint8_t>(FunctionCode::FAILED):
        return {FunctionCode::FAILED, frame.dataAddress, frame.data};
    case static_cast<uint8_t>(FunctionCode::CONFIRM):
        return {FunctionCode::CONFIRM, frame.dataAddress, INIT_DATA};
    case static_cast<uint8_t>(FunctionCode::FUNCTION):
        return handleFunctionCommand(frame);
    default:
        return {FunctionCode::C_DEFAULT, INIT_DATA, INIT_DATA};
    }
}

void UARTComm::sendFrame(const CommFrame &frame)
{
    CommFrame frameCopy = frame;
    uint8_t *framePtr = (uint8_t *)&frameCopy;
    frameCopy.crc = calculateCRC(framePtr, frame.hasData ? sizeof(CommFrame) - 4 : sizeof(CommFrame) - 5);

    if (frame.hasData)
    {
        transceiver->sendBytes(framePtr, sizeof(CommFrame) - 4);
    }
    else
    {
        transceiver->sendBytes(framePtr, sizeof(CommFrame) - 5);
    }

    uint8_t crcBytes[2];
    crcBytes[0] = (uint8_t)frameCopy.crc;
    crcBytes[1] = (uint8_t)(frameCopy.crc >> 8);
    transceiver->sendBytes(crcBytes, 2);
}

void UARTComm::sendHEXMessage(uint8_t sourceAddress, uint8_t targetAddress, uint8_t functionCode, uint8_t dataAddress, uint8_t data, bool hasData)
{
    CommFrame frame(FIRSTBYTE, sourceAddress, targetAddress, functionCode, dataAddress, data, 0, hasData);
    sendFrame(frame);
}

void UARTComm::sendHEXRead(uint8_t targetAddress, uint8_t dataAddress, uint8_t data)
{
    CommFrame frame = {
        FIRSTBYTE,
        ADDmanager.localadd_value,
        targetAddress,
        static_cast<uint8_t>(FunctionCode::READ),
        dataAddress,
        data,
        0,
        true};
    sendFrame(frame);
}

void UARTComm::sendHEXWrite(uint8_t targetAddress, uint8_t dataAddress, uint8_t data)
{
    CommFrame frame = {
        FIRSTBYTE,
        ADDmanager.localadd_value,
        targetAddress,
        static_cast<uint8_t>(FunctionCode::WRITE),
        dataAddress,
        data,
        0,
        true};
    sendFrame(frame);
}

void UARTComm::sendHEXheart(uint8_t targetAddress)
{
    CommFrame frame = {
        FIRSTBYTE,
        ADDmanager.localadd_value,
        targetAddress,
        static_cast<uint8_t>(FunctionCode::HEART),
        ADDmanager.windowType_value,
        VERSION,
        0,
        true};
    sendFrame(frame);
}

void UARTComm::sendscreenCommand(uint8_t dataAddress)
{
    CommFrame frame = {
        FIRSTBYTE,
        0,
        0,
        static_cast<uint8_t>(FunctionCode::FUNCTION),
        dataAddress,
        0,
        0,
        false};
    sendFrame(frame);
}

void UARTComm::sendHEXfunction(uint8_t targetAddress, uint8_t dataAddress)
{
    CommFrame frame = {
        FIRSTBYTE,
        ADDmanager.localadd_value,
        targetAddress,
        static_cast<uint8_t>(FunctionCode::FUNCTION),
        dataAddress,
        0,
        0,
        false};
    sendFrame(frame);
}

void UARTComm::sendCharMessage(const char *functionCode)
{
    WindowCommand cmd;
    strncpy(cmd.function, functionCode, sizeof(cmd.function) - 1);
    cmd.function[sizeof(cmd.function) - 1] = '\0';
    char temp[10];
    snprintf(temp, sizeof(temp), "%s%s%s%s", cmd.address, cmd.function, cmd.length, cmd.data);

    uint16_t checksum = CRC16::lrc_sum(reinterpret_cast<const uint8_t *>(temp), strlen(temp));
    snprintf(cmd.lrc, sizeof(cmd.lrc), "%03d", checksum);

    sendCharFrame(cmd);
}

void UARTComm::sendCharFrame(const WindowCommand &cmd)
{
    transceiver->sendByte(cmd.startByte);
    transceiver->sendString(cmd.address);
    transceiver->sendString(cmd.function);
    transceiver->sendString(cmd.length);
    transceiver->sendString(cmd.data);
    transceiver->sendString(cmd.lrc);
    transceiver->sendByte(cmd.endByte);
}

UARTCommand UARTComm::handleReadCommand(const CommFrame &frame)
{
    uint8_t data;
    bool readSuccess = eeprommanager.readData(frame.dataAddress, &data, 1);

    FunctionCode responseCode = readSuccess ? FunctionCode::SUCCESEE : FunctionCode::FAILED;

    CommFrame responseFrame = {
        FIRSTBYTE,
        ADDmanager.localadd_value,
        frame.sourceAddress,
        static_cast<uint8_t>(responseCode),
        frame.dataAddress,
        data,
        0,
        true};
    sendResponse(responseFrame);
    return {responseCode, frame.dataAddress, data};
}

UARTCommand UARTComm::handleWriteCommand(const CommFrame &frame)
{
    uint8_t data = frame.data, olddata;
    bool readSuccess;
    eeprommanager.readData(frame.dataAddress, &olddata, 1);

    if (data != olddata)
    {
        readSuccess = eeprommanager.writeData(frame.dataAddress, &data, 1);
    }
    else
    {
        readSuccess = 1;
    }

    FunctionCode responseCode = readSuccess ? FunctionCode::SUCCESEE : FunctionCode::FAILED;

    CommFrame responseFrame = {
        FIRSTBYTE,
        ADDmanager.localadd_value,
        frame.sourceAddress,
        static_cast<uint8_t>(responseCode),
        frame.dataAddress,
        frame.data,
        0,
        true};
    sendResponse(responseFrame);
    return {responseCode, frame.dataAddress, frame.data};
}

UARTCommand UARTComm::handleFunctionCommand(const CommFrame &frame)
{
    CommFrame responseFrame = {
        FIRSTBYTE,
        ADDmanager.localadd_value,
        frame.sourceAddress,
        static_cast<uint8_t>(FunctionCode::CONFIRM),
        frame.dataAddress,
        frame.data,
        0,
        false};
    if (comNumber == 0)
    {
        sendResponse(responseFrame);
    }

    return {FunctionCode::FUNCTION, frame.dataAddress, INIT_DATA};
}

UARTCommand UARTComm::handleHeartbeat(const CommFrame &frame)
{
    CommFrame responseFrame = {
        FIRSTBYTE,
        ADDmanager.localadd_value,
        frame.sourceAddress,
        static_cast<uint8_t>(FunctionCode::HEART),
        ADDmanager.windowType_value,
        VERSION,
        0,
        true};
    // sendResponse(responseFrame);
    return {FunctionCode::HEART, ADDmanager.windowType_value, VERSION};
}

uint16_t UARTComm::calculateCRC(const uint8_t *data, uint16_t length)
{
    return CRC16::crc16_modbus(data, length);
}

bool UARTComm::verifyCRC(const uint8_t *data, uint16_t length, uint16_t receivedCRC)
{
    uint16_t calculatedCRC = calculateCRC(data, length);
    return receivedCRC == calculatedCRC;
}

void UARTComm::sendResponse(const CommFrame &responseFrame)
{
    sendFrame(responseFrame);
}
