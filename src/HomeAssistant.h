#ifndef __HOMEASSISTANT__H__
#define __HOMEASSISTANT__H__

#include <Arduino.h>

#include "pmbus.h"
#include "Version.h"
#include "Settings.h"


void HA_loop();
void HA_init(PMBus *pmbus);
void HA_setState(bool state);
float CpuTemperature(void);
void HA_tick(PMBus *pmbus);

extern byte mqttRetryTimes;
extern unsigned long lastMqttRetryTime; // mqtt重连时间间隔

#endif