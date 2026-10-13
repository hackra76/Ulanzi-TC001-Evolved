#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "Config.h"
#include "DisplayMatrix.h"
#include "Sensors.h"
#include "Buttons.h"
#include "Buzzer.h"
#include "Icons.h"
#include "NetworkManager.h"
#include "YouTubeService.h"
#include "WeatherService.h"
#include "CryptoService.h"

// --- Application Modes ---
enum AppMode {
    APP_CLOCK = 0,
    APP_INDOOR,     // Local SHT30 environment
    APP_OUTDOOR,    // Live Open-Meteo weather
    APP_CRYPTO,     // Live Binance Crypto price
    APP_YOUTUBE,    // Live YouTube Subscriber count
    APP_AIRSPACE,   // Live ADS-B radar flight ticker
    APP_BATTERY,    // Battery & power telemetry
    APP_COUNT
};

static AppMode s_currentApp = APP_CLOCK;
static uint32_t s_lastAppSwitch = 0;
static bool s_autoRotate = true;

// Clock seconds counter (synced with NTP or fallback increment)
static uint32_t s_seconds = 12 * 3600 + 34 * 60; // 12:34:00 default
static uint32_t s_lastSecTick = 0;
static bool s_ntpSynced = false;

// Wi-Fi Onboarding & Splash Ticker State
static bool s_ipSplashDone = false;
static bool s_skipAP = false;
static uint8_t s_ipSplashPasses = 0;

// Night Mode Wake State
static uint32_t s_nightWakeUntilMs = 0;

static void checkNtpSync() {
    if (Network_IsConnected() && !s_ntpSynced) {
        configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.google.com");
        struct tm timeinfo;
        if (getLocalTime(&timeinfo, 100)) {
            s_seconds = timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60 + timeinfo.tm_sec;
            s_ntpSynced = true;
            Serial.printf("[NTP] Synchronized time: %02d:%02d:%02d\n",
                          timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        }
    }
}

static bool isNightTime() {
    if (!Network_GetNightMode()) return false;
    int h = (s_seconds / 3600) % 24;
    uint8_t start = Network_GetNightStartHour();
    uint8_t end = Network_GetNightEndHour();
    if (start > end) { // e.g. 23:00 to 07:00
        return (h >= start || h < end);
    } else { // e.g. 01:00 to 06:00
        return (h >= start && h < end);
    }
}

static void renderClockApp() {
    DisplayMatrix_Clear();
    
    bool blink = (s_seconds % 2) == 0;
    DisplayMatrix_DrawIcon(0, 0, ICON_HEART);

    int h = (s_seconds / 3600) % 24;
    int m = (s_seconds / 60) % 60;
    char buf[10];
    snprintf(buf, sizeof(buf), "%02d%c%02d", h, blink ? ':' : ' ', m);

    DisplayMatrix_DrawText(9, 1, buf, C_WHITE);
    DisplayMatrix_Show();
}

static void renderNightClock() {
    DisplayMatrix_Clear();
    
    // Dim red night clock with crescent moon
    DisplayMatrix_DrawIcon(0, 0, ICON_MOON);

    int h = (s_seconds / 3600) % 24;
    int m = (s_seconds / 60) % 60;
    char buf[10];
    snprintf(buf, sizeof(buf), "%02d:%02d", h, m);

    DisplayMatrix_DrawText(9, 1, buf, 0x8000); // Dim Red (RGB565)
    DisplayMatrix_Show();
}

static void renderIndoorApp() {
    DisplayMatrix_Clear();
    const SensorData& s = Sensors_GetData();

    DisplayMatrix_DrawIcon(0, 0, ICON_TEMP);

    char buf[12];
    if (s.shtAvailable) {
        snprintf(buf, sizeof(buf), "%.1fC", s.temperature);
    } else {
        snprintf(buf, sizeof(buf), "21.5C");
    }
    DisplayMatrix_DrawText(9, 1, buf, C_YELLOW);
    DisplayMatrix_Show();
}

static void renderOutdoorApp() {
    DisplayMatrix_Clear();

    const uint16_t* icon = Weather_GetConditionIcon();
    DisplayMatrix_DrawIcon(0, 0, icon);

    const char* tempStr = Weather_GetFormattedTemp();
    DisplayMatrix_DrawText(9, 1, tempStr, C_CYAN);
    DisplayMatrix_Show();
}

static void renderCryptoApp() {
    DisplayMatrix_Clear();

    const uint16_t* icon = Crypto_GetIcon();
    DisplayMatrix_DrawIcon(0, 0, icon);

    const char* priceStr = Crypto_GetFormattedPrice();
    uint16_t col = (strcmp(Crypto_GetSymbol(), "ETH") == 0) ? C_CYAN : C_YELLOW;
    DisplayMatrix_DrawText(9, 1, priceStr, col);
    DisplayMatrix_Show();
}

static void renderYouTubeApp() {
    DisplayMatrix_Clear();
    DisplayMatrix_DrawIcon(0, 0, ICON_YOUTUBE);

    const char* subs = YouTube_GetFormattedSubs();
    DisplayMatrix_DrawText(9, 1, subs, C_WHITE);
    DisplayMatrix_Show();
}

static void renderAirspaceApp() {
    DisplayMatrix_Clear();
    DisplayMatrix_DrawIcon(0, 0, ICON_PLANE);
    DisplayMatrix_DrawText(9, 1, "RYR45", C_CYAN);
    DisplayMatrix_Show();
}

static void renderBatteryApp() {
    DisplayMatrix_Clear();
    const SensorData& s = Sensors_GetData();

    DisplayMatrix_DrawIcon(0, 0, ICON_BATTERY);

    char buf[10];
    snprintf(buf, sizeof(buf), "%d%%", s.batteryPercent);
    uint16_t col = (s.batteryPercent > 40) ? C_GREEN : (s.batteryPercent > 15 ? C_YELLOW : C_RED);
    DisplayMatrix_DrawText(9, 1, buf, col);
    DisplayMatrix_Show();
}

static void renderCurrentApp() {
    switch (s_currentApp) {
        case APP_CLOCK:    renderClockApp();    break;
        case APP_INDOOR:   renderIndoorApp();   break;
        case APP_OUTDOOR:  renderOutdoorApp();  break;
        case APP_CRYPTO:   renderCryptoApp();   break;
        case APP_YOUTUBE:  renderYouTubeApp();  break;
        case APP_AIRSPACE: renderAirspaceApp(); break;
        case APP_BATTERY:  renderBatteryApp();  break;
        default:           renderClockApp();    break;
    }
}

void setup() {
    Serial.begin(115200);
    Serial.printf("\n=========================================\n");
    Serial.printf("  %s v%s\n", FW_NAME, FW_VERSION);
    Serial.printf("  Board: Ulanzi TC001 (8x32 WS2812B NeoPixel)\n");
    Serial.printf("=========================================\n");

    Buzzer_Init();
    Buttons_Init();
    Sensors_Init();
    DisplayMatrix_Init();
    YouTube_Init();
    Weather_Init();
    Crypto_Init();

    // Startup splash & sound
    Buzzer_Click();
    DisplayMatrix_Clear();
    DisplayMatrix_DrawIcon(0, 0, ICON_SUN);
    DisplayMatrix_DrawText(9, 1, "TC001", C_ORANGE);
    DisplayMatrix_Show();
    delay(1200);

    // Initialize Network & Captive Portal
    Network_Init();

    s_lastAppSwitch = millis();
    s_lastSecTick = millis();
}

void loop() {
    uint32_t now = millis();

    // 1. Service Network state machine & Captive Portal WebServer
    Network_Poll();

    // 2. Background HTTP Services polling
    YouTube_Update();
    Weather_Update();
    Crypto_Update();

    // 3. Clock second counter tick
    if (now - s_lastSecTick >= 1000) {
        s_lastSecTick += 1000;
        s_seconds++;
        checkNtpSync();

        // Auto-Brightness check (smoothly updates when enabled and not in night mode)
        bool activeNight = isNightTime() && (now >= s_nightWakeUntilMs);
        if (activeNight) {
            DisplayMatrix_SetBrightness(1); // Ultra dim for sleeping bedroom
        } else if (Network_GetAutoBrightness()) {
            uint8_t autoBri = Sensors_CalculateAutoBrightness(MIN_AUTO_BRI, MAX_AUTO_BRI);
            DisplayMatrix_SetBrightness(autoBri);
        }
    }

    // 4. Poll buttons
    Buttons_Poll();

    // 5. Handle initial provisioning / AP mode screen
    if (Network_IsAPActive() && !s_skipAP) {
        if (Buttons_GetEvent(BTN_ID_OK) == BTN_SHORT_CLICK ||
            Buttons_GetEvent(BTN_ID_RIGHT) == BTN_SHORT_CLICK) {
            Buzzer_Click();
            s_skipAP = true;
            DisplayMatrix_ResetTicker();
            return;
        }

        String apTicker = "AP: " + Network_GetAPSSID() + "  IP: " + Network_GetAPIP() + "  ";
        DisplayMatrix_ScrollTicker(apTicker.c_str(), C_ORANGE, ICON_AP, 45);
        return;
    }

    // 6. Handle connecting state ticker
    if (Network_GetState() == NET_STATE_CONNECTING && !s_skipAP) {
        if (Buttons_GetEvent(BTN_ID_OK) == BTN_SHORT_CLICK) {
            Buzzer_Click();
            s_skipAP = true;
            return;
        }
        String connTicker = "Connecting: " + Network_GetSSID() + " ... ";
        DisplayMatrix_ScrollTicker(connTicker.c_str(), C_CYAN, ICON_WIFI, 45);
        return;
    }

    // 7. Handle post-connect IP announcement ticker
    if (Network_IsConnected() && !s_ipSplashDone) {
        String ipTicker = "Connected! IP: " + Network_GetLocalIP() + "   ";
        bool passComplete = DisplayMatrix_ScrollTicker(ipTicker.c_str(), C_GREEN, ICON_WIFI, 40);
        if (passComplete) {
            s_ipSplashPasses++;
            if (s_ipSplashPasses >= 1) {
                s_ipSplashDone = true;
                Buzzer_Click();
                DisplayMatrix_ResetTicker();
                s_lastAppSwitch = now;
            }
        }
        return;
    }

    // 8. Night Mode enforcement
    bool nightActive = isNightTime() && (now >= s_nightWakeUntilMs);
    if (nightActive) {
        // Any button press temporarily wakes the screen for 12 seconds
        if (Buttons_GetEvent(BTN_ID_OK) != BTN_NONE ||
            Buttons_GetEvent(BTN_ID_LEFT) != BTN_NONE ||
            Buttons_GetEvent(BTN_ID_RIGHT) != BTN_NONE) {
            Buzzer_Click();
            s_nightWakeUntilMs = now + 12000;
            DisplayMatrix_SetBrightness(20);
        } else {
            // Render quiet dim red clock without rotating apps
            renderNightClock();
            return;
        }
    }

    // 9. Standard App Navigation & Button Events
    if (Buttons_GetEvent(BTN_ID_RIGHT) == BTN_SHORT_CLICK) {
        Buzzer_Click();
        s_currentApp = (AppMode)((s_currentApp + 1) % APP_COUNT);
        s_lastAppSwitch = now;
        s_autoRotate = false;
    } else if (Buttons_GetEvent(BTN_ID_LEFT) == BTN_SHORT_CLICK) {
        Buzzer_Click();
        s_currentApp = (AppMode)((s_currentApp - 1 + APP_COUNT) % APP_COUNT);
        s_lastAppSwitch = now;
        s_autoRotate = false;
    } else if (Buttons_GetEvent(BTN_ID_OK) == BTN_SHORT_CLICK) {
        Buzzer_Click();
        // Cycle manual brightness (pauses auto-brightness until reboot or web portal re-enable)
        Network_SetAutoBrightness(false);
        uint8_t cur = DisplayMatrix_GetBrightness();
        uint8_t next = (cur <= 20) ? 35 : (cur <= 45 ? 75 : 15);
        DisplayMatrix_SetBrightness(next);
    } else if (Buttons_GetEvent(BTN_ID_OK) == BTN_LONG_PRESS) {
        Buzzer_Alert();
        s_autoRotate = !s_autoRotate;
    }

    // 10. Auto rotation carousel tick with dynamic rotate delay from Settings
    uint32_t rotateMs = (uint32_t)Network_GetRotateDelaySec() * 1000UL;
    if (s_autoRotate && (now - s_lastAppSwitch >= rotateMs)) {
        s_lastAppSwitch = now;
        s_currentApp = (AppMode)((s_currentApp + 1) % APP_COUNT);
    }

    // 11. Sensor background update (every 2 seconds)
    static uint32_t s_lastSensorUpdate = 0;
    if (now - s_lastSensorUpdate >= 2000) {
        s_lastSensorUpdate = now;
        Sensors_Update();
    }

    // 12. Render active app screen (~20 FPS)
    static uint32_t s_lastRender = 0;
    if (now - s_lastRender >= 50) {
        s_lastRender = now;
        renderCurrentApp();
    }
}
