/*
 * Copyright (c) 2024 Tomosawa
 * https://github.com/Tomosawa/
 * All rights reserved
 */
#ifndef __WIFINETWORK_H_
#define __WIFINETWORK_H_
#include <Arduino.h>
#include <WiFi.h>

class WIFINetwork
{
public:
    WIFINetwork();
    void init();
    void startAP();
    void startWIFI();
    void reconnectWIFI();
    void disconnectWIFI();
    void disconnectAP();
    IPAddress getWIFIIP();
    IPAddress getAPIP();
    wl_status_t getWIFIState();
    String getWIFIStateInfo();
    String getAllSSIDJson();
public:
    IPAddress AP_local_ip; // IP地址
    IPAddress AP_gateway;  // 网关地址
    IPAddress AP_subnet;   // 子网掩码
    String wifi_json;

};

#endif