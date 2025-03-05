#include "RFStorageManager.h"

RFStorageManager::RFStorageManager(uint16_t startAddr)
    : startAddress(startAddr)
{
}

bool RFStorageManager::loadRFData()
{
    uint16_t addr = startAddress;

    for (size_t i = 0; i < NUM_GROUPS; i++)
    {

        RFDataPacketHeader header;
        if (!EEPROMManager::readData(addr, (uint8_t *)&header, sizeof(header)))
        {
            initRFData(i + 1, ADDmanager);
           // break; // 读取失败或到达 EEPROM 尾部
        }
        else if (header.group < NUM_GROUPS && header.dataLen <= RF_NUM_DEFAULT) // 如果头部模式匹配，并且分组号在范围内
        {
            EEPROMManager::readData(addr + sizeof(header), ADDmanager.RF_buffer[header.group], header.dataLen);
        }
        // 修正偏移量计算，使用固定步长
        addr += (sizeof(header) + RF_NUM_DEFAULT);

        for (size_t j = 0; j < RF_NUM_DEFAULT; j++)
        {
            mySerial.printf("%02X ", ADDmanager.RF_buffer[i][j]);
        }
        mySerial.println();
    }
    return true;
}

bool RFStorageManager::saveRFData(RfSendEncoding mode, uint8_t group, uint8_t *data, uint8_t dataLen)
{
    // 计算写入地址：此处假设每个模式预留固定空间（模式内按组顺序排列）
    uint16_t addr = startAddress + (group - 1) * (sizeof(RFDataPacketHeader) + RF_NUM_DEFAULT);
    RFDataPacketHeader header = {mode, (u_int8_t)(group-1), dataLen};
    if (!EEPROMManager::writeData(addr, (uint8_t *)&header, sizeof(header)))
    {
        return false;
    }
    else
    {
        mySerial.println("write header success");
    }
    if (!EEPROMManager::writeData(addr + sizeof(header), data, dataLen))
    {
        return false;
    }
    else
    {
        mySerial.println("write data success");
    }

    // 打印保存的数据
    const char *modeStr;
    switch (mode)
    {
    case RfSendEncoding::TRIBIT:
        modeStr = "TRIBIT";
        break;
    case RfSendEncoding::TRIBIT_INVERTED:
        modeStr = "TRIBIT_INVERTED";
        break;
    case RfSendEncoding::MANCHESTER:
        modeStr = "MANCHESTER";
        break;
    default:
        modeStr = "UNKNOWN";
        break;
    }

    mySerial.print("Saved mode: ");
    mySerial.println(modeStr);
    mySerial.print("Saved data for group ");
    mySerial.print(group);
    mySerial.print(": ");
    for (uint8_t i = 0; i < dataLen; ++i)
    {
        mySerial.print(data[i], HEX);
        mySerial.print(" ");
    }
    mySerial.println();

    return true;
}

void RFStorageManager::generateRandomValues(uint8_t *buffer, uint16_t length, uint8_t fixedLastValue)
{
    for (uint16_t i = 0; i < length - 3; ++i)
    {
        buffer[i] = random(0, 256); // 生成0到255之间的随机数
    }
    buffer[length - 3] = fixedLastValue; // 最后一位固定为指定值
    buffer[length - 2] = fixedLastValue; // 最后一位固定为指定值
    buffer[length - 1] = fixedLastValue; // 最后一位固定为指定值
}

void RFStorageManager::initRFData(uint8_t group, address_Manager &manager)
{
    if (group == static_cast<uint8_t>(Pairing::HANS_1) ||
        group == static_cast<uint8_t>(Pairing::HANS_2) ||
        group == static_cast<uint8_t>(Pairing::HANS_WIRELESS))
    {
        uint8_t hansValues[NUM_GROUPS][RF_NUM_DEFAULT] = {
            {0x00, 0x4b, 0xac, 0x21, 0x66},
            {0x4F, 0x27, 0x84, 0x83, 0xE6},
            {0x4F, 0x27, 0x84, 0x84, 0xE6},
            {0x4F, 0x27, 0x84, 0x85, 0xE6},
            {0x00, 0x4b, 0xac, 0x21, 0x66},
            {0x4F, 0x27, 0x84, 0x85, 0xE6}};
        memcpy(manager.RF_buffer[group - 1], hansValues[group - 1], sizeof(hansValues[group - 1]));
        // 注释下面这行，避免重复调用保存函数
        //saveRFData(RfSendEncoding::TRIBIT, group, manager.RF_buffer[group - 1], RF_NUM_DEFAULT);
    }
    else if (group == static_cast<uint8_t>(Pairing::HOPO_1) ||
             group == static_cast<uint8_t>(Pairing::HOPO_2) ||
             group == static_cast<uint8_t>(Pairing::HOPO_WIRELESS))
    {
        uint8_t hopoValues[NUM_GROUPS][RF_NUM_DEFAULT];
        for (uint16_t i = 0; i < NUM_GROUPS; ++i)
        {
            generateRandomValues(hopoValues[i], RF_NUM_DEFAULT, 0x01);
        }
        memcpy(manager.RF_buffer[group - 1], hopoValues[group - 1], sizeof(hopoValues[group - 1]));
        // 注释下面这行，避免重复调用保存函数
        //saveRFData(RfSendEncoding::TRIBIT, group, manager.RF_buffer[group - 1], RF_NUM_DEFAULT);
    }
}
