#include "WebService.h"
#include <ArduinoJson.h>
#include "EventHandle.h"
#include "Settings.h"
#include "HomeAssistant.h"

extern Settings settings;
extern EventHandle eventHandle;
extern TaskManager taskManager;
extern bool powerOn;

volatile bool restartPending = false;

void handleRequest(AsyncWebServerRequest *request){}
void handleUploadRequest(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final){}

WebService::WebService() : server(80)
{
    domianURL = "mynova.com";
}

void WebService::init(IPAddress ServerIP, WIFINetwork *wifiNetwork, PMBus *psu)
{
    Server_IP = ServerIP;
    pWifiNetwork = wifiNetwork;
    pPSU = psu;
    dnsserver.start(53, domianURL, Server_IP); // 设置DNS的端口、网址、和IP
}

void WebService::serverStart()
{
    if (!LittleFS.begin(true))
    {
        Serial.println("An Error has occurred while mounting LittleFS");
        return;
    }
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    Serial.println("Server on");
    server.on("/api/wifiscan", HTTP_GET, std::bind(&WebService::handleWIFIScanRequest, this, std::placeholders::_1));
    server.on("/api/wifisave", HTTP_POST, handleRequest, handleUploadRequest, std::bind(&WebService::handleWIFISaveRequest, this, std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4,std::placeholders::_5));
    server.on("/api/wificlear", HTTP_DELETE, std::bind(&WebService::handleWIFIClearRequest, this, std::placeholders::_1));
    server.on("/api/wificonnect", HTTP_GET, std::bind(&WebService::handleWIFIConnectRequest, this, std::placeholders::_1));
    //开启或关闭WIFI，wifienable
    server.on("/api/wifienable", HTTP_POST, handleRequest, handleUploadRequest,std::bind(&WebService::handleWIFIEnableRequest, this, std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4,std::placeholders::_5));
    //获取AP&WIF热点信息
    server.on("/api/apwifiinfo", HTTP_GET, std::bind(&WebService::handleAPWIFIInfoRequest, this, std::placeholders::_1));
    //修改AP热点信息
    server.on("/api/apsave", HTTP_POST, handleRequest, handleUploadRequest, std::bind(&WebService::handleAPSaveRequest, this, std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4,std::placeholders::_5));
    //开启或关闭AP热点
    server.on("/api/apenable", HTTP_POST, handleRequest, handleUploadRequest,std::bind(&WebService::handleAPEnableRequest, this, std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4,std::placeholders::_5));
    
    server.on("/api/mqttsave", HTTP_POST, handleRequest, handleUploadRequest, std::bind(&WebService::handleMqttSaveRequest, this, std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4,std::placeholders::_5));
    //开启或关闭MQTT
    server.on("/api/mqttenable", HTTP_POST, handleRequest, handleUploadRequest,std::bind(&WebService::handleMqttEnableRequest, this, std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4,std::placeholders::_5));

    server.on("/api/psuinfo", HTTP_GET, std::bind(&WebService::handlePSUInfoRequest, this, std::placeholders::_1));
    server.on("/api/psusetting", HTTP_GET, std::bind(&WebService::handlePSUSettingRequest, this, std::placeholders::_1));
    server.on("/api/psupower", HTTP_PUT, handleRequest, handleUploadRequest,std::bind(&WebService::handlePSUPowerRequest, this, std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4,std::placeholders::_5));
    server.on("/api/psufan", HTTP_PUT, handleRequest, handleUploadRequest,std::bind(&WebService::handlePSUFanRequest, this, std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4,std::placeholders::_5));
    // 上电自动开机
    server.on("/api/acpoweron", HTTP_POST, handleRequest, handleUploadRequest,std::bind(&WebService::handleACPowerOnRequest, this, std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4,std::placeholders::_5));
    // 任务管理
    server.on("/api/tasks/all", HTTP_GET, std::bind(&WebService::handleTasksRequest, this, std::placeholders::_1));
    server.on("/api/shutdown/timer", HTTP_GET, 
        std::bind(&WebService::handleGetShutdownTimerRequest, this, std::placeholders::_1));
    
    server.on("/api/shutdown/timer", HTTP_POST, handleRequest, handleUploadRequest,
        std::bind(&WebService::handleSetShutdownTimerRequest, this, std::placeholders::_1,
                 std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    
    server.on("/api/shutdown/timer", HTTP_DELETE,
        std::bind(&WebService::handleCancelShutdownTimerRequest, this, std::placeholders::_1));
    
    server.on("/api/tasks/period/add", HTTP_POST, handleRequest, handleUploadRequest,
        std::bind(&WebService::handleAddPeriodTaskRequest, this, std::placeholders::_1, std::placeholders::_2,
                 std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    
    server.on("/api/tasks/condition/add", HTTP_POST, handleRequest, handleUploadRequest,
        std::bind(&WebService::handleAddConditionTaskRequest, this, std::placeholders::_1, std::placeholders::_2,
                 std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    
    // 删除任务的路由
    server.on("/api/tasks/period/delete", HTTP_POST, handleRequest, handleUploadRequest,
        std::bind(&WebService::handleDeletePeriodTaskRequest, this, std::placeholders::_1,
                 std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    
    server.on("/api/tasks/condition/delete", HTTP_POST, handleRequest, handleUploadRequest,
        std::bind(&WebService::handleDeleteConditionTaskRequest, this, std::placeholders::_1,
                 std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    
    // 启用/禁用任务的路由
    server.on("/api/tasks/period/toggle", HTTP_POST, handleRequest, handleUploadRequest,
        std::bind(&WebService::handleTogglePeriodTaskRequest, this, std::placeholders::_1, std::placeholders::_2,
                 std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    
    server.on("/api/tasks/condition/toggle", HTTP_POST, handleRequest, handleUploadRequest,
        std::bind(&WebService::handleToggleConditionTaskRequest, this, std::placeholders::_1, std::placeholders::_2,
                 std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    
    // 同步时间的路由
    server.on("/api/synctime", HTTP_POST, handleRequest, handleUploadRequest,
        std::bind(&WebService::handleSyncTimeRequest, this, std::placeholders::_1,
                 std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));

    // 绑定 OTA 更新路由
    server.on(
        "/api/OTAupdate", HTTP_POST,
        [](AsyncWebServerRequest *request) {}, // 处理普通 POST 请求
        [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
            WebService::handleUploadRequest(request, filename, index, data, len, final);
        }
    );

    // 绑定文件系统 OTA 更新路由
    server.on("/api/FSupdate", HTTP_POST, 
        [](AsyncWebServerRequest *request) {},  // Empty handler for POST request
        [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
            WebService::handleFSUploadRequest(request, filename, index, data, len, final);
        }
    );
    
    // 绑定手动重启路由
    server.on("/api/reboot", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "设备即将重启...");
        Serial.println("Rebooting...");
        delay(2000);
        ESP.restart();
    });
    
    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "OK");  // 设备在线
    });
   

     // 处理所有非 API 路由
    server.onNotFound([](AsyncWebServerRequest *request) {
        // 如果是 API 请求，返回 404
        if (request->url().startsWith("/api/")) {
            request->send(404);
            return;
        }else{
            // 其他所有请求都返回 index.html
            request->send(LittleFS, "/index.html", "text/html");
        }
    });

    server.begin();
}

bool WebService::checkOTASpace(size_t fileSize) {
    const esp_partition_t* updatePartition = esp_ota_get_next_update_partition(NULL);
    if (!updatePartition) {
        Serial.println("无法获取 OTA 分区");
        return false;
    }

    size_t partitionSize = updatePartition->size;
    if (fileSize > partitionSize) {
        Serial.printf("OTA 空间不足: 需要 %uB,分区大小 %uB\n", fileSize, partitionSize);
        return false;
    }

    return true;
}

// 处理固件上传
void WebService::handleUploadRequest(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index) { // 仅在上传开始时执行
        Serial.printf("Update Start: %s\n", filename.c_str());

        size_t fileSize = request->contentLength();
        if (!checkOTASpace(fileSize)) {
            request->send(507, "text/plain", "OTA 空间不足");
            return;
        }

        if (!Update.begin(fileSize)) {
            Update.printError(Serial);
            request->send(500, "text/plain", "OTA 更新初始化失败");
            return;
        }
    }

    // 写入数据
    if (Update.write(data, len) != len) {
        Update.printError(Serial);
        request->send(500, "text/plain", "OTA 写入失败");
        return;
    }

    if (final) { // 最后一次数据包
        if (Update.end(true)) {
            Serial.printf("Update Success: %uB\n", index + len);
            request->send(200, "text/plain", "OK");
        } else {
            Update.printError(Serial);
            request->send(500, "text/plain", "OTA 更新失败");
        }
    }
}

bool WebService::checkFsSpace(size_t fileSize) {
    if (!LittleFS.begin(true)) {
        Serial.println("Failed to mount LittleFS");
        return false;
    }

    size_t totalBytes = LittleFS.totalBytes();  // Total storage space
    size_t usedBytes = LittleFS.usedBytes();    // Used space
    size_t freeSpace = totalBytes - usedBytes; // Available space

    Serial.printf("Filesystem space: Total: %uB, Used: %uB, Free: %uB\r\n", totalBytes, usedBytes, freeSpace);

    if (fileSize > totalBytes) {
        Serial.printf("Not enough space: Required %uB, Available %uB\r\n", fileSize, totalBytes);
        return false;
    }

    return true;
}

void WebService::handleFSUploadRequest(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
    
    size_t fileSize = request->contentLength();
    if (!index) {
        Serial.printf("File System Update Start: %s\r\n", filename.c_str());

        // 获取文件系统分区（SPIFFS）
        const esp_partition_t* fsPartition = esp_partition_find_first(
            ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, NULL
        );
        if (!fsPartition) {
            request->send(507, "text/plain", "文件系统分区未找到");
            return;
        }

        // 检查空间是否足够
        Serial.printf("File size: %uB\r\n", fileSize);
        if (!checkFsSpace(fileSize)) {
            Serial.println("ERROR: checkFsSpace() failed, aborting update.");
            request->send(507, "text/plain", "文件系统空间不足");
            return;
        }
        

        // 开始文件系统更新
        Serial.println("Formatting SPIFFS filesystem...");
        LittleFS.format();
        if (!Update.begin(fileSize, U_SPIFFS)) {
            Update.printError(Serial);
            Serial.printf("Update.hasError: %d\n", Update.hasError());
            request->send(500, "text/plain", "文件系统更新初始化失败");
            return;
        }
    }

    // 写入文件系统数据
    if (!Update.hasError()) {
        if (Update.write(data, fileSize) != fileSize) {
            Update.printError(Serial);
            request->send(500, "text/plain", "文件系统写入失败");
            return;
        }
    }

    // 完成更新
    if (final) {
        if (Update.end(true)) {
            Serial.printf("Filesystem update successful: %uB\r\n", index + fileSize);
            request->send(200, "text/plain", "文件系统更新成功");
        } else {
            Update.printError(Serial);
            request->send(500, "text/plain", "文件系统更新失败");
        }
    }
}


void WebService::processRequest()
{
    dnsserver.processNextRequest();
}

void WebService::handleWIFIScanRequest(AsyncWebServerRequest *request)
{
    request->send(200, "application/json", pWifiNetwork->wifi_json);
}

void WebService::handleWIFISaveRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
    String jsonStr =(char*)data;
  
    // 打印原始POST数据
    Serial.println(jsonStr);
    // 解析JSON数据
    JsonDocument doc; // 根据你的JSON数据大小调整容量
    DeserializationError error = deserializeJson(doc, jsonStr);
    if (error)
    {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        request->send(400, "application/json", "{\"result\":\"failed\",message:\"JSON parsing failed\"}");
        return;
    }
    // 处理解析后的JSON数据
    const char* ssid = doc["SSID"];
    const char* pwd = doc["PWD"];
    settings.ST_ssid = ssid;
    settings.ST_password = pwd;
    settings.enable_ST = true;
    eventHandle.saveSettings();
    // 返回响应
    request->send(200, "application/json", "{\"result\":\"OK\"}");

}

void WebService::handleWIFIClearRequest(AsyncWebServerRequest *request)
{
    eventHandle.disConnectWifi_ST();
    settings.ST_ssid = "";
    settings.ST_password = "";
    settings.enable_ST = false;
    eventHandle.saveSettings();
    // 返回响应
    request->send(200, "application/json", "{\"result\":\"OK\"}");
}

void WebService::handleWIFIConnectRequest(AsyncWebServerRequest *request)
{
    eventHandle.connectWifi_ST();
    settings.enable_ST = true;
    eventHandle.saveSettings();
    request->send(200, "application/json", "{\"result\":\"OK\"}");
}

void WebService::handleWIFIEnableRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
    String jsonStr =(char*)data;
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonStr);
    if (error)
    {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        request->send(400, "application/json", "{\"result\":\"failed\",message:\"JSON parsing failed\"}");
        return;
    }
    //开启或关闭WIFI
    if(doc["WIFI_ENABLE"].as<bool>())
    {
        pWifiNetwork->startWIFI();
        settings.enable_ST = true;
    }
    else
    {
        pWifiNetwork->disconnectWIFI();
        settings.enable_ST = false;
    }
    eventHandle.saveSettings();
    request->send(200, "application/json", "{\"result\":\"OK\"}");
}   

void WebService::handleAPWIFIInfoRequest(AsyncWebServerRequest *request)
{
    //返回AP热点的名称和密码
    JsonDocument json;
    json["WIFI_SSID"] = settings.ST_ssid;
    json["WIFI_PASSWORD"] = settings.ST_password;
    json["WIFI_ENABLE"] = settings.enable_ST;
    json["AP_SSID"] = settings.AP_ssid;
    json["AP_PWD"] = settings.AP_password;
    json["AP_ENABLE"] = settings.enable_AP;
    json["MQTT_IP"] = settings.MQTT_IP;
    json["MQTT_USER"] = settings.MQTT_USER;
    json["MQTT_PSW"] = settings.MQTT_PSW;
    json["MQTT_ENABLE"] = settings.enable_MQTT;
    String strJson;
    serializeJson(json, strJson);
    request->send(200, "application/json", strJson);
}   

void WebService::handleAPSaveRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
    //获取POST过来的AP热点名称和密码并保存
    String jsonStr =(char*)data;
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonStr);
    if (error)
    {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        request->send(400, "application/json", "{\"result\":\"failed\",message:\"JSON parsing failed\"}");
        return;
    }
    settings.AP_ssid = doc["AP_SSID"].as<String>();
    settings.AP_password = doc["AP_PWD"].as<String>();
    eventHandle.saveSettings();
    request->send(200, "application/json", "{\"result\":\"OK\"}");  
}   

void WebService::handleAPEnableRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
    String jsonStr =(char*)data;
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonStr);
    if (error)
    {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        request->send(400, "application/json", "{\"result\":\"failed\",message:\"JSON parsing failed\"}");
        return;
    }
    //开启或关闭AP热点
    if(doc["AP_ENABLE"].as<bool>())
    {
        pWifiNetwork->startAP();
        settings.enable_AP = true;
    }
    else
    {
        pWifiNetwork->disconnectAP();
        settings.enable_AP = false;
    }
    eventHandle.saveSettings();
    request->send(200, "application/json", "{\"result\":\"OK\"}");
}   

void WebService::handleMqttSaveRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
    //MQTT
    String jsonStr =(char*)data;
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonStr);
    if (error)
    {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        request->send(400, "application/json", "{\"result\":\"failed\",message:\"JSON parsing failed\"}");
        return;
    }
    settings.MQTT_IP = doc["MQTT_IP"].as<String>();
    settings.MQTT_USER = doc["MQTT_USER"].as<String>();
    settings.MQTT_PSW = doc["MQTT_PSW"].as<String>();
    eventHandle.saveSettings();
    request->send(200, "application/json", "{\"result\":\"OK\"}");  
}   

void WebService::handleMqttEnableRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
    String jsonStr =(char*)data;
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonStr);
    if (error)
    {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        request->send(400, "application/json", "{\"result\":\"failed\",message:\"JSON parsing failed\"}");
        return;
    }
    //开启或关闭MQTT
    if(doc["MQTT_ENABLE"].as<bool>())
    {
        settings.enable_MQTT = true;
        HA_init(pPSU);
    }
    else
    {
        settings.enable_MQTT = false;
    }
    eventHandle.saveSettings();
    request->send(200, "application/json", "{\"result\":\"OK\"}");
}   

void WebService::handlePSUInfoRequest(AsyncWebServerRequest *request)
{
    request->send(200, "application/json", pPSU->getPSUDataJson()); // 发送带有 JSON 内容
}

void WebService::handlePSUSettingRequest(AsyncWebServerRequest *request)
{
    JsonDocument json;
    json["POWER_SW"] = powerOn;
    json["FAN_SETTING"] = pPSU->fanSpeed;
    json["READ_FAN_SPEED_1"] = pPSU->fan[0];
    if (powerOn)
        pPSU->total_power_on = millis() - pPSU->turn_on_time; 
    else
        pPSU->total_power_on = 0;
    json["RUN_TIME"] = pPSU->total_power_on;
    json["AC_POWER_ON"] = settings.AC_PowerON;

    String strJson;
    serializeJson(json, strJson);
    request->send(200, "application/json", strJson);
}

void WebService::handlePSUPowerRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
    String jsonStr =(char*)data;
  
    // 打印原始POST数据
    Serial.println(jsonStr);
    // 解析JSON数据
    JsonDocument doc; // 根据你的JSON数据大小调整容量
    DeserializationError error = deserializeJson(doc, jsonStr);
    if (error)
    {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        request->send(400, "application/json", "{\"result\":\"failed\",message:\"JSON parsing failed\"}");
        return;
    }
    if(doc["POWER_ON"].as<bool>())
    {
        eventHandle.powerOn();
    }
    else
    {
        eventHandle.powerOff();
    }
    request->send(200, "application/json", "{\"result\":\"OK\"}");
}

void WebService::handlePSUFanRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
    String jsonStr =(char*)data;
  
    // 打印原始POST数据
    Serial.println(jsonStr);
    // 解析JSON数据
    JsonDocument doc; // 根据你的JSON数据大小调整容量
    DeserializationError error = deserializeJson(doc, jsonStr);
    if (error)
    {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        request->send(400, "application/json", "{\"result\":\"failed\",message:\"JSON parsing failed\"}");
        return;
    }

    eventHandle.setSpeed(doc["FAN_SPEED"].as<uint8_t>());
    request->send(200, "application/json", "{\"result\":\"OK\"}");
}

void WebService::handleACPowerOnRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
    String jsonStr = (char*)data;
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonStr);
    
    if (error) {
        request->send(400, "application/json", "{\"result\":\"failed\",\"message\":\"JSON parsing failed\"}");
        return;
    }

    settings.AC_PowerON = doc["AC_POWER_ON"].as<bool>();
    eventHandle.saveSettings();

    request->send(200, "application/json", "{\"result\":\"OK\"}");
}

void WebService::handleTasksRequest(AsyncWebServerRequest *request) {
    request->send(200, "application/json", taskManager.getAllTasksJson());
}

// 添加定时任务
void WebService::handleAddPeriodTaskRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    String jsonStr = (char*)data;
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonStr);
    
    if (error) {
        request->send(400, "application/json", "{\"result\":\"failed\",\"message\":\"JSON parsing failed\"}");
        return;
    }
    
    TimeTaskType type = static_cast<TimeTaskType>(doc["type"].as<int>());
    uint8_t startHour = doc["startHour"];
    uint8_t startMinute = doc["startMinute"];
    uint8_t endHour = doc["endHour"];
    uint8_t endMinute = doc["endMinute"];
    RepeatType repeatType = static_cast<RepeatType>(doc["repeatType"].as<int>());
    
    // 将数组转换为位图
    uint8_t weekDays = 0;
    JsonArray weekDaysArray = doc["weekDays"].as<JsonArray>();
    for (JsonVariant value : weekDaysArray) {
        int dayIndex = value.as<int>();
        switch(dayIndex) {
            case 0:
                weekDays |= SUNDAY;    // 0x01
                break;
            case 1:
                weekDays |= MONDAY;    // 0x02
                break;
            case 2:
                weekDays |= TUESDAY;   // 0x04
                break;
            case 3:
                weekDays |= WEDNESDAY; // 0x08
                break;
            case 4:
                weekDays |= THURSDAY;  // 0x10
                break;
            case 5:
                weekDays |= FRIDAY;    // 0x20
                break;
            case 6:
                weekDays |= SATURDAY;  // 0x40
                break;
        }
    }
    
    uint32_t taskId = taskManager.addPeriodTask(type, startHour, startMinute, 
                                              endHour, endMinute, repeatType, weekDays, true);
    eventHandle.saveSettings();
    eventHandle.printAlarms();

    JsonDocument response;
    response["result"] = "OK";
    response["taskId"] = taskId;
    
    String output;
    serializeJson(response, output);
    request->send(200, "application/json", output);
}

// 添加条件任务
void WebService::handleAddConditionTaskRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    String jsonStr = (char*)data;
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonStr);
    
    if (error) {
        request->send(400, "application/json", "{\"result\":\"failed\",\"message\":\"JSON parsing failed\"}");
        return;
    }
    
    ConditionType conditionType = static_cast<ConditionType>(doc["ConditionType"].as<int>());
    ConditionQuantity conditionQuantity = static_cast<ConditionQuantity>(doc["ConditionQuantity"].as<int>());
    CompareType compareType = static_cast<CompareType>(doc["CompareType"].as<int>());
    float conditionValue = doc["ConditionValue"];
    uint16_t conditionMinutes = doc["ConditionMinutes"];
    
    uint32_t taskId = taskManager.addConditionTask(conditionType, conditionQuantity, compareType, conditionValue, conditionMinutes, true);
    eventHandle.saveSettings();
    
    JsonDocument response;
    response["result"] = "OK";
    response["taskId"] = taskId;
    
    String output;
    serializeJson(response, output);
    request->send(200, "application/json", output);
}

// 修改处理函数
void WebService::handleDeletePeriodTaskRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    String jsonStr = (char*)data;
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonStr);
    
    if (error) {
        request->send(400, "application/json", "{\"result\":\"failed\",\"message\":\"JSON parsing failed\"}");
        return;
    }
    
    uint32_t taskId = doc["taskId"].as<uint32_t>();
    taskManager.removePeriodTask(taskId);
    eventHandle.saveSettings();
    eventHandle.printAlarms();

    request->send(200, "application/json", "{\"result\":\"OK\"}");
}

void WebService::handleDeleteConditionTaskRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    String jsonStr = (char*)data;
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonStr);
    
    if (error) {
        request->send(400, "application/json", "{\"result\":\"failed\",\"message\":\"JSON parsing failed\"}");
        return;
    }
    
    uint32_t taskId = doc["taskId"].as<uint32_t>();
    taskManager.removeConditionTask(taskId);
    eventHandle.saveSettings();

    request->send(200, "application/json", "{\"result\":\"OK\"}");
}

// 启用/禁用任务的处理函数
void WebService::handleTogglePeriodTaskRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    String jsonStr = (char*)data;
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonStr);
    
    if (error) {
        request->send(400, "application/json", "{\"result\":\"failed\",\"message\":\"JSON parsing failed\"}");
        return;
    }
    
    uint8_t taskId = doc["taskId"].as<uint8_t>();
    bool toggle = doc["toggle"].as<bool>();
    
    taskManager.togglePeriodTask(taskId, toggle);
    eventHandle.saveSettings();
    eventHandle.printAlarms();

    JsonDocument response;
    response["result"] = "OK";
    String output;
    serializeJson(response, output);
    request->send(200, "application/json", output);
}

// 启用/禁用任务的处理函数
void WebService::handleToggleConditionTaskRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    String jsonStr = (char*)data;
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonStr);
    
    if (error) {
        request->send(400, "application/json", "{\"result\":\"failed\",\"message\":\"JSON parsing failed\"}");
        return;
    }
    
    uint8_t taskId = doc["taskId"].as<uint8_t>();
    bool toggle = doc["toggle"].as<bool>();
    
    taskManager.toggleConditionTask(taskId, toggle);
    eventHandle.saveSettings();
    
    JsonDocument response;
    response["result"] = "OK";
    String output;
    serializeJson(response, output);
    request->send(200, "application/json", output);
}

void WebService::handleGetShutdownTimerRequest(AsyncWebServerRequest *request) {
    JsonDocument doc;
    doc["active"] = taskManager._shutdownTimerActive;
    doc["remainingSeconds"] = taskManager._shutdownTime;
    
    String output;
    serializeJson(doc, output);
    request->send(200, "application/json", output);
}

void WebService::handleSetShutdownTimerRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    String jsonStr = (char*)data;
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonStr);
    
    if (error) {
        request->send(400, "application/json", "{\"result\":\"failed\",\"message\":\"JSON parsing failed\"}");
        return;
    }
    
    uint32_t seconds = doc["seconds"].as<uint32_t>();
    taskManager.setShutdownTimer(seconds);
    request->send(200, "application/json", "{\"result\":\"OK\"}");
}

void WebService::handleCancelShutdownTimerRequest(AsyncWebServerRequest *request) {
    taskManager.cancelShutdownTimer();
    request->send(200, "application/json", "{\"result\":\"OK\"}");
}

void WebService::handleSyncTimeRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
   
    // 将时间同步请求交给事件处理系统
    eventHandle.syncTime();
    request->send(200, "application/json", "{\"result\":\"OK\"}");
}

