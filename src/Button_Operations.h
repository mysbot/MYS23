#ifndef BUTTON_OPERATIONS_H
#define BUTTON_OPERATIONS_H

#include "Arduino.h"
#include "Config.h"
// Define a type for callback function pointers
typedef void (*ButtonCallback)();

class ButtonOperations {
public:
    ButtonOperations(uint16_t pinA, uint16_t pinB,uint16_t pinC,uint16_t pinD);
    void ButtonInitialize();    
    void onButtonAPressed(ButtonCallback callback);
    void onButtonBPressed(ButtonCallback callback) ;
    void onButtonBShortPressed(ButtonCallback callback) ;
    void onButtonCPressed(ButtonCallback callback);
    void onButtonDPressed(ButtonCallback callback) ;
    void onButtonDShortPressed(ButtonCallback callback) ;
    void onButtonALongPressed(ButtonCallback callback);
    void checkButtons();   
   
private:
    uint16_t pinA, pinB,pinC, pinD;
    uint32_t shortKey=100,key=500,longKey=2000;
    ButtonCallback buttonACallback = nullptr;
    ButtonCallback buttonBCallback = nullptr;
    ButtonCallback buttonBShortPressCallback = nullptr;
    ButtonCallback buttonCCallback = nullptr;
    ButtonCallback buttonDCallback = nullptr;
    ButtonCallback buttonDShortPressCallback = nullptr;
    ButtonCallback buttonALongPressCallback = nullptr;
    // Other private members and methods
};

#endif
