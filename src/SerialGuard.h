#ifndef SERIAL_GUARD_H
#define SERIAL_GUARD_H

#include "SafeSerial.h"

class SerialGuard {
public:
    SerialGuard() {
        SAFE_SERIAL.lock();
    }
    
    ~SerialGuard() {
        SAFE_SERIAL.unlock();
    }
};

#endif // SERIAL_GUARD_H
