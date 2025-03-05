#ifndef APMANAGER_H
#define APMANAGER_H

#include <WiFi.h>
#include <WiFiManager.h>
#include <WiFiClient.h>
#include "EEPROMManager.h"
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include "Config.h"
#include <cstdlib>
#include <ctime>
#include <esp_system.h>
#include <string>
#include <sstream>

class APManager {
public:
    APManager(uint16_t port,address_Manager &AddManager);
    void begin();
    void update();

private:
    std::string ssid;
    std::string password;
    uint16_t port;
    AsyncWebServer server;
    EEPROMManager APmanager;
    bool shouldReset = false;
    uint32_t resetStartTime = 0;
    uint32_t restDelayTime = 3000;
    address_Manager AddManager;
    void setupServer();
    void seedRandomGenerator();
    uint16_t getRandomNumber(uint16_t min, uint16_t max);
    void generateAPCredentials(std::string &ssid, std::string &password);
    void handleGetValues(AsyncWebServerRequest *request);
    void handleSetValues(AsyncWebServerRequest *request, JsonObject json);
    void factoryReset(AsyncWebServerRequest *request, JsonObject json);
    void handleReset();
};

#endif
