#include "CustomAppService.h"
#include "DisplayMatrix.h"
#include "Icons.h"
#include "Buzzer.h"

static CustomApp s_apps[MAX_CUSTOM_APPS];
static uint8_t s_appCount = 0;

// Notification state
static bool s_notifActive = false;
static String s_notifText = "";
static uint16_t s_notifIcon[64];
static bool s_notifHasIcon = false;
static uint16_t s_notifTextColor = C_WHITE;
static int8_t s_notifProgress = -1;
static uint16_t s_notifProgressColor = C_GREEN;
static uint32_t s_notifStartMs = 0;
static uint32_t s_notifDurationMs = 6000;
static int16_t s_notifScrollOffset = 0;
static uint32_t s_notifLastScrollMs = 0;
static uint8_t s_notifTargetPasses = 1;
static uint8_t s_notifPassesDone = 0;

static uint16_t parseColor565(JsonVariantConst v, uint16_t fallback) {
    if (v.isNull()) return fallback;
    if (v.is<JsonArrayConst>()) {
        JsonArrayConst arr = v.as<JsonArrayConst>();
        uint8_t r = arr[0] | 0;
        uint8_t g = arr[1] | 0;
        uint8_t b = arr[2] | 0;
        return DisplayMatrix_Color(r, g, b);
    }
    if (v.is<const char*>()) {
        const char* str = v.as<const char*>();
        if (str[0] == '#') str++;
        uint32_t rgb = strtoul(str, nullptr, 16);
        return DisplayMatrix_Color((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
    }
    if (v.is<uint32_t>()) {
        uint32_t rgb = v.as<uint32_t>();
        return DisplayMatrix_Color((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
    }
    return fallback;
}

static void parseIcon(JsonVariantConst v, uint16_t* iconOut, bool& hasIconOut) {
    hasIconOut = false;
    if (v.isNull()) return;

    if (v.is<const char*>()) {
        const char* name = v.as<const char*>();
        const uint16_t* iconPtr = Icons_GetByName(name);
        if (iconPtr) {
            memcpy(iconOut, iconPtr, 64 * sizeof(uint16_t));
            hasIconOut = true;
        }
    } else if (v.is<JsonArrayConst>()) {
        JsonArrayConst arr = v.as<JsonArrayConst>();
        if (arr.size() == 64) {
            for (size_t i = 0; i < 64; i++) {
                iconOut[i] = parseColor565(arr[i], 0);
            }
            hasIconOut = true;
        }
    }
}

void CustomApp_Init() {
    CustomApp_Clear();
    s_notifActive = false;
}

void CustomApp_Clear() {
    for (int i = 0; i < MAX_CUSTOM_APPS; i++) {
        s_apps[i].name = "";
        s_apps[i].text = "";
        s_apps[i].hasIcon = false;
        s_apps[i].progress = -1;
        s_apps[i].durationSec = 0;
        s_apps[i].lifetimeSec = 0;
        s_apps[i].scrollOffset = 0;
    }
    s_appCount = 0;
}

CustomApp* CustomApp_Find(const String& name) {
    if (name.length() == 0) return nullptr;
    for (int i = 0; i < MAX_CUSTOM_APPS; i++) {
        if (s_apps[i].name.equalsIgnoreCase(name)) {
            return &s_apps[i];
        }
    }
    return nullptr;
}

bool CustomApp_Delete(const String& name) {
    CustomApp* app = CustomApp_Find(name);
    if (app) {
        app->name = "";
        app->text = "";
        app->hasIcon = false;
        app->lifetimeSec = 0;
        return true;
    }
    return false;
}

bool CustomApp_Push(const String& name, JsonVariantConst doc) {
    if (name.length() == 0) return false;

    // Check delete flag or empty text
    bool isDelete = false;
    if (doc["delete"].is<bool>() && doc["delete"].as<bool>()) isDelete = true;
    if (doc["text"].is<const char*>() && strlen(doc["text"].as<const char*>()) == 0) isDelete = true;

    if (isDelete) {
        return CustomApp_Delete(name);
    }

    CustomApp* app = CustomApp_Find(name);
    if (!app) {
        // Find empty slot
        for (int i = 0; i < MAX_CUSTOM_APPS; i++) {
            if (s_apps[i].name.length() == 0) {
                app = &s_apps[i];
                break;
            }
        }
    }

    if (!app) {
        // Evict oldest or expired app
        uint32_t oldestTime = UINT32_MAX;
        int evictIdx = 0;
        uint32_t now = millis();
        for (int i = 0; i < MAX_CUSTOM_APPS; i++) {
            if (s_apps[i].lifetimeSec > 0 && (now - s_apps[i].updatedAtMs >= s_apps[i].lifetimeSec * 1000UL)) {
                evictIdx = i;
                break;
            }
            if (s_apps[i].updatedAtMs < oldestTime) {
                oldestTime = s_apps[i].updatedAtMs;
                evictIdx = i;
            }
        }
        app = &s_apps[evictIdx];
    }

    app->name = name;
    app->text = doc["text"] | "";
    app->textColor = parseColor565(doc["color"], C_WHITE);

    // Icon parsing
    parseIcon(doc["icon"], app->icon, app->hasIcon);

    // Progress bar
    if (doc["progress"].is<int>()) {
        app->progress = constrain(doc["progress"].as<int>(), 0, 100);
    } else {
        app->progress = -1;
    }
    app->progressColor = parseColor565(doc["progressColor"], C_GREEN);

    // Timing & Lifetime
    app->durationSec = doc["duration"] | 0;
    app->lifetimeSec = doc["lifetime"] | 0;
    app->updatedAtMs = millis();

    // Scroll settings
    app->scroll = doc["scroll"] | true;
    app->scrollOffset = 0;
    app->lastScrollMs = millis();

    // Optional Sound
    if (doc["sound"].is<int>()) {
        int snd = doc["sound"].as<int>();
        if (snd == 1) Buzzer_Click();
        else if (snd >= 2) Buzzer_Alert();
    }

    return true;
}

uint8_t CustomApp_GetActiveCount() {
    uint8_t count = 0;
    uint32_t now = millis();
    for (int i = 0; i < MAX_CUSTOM_APPS; i++) {
        if (s_apps[i].name.length() > 0) {
            if (s_apps[i].lifetimeSec > 0 && (now - s_apps[i].updatedAtMs >= s_apps[i].lifetimeSec * 1000UL)) {
                // Expired
                continue;
            }
            count++;
        }
    }
    return count;
}

CustomApp* CustomApp_GetActive(uint8_t activeIndex) {
    uint8_t current = 0;
    uint32_t now = millis();
    for (int i = 0; i < MAX_CUSTOM_APPS; i++) {
        if (s_apps[i].name.length() > 0) {
            if (s_apps[i].lifetimeSec > 0 && (now - s_apps[i].updatedAtMs >= s_apps[i].lifetimeSec * 1000UL)) {
                continue;
            }
            if (current == activeIndex) {
                return &s_apps[i];
            }
            current++;
        }
    }
    return nullptr;
}

void CustomApp_PushNotification(JsonVariantConst doc) {
    s_notifText = doc["text"] | "";
    if (s_notifText.length() == 0) {
        CustomApp_DismissNotification();
        return;
    }

    s_notifTextColor = parseColor565(doc["color"], C_WHITE);
    parseIcon(doc["icon"], s_notifIcon, s_notifHasIcon);

    if (doc["progress"].is<int>()) {
        s_notifProgress = constrain(doc["progress"].as<int>(), 0, 100);
    } else {
        s_notifProgress = -1;
    }
    s_notifProgressColor = parseColor565(doc["progressColor"], C_GREEN);

    uint16_t durSec = doc["duration"] | 6;
    s_notifDurationMs = durSec * 1000UL;
    s_notifTargetPasses = doc["count"] | 1;
    s_notifPassesDone = 0;
    s_notifScrollOffset = 0;
    s_notifLastScrollMs = millis();
    s_notifStartMs = millis();
    s_notifActive = true;

    int snd = doc["sound"] | 2; // Default alert for push notifications
    if (snd == 1) Buzzer_Click();
    else if (snd >= 2) Buzzer_Alert();
}

bool CustomApp_IsNotificationActive() {
    if (!s_notifActive) return false;
    if (millis() - s_notifStartMs >= s_notifDurationMs && s_notifPassesDone >= s_notifTargetPasses) {
        s_notifActive = false;
        return false;
    }
    return true;
}

void CustomApp_DismissNotification() {
    s_notifActive = false;
}

static void renderAppFrame(const char* text, uint16_t textColor, const uint16_t* icon, bool hasIcon,
                           int8_t progress, uint16_t progressColor, bool scroll,
                           int16_t& scrollOffset, uint32_t& lastScrollMs, uint8_t* passesDoneOut = nullptr) {
    DisplayMatrix_Clear();

    int startX = hasIcon ? 9 : 0;
    int windowWidth = hasIcon ? 23 : 32;

    int16_t textW = DisplayMatrix_GetSmallTextWidth(text);
    uint32_t now = millis();
    int16_t textY = 1;

    if (textW <= windowWidth || !scroll) {
        // Fits statically without scroll jitter
        int drawX = startX;
        DisplayMatrix_DrawSmallText(drawX, textY, text, textColor);
    } else {
        // Smooth horizontal ticker
        if (now - lastScrollMs >= 40) {
            lastScrollMs = now;
            scrollOffset++;
            if (scrollOffset > textW + 4) {
                scrollOffset = -windowWidth;
                if (passesDoneOut) (*passesDoneOut)++;
            }
        }
        int16_t drawX = startX - scrollOffset;
        DisplayMatrix_DrawSmallText(drawX, textY, text, textColor);
    }

    // Pinned icon: mask columns 0..8 and draw icon so scrolling text disappears cleanly behind it
    if (hasIcon && icon) {
        DisplayMatrix_GetMatrix().fillRect(0, 0, 9, 8, C_BLACK);
        DisplayMatrix_DrawIcon(0, 0, icon);
    }

    // 1px Progress Bar along bottom row (y=7)
    if (progress >= 0) {
        int pX = hasIcon ? 9 : 0;
        int pW = hasIcon ? 23 : 32;
        int fill = constrain((progress * pW) / 100, 0, pW);
        for (int x = pX; x < pX + fill; x++) {
            DisplayMatrix_GetMatrix().drawPixel(x, 7, progressColor);
        }
    }

    DisplayMatrix_Show();
}

void CustomApp_Render(CustomApp* app) {
    if (!app) return;
    renderAppFrame(app->text.c_str(), app->textColor, app->icon, app->hasIcon,
                   app->progress, app->progressColor, app->scroll,
                   app->scrollOffset, app->lastScrollMs);
}

void CustomApp_RenderByName(const String& name) {
    CustomApp* app = CustomApp_Find(name);
    if (app) CustomApp_Render(app);
}

void CustomApp_RenderNotification() {
    if (!s_notifActive) return;
    renderAppFrame(s_notifText.c_str(), s_notifTextColor, s_notifIcon, s_notifHasIcon,
                   s_notifProgress, s_notifProgressColor, true,
                   s_notifScrollOffset, s_notifLastScrollMs, &s_notifPassesDone);

    if (millis() - s_notifStartMs >= s_notifDurationMs && s_notifPassesDone >= s_notifTargetPasses) {
        s_notifActive = false;
    }
}

void CustomApp_GetJsonList(String& out) {
    JsonDocument doc;
    JsonArray arr = doc["apps"].to<JsonArray>();
    uint32_t now = millis();

    for (int i = 0; i < MAX_CUSTOM_APPS; i++) {
        if (s_apps[i].name.length() > 0) {
            JsonObject obj = arr.add<JsonObject>();
            obj["name"] = s_apps[i].name;
            obj["text"] = s_apps[i].text;
            obj["has_icon"] = s_apps[i].hasIcon;
            obj["progress"] = s_apps[i].progress;
            obj["duration"] = s_apps[i].durationSec;
            obj["lifetime"] = s_apps[i].lifetimeSec;
            uint32_t ageSec = (now - s_apps[i].updatedAtMs) / 1000UL;
            obj["age_sec"] = ageSec;
            if (s_apps[i].lifetimeSec > 0 && ageSec >= s_apps[i].lifetimeSec) {
                obj["expired"] = true;
            } else {
                obj["expired"] = false;
            }
        }
    }
    doc["notification_active"] = s_notifActive;
    serializeJson(doc, out);
}
