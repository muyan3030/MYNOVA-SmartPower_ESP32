/*
 * Copyright (c) 2024 Tomosawa
 * https://github.com/Tomosawa/
 * All rights reserved
 */
#ifndef __WEBSERVICE__H__
#define __WEBSERVICE__H__
#include <Arduino.h>
#include "ESPAsyncWebServer.h"
#include <Update.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <DNSServer.h>
//#include <SPIFFS.h>
#include <LittleFS.h>
#include "WIFINetwork.h"
#include "pmbus.h"
class WebService
{
public:
    WebService();
    void init(IPAddress ServerIP, WIFINetwork *wifiNetwork, PMBus *psu);
    void serverStart();
    void processRequest();

    void handleWIFIScanRequest(AsyncWebServerRequest *request);
    void handleWIFISaveRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    void handleWIFIClearRequest(AsyncWebServerRequest *request);
    void handleWIFIConnectRequest(AsyncWebServerRequest *request);
    void handleWIFIEnableRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    void handleAPWIFIInfoRequest(AsyncWebServerRequest *request);   
    void handleAPSaveRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    void handleAPEnableRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    
    void handleMqttSaveRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    void handleMqttEnableRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);

    void handlePSUInfoRequest(AsyncWebServerRequest *request);
    void handlePSUSettingRequest(AsyncWebServerRequest *request);
    void handlePSUPowerRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    void handlePSUFanRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    void handleACPowerOnRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    void handleTasksRequest(AsyncWebServerRequest *request);

    void handleAddPeriodTaskRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    void handleAddConditionTaskRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);

    void handleDeletePeriodTaskRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    void handleDeleteConditionTaskRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);

    void handleTogglePeriodTaskRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    void handleToggleConditionTaskRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);

    void handleGetShutdownTimerRequest(AsyncWebServerRequest *request);
    void handleSetShutdownTimerRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    void handleCancelShutdownTimerRequest(AsyncWebServerRequest *request);
    void handleSyncTimeRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    
    void handleOTAUpdateRequest(AsyncWebServerRequest *request);
private:
    DNSServer dnsserver;
    AsyncWebServer server;
    String domianURL;    // 访问的域名
    IPAddress Server_IP; // 要访问的地址的IP
    WIFINetwork *pWifiNetwork;
    PMBus *pPSU;
    
    static void handleUploadRequest(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final);
    static void handleFSUploadRequest(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final);
    static bool checkOTASpace(size_t fileSize);
    static bool checkFsSpace(size_t fileSize);
};

#endif