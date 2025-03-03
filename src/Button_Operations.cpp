#include "Button_Operations.h"

ButtonOperations::ButtonOperations(uint16_t pinA, uint16_t pinB,uint16_t pinC,uint16_t pinD) : pinA(pinA), pinB(pinB),pinC(pinC),pinD(pinD) {}

void ButtonOperations::ButtonInitialize() {
    pinMode(pinA, INPUT_PULLUP);
    pinMode(pinB, INPUT_PULLUP);
    pinMode(pinC, INPUT_PULLUP);
    pinMode(pinD, INPUT_PULLUP);
}
uint32_t buttonAPressTime = 0;
bool isButtonALongPressed = false;
uint32_t buttonBPressTime = 0;
bool isButtonBLongPressed = false;
uint32_t buttonCPressTime = 0;
bool isButtonCLongPressed = false;
uint32_t buttonDPressTime = 0;
bool isButtonDLongPressed = false;

void ButtonOperations::onButtonAPressed(ButtonCallback callback) {
    buttonACallback = callback;
}
void ButtonOperations::onButtonALongPressed(ButtonCallback callback) {
    buttonALongPressCallback = callback;
}
void ButtonOperations::onButtonBPressed(ButtonCallback callback) {
    buttonBCallback = callback;
}
void ButtonOperations::onButtonBShortPressed(ButtonCallback callback) {
    buttonBShortPressCallback = callback;
}
void ButtonOperations::onButtonCPressed(ButtonCallback callback) {
    buttonCCallback = callback;
}
void ButtonOperations::onButtonDPressed(ButtonCallback callback) {
    buttonDCallback = callback;
}
void ButtonOperations::onButtonDShortPressed(ButtonCallback callback) {
    buttonDShortPressCallback = callback;
}
void ButtonOperations::checkButtons() {
    if (digitalRead(pinA) == LOW) {
        if (buttonAPressTime == LOW) { // Button A was just pressed
            buttonAPressTime = millis();
        } else if (millis() - buttonAPressTime > longKey) { // Button A has been pressed for over 1 second
            if (!isButtonALongPressed) {
              //Serial.print("A is long pressed");
                isButtonALongPressed = true;
                if (buttonALongPressCallback != nullptr)   buttonALongPressCallback(); ;
            }
        }
    } else if (buttonAPressTime > 0) { // Button A was released
        if (!isButtonALongPressed && (millis() - buttonAPressTime > key)&&(millis() - buttonAPressTime < (longKey/2))) { // Short press
         //Serial.print("A is short pressed");
            if (buttonACallback != nullptr) buttonACallback();
        }
        buttonAPressTime = 0;
        isButtonALongPressed = false;
    }

    // 检查按钮B
    if (digitalRead(pinB) == LOW) {
        if (buttonBPressTime == LOW) { // 按钮B刚被按下
            buttonBPressTime = millis();
        } else if (millis() - buttonBPressTime > key) { // 按钮B长按，这里设定为500毫秒作为长按的判断依据
            if (!isButtonBLongPressed) {
                isButtonBLongPressed = true;
                // 这里可以添加长按开始时的逻辑，例如连续增加数值
                if (buttonBCallback != nullptr) buttonBCallback();
            }
            // 长按持续期间的逻辑，例如连续+1
            
        }
    } else if (buttonBPressTime > 0) { // 按钮B被释放
        if (!isButtonBLongPressed&& (millis() - buttonBPressTime > shortKey)&&(millis() - buttonBPressTime < key)) { // 短按
            if (buttonBShortPressCallback != nullptr) buttonBShortPressCallback(); 
        }
        // 重置按钮B的状态
        buttonBPressTime = 0;
        isButtonBLongPressed = false;
    }
     // 检查按钮C
    if (digitalRead(pinC) == LOW) {
        if (buttonCPressTime == 0) { // 按钮刚被按下
            buttonCPressTime = millis();
        } else if (millis() - buttonCPressTime > key) { // 按钮长按，这里设定为500毫秒作为长按的判断依据
            if (!isButtonCLongPressed) {
                isButtonCLongPressed = true;
                // 这里可以添加长按开始时的逻辑，例如连续增加数值 
                if (buttonCCallback != nullptr) buttonCCallback();               
            }
            // 长按持续期间的逻辑，例如连续+1       
              
        }
    } else if (buttonCPressTime > 0) { // 按钮被释放
        if (!isButtonCLongPressed) { // 短按           
        }
        // 重置按钮的状态
        buttonCPressTime = 0;
        isButtonCLongPressed = false;
    }
     // 检查按钮D
    if (digitalRead(pinD) == LOW) {
        if (buttonDPressTime == 0) { // 按钮刚被按下
            buttonDPressTime = millis();
        } else if (millis() - buttonDPressTime > key) { // 按钮长按，这里设定为500毫秒作为长按的判断依据
            if (!isButtonDLongPressed) {
                isButtonDLongPressed = true;
                // 这里可以添加长按开始时的逻辑，例如连续增加数值
                if (buttonDCallback != nullptr) buttonDCallback();
            }
            // 长按持续期间的逻辑，例如连续+1           
        }
    } else if (buttonDPressTime > 0) { // 按钮被释放
        if (!isButtonDLongPressed&& (millis() - buttonDPressTime > shortKey)&&(millis() - buttonDPressTime < key)) { // 短按
            if (buttonDShortPressCallback != nullptr) buttonDShortPressCallback(); 
        }
        // 重置按钮的状态
        buttonDPressTime = 0;
        isButtonDLongPressed = false;
    }
}


