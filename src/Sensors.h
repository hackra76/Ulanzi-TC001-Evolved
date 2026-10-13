#pragma once
#include <Arduino.h>

struct SensorData {
    float temperature;   // Celsius
    float humidity;      // % RH
    bool  shtAvailable;  // SHT30 detected
    float batteryVolts;  // Volts (3.3 - 4.2V)
    int   batteryPercent;// 0 - 100%
    uint16_t ldrRaw;     // 0 - 4095
};

void Sensors_Init();
void Sensors_Update();
const SensorData& Sensors_GetData();
