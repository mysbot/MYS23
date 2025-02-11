#include "RFManager.h"
#include <cstring>
#include <cstdarg>

const char *encoding_names[] = {
    "RFMOD_TRIBIT",          // T
    "RFMOD_TRIBIT_INVERTED", // N
    "RFMOD_MANCHESTER",      // M
    "<unmanaged encoding>"   // Anything else
};
char serial_printf_buffer[100];
RFManager::RFManager(uint8_t rxPin, uint8_t txPin) 
    : rxPin(rxPin), txPin(txPin), rfSender(nullptr), rfTrack(nullptr), 
      rfTaskHandle(nullptr) {
}

void RFManager::begin() {
    pinMode(rxPin, INPUT);
    pinMode(txPin, OUTPUT);
    rfTrack = new Track(rxPin);
}

void RFManager::startTasks() {
    xTaskCreatePinnedToCore(
        rfTask,
        "RF_Task",
        8192,
        this,
        1,
        &rfTaskHandle,
        0
    );
}
 
void RFManager::rfTask(void* parameter) {
    RFManager* rf = static_cast<RFManager*>(parameter);
    
    while (true) {
        if (rf->monitorMode) {
            rf->rfTrack->treset();
            while (!rf->rfTrack->do_events()) {
                vTaskDelay(1);
            }
           
            Decoder* pdec = rf->rfTrack->get_data(RF433ANY_FD_ALL);
            if (pdec) {
                Serial.println("Received RF Signal:");
                rf->lastReceivedParams.nBit = pdec->get_nb_bits();
                BitVector *pdata = pdec->take_away_data();
                char *buf = pdata->to_str();
                
                if (buf) {  
                    // 修改调用方式，通过 rf 实例调用 stringToHex
                    rf->lastReceivedParams.dataLength = rf->stringToHex(buf, rf->lastReceivedParams.data);                  
                    Serial.print(buf);
                    free(buf);
                }
                rf->storeSignalParams(pdec);
                delete pdec;
            }
        } else {
            if (!rf->sendQueue.empty()) {
                rf->sendStoredSignal();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
const char *RFManager::id_letter_to_encoding_name(char c) {
    if (c == 'T')
        return encoding_names[0];
    else if (c == 'N')
        return encoding_names[1];
    else if (c == 'M')
        return encoding_names[2];

    return encoding_names[3];
}
void serial_printf(const char* msg, ...)
__attribute__((format(printf, 1, 2)));

    
void RFManager::serial_printf(const char* msg, ...) {
    va_list args;

    va_start(args, msg);

    vsnprintf(serial_printf_buffer, sizeof(serial_printf_buffer), msg, args);
    va_end(args);
    Serial.print(serial_printf_buffer);
}

// 添加辅助函数 stringToHex 和 getEncodingFromName
uint8_t RFManager::stringToHex(const char* str, uint8_t* out) {
    uint8_t len = 0;
    while (*str) {
        while (*str && isspace(*str)) { str++; }
        if (!*str) break;
        char c1 = *str++;
        while (*str && isspace(*str)) { str++; }
        if (!*str) break;
        char c2 = *str++;
        auto hexVal = [](char c) -> uint8_t {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            return 0;
        };
        uint8_t byte = (hexVal(c1) << 4) | hexVal(c2);
        out[len++] = byte;
    }
    return len;
}

 RfSendEncoding RFManager::getEncodingFromName(const char* name) {
    if (strcmp(name, "RFMOD_TRIBIT") == 0)
        return RfSendEncoding::TRIBIT;
    else if (strcmp(name, "RFMOD_TRIBIT_INVERTED") == 0)
        return RfSendEncoding::TRIBIT_INVERTED;
    else if (strcmp(name, "RFMOD_MANCHESTER") == 0)
        return RfSendEncoding::MANCHESTER;
    else
        return RfSendEncoding::MANCHESTER; // 默认
}

void RFManager::storeSignalParams(Decoder* pdec) {
    if (!pdec) return;

    TimingsExt tsext;
    pdec->get_tsext(&tsext);
    const char *enc_name = id_letter_to_encoding_name(pdec->get_id_letter());
    
    // 将编码赋值到 RFParams.encoding
    lastReceivedParams.encoding = getEncodingFromName(enc_name);

    serial_printf("\n-----CODE START-----\n");
    serial_printf("// [WRITE THE DEVICE NAME HERE]\n"
                  "rf.register_Receiver(\n");
    serial_printf("\t%s, // mod\n", enc_name);
    serial_printf("\t%u, // initseq\n", tsext.initseq);
    serial_printf("\t%u, // lo_prefix\n", tsext.first_low);
    serial_printf("\t%u, // hi_prefix\n", tsext.first_high);
    serial_printf("\t%u, // first_lo_ign\n", tsext.first_low_ignored);
    serial_printf("\t%u, // lo_short\n", tsext.low_short);
    serial_printf("\t%u, // lo_long\n", tsext.low_long);
    serial_printf("\t%u, // hi_short (0 => take lo_short)\n", tsext.high_short);
    serial_printf("\t%u, // hi_long  (0 => take lo_long)\n", tsext.high_long);
    serial_printf("\t%u, // lo_last\n", tsext.last_low);
    serial_printf("\t%u, // sep\n", tsext.sep);
    serial_printf("\t%u  // nb_bits\n", lastReceivedParams.nBit);
    serial_printf(");\n");
    serial_printf("-----CODE END-----\n\n");

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
    BitVector* pdata = pdec->take_away_data();
    if (pdata) {
        char* dataStr = pdata->to_str();
        if (dataStr) {
           
            
            free(dataStr);
        }
        delete pdata;
    }
    
    sendQueue.push(lastReceivedParams);
    
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
    Serial.print("Data: ");
    for (size_t i = 0; i < lastReceivedParams.dataLength; i++) {
        Serial.printf("%02X ", lastReceivedParams.data[i]);
    }
    Serial.println();
}

void RFManager::setupRFSender(const RFParams& params) {
    if (rfSender) {
        delete rfSender;
    }
    
    rfSender = rfsend_builder(
        RfSendEncoding::MANCHESTER,
        txPin,
        RFSEND_DEFAULT_CONVENTION,
        4,
        nullptr,
        params.timings[0],
        params.timings[1],
        params.timings[2],
        params.timings[3],
        params.timings[4],
        params.timings[5],
        params.timings[6],
        params.timings[7],
        params.timings[8],
        params.timings[9],
        params.dataLength * 8
    );
}

void RFManager::sendStoredSignal() {
    if (sendQueue.empty()) return;
    
    RFParams params = sendQueue.front();
    sendQueue.pop();
    
    setupRFSender(params);
    if (rfSender) {
        Serial.println("Sending stored RF signal...");
        rfSender->send(params.dataLength, params.data);
        Serial.println("RF signal sent.");
    }
}

bool RFManager::sendData(uint8_t* data, size_t length) {
    if (!rfSender) return false;
    return rfSender->send(length, data) > 0;
}
