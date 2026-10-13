#include "Sensors.h"
#include "Config.h"
#include <Wire.h>
#include <Adafruit_SHT31.h>

static Adafruit_SHT31 s_sht = Adafruit_SHT31();
static SensorData s_data = { 0.0f, 0.0f, false, 4.0f, 100, 2048 };

void Sensors_Init() {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 100000);
    s_data.shtAvailable = s_sht.begin(SHT30_I2C_ADDR);
    if (!s_data.shtAvailable) {
        // Retry address 0x45 in case of alternate strapping
        s_data.shtAvailable = s_sht.begin(0x45);
    }

    pinMode(PIN_BATTERY_ADC, INPUT);
    pinMode(PIN_LDR_ADC, INPUT);
    analogReadResolution(12);

    Sensors_Update();
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
    // ESP32 ADC: 0-4095 corresponds to ~0-3.3V (with 11dB attenuation default)
    float pinVolts = (adcRaw / 4095.0f) * 3.3f;
    s_data.batteryVolts = pinVolts * BATTERY_DIVIDER;
    
    // Percentage estimate (Li-ion curve: 3.30V = 0%, 4.18V = 100%)
    float pct = (s_data.batteryVolts - BATTERY_MIN_VOLTS) / (BATTERY_MAX_VOLTS - BATTERY_MIN_VOLTS) * 100.0f;
    s_data.batteryPercent = constrain((int)roundf(pct), 0, 100);

    // 3. Ambient light sensor (LDR)
    s_data.ldrRaw = analogRead(PIN_LDR_ADC);
}

const SensorData& Sensors_GetData() {
    return s_data;
}
