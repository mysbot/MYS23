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
        if (!EEPROMManager::readData(addr, (uint8_t*)&header, sizeof(header))) {
           initRFData(i+1);
            break; // 读取失败或到达 EEPROM 尾部
        }
        // 如果头部模式匹配，并且分组号在范围内
        if ( header.group < NUM_GROUPS && header.dataLen <= RF_NUM_DEFAULT) {
            EEPROMManager::readData(addr + sizeof(header), RF_buffer[header.group], header.dataLen);
        }
        // 移动到下一个数据包位置
        addr += (sizeof(header) + header.dataLen)*i;
    }
    return true;
}

bool RFStorageManager::saveRFData(RfSendEncoding mode, uint8_t group,  uint8_t* data, uint8_t dataLen)
{
    // 计算写入地址：此处假设每个模式预留固定空间（模式内按组顺序排列）
    uint16_t addr = startAddress +  (group-1)*(sizeof(RFDataPacketHeader)+RF_NUM_DEFAULT );
    RFDataPacketHeader header = { mode, group, dataLen };
    if (!EEPROMManager::writeData(addr, (uint8_t*)&header, sizeof(header))) {
        return false;
    }
    if (!EEPROMManager::writeData(addr + sizeof(header), data, dataLen)) {
        return false;
    }
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

void RFStorageManager::initRFData(uint8_t group)
{
    if (group == static_cast<uint8_t>(Pairing::HANS_1)||group==static_cast<uint8_t>(Pairing::HANS_2)||group==static_cast<uint8_t>(Pairing::HANS_WIRELESS))
    {
        uint8_t hansValues[NUM_GROUPS][RF_NUM_DEFAULT] = {
            {0x4F, 0x27, 0x84, 0x82, 0xE6},
            {0x4F, 0x27, 0x84, 0x83, 0xE6},
            {0x4F, 0x27, 0x84, 0x84, 0xE6},
            {0x4F, 0x27, 0x84, 0x85, 0xE6},
            {0x4F, 0x27, 0x84, 0x84, 0xE6},
            {0x4F, 0x27, 0x84, 0x85, 0xE6}
        };
        memcpy(RF_buffer[group-1], hansValues[group-1], sizeof(hansValues[group-1]));
        saveRFData(RfSendEncoding::TRIBIT, group, RF_buffer[group-1], RF_NUM_DEFAULT);
    }
    else if (group == static_cast<uint8_t>(Pairing::HOPO_1)||group==static_cast<uint8_t>(Pairing::HOPO_2)||group==static_cast<uint8_t>(Pairing::HOPO_WIRELESS))
    {
        uint8_t hopoValues[NUM_GROUPS][RF_NUM_DEFAULT];
        for (uint16_t i = 0; i < NUM_GROUPS; ++i)
        {
            generateRandomValues(hopoValues[i], RF_NUM_DEFAULT, 0x01);
        }
        memcpy(RF_buffer[group-1], hopoValues[group-1], sizeof(hopoValues[group-1]));
        saveRFData(RfSendEncoding::TRIBIT, group, RF_buffer[group-1], RF_NUM_DEFAULT);
    }
    
   
}
