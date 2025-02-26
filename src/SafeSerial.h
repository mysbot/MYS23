#ifndef SAFE_SERIAL_H
#define SAFE_SERIAL_H

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "Config.h"

class SafeSerial {
public:
    static SafeSerial& getInstance() {
        static SafeSerial instance;
        return instance;
    }

    void init() {
        mutex = xSemaphoreCreateMutex();
    }

    bool lock(TickType_t timeout = pdMS_TO_TICKS(10)) {  // 减少超时时间到10ms
        return xSemaphoreTake(mutex, timeout) == pdTRUE;
    }

    void unlock() {
        if (mutex != NULL) {
            xSemaphoreGive(mutex);
        }
    }

    bool isLocked() {
        return uxSemaphoreGetCount(mutex) == 0;
    }

private:
    SafeSerial() : mutex(NULL) {}
    SemaphoreHandle_t mutex;
};

#define SAFE_SERIAL SafeSerial::getInstance()

#endif // SAFE_SERIAL_H
