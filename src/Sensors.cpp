#include "Sensors.h"
#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <math.h>

static Adafruit_SHT31 s_sht = Adafruit_SHT31();
static SensorData s_data = { 0.0f, 0.0f, false, 4.0f, 100, 2048 };
static float s_smoothedLdr = 1800.0f;

void Sensors_Init() {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 100000);
    s_data.shtAvailable = s_sht.begin(SHT30_I2C_ADDR);
    if (!s_data.shtAvailable) {
        s_data.shtAvailable = s_sht.begin(0x45);
    }

    pinMode(PIN_BATTERY_ADC, INPUT);
    pinMode(PIN_LDR_ADC, INPUT);
    analogReadResolution(12);

    Sensors_Update();
    s_smoothedLdr = (float)s_data.ldrRaw;
}

void Sensors_Update() {
    // 1. SHT30 readout
    if (s_data.shtAvailable) {
        float t = s_sht.readTemperature();
        float h = s_sht.readHumidity();
        if (!isnan(t) && !isnan(h)) {
            s_data.temperature = t;
            s_data.humidity = h;
        }
    }

    // 2. Battery voltage via ADC divider
    uint32_t adcRaw = analogRead(PIN_BATTERY_ADC);
    float pinVolts = (adcRaw / 4095.0f) * 3.3f;
    s_data.batteryVolts = pinVolts * BATTERY_DIVIDER;
    
    // Percentage estimate (Li-ion curve: 3.30V = 0%, 4.18V = 100%)
    float pct = (s_data.batteryVolts - BATTERY_MIN_VOLTS) / (BATTERY_MAX_VOLTS - BATTERY_MIN_VOLTS) * 100.0f;
    s_data.batteryPercent = constrain((int)roundf(pct), 0, 100);

    // 3. Ambient light sensor (LDR) with exponential moving average filter
    s_data.ldrRaw = analogRead(PIN_LDR_ADC);
    s_smoothedLdr = (s_smoothedLdr * 0.82f) + ((float)s_data.ldrRaw * 0.18f);
}

const SensorData& Sensors_GetData() {
    return s_data;
}

uint16_t Sensors_GetSmoothedLDR() {
    return (uint16_t)roundf(s_smoothedLdr);
}

uint8_t Sensors_CalculateAutoBrightness(uint8_t minBri, uint8_t maxBri) {
    // Map smoothed LDR (200 to 3600) to brightness range with smooth gamma curve
    float norm = (s_smoothedLdr - 200.0f) / (3600.0f - 200.0f);
    norm = constrain(norm, 0.0f, 1.0f);
    float curved = powf(norm, 1.4f);
    uint8_t bri = minBri + (uint8_t)roundf(curved * (maxBri - minBri));
    return constrain(bri, minBri, maxBri);
}
