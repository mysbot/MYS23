#ifndef SEND_H
#define SEND_H

#include "RF433send.h"

class RFSender {
public:
    RFSender(uint8_t pin);
    void begin();
    void sendLoop();

private:
    uint8_t pin;
    RfSend* tx_whatever;
    byte data[4] = { 0x03, 0x14, 0x15, 0x93 };
};

#endif // SEND_H
