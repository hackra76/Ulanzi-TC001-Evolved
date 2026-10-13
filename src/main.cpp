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

// --- Application Modes ---
enum AppMode {
    APP_CLOCK = 0,
    APP_WEATHER,
    APP_AIRSPACE,
    APP_BATTERY,
    APP_COUNT
};

static AppMode s_currentApp = APP_CLOCK;
static uint32_t s_lastAppSwitch = 0;
static bool s_autoRotate = true;
static const uint32_t AUTO_ROTATE_MS = 8000;

// Clock seconds counter (synced with NTP or fallback increment)
static uint32_t s_seconds = 12 * 3600 + 34 * 60; // 12:34:00 default
static uint32_t s_lastSecTick = 0;
static bool s_ntpSynced = false;

// Wi-Fi Onboarding & Splash Ticker State
static bool s_ipSplashDone = false;
static bool s_skipAP = false;
static uint8_t s_ipSplashPasses = 0;

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

static void renderClockApp() {
    DisplayMatrix_Clear();
    
    // Draw pulsing heart or wifi indicator
    bool blink = (s_seconds % 2) == 0;
    DisplayMatrix_DrawIcon(0, 0, ICON_HEART);

    // Format HH:MM
    int h = (s_seconds / 3600) % 24;
    int m = (s_seconds / 60) % 60;
    char buf[10];
    snprintf(buf, sizeof(buf), "%02d%c%02d", h, blink ? ':' : ' ', m);

    DisplayMatrix_DrawText(9, 1, buf, C_WHITE);
    DisplayMatrix_Show();
}

static void renderWeatherApp() {
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
        case APP_WEATHER:  renderWeatherApp();  break;
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

    // 2. Clock second counter tick
    if (now - s_lastSecTick >= 1000) {
        s_lastSecTick += 1000;
        s_seconds++;
        checkNtpSync();
    }

    // 3. Poll buttons
    Buttons_Poll();

    // 4. Handle initial provisioning / AP mode screen
    if (Network_IsAPActive() && !s_skipAP) {
        // Allow skipping AP mode on button press
        if (Buttons_GetEvent(BTN_ID_OK) == BTN_SHORT_CLICK ||
            Buttons_GetEvent(BTN_ID_RIGHT) == BTN_SHORT_CLICK) {
            Buzzer_Click();
            s_skipAP = true;
            DisplayMatrix_ResetTicker();
            return;
        }

        // Scroll AP SSID and IP across display with pinned AP icon
        String apTicker = "AP: " + Network_GetAPSSID() + "  IP: " + Network_GetAPIP() + "  ";
        DisplayMatrix_ScrollTicker(apTicker.c_str(), C_ORANGE, ICON_AP, 45);
        return;
    }

    // 5. Handle connecting state ticker
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

    // 6. Handle post-connect IP announcement ticker
    if (Network_IsConnected() && !s_ipSplashDone) {
        String ipTicker = "Connected! IP: " + Network_GetLocalIP() + "   ";
        bool passComplete = DisplayMatrix_ScrollTicker(ipTicker.c_str(), C_GREEN, ICON_WIFI, 40);
        if (passComplete) {
            s_ipSplashPasses++;
            if (s_ipSplashPasses >= 1) { // Announce IP pass
                s_ipSplashDone = true;
                Buzzer_Click();
                DisplayMatrix_ResetTicker();
                s_lastAppSwitch = now;
            }
        }
        return;
    }

    // 7. Standard App Navigation & Button Events
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
        // Cycle brightness
        uint8_t cur = DisplayMatrix_GetBrightness();
        uint8_t next = (cur <= 20) ? 35 : (cur <= 45 ? 75 : 15);
        DisplayMatrix_SetBrightness(next);
    } else if (Buttons_GetEvent(BTN_ID_OK) == BTN_LONG_PRESS) {
        Buzzer_Alert();
        s_autoRotate = !s_autoRotate;
    }

    // 8. Auto rotation tick
    if (s_autoRotate && (now - s_lastAppSwitch >= AUTO_ROTATE_MS)) {
        s_lastAppSwitch = now;
        s_currentApp = (AppMode)((s_currentApp + 1) % APP_COUNT);
    }

    // 9. Sensor background update (every 2 seconds)
    static uint32_t s_lastSensorUpdate = 0;
    if (now - s_lastSensorUpdate >= 2000) {
        s_lastSensorUpdate = now;
        Sensors_Update();
    }

    // 10. Render active app screen (~20 FPS)
    static uint32_t s_lastRender = 0;
    if (now - s_lastRender >= 50) {
        s_lastRender = now;
        renderCurrentApp();
    }
}
