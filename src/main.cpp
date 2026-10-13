#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "Config.h"
#include "DisplayMatrix.h"
#include "Sensors.h"
#include "Buttons.h"
#include "Buzzer.h"
#include "Icons.h"

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

// Simulated clock counter if NTP is not yet synced
static uint32_t s_seconds = 12 * 3600 + 34 * 60; // 12:34:00 default
static uint32_t s_lastSecTick = 0;

static void renderClockApp() {
    DisplayMatrix_Clear();
    
    // Draw icon or pulsating heart on left (columns 0..7)
    bool blink = (s_seconds % 2) == 0;
    DisplayMatrix_DrawIcon(0, 0, ICON_HEART);

    // Format HH:MM
    int h = (s_seconds / 3600) % 24;
    int m = (s_seconds / 60) % 60;
    char buf[10];
    snprintf(buf, sizeof(buf), "%02d%c%02d", h, blink ? ':' : ' ', m);

    // Text starts at x=9, y=0 (font 5x7 fits neatly on 8 rows)
    DisplayMatrix_DrawText(9, 1, buf, C_WHITE);
    DisplayMatrix_Show();
}

static void renderWeatherApp() {
    DisplayMatrix_Clear();
    const SensorData& s = Sensors_GetData();

    // Icon on left
    DisplayMatrix_DrawIcon(0, 0, ICON_TEMP);

    // SHT30 temperature text: e.g. "23.5C"
    char buf[12];
    if (s.shtAvailable) {
        snprintf(buf, sizeof(buf), "%.1fC", s.temperature);
    } else {
        snprintf(buf, sizeof(buf), "21.5C"); // demo fallback
    }
    DisplayMatrix_DrawText(9, 1, buf, C_YELLOW);
    DisplayMatrix_Show();
}

static void renderAirspaceApp() {
    DisplayMatrix_Clear();
    DisplayMatrix_DrawIcon(0, 0, ICON_PLANE);

    // Live ADS-B radar flight ticker mockup
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

    // Welcome chirp & startup splash
    Buzzer_Click();
    DisplayMatrix_Clear();
    DisplayMatrix_DrawIcon(0, 0, ICON_SUN);
    DisplayMatrix_DrawText(9, 1, "TC001", C_ORANGE);
    DisplayMatrix_Show();
    delay(1200);

    s_lastAppSwitch = millis();
    s_lastSecTick = millis();
}

void loop() {
    uint32_t now = millis();

    // 1. Clock second tick
    if (now - s_lastSecTick >= 1000) {
        s_lastSecTick += 1000;
        s_seconds++;
    }

    // 2. Poll buttons & handle events
    Buttons_Poll();

    if (Buttons_GetEvent(BTN_ID_RIGHT) == BTN_SHORT_CLICK) {
        Buzzer_Click();
        s_currentApp = (AppMode)((s_currentApp + 1) % APP_COUNT);
        s_lastAppSwitch = now;
        s_autoRotate = false; // pause auto rotation on manual user interaction
    } else if (Buttons_GetEvent(BTN_ID_LEFT) == BTN_SHORT_CLICK) {
        Buzzer_Click();
        s_currentApp = (AppMode)((s_currentApp - 1 + APP_COUNT) % APP_COUNT);
        s_lastAppSwitch = now;
        s_autoRotate = false;
    } else if (Buttons_GetEvent(BTN_ID_OK) == BTN_SHORT_CLICK) {
        Buzzer_Click();
        // Cycle brightness: 15 -> 35 -> 70 -> 15
        uint8_t cur = DisplayMatrix_GetBrightness();
        uint8_t next = (cur <= 20) ? 35 : (cur <= 45 ? 75 : 15);
        DisplayMatrix_SetBrightness(next);
    } else if (Buttons_GetEvent(BTN_ID_OK) == BTN_LONG_PRESS) {
        Buzzer_Alert();
        s_autoRotate = !s_autoRotate; // Toggle auto rotation mode
    }

    // 3. Auto rotation tick
    if (s_autoRotate && (now - s_lastAppSwitch >= AUTO_ROTATE_MS)) {
        s_lastAppSwitch = now;
        s_currentApp = (AppMode)((s_currentApp + 1) % APP_COUNT);
    }

    // 4. Update sensors periodically (every 2 seconds)
    static uint32_t s_lastSensorUpdate = 0;
    if (now - s_lastSensorUpdate >= 2000) {
        s_lastSensorUpdate = now;
        Sensors_Update();
    }

    // 5. Render active screen (refresh at ~20 FPS)
    static uint32_t s_lastRender = 0;
    if (now - s_lastRender >= 50) {
        s_lastRender = now;
        renderCurrentApp();
    }
}
