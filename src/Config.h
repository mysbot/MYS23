#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include "esp_task_wdt.h"

#define VERSION 0x21
// Pin definitions
#define TX0_PIN 1
#define RX0_PIN 3
#define TX1_PIN 17
#define RX1_PIN 16

#define BUTTON_MODE 34

#define BUTTON_UP 33
#define BUTTON_DOWN 32
#define BUTTON_STOP 18
#define RF_RECEIVER_PIN 19
#define RF_TRANSMITTER_PIN 21

#define RELAY_BUTTON1 22
// #define RELAY_BUTTON1 4
#define RELAY_BUTTON2 23
// #define RELAY_BUTTON2 5

// EEPROM configuration
#define EEPROM_SIZE 512
#define FIRST_ADDRESS_FOR_RF_SIGNAL 256

#define PRODUCTION_TEST_MODE_ADDRESS 0x3C
// #define SLEEP_TIME_ADDR 0x2c
// #define RFMODE_VALUE_ADDRESS 0x28
#define RFWORKMode_ADDRESS 0x20
#define MYS_ADDRESS 0x30
// #define PAIRMODE_ADDRESS 0x1c
#define CONTROL_GROUP_ADDRESS 0x38
#define SLIDINGDOOR_MODE_ADDRESS 0x0c
#define RAINSIGNAL_CONTROL_ADDRESS 0x10
#define MUTUAL_CONTROL_ADDRESS 0x40
#define WINDOWTYPE_ADDRESS 0x64
#define SECURITY_ADDRESS 0x68

struct address_Manager
{

    uint8_t HOPOTransmit = 0;
    uint8_t localAddress = MYS_ADDRESS;
    // uint8_t  RFmodeAddress=RFMODE_VALUE_ADDRESS;
    uint8_t RFworkingModeAddress = RFWORKMode_ADDRESS;
    // uint8_t  RFpairingModeAddress=PAIRMODE_ADDRESS;
    uint8_t controlGroupAddress = CONTROL_GROUP_ADDRESS;
    uint8_t rainSignalAddress = RAINSIGNAL_CONTROL_ADDRESS;
    uint8_t mutualAddress = MUTUAL_CONTROL_ADDRESS;
    uint8_t windowTypeAddress = WINDOWTYPE_ADDRESS;
    uint8_t securityAddress = SECURITY_ADDRESS;
    uint8_t slidingDoorModeAddress = SLIDINGDOOR_MODE_ADDRESS;
    uint8_t localadd_value = 0;      // 默认0，0-FE为可用。
    uint8_t RFpairingMode_value = 0; // 1，1组配对模式，2，2组配对模式；F,清码。其他，工作模式
    uint8_t RFworkingMode_value = 0; // 0，接收模式，1，发射模式;其他，未定义，默认为0
    // uint8_t RFmode_value=0; //0，HANS协议；1，HOPO协议；其他，未定义，默认为0
    // uint8_t previousRFmode_value=0;
    uint8_t rainSignal_value = 1;
    uint8_t Is_mutual_value = 1;
    uint8_t Is_security_value = 3;
    uint8_t windowType_value = 0X06;   // 1，提升窗。2，推拉门，3，外开窗，4，内开内倒窗。5，电动上悬/平推/天窗。6，卷帘/百叶
    uint8_t slidingDoorMode_value = 0; // 0全开/1半开
    uint8_t controlGroup_value = 0;
    uint8_t productionTestModeTriggered = 0; // 新增：产测模式触发标志
};
extern address_Manager ADDmanager;

// Define RF modes
// #define RF_HANS_MODE 0
// #define RF_HOPO_MODE 1
// #define RF_GU_MODE 2
// #define RF_ED_MODE 3
//
#define RF_BIT_DEFAULT 39
#define RF_NUM_DEFAULT 5
// extern uint8_t RF_BIT ;
// extern uint8_t RF_NUM ;

#define NUM_GROUPS 6

#define INIT_DATA 0xFF

// UART data
#define BAUDRATE 9600
// OPEN ONCE TIME DELAY TIME
#define HEART_BEAT_TIME 500
#define OPEN_DELAY_TIME 2000
#define PAIRING_TIMEOUT 120000 // 120 seconds

#define TURN_ON (1)
#define TURN_OFF (0)

#define FIRSTBYTE 0X55
#define THE_THIRD_PART 0X66
extern uint8_t TARGET_ADDRESS;

// Define mySerial here, either as HardwareSerial or SoftwareSerial
extern HardwareSerial mySerial; // for HardwareSerial

extern uint8_t hansValues[NUM_GROUPS][RF_NUM_DEFAULT];

enum class rainSignalMode : uint8_t
{
    TURNOFF = 0,
    WIRELESS = 1,
    WIRED = 2,

    OTHER = 0xFF

};
enum class antiClampMode : uint8_t
{
    TURNOFF = 0,
    EXCEPT = 1,
    REFRESH = 2,
    TURNON = 3,

    OTHER = 0xFF

};
enum class RFworkMode : uint8_t
{
    HANS_RECEIVER = 0,
    HANS_TRANSMITTER = 1,
    HOPO_RECEIVER = 2,
    HOPO_TRANSMITTER = 3,
    HANS_HOPO = 4,
    HOPO_HANS = 6,
    HANS_BOTH = 0xEE,
    OTHER = 0xFF

};
enum class ControlGroup : uint8_t
{
    GROUP1 = 1,
    GROUP2 = 2,
    RAINSIGNAL = 3,
    ALL = 0,

    OTHER = 0xFF

};

enum class SourceType : uint8_t
{
    BUTTON = 1,
    RFINDEX = 2,
    COMM1 = 3,
    COMM0 = 4,
    RAINSIGNAL = 5,

    OTHER = 0xFF

};
enum class ControlType : uint8_t
{
    RELAY_CONTROL = 1,
    TRANSMITTER = 2,
    COMM1 = 3,
    COMM0 = 4,

    OTHER = 0xFF

};
enum class WindowType : uint8_t
{
    AUTOLIFTWINDOW = 1,
    AUTOSLIDINGDOOR = 2,
    TILT_TURNWINDOW = 3,
    OUTWARDWINDOW = 4,
    SKYLIGHTWINDOW = 5,
    CURTAIN = 6,

    OTHER = 0xFF

};
enum class FunctionCode : uint8_t
{
    READ = 1,
    WRITE = 2,
    FUNCTION = 3,
    CONFIRM = 5,
    SUCCESEE = 6,
    FAILED = 7,
    HEART = 4,

    C_DEFAULT = 0xFF

};
enum class Command : uint8_t
{

    DOOR_ONCE = 1,
    DOOR_OPEN_ALWAYS = 2,
    DOOR_CLOSE = 3,

    SCREEN_UP = 1,
    SCREEN_DOWN = 2,
    SCREEN_STOP = 3,

    CASEMENT_STOP = 0,
    CASEMENT_TILT = 1,
    CASEMENT_OPEN = 2,
    CASEMENT_CLOSE = 3,

    WINDOW_UP = 0x11,
    WINDOW_DOWN = 0x12,
    WINDOW_STOP = 0x13,

    SKYLIGHT_OPEN = 1,
    SKYLIGHT_CLOSE = 0X11,
    SKYLIGHT_STOP = 0X13,

    CASEMENT_STOP_ALT = 0X10,
    CASEMENT_TILT_ALT = 0X11,
    CASEMENT_OPEN_ALT = 0X12,
    CASEMENT_CLOSE_ALT = 0X13,

    LIGHT_ON = 0x31,
    LIGHT_OFF = 0x32,

    ISWINDOW_UP = 0x51,
    ISWINDOW_DOWN = 0x52,
    ISWINDOW_STOP = 0X53,
    ISLIGHT_ON = 0X56,
    ISLIGHT_OFF = 0X57,
    ISMOTOER_SOS = 0X58,
    ISWINDOW_UPTILL = 0X91,
    ISWINDOW_DOWNTILL = 0X92,
    ISWINDOW_SAFESENSOR = 0X93,

    RAIN_SIGNAL = 0xAA,

    C_DEFAULT = 0xFF

};
enum class Pairing : uint8_t
{
    PAIR_OUT_TO_WORK = 0,

    HANS_1 = 0X1,
    HANS_2 = 0X2,
    HOPO_1 = 0X3,
    HOPO_2 = 0X4,
    // HANS_RECEIVER=0X5,
    // HOPO_TRANSMITTER=0X6,
    // HOPO_RECEIVER=0X7,
    // HANS_TRANSMITTER=0X8,
    HANS_WIRELESS = 0X05,
    HOPO_WIRELESS = 0X06,
    PAIR_CLEAR = 0X0F

};
// extern Command RF_index;
struct UARTCommand
{
    FunctionCode responseCode;
    uint8_t dataAddress;
    uint8_t data;
};
struct RFCommand
{
    Command index;
    uint8_t data[RF_NUM_DEFAULT];
    uint8_t group;
};
struct ScreenCommand
{
    uint8_t startByte = 0x55;
    uint8_t sourceaddress = 0;    // 修改为两个字节
    uint8_t targetaddress = 0x00; // 修改为两个字节
    uint8_t function = 0x03;
    uint8_t command;
    uint16_t crc;
};
struct WindowCommand
{
    char startByte = '(';
    char address[4] = "001"; // 地址长度为4，包含空终止符
    char function[3];        // 功能长度为3，包含空终止符
    char length[3] = "01";   // 数据长度为3，包含空终止符
    char data[2] = "0";      // 数据长度为2，包含空终止符
    char lrc[4];             // 校验和长度为4，包含空终止符
    char endByte = ')';
};
const uint16_t screenCommandSize = 7;
const uint16_t windowCommandSize = 13;
// Function to set RF parameters based on RFmode_value

extern uint8_t RF_buffer[NUM_GROUPS][RF_NUM_DEFAULT];

/****************************************** */
// Used to store valid data
void initializeComponents();
void updateComponents();

void loadEEPROMSettings();
void setRelayByWindowType();
void setRFWorkModeByWindowType(address_Manager &manager);
void setRFTransmitterModeByworkmode(address_Manager &manager);

void enterProductionTestMode();
void endProductionTestMode();
void handleProductionRFLoop(Command RF_index);
void handleProductionTestMode();

void startPairingMode(uint16_t mode);
void clearPairing();
void stopPairingMode();
void checkPairingTimeout();
void managePairingMode();

void onCommandFromComm0(UARTCommand cmd);
void onCommandFromComm1(UARTCommand cmd);
void processCommand(UARTCommand command, u_int8_t uartNumber);

void handleButtonPress(Command screenCommand, Command windowCommand);

void processRFCommand(RFCommand cmd);

void heartBeatTimer();

void updateAddress(uint8_t address, uint8_t value);
void updateParameter();

void startTasks();


/****************************************** */

#endif // CONFIG_H
