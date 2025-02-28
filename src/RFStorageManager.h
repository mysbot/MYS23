#ifndef RF_STORAGE_MANAGER_H
#define RF_STORAGE_MANAGER_H

#include <Config.h>
#include "EEPROMManager.h"  // 假设已封装 EEPROM 读写接口

// 统一数据包头（数据包存储格式：[Header][Data]）
struct RFDataPacketHeader {
    uint8_t mode;      // RF 工作模式标识（参见 RFworkMode 枚举）
    uint8_t group;     // 分组号（如 0：第一组，1：第二组）
    uint8_t dataLen;   // 数据有效长度
};

class RFStorageManager {
public:
    RFStorageManager(uint16_t startAddr, uint16_t size);
    // 根据当前工作模式，扫描 EEPROM 区域，将数据加载到 RF_buffer 中
    bool loadRFData(uint8_t currentMode, uint8_t RF_buffer[][RF_NUM_DEFAULT]);
    // 保存单个数据包到 EEPROM（这里采用固定偏移计算方式，可按需要修改为查找更新）
    bool saveRFData(uint8_t mode, uint8_t group,  uint8_t* data, uint8_t dataLen);
private:
    uint16_t startAddress;
    uint16_t eepromSize;
};

#endif // RF_STORAGE_MANAGER_H
