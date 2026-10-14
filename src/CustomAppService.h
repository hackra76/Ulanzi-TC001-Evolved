#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>

#define MAX_CUSTOM_APPS 8

struct CustomApp {
    String name;
    String text;
    uint16_t icon[64];
    bool hasIcon;
    uint16_t textColor;
    int8_t progress;       // -1: disabled, 0..100: percentage
    uint16_t progressColor;
    uint16_t durationSec;  // Carousel display duration (0: default)
    uint32_t lifetimeSec;  // Eviction timeout in sec (0: permanent until deleted)
    uint32_t updatedAtMs;
    bool scroll;
    int16_t scrollOffset;
    uint32_t lastScrollMs;
};

void CustomApp_Init();
bool CustomApp_Push(const String& name, JsonVariantConst doc);
bool CustomApp_Delete(const String& name);
void CustomApp_Clear();

uint8_t CustomApp_GetActiveCount();
CustomApp* CustomApp_GetActive(uint8_t activeIndex);
CustomApp* CustomApp_Find(const String& name);

// Push Notification (immediate high-priority override)
void CustomApp_PushNotification(JsonVariantConst doc);
bool CustomApp_IsNotificationActive();
void CustomApp_DismissNotification();
void CustomApp_RenderNotification();

// Carousel custom app rendering
void CustomApp_Render(CustomApp* app);
void CustomApp_RenderByName(const String& name);

// Query for Web Portal / REST API
void CustomApp_GetJsonList(String& out);
