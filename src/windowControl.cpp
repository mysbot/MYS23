#include "windowControl.h"

// Constructor
WindowControl::WindowControl(address_Manager &AddManager)
    : AddManager(AddManager),                     // Initialize address manager
      relayControl(RELAY_BUTTON1, RELAY_BUTTON2), // Initialize relay control
      rfTransmitter(RF_TRANSMITTER_PIN,AddManager)           // Initialize RF transmitter

{
}

void WindowControl::begin()
{
  rfTransmitter.setup();
}
void WindowControl::ControlUpdate()
{
  if (mutualActive && (millis() - mutualstartMillis >= mutualdelayDuration))
  {
    if (AddManager.Is_security_value != static_cast<uint8_t>(antiClampMode::EXCEPT))
    {

      controlBasedOnWindowType(ControlType::TRANSMITTER, Command::SCREEN_DOWN);
      // 窗纱联动
      controlBasedOnWindowType(ControlType::COMM1, Command::SCREEN_DOWN);
    }
    mutualActive = 0;
  }
  relayControl.update();
  rfTransmitter.update();
}
void WindowControl::isMutualControl()
{
  if ((!AddManager.Is_mutual_value))
  {
    mutualActive = true;
    mutualstartMillis = millis();
  }
}
// 统一处理窗型控制入口
void WindowControl::controlBasedOnWindowType(ControlType controltype, Command command)
{
  switch (controltype)
  {
  case ControlType::RELAY_CONTROL:
    controlRelay(AddManager.windowType_value, command);
    break;
  case ControlType::TRANSMITTER:
    controlTransmitter(AddManager.windowType_value, command);
    break;

  case ControlType::COMM1:
    controlCom(AddManager.windowType_value, command);
    break;
  }
}
void WindowControl::controlRelay(uint16_t windowType, Command command)
{
  switch (static_cast<WindowType>(windowType))
  {
  case WindowType::AUTOLIFTWINDOW:
    controlAutoLiftRelay(command);
    break;
  case WindowType::AUTOSLIDINGDOOR:
    controlAutoSlidingDoorRelay(command);
    break;
  case WindowType::TILT_TURNWINDOW:
  case WindowType::OUTWARDWINDOW:
    controlCasementWindowRelay(command);
    break;
  case WindowType::SKYLIGHTWINDOW:
    controlSkylightWindowRelay(command);
    break;
  case WindowType::CURTAIN:
    controlCurtainRelay(command);
    break;
  default:

    break;
  }
}
void WindowControl::controlAutoLiftRelay(Command command)
{
  if (AddManager.Is_security_value == static_cast<uint8_t>(antiClampMode::TURNON))
  {
    relayControl.controlRelay(Command::SCREEN_DOWN); // 常开
    relayControl.controlRelay(Command::WINDOW_DOWN); // 常开
  }
  else if (AddManager.Is_security_value == static_cast<uint8_t>(antiClampMode::TURNOFF))
  {
    relayControl.controlRelay(Command::SCREEN_STOP); // 常闭
    relayControl.controlRelay(Command::WINDOW_STOP); // 常闭
  }
}
void WindowControl::controlAutoSlidingDoorRelay(Command command)
{
  switch (command)
  {
  case Command::DOOR_ONCE:
  case Command::DOOR_OPEN_ALWAYS:
  case Command::DOOR_CLOSE:
    relayControl.controlRelay(command);
    break;
  default:
    break;
  }
  if (AddManager.slidingDoorMode_value)
  {
    relayControl.controlRelay(Command::WINDOW_DOWN); // RELAY2 接HALF模式
  }
  else
  {
    relayControl.controlRelay(Command::WINDOW_STOP); // 全开模式
  }
}
void WindowControl::controlCasementWindowRelay(Command command)
{

  switch (command)
  {
  case Command::CASEMENT_STOP:
  case Command::CASEMENT_STOP_ALT:
  case Command::CASEMENT_TILT:
  case Command::CASEMENT_TILT_ALT:
  case Command::CASEMENT_OPEN:
  case Command::CASEMENT_OPEN_ALT:
  case Command::CASEMENT_CLOSE:
  case Command::CASEMENT_CLOSE_ALT:
    relayControl.controlRelay(command);
    break;
  default:
    // Handle unknown commands
    break;
  }
}
void WindowControl::controlSkylightWindowRelay(Command command)
{
  switch (command)
  {
  case Command::SKYLIGHT_OPEN:
  case Command::SKYLIGHT_CLOSE:
    relayControl.controlRelay(command);
    break;
  case Command::SKYLIGHT_STOP:
    relayControl.controlRelay(Command::SKYLIGHT_OPEN);
    relayControl.controlRelay(Command::SKYLIGHT_CLOSE);
    break;

  case Command::RAIN_SIGNAL:
    if (AddManager.rainSignal_value)
      relayControl.controlRelay(Command::SKYLIGHT_CLOSE);
    break;
  }
}
void WindowControl::controlCurtainRelay(Command command)
{
  switch (command)
  {
  case Command::SCREEN_UP:
  case Command::SCREEN_DOWN:
  case Command::SCREEN_STOP:
  case Command::WINDOW_UP:
  case Command::WINDOW_STOP:
  case Command::WINDOW_DOWN:
    relayControl.controlRelay(command);
    break;
  default:
    // Handle unknown commands
    break;
  }
}
/**/
bool WindowControl::isWindowCommand(Command command)
{
  return command == Command::CASEMENT_TILT_ALT || command == Command::CASEMENT_OPEN_ALT || command == Command::CASEMENT_CLOSE_ALT || command == Command::CASEMENT_STOP_ALT;
}
void WindowControl::controlTransmitter(uint16_t windowType, Command command)
{
  bool isWindow = isWindowCommand(command);
  uint16_t bufferIndex = (isWindow ? static_cast<uint8_t>(ControlGroup::GROUP2) : static_cast<uint8_t>(ControlGroup::GROUP1)) - 1;
  switch (static_cast<WindowType>(windowType))
  {
  case WindowType::AUTOLIFTWINDOW:
    controlAutoLiftTransmitter(command, bufferIndex);
    break;
  case WindowType::AUTOSLIDINGDOOR:
    controlAutoSlidingDoorTransmitter(command, bufferIndex);
    break;
  case WindowType::TILT_TURNWINDOW:
  case WindowType::OUTWARDWINDOW:
    controlCasementWindowTransmitter(command, bufferIndex);
    break;
  case WindowType::SKYLIGHTWINDOW:
    controlSkylightWindowTransmitter(command, bufferIndex);
    break;
  case WindowType::CURTAIN:
    controlCurtainTransmitter(command, bufferIndex);
    break;
  }
}
void WindowControl::controlAutoLiftTransmitter(Command command, uint16_t bufferIndex)
{
  switch (command)
  {
  case Command::SCREEN_UP:
  case Command::SCREEN_DOWN:
  case Command::SCREEN_STOP:
  case Command::WINDOW_UP:
  case Command::WINDOW_STOP:
  case Command::WINDOW_DOWN:
    rfTransmitter.sendCode(command, AddManager.RF_buffer[bufferIndex], bufferIndex); // 发送自动升降窗的命令
    break;
  case Command::LIGHT_ON:
  case Command::LIGHT_OFF:
    // Handle unknown commands
    break;

  case Command::RAIN_SIGNAL:
    if (AddManager.rainSignal_value)
      rfTransmitter.sendCode(Command::WINDOW_UP, AddManager.RF_buffer[static_cast<uint8_t>(ControlGroup::GROUP2) - 1], static_cast<uint8_t>(ControlGroup::GROUP2) - 1);
    break;
  default:
    // Handle unknown commands
    break;
  }
}
void WindowControl::controlAutoSlidingDoorTransmitter(Command command, uint16_t bufferIndex)
{
}
void WindowControl::controlCasementWindowTransmitter(Command command, uint16_t bufferIndex)
{
  switch (command)
  {

  case Command::CASEMENT_STOP:
  case Command::CASEMENT_STOP_ALT:
  case Command::CASEMENT_TILT:
  case Command::CASEMENT_TILT_ALT:
  case Command::CASEMENT_OPEN:
  case Command::CASEMENT_OPEN_ALT:
  case Command::CASEMENT_CLOSE:
  case Command::CASEMENT_CLOSE_ALT:
    rfTransmitter.sendCode(command, AddManager.RF_buffer[bufferIndex], bufferIndex);
    break;
  case Command::RAIN_SIGNAL:
    if (AddManager.rainSignal_value)
    {
      rfTransmitter.sendCode(Command::CASEMENT_CLOSE, AddManager.RF_buffer[static_cast<uint8_t>(ControlGroup::GROUP1) - 1], static_cast<uint8_t>(ControlGroup::GROUP1) - 1);
      rfTransmitter.sendCode(Command::CASEMENT_CLOSE_ALT, AddManager.RF_buffer[static_cast<uint8_t>(ControlGroup::GROUP2) - 1], static_cast<uint8_t>(ControlGroup::GROUP2) - 1);
    };
    break;
  default:
    // Handle unknown commands
    break;
  }
}
void WindowControl::controlSkylightWindowTransmitter(Command command, uint16_t bufferIndex)
{
}
void WindowControl::controlCurtainTransmitter(Command command, uint16_t bufferIndex)
{
  switch (command)
  {
  case Command::SCREEN_UP:
  case Command::SCREEN_DOWN:
  case Command::SCREEN_STOP:
  case Command::WINDOW_UP:
  case Command::WINDOW_STOP:
  case Command::WINDOW_DOWN:
    rfTransmitter.sendCode(command, AddManager.RF_buffer[bufferIndex], bufferIndex); // 发送自动升降窗的命令
    break;

  default:
    // Handle unknown commands
    break;
  }
}

void WindowControl::controlCom(uint16_t windowType, Command command)
{
  switch (static_cast<WindowType>(windowType))
  {
  case WindowType::AUTOLIFTWINDOW:
    controlAutoLiftCom(command);
    break;
  case WindowType::AUTOSLIDINGDOOR:
    controlAutoSlidingDoorCom(command);
    break;
  case WindowType::TILT_TURNWINDOW:
  case WindowType::OUTWARDWINDOW:
    controlCasementWindowCom(command);
    break;
  case WindowType::SKYLIGHTWINDOW:
    controlSkylightWindowCom(command);
    break;
  case WindowType::CURTAIN:
    controlCurtainCom(command);
    break;
  }
}
void WindowControl::controlAutoLiftCom(Command command)
{

  switch (command)
  {
  case Command::SCREEN_UP:
  case Command::SCREEN_DOWN:
  case Command::SCREEN_STOP:
  case Command::WINDOW_UP:
  case Command::WINDOW_STOP:
  case Command::WINDOW_DOWN:
  case Command::LIGHT_ON:
  case Command::LIGHT_OFF:
    SERIAL_MANAGER.serial1SendCommand(command); // 发送自动升降窗的命令
    break;

  case Command::RAIN_SIGNAL:
    if (AddManager.rainSignal_value)
      SERIAL_MANAGER.serial1SendCommand(Command::WINDOW_UP); // 发送自动升降窗的命令
    break;
  case Command::ISWINDOW_SAFESENSOR:
  {
    AddManager.Is_security_value = static_cast<uint8_t>(antiClampMode::EXCEPT);
    updateAddress(AddManager.securityAddress, AddManager.Is_security_value);
  }
  break;

  case Command::ISWINDOW_DOWN:

    if (AddManager.Is_security_value)
    {
      AddManager.Is_security_value = static_cast<uint8_t>(antiClampMode::REFRESH);
      updateAddress(AddManager.securityAddress, AddManager.Is_security_value);
    }
    isMutualControl();

    break;
  default:
    // Handle unknown commands
    break;
  }
}
// 通用的地址更新函数
void WindowControl::updateAddress(uint8_t address, uint8_t value)
{
  uint8_t currentValue;
  eepromManager.readData(address, &currentValue, 1);
  // 只有在值变更时才写入EEPROM
  if (currentValue != value)
  {
    eepromManager.writeData(address, &value, 1);
  }
}
void WindowControl::controlAutoSlidingDoorCom(Command command)
{
}
void WindowControl::controlCasementWindowCom(Command command)
{
  switch (command)
  {
  case Command::CASEMENT_STOP:
  case Command::CASEMENT_STOP_ALT:
  case Command::CASEMENT_TILT:
  case Command::CASEMENT_TILT_ALT:
  case Command::CASEMENT_OPEN:
  case Command::CASEMENT_OPEN_ALT:
  case Command::CASEMENT_CLOSE:
  case Command::CASEMENT_CLOSE_ALT:
    // 发送操作窗的命令
    break;

  case Command::RAIN_SIGNAL:
    if (AddManager.rainSignal_value)
    { // comm1.sendUart1Data(Command::CASEMENT_CLOSE);  // 发送自动关窗的命令
      // comm1.sendUart1Data(Command::CASEMENT_CLOSE_ALT);
    };
    break;

  default:
    // Handle unknown commands
    break;
  }
}
void WindowControl::controlSkylightWindowCom(Command command)
{

  switch (command)
  {
  case Command::SCREEN_UP:
  case Command::SCREEN_DOWN:
  case Command::SCREEN_STOP:
  case Command::WINDOW_UP:
  case Command::WINDOW_STOP:
  case Command::WINDOW_DOWN:
    // 发送操作窗的命令
    break;

  case Command::RAIN_SIGNAL:
    if (AddManager.rainSignal_value)
    {
    };
    // comm1.sendUart1Data(Command::SKYLIGHT_CLOSE);  // 发送自动关窗的命令
    break;

  default:
    // Handle unknown commands
    break;
  }
}
void WindowControl::controlCurtainCom(Command command)
{
}
