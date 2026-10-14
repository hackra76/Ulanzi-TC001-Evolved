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

// 8x8 Energy Charge / Lightning Bolt Icon
static const uint16_t ICON_LIGHTNING[64] = {
    0, 0, 0, 0,        C_YELLOW, C_WHITE,  0,        0,
    0, 0, 0, C_YELLOW, C_WHITE,  0,        0,        0,
    0, 0, C_YELLOW,    C_WHITE,  0,        0,        0, 0,
    0, C_YELLOW, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_YELLOW, 0,
    0, 0, 0, 0,        C_YELLOW, C_WHITE,  0,        0,
    0, 0, 0, C_YELLOW, C_WHITE,  0,        0,        0,
    0, 0, C_YELLOW,    C_WHITE,  0,        0,        0, 0,
    0, 0, C_YELLOW,    0,        0,        0,        0, 0
};

// 8x8 Battery Outline Icon
static const uint16_t ICON_BATTERY[64] = {
    0, 0, 0, 0,        C_YELLOW, C_WHITE,  0,        0,
    0, 0, 0, C_YELLOW, C_WHITE,  0,        0,        0,
    0, 0, C_YELLOW,    C_WHITE,  0,        0,        0, 0,
    0, C_YELLOW, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_YELLOW, 0,
    0, 0, 0, 0,        C_YELLOW, C_WHITE,  0,        0,
    0, 0, 0, C_YELLOW, C_WHITE,  0,        0,        0,
    0, 0, C_YELLOW,    C_WHITE,  0,        0,        0, 0,
    0, 0, C_YELLOW,    0,        0,        0,        0, 0
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

// 8x8 Bold YouTube Play Button Icon (prominent centered play triangle)
static const uint16_t ICON_YOUTUBE[64] = {
    0,       C_RED,   C_RED,   C_RED,   C_RED,   C_RED,   C_RED,   0,
    C_RED,   C_RED,   C_WHITE, C_RED,   C_RED,   C_RED,   C_RED,   C_RED,
    C_RED,   C_RED,   C_WHITE, C_WHITE, C_WHITE, C_RED,   C_RED,   C_RED,
    C_RED,   C_RED,   C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_RED,   C_RED,
    C_RED,   C_RED,   C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_RED,   C_RED,
    C_RED,   C_RED,   C_WHITE, C_WHITE, C_WHITE, C_RED,   C_RED,   C_RED,
    C_RED,   C_RED,   C_WHITE, C_RED,   C_RED,   C_RED,   C_RED,   C_RED,
    0,       C_RED,   C_RED,   C_RED,   C_RED,   C_RED,   C_RED,   0
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

// 8x8 Apple Inc. Logo
static const uint16_t ICON_APPLE[64] = {
    0,       0,       0,       0,       C_GREEN, 0,       0,       0,
    0,       0,       0,       C_GREEN, 0,       0,       0,       0,
    0,       C_WHITE, C_WHITE, 0,       C_WHITE, C_WHITE, 0,       0,
    C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, 0,
    C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, 0,       0,
    C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, 0,
    C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, 0,
    0,       C_WHITE, C_WHITE, 0,       C_WHITE, C_WHITE, 0,       0
};

// 8x8 Gold Bullion / Ingot
static const uint16_t ICON_GOLD[64] = {
    0,        0,        C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, 0,        0,
    0,        C_YELLOW, C_WHITE,  C_YELLOW, C_YELLOW, C_WHITE,  C_YELLOW, 0,
    C_ORANGE, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_ORANGE,
    C_ORANGE, C_YELLOW, C_WHITE,  C_YELLOW, C_YELLOW, C_WHITE,  C_YELLOW, C_ORANGE,
    C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE,
    0,        C_ORANGE, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_ORANGE, 0,
    0,        C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE, C_ORANGE, 0,
    0,        0,        0,        0,        0,        0,        0,        0
};

// 8x8 USD Coin ($)
static const uint16_t ICON_USD[64] = {
    0,       C_GREEN, C_GREEN, C_GREEN, C_GREEN, C_GREEN, C_GREEN, 0,
    C_GREEN, 0,       0,       C_WHITE, 0,       0,       0,       C_GREEN,
    C_GREEN, 0,       C_WHITE, C_WHITE, C_WHITE, 0,       0,       C_GREEN,
    C_GREEN, 0,       C_WHITE, C_WHITE, 0,       0,       0,       C_GREEN,
    C_GREEN, 0,       0,       C_WHITE, C_WHITE, 0,       0,       C_GREEN,
    C_GREEN, 0,       C_WHITE, C_WHITE, C_WHITE, 0,       0,       C_GREEN,
    C_GREEN, 0,       0,       C_WHITE, 0,       0,       0,       C_GREEN,
    0,       C_GREEN, C_GREEN, C_GREEN, C_GREEN, C_GREEN, C_GREEN, 0
};

// 8x8 EUR Coin (€)
static const uint16_t ICON_EUR[64] = {
    0,       C_BLUE,  C_BLUE,  C_BLUE,  C_BLUE,  C_BLUE,  C_BLUE,  0,
    C_BLUE,  0,       0,       C_YELLOW,C_YELLOW,C_YELLOW,0,       C_BLUE,
    C_BLUE,  0,       C_YELLOW,0,       0,       0,       0,       C_BLUE,
    C_BLUE,  C_YELLOW,C_YELLOW,C_YELLOW,C_YELLOW,0,       0,       C_BLUE,
    C_BLUE,  C_YELLOW,C_YELLOW,C_YELLOW,C_YELLOW,0,       0,       C_BLUE,
    C_BLUE,  0,       C_YELLOW,0,       0,       0,       0,       C_BLUE,
    C_BLUE,  0,       0,       C_YELLOW,C_YELLOW,C_YELLOW,0,       C_BLUE,
    0,       C_BLUE,  C_BLUE,  C_BLUE,  C_BLUE,  C_BLUE,  C_BLUE,  0
};

// 8x8 Stock Market Trend / Chart
static const uint16_t ICON_CHART[64] = {
    0,       0,       0,       0,       0,       C_GREEN, C_GREEN, C_GREEN,
    0,       0,       0,       0,       0,       0,       C_GREEN, C_GREEN,
    0,       0,       0,       0,       C_GREEN, C_GREEN, 0,       C_GREEN,
    0,       0,       0,       C_GREEN, C_GREEN, 0,       0,       0,
    0,       C_GREEN, C_GREEN, C_GREEN, 0,       0,       0,       0,
    C_GREEN, C_GREEN, 0,       0,       0,       0,       0,       0,
    C_GRAY,  C_GRAY,  C_GRAY,  C_GRAY,  C_GRAY,  C_GRAY,  C_GRAY,  C_GRAY,
    0,       0,       0,       0,       0,       0,       0,       0
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

// 8x8 Analog Clock Face Icon
static const uint16_t ICON_CLOCK[64] = {
    0,       0,       C_CYAN,  C_CYAN,  C_CYAN,  C_CYAN,  0,       0,
    0,       C_CYAN,  0,       0,       0,       0,       C_CYAN,  0,
    C_CYAN,  0,       0,       C_WHITE, 0,       0,       0,       C_CYAN,
    C_CYAN,  0,       0,       C_WHITE, C_WHITE, C_WHITE, 0,       C_CYAN,
    C_CYAN,  0,       0,       C_WHITE, 0,       0,       0,       C_CYAN,
    C_CYAN,  0,       0,       0,       0,       0,       0,       C_CYAN,
    0,       C_CYAN,  0,       0,       0,       0,       C_CYAN,  0,
    0,       0,       C_CYAN,  C_CYAN,  C_CYAN,  C_CYAN,  0,       0
};

// 8x8 Calendar Icon Base
static const uint16_t ICON_CALENDAR[64] = {
    0,       C_RED,   C_WHITE, C_RED,   C_RED,   C_WHITE, C_RED,   0,
    C_RED,   C_RED,   C_RED,   C_RED,   C_RED,   C_RED,   C_RED,   C_RED,
    C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE,
    C_WHITE, C_WHITE, C_RED,   C_RED,   C_RED,   C_WHITE, C_WHITE, C_WHITE,
    C_WHITE, C_WHITE, C_WHITE, C_RED,   C_WHITE, C_WHITE, C_WHITE, C_WHITE,
    C_WHITE, C_WHITE, C_WHITE, C_RED,   C_WHITE, C_WHITE, C_WHITE, C_WHITE,
    C_WHITE, C_WHITE, C_RED,   C_RED,   C_RED,   C_WHITE, C_WHITE, C_WHITE,
    0,       C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, 0
};

#define C_SPOTIFY 0x1EAC // #1DB954 Spotify Brand Green

// 8x8 Spotify Green Circular Wave Icon
static const uint16_t ICON_SPOTIFY[64] = {
    0,         0,         C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, 0,         0,
    0,         C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, 0,
    C_SPOTIFY, 0,         0,         0,         0,         0,         C_SPOTIFY, C_SPOTIFY,
    C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, C_SPOTIFY,
    C_SPOTIFY, C_SPOTIFY, 0,         0,         0,         0,         C_SPOTIFY, C_SPOTIFY,
    C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, C_SPOTIFY,
    0,         C_SPOTIFY, C_SPOTIFY, 0,         0,         C_SPOTIFY, C_SPOTIFY, 0,
    0,         0,         C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, C_SPOTIFY, 0,         0
};

// 8x8 Bell / Alert Notification Icon
static const uint16_t ICON_BELL[64] = {
    0,        0,        0,        C_YELLOW, C_YELLOW, 0,        0,        0,
    0,        0,        C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, 0,        0,
    0,        C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, 0,
    0,        C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, 0,
    0,        C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, 0,
    C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW, C_YELLOW,
    0,        0,        0,        C_ORANGE, C_ORANGE, 0,        0,        0,
    0,        0,        0,        0,        0,        0,        0,        0
};

// 8x8 Checkmark Icon
static const uint16_t ICON_CHECK[64] = {
    0,       0,       0,       0,       0,       0,       0,       C_GREEN,
    0,       0,       0,       0,       0,       0,       C_GREEN, C_GREEN,
    0,       0,       0,       0,       0,       C_GREEN, C_GREEN, 0,
    0,       0,       0,       0,       C_GREEN, C_GREEN, 0,       0,
    C_GREEN, 0,       0,       C_GREEN, C_GREEN, 0,       0,       0,
    C_GREEN, C_GREEN, C_GREEN, C_GREEN, 0,       0,       0,       0,
    0,       C_GREEN, C_GREEN, 0,       0,       0,       0,       0,
    0,       0,       0,       0,       0,       0,       0,       0
};

// 8x8 Cross / Cancel Icon
static const uint16_t ICON_CROSS[64] = {
    C_RED,   C_RED,   0,       0,       0,       0,       C_RED,   C_RED,
    C_RED,   C_RED,   C_RED,   0,       0,       C_RED,   C_RED,   C_RED,
    0,       C_RED,   C_RED,   C_RED,   C_RED,   C_RED,   C_RED,   0,
    0,       0,       C_RED,   C_RED,   C_RED,   C_RED,   0,       0,
    0,       0,       C_RED,   C_RED,   C_RED,   C_RED,   0,       0,
    0,       C_RED,   C_RED,   C_RED,   C_RED,   C_RED,   C_RED,   0,
    C_RED,   C_RED,   C_RED,   0,       0,       C_RED,   C_RED,   C_RED,
    C_RED,   C_RED,   0,       0,       0,       0,       C_RED,   C_RED
};

inline const uint16_t* Icons_GetByName(const String& name) {
    String lower = name;
    lower.toLowerCase();
    lower.trim();

    if (lower == "spotify") return ICON_SPOTIFY;
    if (lower == "sun") return ICON_SUN;
    if (lower == "heart") return ICON_HEART;
    if (lower == "rain" || lower == "cloud") return ICON_RAIN;
    if (lower == "plane" || lower == "radar" || lower == "flight") return ICON_PLANE;
    if (lower == "temp" || lower == "temperature") return ICON_TEMP;
    if (lower == "humid" || lower == "humidity") return ICON_HUMID;
    if (lower == "lightning" || lower == "power" || lower == "charge") return ICON_LIGHTNING;
    if (lower == "battery") return ICON_BATTERY;
    if (lower == "wifi") return ICON_WIFI;
    if (lower == "ap" || lower == "hotspot") return ICON_AP;
    if (lower == "youtube" || lower == "yt") return ICON_YOUTUBE;
    if (lower == "btc" || lower == "crypto" || lower == "bitcoin") return ICON_BTC;
    if (lower == "usd" || lower == "dollar") return ICON_USD;
    if (lower == "gold") return ICON_GOLD;
    if (lower == "eur" || lower == "euro") return ICON_EUR;
    if (lower == "chart" || lower == "stock" || lower == "apple") return ICON_CHART;
    if (lower == "moon" || lower == "night") return ICON_MOON;
    if (lower == "clock" || lower == "time") return ICON_CLOCK;
    if (lower == "calendar" || lower == "date") return ICON_CALENDAR;
    if (lower == "bell" || lower == "alert" || lower == "notify") return ICON_BELL;
    if (lower == "check" || lower == "ok") return ICON_CHECK;
    if (lower == "cross" || lower == "cancel" || lower == "error") return ICON_CROSS;

    return nullptr;
}



