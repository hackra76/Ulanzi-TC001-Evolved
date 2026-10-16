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
#define PREF_KEY_AUTO_ROTATE    "auto_rot"
#define PREF_KEY_ACTIVE_APP     "act_app"
#define PREF_KEY_ROTATE_SEC     "rot_sec"
#define PREF_KEY_CAROUSEL_MASK  "car_mask"   // Bitmask of enabled carousel screens (0x7F = all 7)
#define PREF_KEY_CAROUSEL_ORDER "car_ord"    // Comma-separated order string, e.g. "0,1,2,3,4,5,6"
#define DEFAULT_ROTATE_SEC      8
#define MIN_ROTATE_SEC          3
#define MAX_ROTATE_SEC          120

// --- Screen Transition Effects ---
#define PREF_KEY_TRANSITION_FX  "trans_fx"   // 0: None, 1: Slide Left, 2: Slide Right, 3: Slide Up, 4: Slide Down, 5: Crossfade, 6: Curtain, 7: Random
#define DEFAULT_TRANSITION_FX   1            // 1: Slide Left by default

// --- Battery Telemetry Customization ---
#define PREF_KEY_BATTERY_MODE   "bat_mode"   // 0: Percentage, 1: Voltage, 2: Alternating (Pct/Volt)

// --- Clock, Calendar & Weekday Bar Customization (Awtrix Style) ---
#define PREF_KEY_CLOCK_MODE     "clk_mode"   // 0: Time Only, 1: Combined (Time+Date), 2: Date Only
#define PREF_KEY_SHOW_SECONDS   "show_sec"   // 1: Show seconds, 0: Hide seconds
#define PREF_KEY_WEEKDAY_BAR    "wd_bar"     // 1: Show 7 weekday bars, 0: Hide
#define PREF_KEY_WD_START_MON   "wd_mon"     // 1: Start Monday, 0: Start Sunday
#define PREF_KEY_WD_ACTIVE_COL  "wd_acol"    // Active day color
#define PREF_KEY_WD_INACT_COL   "wd_icol"    // Inactive day color
#define PREF_KEY_WD_WKND_COL    "wd_wcol"    // Weekend day color (0: Same as active, 1: Red, etc.)
#define PREF_KEY_PORTAL_LANG    "ui_lang"    // Portal language ("en" or "sk")
#define PREF_KEY_DATE_FORMAT    "date_fmt"   // 0: DD.MM., 1: DD MMM, 2: MM/DD, 3: DAY DD
#define PREF_KEY_CLOCK_ICON     "clk_icon"   // 0: Dynamic Calendar, 1: Heart, 2: Clock Face, 3: None
#define PREF_KEY_TEXT_ALIGN     "txt_align"  // 0: Left-aligned with icon (Awtrix), 1: Centered

#define PREF_KEY_YT_CHANNEL     "yt_chan"
#define PREF_KEY_YT_APIKEY      "yt_key"
#define YT_FETCH_INTERVAL_MS    300000 // 5 minutes

// --- Ambient Intelligence & Screen Management ---
#define PREF_KEY_AUTO_BRI       "auto_bri"
#define PREF_KEY_NIGHT_MODE     "night_en"
#define PREF_KEY_NIGHT_START    "night_st"
#define PREF_KEY_NIGHT_END      "night_end"
#define PREF_KEY_DAY_BRI        "day_bri"
#define PREF_KEY_NIGHT_BRI      "ngt_bri"
#define DEFAULT_NIGHT_START     23
#define DEFAULT_NIGHT_END       7
#define DEFAULT_DAY_BRI         35
#define DEFAULT_NIGHT_BRI       2
#define MIN_AUTO_BRI            4
#define MAX_AUTO_BRI            85

// --- Retro Ambient Screensavers ---
#define PREF_KEY_SS_STYLE       "ss_style"
#define PREF_KEY_SS_TIMEOUT     "ss_time"
#define DEFAULT_SS_STYLE        0  // Matrix Digital Rain
#define DEFAULT_SS_TIMEOUT      0  // 0 = Disabled (carousel only)

// --- Outdoor Weather (Open-Meteo) ---
#define PREF_KEY_WEATHER_LAT    "w_lat"
#define PREF_KEY_WEATHER_LON    "w_lon"
#define PREF_KEY_WEATHER_CITY   "w_city"
#define DEFAULT_WEATHER_LAT     48.1486f
#define DEFAULT_WEATHER_LON     17.1077f
#define DEFAULT_WEATHER_CITY    "Bratislava"
#define WEATHER_FETCH_INTERVAL_MS 900000 // 15 minutes

// --- Crypto & Financial Tickers (Binance Public API) ---
#define PREF_KEY_CRYPTO_SYM     "c_sym"
#define DEFAULT_CRYPTO_SYM      "BTC"
#define CRYPTO_FETCH_INTERVAL_MS 60000 // 1 minute

// --- Home Assistant & MQTT Integration ---
#define PREF_KEY_MQTT_ENABLED    "mqtt_en"
#define PREF_KEY_MQTT_SERVER     "mqtt_srv"
#define PREF_KEY_MQTT_PORT       "mqtt_port"
#define PREF_KEY_MQTT_USER       "mqtt_user"
#define PREF_KEY_MQTT_PASS       "mqtt_pass"
#define PREF_KEY_MQTT_PREFIX     "mqtt_pre"
#define DEFAULT_MQTT_PORT        1883
#define DEFAULT_MQTT_PREFIX      "ulanzi-tc001"

// --- Firmware Metadata & OTA ---
#define FW_NAME             "Ulanzi TC001 Evolved"
#define FW_VERSION          "1.7.0"
#define GITHUB_REPO         "hackra76/Ulanzi-TC001-Evolved"



