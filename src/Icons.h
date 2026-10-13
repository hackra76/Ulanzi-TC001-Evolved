#pragma once
#include <Arduino.h>

// 8x8 RGB565 Bitmaps for 8-pixel high matrix screens
// Each icon is uint16_t[64] in RGB565 format (0x0000 = transparent / off)

// Colors
#define C_BLACK   0x0000
#define C_WHITE   0xFFFF
#define C_RED     0xF800
#define C_GREEN   0x07E0
#define C_BLUE    0x001F
#define C_CYAN    0x07FF
#define C_MAGENTA 0xF81F
#define C_YELLOW  0xFFE0
#define C_ORANGE  0xFD20
#define C_GRAY    0x8410

// 8x8 Heart Icon
static const uint16_t ICON_HEART[64] = {
    0, C_RED, C_RED, 0, 0, C_RED, C_RED, 0,
    C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED,
    C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED,
    C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED,
    0, C_RED, C_RED, C_RED, C_RED, C_RED, C_RED, 0,
    0, 0, C_RED, C_RED, C_RED, C_RED, 0, 0,
    0, 0, 0, C_RED, C_RED, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

// 8x8 Sun Icon
static const uint16_t ICON_SUN[64] = {
    0, 0, 0, C_ORANGE, C_ORANGE, 0, 0, 0,
    C_ORANGE, 0, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, 0, C_ORANGE,
    0, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, 0,
    C_ORANGE, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_ORANGE,
    C_ORANGE, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_ORANGE,
    0, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, 0,
    C_ORANGE, 0, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, 0, C_ORANGE,
    0, 0, 0, C_ORANGE, C_ORANGE, 0, 0, 0
};

// 8x8 Rain Cloud Icon
static const uint16_t ICON_RAIN[64] = {
    0, 0, C_GRAY, C_GRAY, C_GRAY, 0, 0, 0,
    0, C_GRAY, C_WHITE, C_WHITE, C_WHITE, C_GRAY, 0, 0,
    C_GRAY, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_GRAY, 0,
    C_GRAY, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_GRAY,
    0, C_GRAY, C_GRAY, C_GRAY, C_GRAY, C_GRAY, C_GRAY, 0,
    0, C_CYAN, 0, C_CYAN, 0, C_CYAN, 0, 0,
    C_CYAN, 0, C_CYAN, 0, C_CYAN, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

// 8x8 Aircraft Icon
static const uint16_t ICON_PLANE[64] = {
    0, 0, 0, C_CYAN, C_CYAN, 0, 0, 0,
    0, 0, 0, C_CYAN, C_CYAN, 0, 0, 0,
    C_CYAN, C_CYAN, C_CYAN, C_CYAN, C_CYAN, C_CYAN, C_CYAN, C_CYAN,
    0, 0, C_WHITE, C_CYAN, C_CYAN, C_WHITE, 0, 0,
    0, 0, 0, C_CYAN, C_CYAN, 0, 0, 0,
    0, 0, 0, C_CYAN, C_CYAN, 0, 0, 0,
    0, 0, C_CYAN, C_CYAN, C_CYAN, C_CYAN, 0, 0,
    0, 0, C_CYAN, 0, 0, C_CYAN, 0, 0
};

// 8x8 Thermometer Icon
static const uint16_t ICON_TEMP[64] = {
    0, 0, C_RED, C_WHITE, C_WHITE, 0, 0, 0,
    0, 0, C_RED, C_RED, C_WHITE, 0, 0, 0,
    0, 0, C_RED, C_WHITE, C_WHITE, 0, 0, 0,
    0, 0, C_RED, C_RED, C_WHITE, 0, 0, 0,
    0, 0, C_RED, C_WHITE, C_WHITE, 0, 0, 0,
    0, C_RED, C_RED, C_RED, C_RED, C_RED, 0, 0,
    0, C_RED, C_RED, C_RED, C_RED, C_RED, 0, 0,
    0, 0, C_RED, C_RED, C_RED, 0, 0, 0
};

// 8x8 Humidity Droplet Icon
static const uint16_t ICON_HUMID[64] = {
    0, 0, 0, C_CYAN, 0, 0, 0, 0,
    0, 0, C_CYAN, C_CYAN, 0, 0, 0, 0,
    0, C_CYAN, C_WHITE, C_CYAN, C_CYAN, 0, 0, 0,
    C_CYAN, C_WHITE, C_WHITE, C_CYAN, C_CYAN, C_CYAN, 0, 0,
    C_CYAN, C_WHITE, C_CYAN, C_CYAN, C_CYAN, C_CYAN, 0, 0,
    C_CYAN, C_CYAN, C_CYAN, C_CYAN, C_CYAN, C_CYAN, 0, 0,
    0, C_CYAN, C_CYAN, C_CYAN, C_CYAN, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

// 8x8 Battery Icon
static const uint16_t ICON_BATTERY[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, C_GRAY, C_GRAY, C_GRAY, C_GRAY, C_GRAY, 0, 0,
    0, C_GRAY, C_GREEN, C_GREEN, C_GREEN, C_GRAY, C_GRAY, 0,
    0, C_GRAY, C_GREEN, C_GREEN, C_GREEN, C_GRAY, C_GRAY, 0,
    0, C_GRAY, C_GREEN, C_GREEN, C_GREEN, C_GRAY, C_GRAY, 0,
    0, C_GRAY, C_GREEN, C_GREEN, C_GREEN, C_GRAY, 0, 0,
    0, C_GRAY, C_GRAY, C_GRAY, C_GRAY, C_GRAY, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

// 8x8 Wi-Fi Station Icon
static const uint16_t ICON_WIFI[64] = {
    0, C_CYAN, C_CYAN, C_CYAN, C_CYAN, C_CYAN, C_CYAN, 0,
    C_CYAN, 0, 0, 0, 0, 0, 0, C_CYAN,
    0, 0, C_CYAN, C_CYAN, C_CYAN, C_CYAN, 0, 0,
    0, C_CYAN, 0, 0, 0, 0, C_CYAN, 0,
    0, 0, 0, C_CYAN, C_CYAN, 0, 0, 0,
    0, 0, C_CYAN, 0, 0, C_CYAN, 0, 0,
    0, 0, 0, C_CYAN, C_CYAN, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

// 8x8 SoftAP / Hotspot Icon
static const uint16_t ICON_AP[64] = {
    C_ORANGE, 0, 0, 0, 0, 0, 0, C_ORANGE,
    0, C_ORANGE, 0, C_WHITE, C_WHITE, 0, C_ORANGE, 0,
    0, 0, C_ORANGE, C_WHITE, C_WHITE, C_ORANGE, 0, 0,
    0, 0, 0, C_ORANGE, C_ORANGE, 0, 0, 0,
    0, 0, 0, C_ORANGE, C_ORANGE, 0, 0, 0,
    0, 0, C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE, 0, 0,
    0, C_ORANGE, 0, 0, 0, 0, C_ORANGE, 0,
    C_ORANGE, 0, 0, 0, 0, 0, 0, C_ORANGE
};

// 8x8 YouTube Play Button Icon
static const uint16_t ICON_YOUTUBE[64] = {
    0,     C_RED, C_RED,   C_RED,   C_RED, C_RED, C_RED, 0,
    C_RED, C_RED, C_RED,   C_RED,   C_RED, C_RED, C_RED, C_RED,
    C_RED, C_RED, C_WHITE, C_RED,   C_RED, C_RED, C_RED, C_RED,
    C_RED, C_RED, C_WHITE, C_WHITE, C_RED, C_RED, C_RED, C_RED,
    C_RED, C_RED, C_WHITE, C_WHITE, C_RED, C_RED, C_RED, C_RED,
    C_RED, C_RED, C_WHITE, C_RED,   C_RED, C_RED, C_RED, C_RED,
    C_RED, C_RED, C_RED,   C_RED,   C_RED, C_RED, C_RED, C_RED,
    0,     C_RED, C_RED,   C_RED,   C_RED, C_RED, C_RED, 0
};

// 8x8 Cloud Icon
static const uint16_t ICON_CLOUD[64] = {
    0, 0, 0, C_WHITE, C_WHITE, 0, 0, 0,
    0, 0, C_WHITE, C_WHITE, C_WHITE, C_WHITE, 0, 0,
    0, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, 0,
    C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE,
    C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE,
    0, C_GRAY, C_GRAY, C_GRAY, C_GRAY, C_GRAY, C_GRAY, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

// 8x8 Snowflake Icon
static const uint16_t ICON_SNOW[64] = {
    0, 0, 0, C_CYAN, C_CYAN, 0, 0, 0,
    C_CYAN, 0, C_CYAN, C_CYAN, C_CYAN, C_CYAN, 0, C_CYAN,
    0, C_CYAN, 0, C_WHITE, C_WHITE, 0, C_CYAN, 0,
    C_CYAN, C_CYAN, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_CYAN, C_CYAN,
    C_CYAN, C_CYAN, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_CYAN, C_CYAN,
    0, C_CYAN, 0, C_WHITE, C_WHITE, 0, C_CYAN, 0,
    C_CYAN, 0, C_CYAN, C_CYAN, C_CYAN, C_CYAN, 0, C_CYAN,
    0, 0, 0, C_CYAN, C_CYAN, 0, 0, 0
};

// 8x8 Thunderstorm Icon
static const uint16_t ICON_STORM[64] = {
    0, C_GRAY, C_GRAY, C_GRAY, C_GRAY, C_GRAY, 0, 0,
    C_GRAY, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_GRAY, 0, 0,
    C_GRAY, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_GRAY, 0,
    0, C_GRAY, C_GRAY, C_YELLOW, C_GRAY, C_GRAY, 0, 0,
    0, 0, C_YELLOW, C_YELLOW, 0, 0, 0, 0,
    0, 0, 0, C_YELLOW, C_YELLOW, 0, 0, 0,
    0, 0, C_YELLOW, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

// 8x8 Bitcoin Icon (Orange coin with white B)
static const uint16_t ICON_BTC[64] = {
    0, C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE, 0,
    C_ORANGE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_ORANGE, C_ORANGE, C_ORANGE,
    C_ORANGE, C_WHITE, C_WHITE, C_ORANGE, C_ORANGE, C_WHITE, C_WHITE, C_ORANGE,
    C_ORANGE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_ORANGE, C_ORANGE,
    C_ORANGE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_ORANGE, C_ORANGE,
    C_ORANGE, C_WHITE, C_WHITE, C_ORANGE, C_ORANGE, C_WHITE, C_WHITE, C_ORANGE,
    C_ORANGE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_ORANGE, C_ORANGE, C_ORANGE,
    0, C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE, 0
};

// 8x8 Ethereum Icon (Cyan/White Diamond)
static const uint16_t ICON_ETH[64] = {
    0, 0, 0, C_WHITE, C_WHITE, 0, 0, 0,
    0, 0, C_CYAN, C_WHITE, C_WHITE, C_CYAN, 0, 0,
    0, C_CYAN, C_CYAN, C_WHITE, C_WHITE, C_CYAN, C_CYAN, 0,
    C_CYAN, C_CYAN, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_CYAN, C_CYAN,
    0, C_CYAN, C_CYAN, C_WHITE, C_WHITE, C_CYAN, C_CYAN, 0,
    0, 0, C_CYAN, C_WHITE, C_WHITE, C_CYAN, 0, 0,
    0, 0, 0, C_CYAN, C_CYAN, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

// 8x8 Crescent Moon Icon (Night Mode)
static const uint16_t ICON_MOON[64] = {
    0, 0, 0, C_YELLOW, C_YELLOW, 0, 0, 0,
    0, 0, C_YELLOW, C_YELLOW, 0, 0, 0, 0,
    0, C_YELLOW, C_YELLOW, 0, 0, 0, 0, 0,
    0, C_YELLOW, C_YELLOW, 0, 0, 0, 0, 0,
    0, C_YELLOW, C_YELLOW, 0, 0, 0, 0, 0,
    0, C_YELLOW, C_YELLOW, 0, 0, 0, 0, 0,
    0, 0, C_YELLOW, C_YELLOW, 0, 0, 0, 0,
    0, 0, 0, C_YELLOW, C_YELLOW, 0, 0, 0
};



