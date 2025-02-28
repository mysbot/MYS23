#ifndef SET_RFPARAMETER_H
#define SET_RFPARAMETER_H

#include <Arduino.h>
#include "RF433send.h"
#include "EEPROMManager.h"
#include "RFStorageManager.h"
#include "Config.h"
#include <ctype.h> // 引入ctype库以使用isxdigit函数

struct ProtocolParams
{
  uint8_t nb_bits;
  uint8_t dataLen; // 原先的 RF_NUM
  uint16_t initseq;
  uint16_t lo_prefix;
  uint16_t hi_prefix;
  uint16_t lo_short;
  uint16_t lo_long;
  uint16_t hi_short;
  uint16_t hi_long;
  uint16_t lo_last;
  uint16_t sep;
};

const ProtocolParams PROTOCOL_HANS = {39, 5, 8000, 4800, 1500, 360, 720, 0, 0, 700, 8000};
const ProtocolParams PROTOCOL_HOPO = {24, 3, 12000, 0, 0, 380, 1200, 0, 0, 380, 12000};

extern const ProtocolParams *txParams;
//extern const ProtocolParams *rxParams;

class setRFpara
{
public:
  void setRFParameters(uint8_t workmode);
  //static uint8_t defaultValues[NUM_GROUPS][RF_NUM_DEFAULT];
  //void generateRandomValues(uint8_t *buffer, uint16_t length, uint8_t fixedLastValue);
  //void initDefaultValues();

private:
  static EEPROMManager eeprommanager;
};

#endif