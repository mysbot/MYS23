#include "setRFparameter.h"

uint8_t setRFpara::defaultValues[NUM_GROUPS][RF_NUM_DEFAULT] = {{0}};
const ProtocolParams *txParams;
const ProtocolParams *rxParams;
uint8_t hansValues[NUM_GROUPS][RF_NUM_DEFAULT] = {
    {0x4F, 0x27, 0x84, 0x82, 0xE6},
    {0x4F, 0x27, 0x84, 0x83, 0xE6},
    //{0x4F, 0x27, 0x84, 0x84, 0xE6},
    //{0x4F, 0x27, 0x84, 0x85, 0xE6}
};

void setRFpara::setRFParameters(uint8_t workmode)
{
    switch ((RFworkMode)workmode)
    {
    case RFworkMode::HANS_RECEIVER:
    case RFworkMode::HANS_BOTH:
    case RFworkMode::HANS_TRANSMITTER:
        rxParams = &PROTOCOL_HANS;
        txParams = &PROTOCOL_HANS;
        mySerial.println(F("RX AND TX IS HANS."));
        break;
    case RFworkMode::HOPO_RECEIVER:
    case RFworkMode::HOPO_TRANSMITTER:
        rxParams = &PROTOCOL_HOPO;
        txParams = &PROTOCOL_HOPO;
        mySerial.println(F("RX AND TX IS HOPO."));
        break;
    case RFworkMode::HOPO_HANS:
        rxParams = &PROTOCOL_HOPO;
        txParams = &PROTOCOL_HANS;
        mySerial.println(F("RX IS HOPO, TX IS HANS."));
        break;
    case RFworkMode::HANS_HOPO:
        rxParams = &PROTOCOL_HANS;
        txParams = &PROTOCOL_HOPO;
        mySerial.println(F("TX IS HOPO, RX IS HANS."));
        break;
    default:
        rxParams = &PROTOCOL_HANS;
        txParams = &PROTOCOL_HANS;
        mySerial.println(F("DEFAULT,RX AND TX IS HANS."));
        break;
    }
}

void setRFpara::generateRandomValues(uint8_t *buffer, uint16_t length, uint8_t fixedLastValue)
{
    for (uint16_t i = 0; i < length - 3; ++i)
    {
        buffer[i] = random(0, 256); // 生成0到255之间的随机数
    }
    buffer[length - 3] = fixedLastValue; // 最后一位固定为指定值
    buffer[length - 2] = fixedLastValue; // 最后一位固定为指定值
    buffer[length - 1] = fixedLastValue; // 最后一位固定为指定值
}

void setRFpara::initDefaultValues()
{
    if (ADDmanager.RFworkingMode_value == static_cast<uint8_t>(RFworkMode::HANS_RECEIVER) ||
        ADDmanager.RFworkingMode_value == static_cast<uint8_t>(RFworkMode::HANS_TRANSMITTER))
    {
        for (size_t i = 0; i < NUM_GROUPS; i++)
        {
            if (!eeprommanager.readData(FIRST_ADDRESS_FOR_RF_SIGNAL + ((int)(ADDmanager.RFworkingMode_value / 2) + i) * RF_NUM_DEFAULT, defaultValues[i], PROTOCOL_HANS.dataLen))
            {
                mySerial.printf("RF group %d belong to mode %d is reset", i, ADDmanager.RFworkingMode_value);
                memcpy(defaultValues[i], hansValues[i], sizeof(hansValues[i]));
            }
        }
    }
    else if (ADDmanager.RFworkingMode_value == static_cast<uint8_t>(RFworkMode::HOPO_RECEIVER) ||
             ADDmanager.RFworkingMode_value == static_cast<uint8_t>(RFworkMode::HOPO_TRANSMITTER))
    {
        for (size_t i = 0; i < NUM_GROUPS; i++)
        {
            if (!eeprommanager.readData(FIRST_ADDRESS_FOR_RF_SIGNAL + ((int)(ADDmanager.RFworkingMode_value / 2) + i + 1) * RF_NUM_DEFAULT, defaultValues[i], PROTOCOL_HOPO.dataLen))
            {
                mySerial.printf("RF group %d belong to mode %d is reset", i, ADDmanager.RFworkingMode_value);
                uint8_t hopoValues[NUM_GROUPS][RF_NUM_DEFAULT];
                for (uint16_t i = 0; i < NUM_GROUPS; ++i)
                {
                    generateRandomValues(hopoValues[i], RF_NUM_DEFAULT, 0x01);
                }
                memcpy(defaultValues, hopoValues, sizeof(hopoValues));
            }
        }
    }
    else if (ADDmanager.RFworkingMode_value == static_cast<uint8_t>(RFworkMode::HANS_HOPO))
    {
        for (size_t i = 0; i < NUM_GROUPS; i++)
        {
            if (!eeprommanager.readData(FIRST_ADDRESS_FOR_RF_SIGNAL + (ADDmanager.RFworkingMode_value + i) * RF_NUM_DEFAULT, defaultValues[i], i == 1 ? PROTOCOL_HOPO.dataLen : PROTOCOL_HANS.dataLen))
            {
                mySerial.printf("RF group %d belong to mode %d is reset", i, ADDmanager.RFworkingMode_value);
                if (i == 1)
                {

                    uint8_t hopoValues[NUM_GROUPS][RF_NUM_DEFAULT];
                    for (uint16_t i = 0; i < NUM_GROUPS; ++i)
                    {
                        generateRandomValues(hopoValues[i], RF_NUM_DEFAULT, 0x01);
                    }
                    memcpy(defaultValues, hopoValues, sizeof(hopoValues));
                }
                else
                {
                    memcpy(defaultValues[i], hansValues[i], sizeof(hansValues[i]));
                }
            }
        }
    }
    else if (ADDmanager.RFworkingMode_value == static_cast<uint8_t>(RFworkMode::HOPO_HANS))
    {
        for (size_t i = 0; i < NUM_GROUPS; i++)
        {
            if (!eeprommanager.readData(FIRST_ADDRESS_FOR_RF_SIGNAL + (ADDmanager.RFworkingMode_value + i) * RF_NUM_DEFAULT, defaultValues[i], i == 0 ? PROTOCOL_HOPO.dataLen : PROTOCOL_HANS.dataLen))
            {
                mySerial.printf("RF group %d belong to mode %d is reset", i, ADDmanager.RFworkingMode_value);
                if (i == 0)
                {

                    uint8_t hopoValues[NUM_GROUPS][RF_NUM_DEFAULT];
                    for (uint16_t i = 0; i < NUM_GROUPS; ++i)
                    {
                        generateRandomValues(hopoValues[i], RF_NUM_DEFAULT, 0x01);
                    }
                    memcpy(defaultValues, hopoValues, sizeof(hopoValues));
                }
                else
                {
                    memcpy(defaultValues[i], hansValues[i], sizeof(hansValues[i]));
                }
            }
        }
    }
    else if (ADDmanager.RFworkingMode_value == static_cast<uint8_t>(RFworkMode::HANS_BOTH))
    {
        for (size_t i = 0; i < NUM_GROUPS; i++)
        {

            {
                mySerial.printf("RF group %d belong to mode %d is reset", i, ADDmanager.RFworkingMode_value);
                memcpy(defaultValues[i], hansValues[i], sizeof(hansValues[i]));
            }
        }
    }
}
