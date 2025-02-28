#include "RFStorageManager.h"

RFStorageManager::RFStorageManager(uint16_t startAddr, uint16_t size)
    : startAddress(startAddr), eepromSize(size)
{
}

bool RFStorageManager::loadRFData(uint8_t currentMode, uint8_t RF_buffer[][RF_NUM_DEFAULT])
{
    uint16_t addr = startAddress;
    // 清空缓存，若无数据则可用默认值填充（默认值可在上层处理）
    for (uint8_t i = 0; i < NUM_GROUPS; i++) {
        memset(RF_buffer[i], INIT_DATA, RF_NUM_DEFAULT);
    }

    while (addr < eepromSize) {
        RFDataPacketHeader header;
        if (!EEPROMManager::readData(addr, (uint8_t*)&header, sizeof(header))) {
            break; // 读取失败或到达 EEPROM 尾部
        }
        // 如果头部模式匹配，并且分组号在范围内
        if (header.mode == currentMode && header.group < NUM_GROUPS && header.dataLen <= RF_NUM_DEFAULT) {
            EEPROMManager::readData(addr + sizeof(header), RF_buffer[header.group], header.dataLen);
        }
        // 移动到下一个数据包位置
        addr += sizeof(header) + header.dataLen;
    }
    return true;
}

bool RFStorageManager::saveRFData(uint8_t mode, uint8_t group,  uint8_t* data, uint8_t dataLen)
{
    // 计算写入地址：此处假设每个模式预留固定空间（模式内按组顺序排列）
    uint16_t addr = startAddress + (mode * NUM_GROUPS + group) * (sizeof(RFDataPacketHeader) + RF_NUM_DEFAULT);
    RFDataPacketHeader header = { mode, group, dataLen };
    if (!EEPROMManager::writeData(addr, (uint8_t*)&header, sizeof(header))) {
        return false;
    }
    if (!EEPROMManager::writeData(addr + sizeof(header), data, dataLen)) {
        return false;
    }
    return true;
}
