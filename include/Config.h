#pragma once
#include <Arduino.h>

// =============================================================================
//  Ulanzi TC001 Smart Pixel Desktop Clock
//  Hardware Pinout & Configuration
// =============================================================================

// --- 8x32 WS2812B RGB NeoPixel Matrix ---
#define PIN_MATRIX          32
#define MATRIX_WIDTH        32
#define MATRIX_HEIGHT       8
#define MATRIX_NUM_LEDS     (MATRIX_WIDTH * MATRIX_HEIGHT) // 256 LEDs
#define DEFAULT_BRIGHTNESS  35   // 0 - 255 (keep <= 80 on USB power to prevent heat/brownout)
#define MAX_BRIGHTNESS      120

// --- I2C Bus & SHT30 Environment Sensor ---
#define PIN_I2C_SDA         21
#define PIN_I2C_SCL         22
#define SHT30_I2C_ADDR      0x44

// --- Navigation Buttons (Active LOW with internal pull-up) ---
#define PIN_BTN_LEFT        26
#define PIN_BTN_OK          27
#define PIN_BTN_RIGHT       14
#define BTN_DEBOUNCE_MS     40
#define BTN_LONG_PRESS_MS   700

// --- Passive/Active Buzzer ---
#define PIN_BUZZER          15
#define BUZZER_LEDC_CH      0
#define BUZZER_LEDC_RES     8

// --- Power & Analog Telemetry ---
#define PIN_BATTERY_ADC     34   // ADC1 CH6 (resistor divider 1:1, max ~4.2V battery)
#define PIN_LDR_ADC         33   // ADC1 CH5 (Light Dependent Resistor / photoresistor)
#define BATTERY_DIVIDER     2.0f // R1=100k, R2=100k divider factor
#define BATTERY_MIN_VOLTS   3.3f // 0%
#define BATTERY_MAX_VOLTS   4.18f// 100%

// --- Wi-Fi & Provisioning Configuration ---
#define WIFI_AP_SSID_PREFIX     "Ulanzi-Setup-"
#define WIFI_CONNECT_TIMEOUT_MS 12000
#define PREF_NAMESPACE          "ulanzi_cfg"
#define PREF_KEY_SSID           "wifi_ssid"
#define PREF_KEY_PASS           "wifi_pass"

// --- Carousel & YouTube Analytics ---
#define PREF_KEY_ROTATE_SEC     "rot_sec"
#define DEFAULT_ROTATE_SEC      8
#define MIN_ROTATE_SEC          3
#define MAX_ROTATE_SEC          120

#define PREF_KEY_YT_CHANNEL     "yt_chan"
#define PREF_KEY_YT_APIKEY      "yt_key"
#define YT_FETCH_INTERVAL_MS    300000 // 5 minutes

// --- Firmware Metadata ---
#define FW_NAME             "Ulanzi TC001 Evolved"
#define FW_VERSION          "1.0.0"

