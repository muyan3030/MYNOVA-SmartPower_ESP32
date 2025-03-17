#include "HomeAssistant.h"
#include "Version.h"
#include "pmbus.h"
#include <ArduinoHA.h>
#include <WiFi.h>
#include "EventHandle.h"

extern bool powerOn;
extern Settings settings;
extern EventHandle eventHandle;

WiFiClient clientHA;
HADevice device;
HAMqtt HAmqtt(clientHA, device);

byte
    mac[6],
    mqttRetryTimes = 0;

unsigned long lastMqttRetryTime = 0; // mqtt重连时间间隔

HASwitch switch1("mySwitch1"); // 输出控制

HASensorNumber analogSensor1("myAnalogInput1", HASensorNumber::PrecisionP3); // 输入电压
HASensorNumber analogSensor2("myAnalogInput2", HASensorNumber::PrecisionP3); // 输入电流
HASensorNumber analogSensor3("myAnalogInput3", HASensorNumber::PrecisionP3); // 输入功率

HASensorNumber analogSensor4("myAnalogInput4", HASensorNumber::PrecisionP3);   // 输出电压
HASensorNumber analogSensor5("myAnalogInput5", HASensorNumber::PrecisionP3);   // 输出电流
HASensorNumber analogSensor6("myAnalogInput6", HASensorNumber::PrecisionP3);   // 输出功率

HASensorNumber analogSensor7("myAnalogInput7", HASensorNumber::PrecisionP3);
HASensorNumber analogSensor8("myAnalogInput8", HASensorNumber::PrecisionP3);
HASensorNumber analogSensor9("myAnalogInput9", HASensorNumber::PrecisionP3);
HASensorNumber analogSensor10("myAnalogInput10", HASensorNumber::PrecisionP3);
HASensorNumber analogSensor11("myAnalogInput11", HASensorNumber::PrecisionP2); // 主控温度

HASensorNumber uptimeSensor1("myUptime1");

HABinarySensor sensor("myInput");


String byteToHexString(uint8_t value)
{
  char buffer[3];
  snprintf(buffer, sizeof(buffer), "%02X", value);
  return String(buffer);
}

void onSwitchCommand(bool state, HASwitch *sender)
{
    if (sender == &switch1)
    {
        if (state == true)
        {
            eventHandle.powerOn();
        }
        else
        {
            eventHandle.powerOff();
        }
        // digitalWrite(RelayPin[0], LightData.LightOn[0]); // 长按打开或者关闭输出
    }
    sender->setState(state); // report state back to the Home Assistant
}

void HA_init(PMBus *psu)
{
    if(!settings.enable_MQTT){
        Serial.println("MQTT Disabled");
        return;
    }

    if(settings.MQTT_IP.isEmpty())
    {
        Serial.println("MQTT Server IP is empty");
        return;
    }

    WiFi.macAddress(mac);
    String apSSID = String("PSU_") + byteToHexString(mac[3]) + byteToHexString(mac[4]) + byteToHexString(mac[5]);

    device.setUniqueId(mac, sizeof(mac));
    device.setName((const char *)apSSID.c_str());
    device.setManufacturer(psu->mfr_id.c_str());
    device.setModel(psu->mfr_model.c_str());
    device.setSoftwareVersion(psu->mfr_revision.c_str());
    device.enableExtendedUniqueIds();

    switch1.setName("Power Switch");
    switch1.setIcon("mdi:lightbulb");
    switch1.onCommand(onSwitchCommand);

    analogSensor1.setIcon("mdi:eye");
    analogSensor1.setName("Input voltage");
    analogSensor1.setUnitOfMeasurement("V");

    analogSensor2.setIcon("mdi:eye");
    analogSensor2.setName("Input Current");
    analogSensor2.setUnitOfMeasurement("A");

    analogSensor3.setIcon("mdi:eye");
    analogSensor3.setName("Input Power");
    analogSensor3.setUnitOfMeasurement("W");

    analogSensor4.setIcon("mdi:eye");
    analogSensor4.setName("Output Voltage");
    analogSensor4.setUnitOfMeasurement("V");

    analogSensor5.setIcon("mdi:eye");
    analogSensor5.setName("Output Current");
    analogSensor5.setUnitOfMeasurement("A");

    analogSensor6.setIcon("mdi:eye");
    analogSensor6.setName("Output Power");
    analogSensor6.setUnitOfMeasurement("W");

    analogSensor7.setIcon("mdi:eye");
    analogSensor7.setName("Temperature 1");
    analogSensor11.setUnitOfMeasurement("℃");

    analogSensor8.setIcon("mdi:eye");
    analogSensor8.setName("Temperature 2");
    analogSensor11.setUnitOfMeasurement("℃");

    analogSensor9.setIcon("mdi:eye");
    analogSensor9.setName("Temperature 3");
    analogSensor11.setUnitOfMeasurement("℃");

    analogSensor10.setIcon("mdi:eye");
    analogSensor10.setName("Fan Speed");
    analogSensor11.setUnitOfMeasurement("RPM");

    analogSensor11.setIcon("mdi:eye");
    analogSensor11.setName("MCU Temperature");
    analogSensor11.setUnitOfMeasurement("℃");

    uptimeSensor1.setIcon("mdi:home");
    uptimeSensor1.setName("Total Power_on");
    uptimeSensor1.setUnitOfMeasurement("s");

    sensor.setCurrentState(powerOn);
    sensor.setName("PowerStatus sensor");
    sensor.setDeviceClass("power");
    sensor.setIcon("mdi:power-plug");

    // device.enableSharedAvailability();

    device.enableLastWill();

    // 将 IP 字符串转换为 IPAddress
    IPAddress mqttIp;
    mqttIp.fromString(settings.MQTT_IP);

    HAmqtt.begin(mqttIp, settings.MQTT_USER.c_str(), settings.MQTT_PSW.c_str());
    
    Serial.println("MQTT Server is Ready");
}

void HA_setState(bool state)
{
    sensor.setState(state);
    if (state)
        sensor.setIcon("mdi:power-plug");
    else
        sensor.setIcon("mdi:power-plug-off");
}

void HA_loop()
{
    if(!settings.enable_MQTT){
        return;
    }

    if (mqttRetryTimes > 5) // 大于5次重试失败，则不再重试，防止死循环，等1分钟再尝试
    {
        if ((millis() - lastMqttRetryTime > 60000) || HAmqtt.isConnected())
            mqttRetryTimes = 0;
    }
    else
    {
        HAmqtt.loop();
        if (!HAmqtt.isConnected())
        {
            mqttRetryTimes++;
            lastMqttRetryTime = millis();
        }
        else
        {
            HA_setState(powerOn);
            device.setAvailability(true);
            mqttRetryTimes = 0;
        }
    }
}

float CpuTemperature(void)
{
#ifdef CONFIG_IDF_TARGET_ESP32
    return (float)temperatureRead(); // In Celsius
/*
  // These jumps are not stable either. Sometimes it jumps to 77.3
  float t = (float)temperatureRead();  // In Celsius
  if (t > 81) { t = t - 27.2; }        // Fix temp jump observed on some ESP32 like DualR3
  return t;
*/
#else
    // Currently (20210801) repeated calls to temperatureRead() on ESP32C3 and ESP32S2 result in IDF error messages
    static float t = NAN;
    if (isnan(t))
    {
        t = (float)temperatureRead(); // In Celsius
    }
    return t;
#endif
}

void HA_tick(PMBus *psu)
{
    if(!settings.enable_MQTT){
        return;
    }
    // 处理传感器数据
    analogSensor1.setValue(psu->V_in);
    analogSensor2.setValue(psu->I_in);
    analogSensor3.setValue(psu->W_in);

    analogSensor4.setValue(psu->V_out);
    analogSensor5.setValue(psu->I_out);
    analogSensor6.setValue(psu->W_out);

    analogSensor7.setValue(psu->T[0]);
    analogSensor8.setValue(psu->T[1]);
    analogSensor9.setValue(psu->T[2]);
    analogSensor10.setValue(psu->fan[0]);
    analogSensor11.setValue(CpuTemperature());

    uptimeSensor1.setValue(static_cast<int32_t>(psu->total_power_on));
}