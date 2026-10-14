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
#include "AirspaceService.h"
#include "OtaManager.h"

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

static void renderIconWithSmallText(const uint16_t* icon, const char* text, uint16_t color) {
    DisplayMatrix_Clear();
    if (icon) {
        DisplayMatrix_DrawIcon(0, 0, icon);
    }
    int16_t textW = DisplayMatrix_GetSmallTextWidth(text);
    int16_t startX = 9; // Default left-aligned with icon (Awtrix style)
    if (Network_GetTextAlign() == 1) { // Centered in 24px window
        startX = 8 + (24 - textW) / 2;
        if (startX < 8) startX = 8;
    }
    DisplayMatrix_DrawSmallText(startX, 1, text, color);
    DisplayMatrix_Show();
}

static void renderClockApp() {
    int h = (s_seconds / 3600) % 24;
    int m = (s_seconds / 60) % 60;

    int day = 14;
    int mon = 9;  // 0-based: 9 = Oct
    int wday = 3; // Wednesday
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 10)) {
        day = timeinfo.tm_mday;
        mon = timeinfo.tm_mon;
        wday = timeinfo.tm_wday;
    }

    uint8_t mode = Network_GetClockMode(); // 0: Time Only, 1: Combined, 2: Date Only
    bool showDate = (mode == 2);
    if (mode == 1) {
        // Combined mode: 5s Time, 3s Date (8s cycle)
        showDate = ((s_seconds % 8) >= 5);
    }

    DisplayMatrix_Clear();

    if (showDate) {
        // Show Date screen
        DisplayMatrix_DrawCalendarIcon(0, 0, day);

        static const char* kMonths[] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
        static const char* kWDays[] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };

        char buf[12];
        uint8_t fmt = Network_GetDateFormat();
        if (fmt == 1) { // DD MMM
            snprintf(buf, sizeof(buf), "%02d %s", day, kMonths[constrain(mon, 0, 11)]);
        } else if (fmt == 2) { // MM/DD
            snprintf(buf, sizeof(buf), "%02d/%02d", mon + 1, day);
        } else if (fmt == 3) { // DAY DD
            snprintf(buf, sizeof(buf), "%s %02d", kWDays[constrain(wday, 0, 6)], day);
        } else { // DD.MM.
            snprintf(buf, sizeof(buf), "%02d.%02d.", day, mon + 1);
        }

        int16_t textW = DisplayMatrix_GetSmallTextWidth(buf);
        int16_t startX = (Network_GetTextAlign() == 1) ? (8 + (24 - textW) / 2) : 9;
        DisplayMatrix_DrawSmallText(startX, 1, buf, C_CYAN);
    } else {
        // Show Time screen
        bool blink = (s_seconds % 2) == 0;
        int s = s_seconds % 60;
        bool showSeconds = Network_GetShowSeconds();

        uint8_t iconStyle = Network_GetClockIcon(); // 0: Dynamic Calendar, 1: Heart, 2: Clock Face, 3: None
        bool hasIcon = (iconStyle != 3) && !showSeconds;
        if (hasIcon) {
            if (iconStyle == 0) {
                DisplayMatrix_DrawCalendarIcon(0, 0, day);
            } else if (iconStyle == 1) {
                DisplayMatrix_DrawIcon(0, 0, ICON_HEART);
            } else if (iconStyle == 2) {
                DisplayMatrix_DrawIcon(0, 0, ICON_CLOCK);
            }
        }

        bool showWd = Network_GetWeekdayBar();
        int activeIdx = 0;
        if (Network_GetWeekdayStartMonday()) {
            activeIdx = (wday + 6) % 7; // Mon=0 .. Sun=6
        } else {
            activeIdx = wday; // Sun=0 .. Sat=6
        }
        uint16_t aCol = Network_GetWeekdayActiveColor();
        uint16_t iCol = Network_GetWeekdayInactiveColor();

        // 3x5 font at y=1 leaves row 0 empty above and row 6 empty below.
        // Weekday bar is drawn at y=7, eliminating vertical touching completely!
        if (showSeconds) {
            // Full width without icon: width 27 centered at startX = 2
            DisplayMatrix_DrawFullTime3x5(2, 1, h, m, s, blink, C_WHITE, C_WHITE, 0x07FF);
            if (showWd) {
                DisplayMatrix_DrawWeekdayBar(activeIdx, aCol, iCol, 6, 7);
            }
        } else {
            if (hasIcon) {
                int16_t startX = (Network_GetTextAlign() == 1) ? 12 : 10;
                DisplayMatrix_DrawTime3x5(startX, 1, h, m, blink, C_WHITE, C_WHITE);
                if (showWd) {
                    DisplayMatrix_DrawWeekdayBar(activeIdx, aCol, iCol, 10, 7);
                }
            } else {
                int16_t startX = 7;
                DisplayMatrix_DrawTime3x5(startX, 1, h, m, blink, C_WHITE, C_WHITE);
                if (showWd) {
                    DisplayMatrix_DrawWeekdayBar(activeIdx, aCol, iCol, 6, 7);
                }
            }
        }
    }

    DisplayMatrix_Show();
}

static void renderNightClock() {
    int h = (s_seconds / 3600) % 24;
    int m = (s_seconds / 60) % 60;
    char buf[10];
    snprintf(buf, sizeof(buf), "%02d:%02d", h, m);
    renderIconWithSmallText(ICON_MOON, buf, 0x8000); // Dim Red (RGB565)
}

static void renderIndoorApp() {
    const SensorData& s = Sensors_GetData();
    char buf[14];
    if (s.shtAvailable) {
        snprintf(buf, sizeof(buf), "%.1f\xC2\xB0" "C", s.temperature);
    } else {
        snprintf(buf, sizeof(buf), "21.5\xC2\xB0" "C");
    }
    renderIconWithSmallText(ICON_TEMP, buf, C_YELLOW);
}

static void renderOutdoorApp() {
    const uint16_t* icon = Weather_GetConditionIcon();
    const char* tempStr = Weather_GetFormattedTemp();
    renderIconWithSmallText(icon, tempStr, C_CYAN);
}

static void renderCryptoApp() {
    const uint16_t* icon = Crypto_GetIcon();
    const char* priceStr = Crypto_GetFormattedPrice();
    uint16_t col = Crypto_GetColor();
    renderIconWithSmallText(icon, priceStr, col);
}

static void renderYouTubeApp() {
    const char* subs = YouTube_GetFormattedSubs();
    renderIconWithSmallText(ICON_YOUTUBE, subs, C_WHITE);
}

static void renderAirspaceApp() {
    const char* cs = Airspace_GetCallsign();
    uint16_t col = (Airspace_GetFlightCount() > 0) ? C_CYAN : C_GREEN;
    renderIconWithSmallText(ICON_PLANE, cs, col);
}

static void renderBatteryApp() {
    const SensorData& s = Sensors_GetData();
    uint8_t mode = Network_GetBatteryMode(); // 0: %, 1: V, 2: Alternate
    bool showVolts = (mode == 1);
    if (mode == 2) {
        showVolts = ((s_seconds / 3) % 2) == 1; // Alternate every 3 seconds
    }

    char buf[10];
    if (showVolts) {
        snprintf(buf, sizeof(buf), "%.2fV", s.batteryVolts);
    } else {
        snprintf(buf, sizeof(buf), "%d%%", s.batteryPercent);
    }

    uint16_t col = (s.batteryPercent > 40) ? C_GREEN : (s.batteryPercent > 15 ? C_YELLOW : C_RED);
    renderIconWithSmallText(ICON_LIGHTNING, buf, col);
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
    Airspace_Init();

    // Startup splash & sound
    Buzzer_Click();
    DisplayMatrix_Clear();
    DisplayMatrix_DrawIcon(0, 0, ICON_SUN);
    DisplayMatrix_DrawText(9, 1, "TC001", C_ORANGE);
    DisplayMatrix_Show();
    delay(1200);

    // Initialize Network & Captive Portal
    Network_Init();
    DisplayMatrix_SetBrightness(Network_GetDayBrightness());

    s_autoRotate = Network_GetAutoRotate();
    s_currentApp = (AppMode)Network_GetActiveApp();
    s_lastAppSwitch = millis();
    s_lastSecTick = millis();
}

void loop() {
    uint32_t now = millis();

    // 0. High priority: OTA flashing indicator
    if (OTA_IsBusy()) {
        DisplayMatrix_Clear();
        DisplayMatrix_DrawIcon(0, 0, ICON_WIFI);
        char otaBuf[8];
        snprintf(otaBuf, sizeof(otaBuf), "%d%%", OTA_GetProgress());
        DisplayMatrix_DrawText(9, 1, otaBuf, C_CYAN);
        DisplayMatrix_Show();
        delay(30);
        return;
    }

    // 1. Service Network state machine & Captive Portal WebServer
    Network_Poll();

    // 2. Background HTTP Services polling
    YouTube_Update();
    Weather_Update();
    Crypto_Update();
    Airspace_Update();

    // 3. Clock second counter tick
    if (now - s_lastSecTick >= 1000) {
        s_lastSecTick += 1000;
        s_seconds++;
        checkNtpSync();

        // Dynamic Brightness: Day/Night + Ambient Sensor Control
        bool activeNight = isNightTime() && (now >= s_nightWakeUntilMs);
        if (activeNight) {
            DisplayMatrix_SetBrightness(Network_GetNightBrightness());
        } else if (Network_GetAutoBrightness()) {
            uint8_t autoBri = Sensors_CalculateAutoBrightness(MIN_AUTO_BRI, Network_GetDayBrightness());
            DisplayMatrix_SetBrightness(autoBri);
        } else {
            DisplayMatrix_SetBrightness(Network_GetDayBrightness());
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
            DisplayMatrix_SetBrightness(Network_GetDayBrightness());
        } else {
            // Render quiet dim red clock without rotating apps
            DisplayMatrix_SetBrightness(Network_GetNightBrightness());
            renderNightClock();
            return;
        }
    }

    // Sync active screen changes requested via Web Portal / API
    if (!Network_GetAutoRotate() && Network_GetActiveApp() != (uint8_t)s_currentApp) {
        s_currentApp = (AppMode)Network_GetActiveApp();
    }

    // 9. Standard App Navigation & Button Events
    if (Buttons_GetEvent(BTN_ID_RIGHT) == BTN_SHORT_CLICK) {
        Buzzer_Click();
        uint8_t mask = Network_GetCarouselMask();
        if (mask == 0) mask = 0x7F;
        const uint8_t* order = Network_GetCarouselOrder();
        int curPos = 0;
        for (int i = 0; i < APP_COUNT; i++) {
            if (order[i] == (uint8_t)s_currentApp) {
                curPos = i;
                break;
            }
        }
        uint8_t nextApp = (uint8_t)s_currentApp;
        for (int step = 1; step <= APP_COUNT; step++) {
            int pos = (curPos + step) % APP_COUNT;
            uint8_t cand = order[pos];
            if (mask & (1 << cand)) {
                nextApp = cand;
                break;
            }
        }
        s_currentApp = (AppMode)nextApp;
        s_lastAppSwitch = now;
        s_autoRotate = false;
        Network_SetAutoRotate(false);
        Network_SetActiveApp((uint8_t)s_currentApp);
    } else if (Buttons_GetEvent(BTN_ID_LEFT) == BTN_SHORT_CLICK) {
        Buzzer_Click();
        uint8_t mask = Network_GetCarouselMask();
        if (mask == 0) mask = 0x7F;
        const uint8_t* order = Network_GetCarouselOrder();
        int curPos = 0;
        for (int i = 0; i < APP_COUNT; i++) {
            if (order[i] == (uint8_t)s_currentApp) {
                curPos = i;
                break;
            }
        }
        uint8_t prevApp = (uint8_t)s_currentApp;
        for (int step = 1; step <= APP_COUNT; step++) {
            int pos = (curPos - step + APP_COUNT) % APP_COUNT;
            uint8_t cand = order[pos];
            if (mask & (1 << cand)) {
                prevApp = cand;
                break;
            }
        }
        s_currentApp = (AppMode)prevApp;
        s_lastAppSwitch = now;
        s_autoRotate = false;
        Network_SetAutoRotate(false);
        Network_SetActiveApp((uint8_t)s_currentApp);
    } else if (Buttons_GetEvent(BTN_ID_OK) == BTN_SHORT_CLICK) {
        Buzzer_Click();
        // Cycle manual brightness (pauses auto-brightness until re-enabled in web portal)
        Network_SetAutoBrightness(false);
        uint8_t cur = DisplayMatrix_GetBrightness();
        uint8_t next = (cur <= 15) ? 35 : (cur <= 50 ? 80 : 10);
        Network_SetDayBrightness(next);
        DisplayMatrix_SetBrightness(next);
    } else if (Buttons_GetEvent(BTN_ID_OK) == BTN_LONG_PRESS) {
        Buzzer_Alert();
        s_autoRotate = !s_autoRotate;
        Network_SetAutoRotate(s_autoRotate);
    }

    // 10. Auto rotation carousel tick with dynamic rotate delay from Settings
    uint32_t rotateMs = (uint32_t)Network_GetRotateDelaySec() * 1000UL;
    if (Network_GetAutoRotate() && (now - s_lastAppSwitch >= rotateMs)) {
        s_lastAppSwitch = now;
        uint8_t mask = Network_GetCarouselMask();
        if (mask == 0) mask = 0x01; // Default to at least Clock
        const uint8_t* order = Network_GetCarouselOrder();
        int curPos = 0;
        for (int i = 0; i < APP_COUNT; i++) {
            if (order[i] == (uint8_t)s_currentApp) {
                curPos = i;
                break;
            }
        }
        uint8_t nextApp = (uint8_t)s_currentApp;
        for (int step = 1; step <= APP_COUNT; step++) {
            int pos = (curPos + step) % APP_COUNT;
            uint8_t cand = order[pos];
            if (mask & (1 << cand)) {
                nextApp = cand;
                break;
            }
        }
        s_currentApp = (AppMode)nextApp;
        Network_SetActiveApp((uint8_t)s_currentApp);
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
