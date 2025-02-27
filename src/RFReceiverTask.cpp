
#include "RFReceiverTask.h"

const char *encoding_names[] = {
    "RFMOD_TRIBIT",          // T
    "RFMOD_TRIBIT_INVERTED", // N
    "RFMOD_MANCHESTER",      // M
    "<unmanaged encoding>"   // Anything else
};

RFReceiver::RFReceiver(uint8_t rxPin)
    : rxPin(rxPin), rfTaskHandle(nullptr)
{
}

void RFReceiver::begin()
{
    pinMode(rxPin, INPUT);
    rfTrack = new Track(rxPin);
}
// 接收任务，运行在 core1
void RFReceiver::RFReceiverTask(void *parameter)
{
    // 配置任务参数
    esp_task_wdt_init(10, false); // 禁用看门狗复位

    // 添加信号接收时间控制
    uint32_t lastReceiveTime = 0;
    const uint32_t RECEIVE_INTERVAL = 1000; // 1秒的接收间隔

    for (;;)
    {
        uint32_t currentTime = millis();
        // 简化临界区处理
        getRFTrack()->treset();

        while (!getRFTrack()->do_events())
        {
            vTaskDelay(1);
        }

        Decoder *pdec = getRFTrack()->get_data(RF433ANY_FD_ALL);
        if (pdec)
        {
            // 检查是否已经过了接收间隔时间
            if (currentTime - lastReceiveTime >= RECEIVE_INTERVAL)
            {
                //Serial.println("Received RF Signal");
                storeSignalParams(pdec);
                lastReceiveTime = currentTime; // 更新最后接收时间
                //Serial.printf("Next receive will be after: %lu ms\n", currentTime + RECEIVE_INTERVAL);
            }
            else
            {
                //Serial.println("Signal ignored - too soon after last receive");
            }
            delete pdec;
        }

        // 适当的任务延时
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

const char *RFReceiver::id_letter_to_encoding_name(char c)
{
    if (c == 'T')
        return encoding_names[0];
    else if (c == 'N')
        return encoding_names[1];
    else if (c == 'M')
        return encoding_names[2];

    return encoding_names[3];
}

// 添加辅助函数 stringToHex 和 getEncodingFromName
uint8_t RFReceiver::stringToHex(const char *str, uint8_t *out)
{
    uint8_t len = 0;
    while (*str)
    {
        while (*str && isspace(*str))
        {
            str++;
        }
        if (!*str)
            break;
        char c1 = *str++;
        while (*str && isspace(*str))
        {
            str++;
        }
        if (!*str)
            break;
        char c2 = *str++;
        auto hexVal = [](char c) -> uint8_t
        {
            if (c >= '0' && c <= '9')
                return c - '0';
            if (c >= 'A' && c <= 'F')
                return c - 'A' + 10;
            if (c >= 'a' && c <= 'f')
                return c - 'a' + 10;
            return 0;
        };
        uint8_t byte = (hexVal(c1) << 4) | hexVal(c2);
        out[len++] = byte;
    }
    return len;
}

RfSendEncoding RFReceiver::getEncodingFromName(const char *name)
{
    if (strcmp(name, "RFMOD_TRIBIT") == 0)
        return RfSendEncoding::TRIBIT;
    else if (strcmp(name, "RFMOD_TRIBIT_INVERTED") == 0)
        return RfSendEncoding::TRIBIT_INVERTED;
    else if (strcmp(name, "RFMOD_MANCHESTER") == 0)
        return RfSendEncoding::MANCHESTER;
    else
        return RfSendEncoding::MANCHESTER; // 默认
}

void RFReceiver::storeSignalParams(Decoder *pdec)
{
    if (!pdec)
        return;

    TimingsExt tsext;
    pdec->get_tsext(&tsext);
    const char *enc_name = id_letter_to_encoding_name(pdec->get_id_letter());

    // 将编码赋值到 RFParams.encoding
    lastReceivedParams.encoding = getEncodingFromName(enc_name);

    lastReceivedParams.timings[0] = tsext.initseq;
    lastReceivedParams.timings[1] = tsext.first_low;
    lastReceivedParams.timings[2] = tsext.first_high;
    lastReceivedParams.timings[3] = tsext.first_low_ignored;
    lastReceivedParams.timings[4] = tsext.low_short;
    lastReceivedParams.timings[5] = tsext.low_long;
    lastReceivedParams.timings[6] = tsext.high_short;
    lastReceivedParams.timings[7] = tsext.high_long;
    lastReceivedParams.timings[8] = tsext.last_low;
    lastReceivedParams.timings[9] = tsext.sep;

    // 使用 stringToHex 将数据转换为实际的二进制
    lastReceivedParams.nBit = pdec->get_nb_bits();
    BitVector *pdata = pdec->take_away_data();
    if (pdata)
    {

        char *dataStr = pdata->to_str();
        if (dataStr)
        {
            lastReceivedParams.dataLength = stringToHex(dataStr, lastReceivedParams.data);

            free(dataStr);
        }
        delete pdata;
    }

    sendQueue.push(lastReceivedParams);
    /*
        Serial.printf("Stored RF Parameters:\n");
        Serial.printf("mod:%d\n", lastReceivedParams.encoding);
        Serial.printf("Init seq: %d\n", lastReceivedParams.timings[0]);
        Serial.printf("first_low: %d\n", lastReceivedParams.timings[1]);
        Serial.printf("first_high: %d\n", lastReceivedParams.timings[2]);
        Serial.printf("first_low_ignored: %d\n", lastReceivedParams.timings[3]);
        Serial.printf("low_short: %d\n", lastReceivedParams.timings[4]);
        Serial.printf("low_long: %d\n", lastReceivedParams.timings[5]);
        Serial.printf("high_short: %d\n", lastReceivedParams.timings[6]);
        Serial.printf("high_long: %d\n", lastReceivedParams.timings[7]);
        Serial.printf("last_low: %d\n", lastReceivedParams.timings[8]);
        Serial.printf("seq: %d\n", lastReceivedParams.timings[9]);

        Serial.printf("n bits: %d\n", lastReceivedParams.nBit);
        Serial.printf("**%d** Data: \n", lastReceivedParams.dataLength);
        for (size_t i = 0; i < lastReceivedParams.dataLength; i++)
        {
            Serial.printf("%02X ", lastReceivedParams.data[i]);
        }
        Serial.println();
        */
    // 确保EEPROM写入成功
    /*
    if (!EEPROMManager::writeData(FIRST_ADDRESS_FOR_RF_SIGNAL, (uint8_t *)&lastReceivedParams, sizeof(RFParams)))
    {
        Serial.println("Failed to save EEPROM data");
        return; // 如果保存失败，直接返回
    }

    Serial.println("EEPROM data saved successfully");
    */
    // 增加调试信息
    // Serial.printf("Data length: %d, Bits: %d, Encoding: %d\n",
    //               lastReceivedParams.dataLength,
    //               lastReceivedParams.nBit,
    //               (int)lastReceivedParams.encoding);
}