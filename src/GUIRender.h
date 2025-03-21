/*
 * Copyright (c) 2024 Tomosawa
 * https://github.com/Tomosawa/
 * All rights reserved
 */
#ifndef __GUIRENDER_H_
#define __GUIRENDER_H_
#include <Arduino.h>
#include "pmbus.h"
#include "WIFINetwork.h"
#include "Version.h"

class GUIRender
{
public:
    GUIRender();
    void init();
    void drawOLED(int displayPage);
    void setContrast(int contrast);
    void setValues(PMBus *pmbus, WIFINetwork *wifiNetwork);
public:
    float V_in = 0.000;
    float I_in = 0.000;
    float W_in = 0.000;
    float V_out = 0.000;
    float I_out = 0.000;
    float W_out = 0.000;
    float Temp1 = 0.0;
    float Temp2 = 0.0;
    float Temp3 = 0.0;
    float Fan1 = 0.000;
    int FanSpeed = 0;
    int menuSel = 0;//选择的菜单项
    int animeFrame = 0;
    int frameDown = 2;
    int leftBtnFrame = 0;
    WIFINetwork *wifiNetwork;
};

#endif