#include "APManager.h"
static std::vector<uint8_t> requestBody;
APManager::APManager(uint16_t port)
    : port(port), server(port) {}

void APManager::begin()
{
    APmanager.EEPROMInitialize();
    seedRandomGenerator();
    generateAPCredentials(ssid, password);

    if (!SPIFFS.begin(true))
    {
        Serial.println("Failed to mount file system");
        return;
    }

    Serial.println("Configuring access point...");
    Serial.print("SSID: ");
    Serial.println(ssid.c_str());
    Serial.print("Password: ");
    Serial.println(password.c_str());

    if (WiFi.softAP(ssid.c_str(), password.c_str()))
    {
        IPAddress myIP = WiFi.softAPIP();
        Serial.print("AP IP address: ");
        Serial.println(myIP);
    }
    else
    {
        Serial.println("Failed to start Access Point");
        return;
    }

    setupServer();
}

void APManager::setupServer()
{
    server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

    server.on("/get_values", HTTP_GET, [this](AsyncWebServerRequest *request)
              { this->handleGetValues(request); });
    server.on("/set_values", HTTP_POST,
              // onRequest (在请求头解析完后触发)
              [](AsyncWebServerRequest *request) {},
              // onUpload (一般用于上传文件，这里不用就空lambda)
              [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {},
              // onBody 回调
              [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
              {
        // index==0，表示body开始
        if (index == 0) {
            requestBody.clear();
            // 提前为vector分配空间，减少反复realloc
            requestBody.reserve(total);
            Serial.println("Received POST request to /set_values (start)");
        }

        // 把本次分块 data 追加到 requestBody
        requestBody.insert(requestBody.end(), data, data + len);

        // 如果收完了（index + len == total），说明body完整拿到了
        if (index + len == total) {
            Serial.printf("Received POST request to /set_values, total length=%u\n", total);

            // 统一解析 JSON
            DynamicJsonDocument doc(4096); // 适当加大
            DeserializationError error = deserializeJson(doc, requestBody.data(), requestBody.size());
            if (error) {
                Serial.print("Failed to parse JSON in /set_values: ");
                Serial.println(error.c_str());
                request->send(400, "application/json", R"({"status":"error","message":"Invalid JSON"})");
                return;
            }

            // 如果能解析成功就调用你真正的处理函数
            JsonObject json = doc.as<JsonObject>();
            handleSetValues(request, json);
        } });

    server.on("/factory-reset", HTTP_POST, nullptr, nullptr, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
              {
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, data, len);
        if (error) {
            Serial.println("Failed to parse JSON");
            request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
            return;
        }
        JsonObject json = doc.as<JsonObject>();
        factoryReset(request, json); });

    server.on("/log", HTTP_POST, nullptr, nullptr, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
              {
        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, data, len);
        if (error) {
            Serial.print("Failed to parse log JSON: ");
            Serial.println(error.c_str());
            request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
            return;
        }

        JsonObject json = doc.as<JsonObject>();
        if (json.containsKey("log")) {
           /*  Serial.print("Debug log: ");
            Serial.println(json["log"].as<String>()); */
        }

        request->send(200, "application/json", "{\"status\":\"success\"}"); });

    server.begin();
    Serial.println("Server started");
}

void APManager::seedRandomGenerator()
{
    uint32_t seed = static_cast<uint32_t>(time(0)) ^ (ESP.getEfuseMac() & 0xFFFFFFFF);
    std::srand(seed);
}

uint16_t APManager::getRandomNumber(uint16_t min, uint16_t max)
{
    return min + std::rand() % ((max + 1) - min);
}

void APManager::generateAPCredentials(std::string &ssid, std::string &password)
{
    std::stringstream ssidStream, passwordStream;

    uint16_t randomX = getRandomNumber(0, 15);
    std::stringstream ss;
    ss << std::hex << randomX;

    uint16_t randomY = getRandomNumber(0, 255);
    std::stringstream yy;
    yy << std::hex << randomY;
    std::string randomYHex = yy.str();
    if (randomYHex.length() == 1)
    {
        randomYHex = "0" + randomYHex;
    }

    ssidStream << "PHONPA" << ss.str() << randomYHex;
    ssid = ssidStream.str();

    passwordStream << "PHONPA00" << ss.str();
    password = passwordStream.str();
}

void APManager::handleGetValues(AsyncWebServerRequest *request)
{
    DynamicJsonDocument doc(2048); // 增加缓冲区大小

    // 创建 RF 工作模式选项
    JsonArray rfWorkingModeOptions = doc.createNestedArray("RFworkingModeOptions");

    JsonObject hansReceiver = rfWorkingModeOptions.createNestedObject();
    hansReceiver["value"] = (uint8_t)RFworkMode::HANS_RECEIVER;
    hansReceiver["label"] = "HANS接收模式";

    JsonObject hansTransmitter = rfWorkingModeOptions.createNestedObject();
    hansTransmitter["value"] = (uint8_t)RFworkMode::HANS_TRANSMITTER;
    hansTransmitter["label"] = "HANS发射模式";

    JsonObject hopoReceiver = rfWorkingModeOptions.createNestedObject();
    hopoReceiver["value"] = (uint8_t)RFworkMode::HOPO_RECEIVER;
    hopoReceiver["label"] = "HOPO接收模式";

    JsonObject hopoTransmitter = rfWorkingModeOptions.createNestedObject();
    hopoTransmitter["value"] = (uint8_t)RFworkMode::HOPO_TRANSMITTER;
    hopoTransmitter["label"] = "HOPO发射模式";

    JsonObject hansBoth = rfWorkingModeOptions.createNestedObject();
    hansBoth["value"] = (uint8_t)RFworkMode::HANS_BOTH;
    hansBoth["label"] = "HANS收发模式";

    JsonObject hansHopo = rfWorkingModeOptions.createNestedObject();
    hansHopo["value"] = (uint8_t)RFworkMode::HANS_HOPO;
    hansHopo["label"] = "HANS收HOPO发模式";

    JsonObject hopoHans = rfWorkingModeOptions.createNestedObject();
    hopoHans["value"] = (uint8_t)RFworkMode::HOPO_HANS;
    hopoHans["label"] = "HOPO收HANS发模式";

    // 创建配对模式选项
    JsonArray rfPairingModeOptions = doc.createNestedArray("RFpairingModeOptions");

    JsonObject workMode = rfPairingModeOptions.createNestedObject();
    workMode["value"] = (uint8_t)Pairing::PAIR_OUT_TO_WORK;
    workMode["label"] = "工作模式";

    JsonObject hans1 = rfPairingModeOptions.createNestedObject();
    hans1["value"] = (uint8_t)Pairing::HANS_1;
    hans1["label"] = "HANS频道1配对";

    JsonObject hans2 = rfPairingModeOptions.createNestedObject();
    hans2["value"] = (uint8_t)Pairing::HANS_2;
    hans2["label"] = "HANS频道2配对";

    JsonObject hopo1 = rfPairingModeOptions.createNestedObject();
    hopo1["value"] = (uint8_t)Pairing::HOPO_1;
    hopo1["label"] = "HOPO频道1配对";

    JsonObject hopo2 = rfPairingModeOptions.createNestedObject();
    hopo2["value"] = (uint8_t)Pairing::HOPO_2;
    hopo2["label"] = "HOPO频道2配对";

    // 添加新的配对模式 HANS_WIRELESS 和 HOPO_WIRELESS
    JsonObject hansWireless = rfPairingModeOptions.createNestedObject();
    hansWireless["value"] = (uint8_t)Pairing::HANS_WIRELESS;
    hansWireless["label"] = "HANS无线风雨配对";

    JsonObject hopoWireless = rfPairingModeOptions.createNestedObject();
    hopoWireless["value"] = (uint8_t)Pairing::HOPO_WIRELESS;
    hopoWireless["label"] = "HOPO无线风雨配对";

    // ...保持其他选项不变...

    JsonArray controlGroupOptions = doc.createNestedArray("controlGroupOptions");
    JsonObject group1 = controlGroupOptions.createNestedObject();
    group1["value"] = (uint8_t)(ControlGroup::GROUP1);
    group1["label"] = "1组";

    JsonObject group2 = controlGroupOptions.createNestedObject();
    group2["value"] = (uint8_t)(ControlGroup::GROUP2);
    group2["label"] = "2组";

    JsonObject group3 = controlGroupOptions.createNestedObject();
    group3["value"] = (uint8_t)(ControlGroup::ALL);
    group3["label"] = "总控";
    /*
        JsonArray rfModeOptions = doc.createNestedArray("RFmodeOptions");
        JsonObject hansMode = rfModeOptions.createNestedObject();
        hansMode["value"] = RF_HANS_MODE;
        hansMode["label"] = "HANS协议";

        JsonObject hopoMode = rfModeOptions.createNestedObject();
        hopoMode["value"] = RF_HOPO_MODE;
        hopoMode["label"] = "HOPO协议";
    */
    JsonArray windowTypeOptions = doc.createNestedArray("windowTypeOptions");
    JsonObject autoliftWindow = windowTypeOptions.createNestedObject();
    autoliftWindow["value"] = (uint8_t)(WindowType::AUTOLIFTWINDOW);
    autoliftWindow["label"] = "电动升降窗";

    JsonObject autoSlidingDoor = windowTypeOptions.createNestedObject();
    autoSlidingDoor["value"] = (uint8_t)(WindowType::AUTOSLIDINGDOOR);
    autoSlidingDoor["label"] = "电动推拉门";

    JsonObject outwardWindow = windowTypeOptions.createNestedObject();
    outwardWindow["value"] = (uint8_t)(WindowType::OUTWARDWINDOW);
    outwardWindow["label"] = "电动外平开窗";

    JsonObject tiltTurnWindow = windowTypeOptions.createNestedObject();
    tiltTurnWindow["value"] = (uint8_t)(WindowType::TILT_TURNWINDOW);
    tiltTurnWindow["label"] = "电动内开内倒窗";

    JsonObject skylightWindow = windowTypeOptions.createNestedObject();
    skylightWindow["value"] = (uint8_t)(WindowType::SKYLIGHTWINDOW);
    skylightWindow["label"] = "电动上悬/平推/天窗";

    JsonObject curtain = windowTypeOptions.createNestedObject();
    curtain["value"] = (uint8_t)(WindowType::CURTAIN);
    curtain["label"] = "电动卷帘/百叶";

    JsonArray rainSignalOptions = doc.createNestedArray("RainSignalOptions");
    JsonObject turnOff = rainSignalOptions.createNestedObject();
    turnOff["value"] = (uint8_t)(rainSignalMode::TURNOFF);
    turnOff["label"] = "关闭";

    JsonObject wireless = rainSignalOptions.createNestedObject();
    wireless["value"] = (uint8_t)(rainSignalMode::WIRELESS);
    wireless["label"] = "无线";

    JsonObject wired = rainSignalOptions.createNestedObject();
    wired["value"] = (uint8_t)(rainSignalMode::WIRED);
    wired["label"] = "有线";

    doc["mysversion"] = VERSION;
    // doc["HANS_"] = RF_HANS_MODE;
    // doc["HOPO_"] = RF_HOPO_MODE;
    doc["windowType"] = ADDmanager.windowType_value;
    // doc["RFmode"] = ADDmanager.RFmode_value;
    doc["controlGroup"] = ADDmanager.controlGroup_value;
    doc["RFworkingMode"] = ADDmanager.RFworkingMode_value;
    doc["RFpairingMode"] = ADDmanager.RFpairingMode_value;
    doc["localadd"] = ADDmanager.localadd_value;
    doc["RainSignal"] = ADDmanager.rainSignal_value;

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void APManager::handleSetValues(AsyncWebServerRequest *request, JsonObject json)
{
    if (!json.isNull())
    {
        /* Serial.println("\n=== Received JSON data ===");
        String jsonStr;
        serializeJson(json, jsonStr);
        Serial.println(jsonStr); */

        bool success = true; // 跟踪所有写入操作是否成功

        // 处理每个字段并检查写入结果
        if (json.containsKey("localadd"))
        {
            uint8_t localAdd = json["localadd"].as<uint8_t>();
            ADDmanager.localadd_value = localAdd;
            if (!APmanager.writeData(ADDmanager.localAddress, &ADDmanager.localadd_value, sizeof(ADDmanager.localadd_value)))
            {
                // Serial.println("Failed to write localadd");
                success = false;
            }
        }

        if (json.containsKey("RFworkingMode"))
        {
            ADDmanager.RFworkingMode_value = json["RFworkingMode"].as<uint8_t>();
            if (!APmanager.writeData(ADDmanager.RFworkingModeAddress, &ADDmanager.RFworkingMode_value, sizeof(ADDmanager.RFworkingMode_value)))
            {
                Serial.println("Failed to write RFworkingMode");
                success = false;
            }
        }

        if (json.containsKey("controlGroup"))
        {
            ADDmanager.controlGroup_value = json["controlGroup"].as<uint8_t>();
            if (!APmanager.writeData(ADDmanager.controlGroupAddress, &ADDmanager.controlGroup_value, sizeof(ADDmanager.controlGroup_value)))
            {
                Serial.println("Failed to write controlGroup");
                success = false;
            }
        }

        if (json.containsKey("RainSignal"))
        {
            ADDmanager.rainSignal_value = json["RainSignal"].as<uint8_t>();
            if (!APmanager.writeData(ADDmanager.rainSignalAddress, &ADDmanager.rainSignal_value, sizeof(ADDmanager.rainSignal_value)))
            {
                Serial.println("Failed to write RainSignal");
                success = false;
            }
        }

        if (json.containsKey("windowType"))
        {
            ADDmanager.windowType_value = json["windowType"].as<uint8_t>();
            if (!APmanager.writeData(ADDmanager.windowTypeAddress, &ADDmanager.windowType_value, sizeof(ADDmanager.windowType_value)))
            {
                Serial.println("Failed to write windowType");
                success = false;
            }
        }

        if (json.containsKey("RFpairingMode"))
        {
            ADDmanager.RFpairingMode_value = json["RFpairingMode"].as<uint8_t>();
            // if (!APmanager.writeData(ADDmanager.RFpairingModeAddress, &ADDmanager.RFpairingMode_value, sizeof(ADDmanager.RFpairingMode_value)))
            {
                Serial.println(" RFpairingMode");
            }
        }
        /*
                if (json.containsKey("RFmode")) {
                    ADDmanager.RFmode_value = json["RFmode"].as<uint8_t>();
                    if (!APmanager.writeData(ADDmanager.RFmodeAddress, &ADDmanager.RFmode_value, sizeof(ADDmanager.RFmode_value))) {
                        Serial.println("Failed to write RFmode");
                        success = false;
                    }
                }
        */
        if (json.containsKey("HOPOtrans"))
        {
            ADDmanager.HOPOTransmit = json["HOPOtrans"].as<uint8_t>();
            if (!APmanager.writeData(ADDmanager.securityAddress, &ADDmanager.HOPOTransmit, sizeof(ADDmanager.HOPOTransmit)))
            {
                Serial.println("Failed to write HOPOtrans");
                success = false;
            }
        }

        if (success)
        {
            Serial.println("All values saved successfully");
            DynamicJsonDocument responseDoc(128);
            responseDoc["status"] = "success";
            responseDoc["message"] = "参数已保存成功";
            String response;
            serializeJson(responseDoc, response);
            request->send(200, "application/json", response);
        }
        else
        {
            Serial.println("Failed to save some values");
            DynamicJsonDocument errorDoc(128);
            errorDoc["status"] = "error";
            errorDoc["message"] = "保存参数失败";
            String response;
            serializeJson(errorDoc, response);
            request->send(500, "application/json", response);
        }
    }
    else
    {
        Serial.println("Received null JSON object");
        request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON data\"}");
    }
}

void APManager::factoryReset(AsyncWebServerRequest *request, JsonObject json)
{
    DynamicJsonDocument doc(128);
    doc["status"] = "success";
    doc["message"] = "恢复出厂设置已启动";

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);

    shouldReset = true;
    resetStartTime = millis();
}

void APManager::handleReset()
{
    if (shouldReset && (millis() - resetStartTime > restDelayTime))
    {
        WiFi.mode(WIFI_OFF);
        delay(100);
        uint8_t defaultVal_buf[EEPROM_SIZE];
        memset(defaultVal_buf, INIT_DATA, EEPROM_SIZE);
        if (APmanager.writeData(0, defaultVal_buf, EEPROM_SIZE))
        {
            ESP.restart();
        }
    }
}

void APManager::loop()
{
    handleReset();
}
