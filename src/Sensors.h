#pragma once
#include <Arduino.h>
#include "Config.h"

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

uint16_t Sensors_GetSmoothedLDR();
uint8_t Sensors_CalculateAutoBrightness(uint8_t minBri = MIN_AUTO_BRI, uint8_t maxBri = MAX_AUTO_BRI);
