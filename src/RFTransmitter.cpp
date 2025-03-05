#include "RFTransmitter.h"
setRFpara setRFPara2;
RFTransmitter::RFTransmitter(uint16_t outputPin,address_Manager &AddManager)
    : outputPin(outputPin),AddManager(AddManager)
{
}
void RFTransmitter::rfsend_build(uint8_t rfWorkMode)
{
    setRFPara2.setRFParameters(rfWorkMode);
    rfStorageManager.loadRFData(ADDmanager);
    tx_whatever = rfsend_builder(
        RfSendEncoding::TRIBIT,
        outputPin,
        RFSEND_DEFAULT_CONVENTION, // Do we want to invert 0 and 1 bits? No.
        txParams->dataLen,         // Number of sendings
        nullptr,                   // No callback to keep/stop sending (if you want to send
                                   // SO LONG AS a button is pressed, the function reading the
                                   // button state is to be put here).

        txParams->initseq,
        txParams->lo_prefix,
        txParams->hi_prefix,
        0, // first_lo_ign
        txParams->lo_short,
        txParams->lo_long,
        txParams->hi_short,
        txParams->hi_long,
        txParams->lo_last,
        txParams->sep,
        txParams->nb_bits);
    //if (tx_whatever->send(txParams->dataLen, RF_buffer[0]))
        ;
};

void RFTransmitter::setup()
{
    pinMode(outputPin, OUTPUT);
    rfsend_build(ADDmanager.RFworkingMode_value);
    // mySerial.println("RF transmitter mode is ok. ");
}

void RFTransmitter::enqueueCommand(RFCommand command)
{
    if (!isQueueFull())
    {
        // 检查当前队列尾部的命令是否与新命令重复
        if (queueTail != queueHead)
        {                                                                   // 队列中有命令时才做比较
            uint16_t prevIndex = (queueTail - 1 + QUEUE_SIZE) % QUEUE_SIZE; // 获取队列尾部上一个命令的索引
            RFCommand lastCommand = commandQueue[prevIndex];

            // 比较 index, group 和 data 是否相同
            if (lastCommand.index == command.index &&
                lastCommand.group == command.group &&
                memcmp(lastCommand.data, command.data, txParams->dataLen) == 0)
            {
                // 如果完全相同，跳过此命令
                Serial.println("Duplicate command, skipping enqueue.");
                return;
            }
        }

        // 如果没有重复，正常入列
        commandQueue[queueTail] = command;
        queueTail = (queueTail + 1) % QUEUE_SIZE;
    }
    else
    {
        // 处理队列溢出，例如拒绝新命令或覆盖最早的命令
        Serial.println("Queue full, command rejected.");
    }
}

RFCommand RFTransmitter::dequeueCommand()
{
    RFCommand command = commandQueue[queueHead];
    queueHead = (queueHead + 1) % QUEUE_SIZE;
    return command;
}

void RFTransmitter::sendCode(Command index, uint8_t *data, uint8_t group)
{
    if ((ADDmanager.RFpairingMode_value == static_cast<uint8_t>(Pairing::PAIR_OUT_TO_WORK)) && (ADDmanager.RFworkingMode_value == static_cast<uint8_t>(RFworkMode::HOPO_TRANSMITTER) || ADDmanager.RFworkingMode_value == static_cast<uint8_t>(RFworkMode::HANS_TRANSMITTER) || ADDmanager.RFworkingMode_value == static_cast<uint8_t>(RFworkMode::HANS_BOTH) || ADDmanager.RFworkingMode_value == static_cast<uint8_t>(RFworkMode::HANS_HOPO) || ADDmanager.RFworkingMode_value == static_cast<uint8_t>(RFworkMode::HOPO_HANS)))
    {
        RFCommand newCommand;
        newCommand.index = index;
        for (uint16_t i = 0; i < txParams->dataLen; i++)
        {
            newCommand.data[i] = data[i];
        }
        newCommand.group = group;

        // Enqueue the command for sending
        enqueueCommand(newCommand);
    }
}

void RFTransmitter::update()
{
    if (!isQueueEmpty() && !isSending)
    {
        // Dequeue the next command
        RFCommand command = dequeueCommand();

        // Prepare the data for sending
        uint16_t temp = (static_cast<uint8_t>(command.index)) - command.group * 0x10;
        memcpy(currentSendData, command.data, txParams->dataLen);
        switch (static_cast<RFworkMode>(ADDmanager.RFworkingMode_value))
        {
        case RFworkMode::HOPO_HANS:
        case RFworkMode::HANS_BOTH:
        case RFworkMode::HANS_TRANSMITTER:
            prepareSendDataHans(currentSendData, command.data, temp);
            break;
        case RFworkMode::HANS_HOPO:
        case RFworkMode::HOPO_TRANSMITTER:
            prepareSendDataHopo(currentSendData, command.data, temp);
            break;
            /*
           case :
               prepareSendDataGu(currentSendData, command.data, temp);
               break;
               */
        default:
            break;
        }
        /*Serial.print("currentSendData[i]");
        for(uint16_t i=0;i < RF_NUM; i++)
        Serial.print(currentSendData[i],HEX);
        Serial.println("""");
        */
        // Initialize sending state
        isSending = true;
        sendStep = 0;
        lastSendTime = millis(); // Start sending immediately
    }

    // Handle the sending process
    if (isSending)
    {
        uint32_t currentTime = millis();
        if (sendStep == 0)
        {
            if (currentTime - lastSendTime >= sendInterval)
            {
                tx_whatever->send(txParams->dataLen, currentSendData);
                sendStep = 1;
                lastSendTime = millis();
            }
        }
        else if (sendStep == 1)
        {
            if (currentTime - lastSendTime >= sendInterval)
            {
                tx_whatever->send(txParams->dataLen, currentSendData);
                isSending = false;
            }
        }
    }
}
void RFTransmitter::prepareSendDataHans(uint8_t *send_data, uint8_t *data, uint16_t temp)
{
    send_data[txParams->dataLen - 1] = data[txParams->dataLen - 1] - 0x3C - (3 - temp) * 0x11;
}

void RFTransmitter::prepareSendDataHopo(uint8_t *send_data, uint8_t *data, uint16_t temp)
{
    data[txParams->dataLen - 1] &= 0xF0;
    data[txParams->dataLen - 1] = data[txParams->dataLen - 1] + (0x08 >> temp);
    send_data[txParams->dataLen - 1] = data[txParams->dataLen - 1];
}

void RFTransmitter::prepareSendDataGu(uint8_t *send_data, uint8_t *data, uint16_t temp)
{
    send_data[txParams->dataLen - 1] = data[txParams->dataLen - 1] + ((txParams->dataLen - 1) * (3 - temp) - (temp - 2) * (temp - 3) / 2) * 0x10;
}
