#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include "Config.h"
#include "Icons.h"

void DisplayMatrix_Init();
void DisplayMatrix_SetBrightness(uint8_t brightness);
uint8_t DisplayMatrix_GetBrightness();
void DisplayMatrix_Clear();
void DisplayMatrix_Show();
void DisplayMatrix_DrawIcon(int16_t x, int16_t y, const uint16_t* bitmap8x8);
void DisplayMatrix_DrawText(int16_t x, int16_t y, const char* text, uint16_t color);
int16_t DisplayMatrix_GetTextWidth(const char* text);
bool DisplayMatrix_ScrollTicker(const char* text, uint16_t color, const uint16_t* pinnedIcon = nullptr, int speedMs = 45);
void DisplayMatrix_ResetTicker();
Adafruit_NeoMatrix& DisplayMatrix_GetMatrix();
uint32_t DisplayMatrix_GetPixel(int16_t x, int16_t y);
void DisplayMatrix_StreamBMP(Client& client, uint8_t scale = 10);
String DisplayMatrix_GetAsciiArt();
void DisplayMatrix_GetJson(String& out);
int16_t DisplayMatrix_DrawSmallChar(int16_t x, int16_t y, char c, uint16_t color);
int16_t DisplayMatrix_DrawSmallText(int16_t x, int16_t y, const char* text, uint16_t color);
int16_t DisplayMatrix_GetSmallTextWidth(const char* text);
void DisplayMatrix_DrawCalendarIcon(int16_t x, int16_t y, int dayOfMonth);
void DisplayMatrix_DrawWeekdayBar(int activeDayIndex, uint16_t activeColor, uint16_t inactiveColor, uint16_t weekendColor = 0, uint16_t inactiveWeekendColor = 0, bool isMondayStart = true, int startX = 10, int y = 7);

// 3x5 Compact Digit & Time Rendering (eliminates vertical collision with weekday bars)
int16_t DisplayMatrix_DrawDigit3x5(int16_t x, int16_t y, uint8_t digit, uint16_t color);
void DisplayMatrix_DrawTime3x5(int16_t x, int16_t y, int h, int m, bool blinkColon, uint16_t colTime = 0xFFFF, uint16_t colColon = 0xFFFF);
void DisplayMatrix_DrawTimeWithSeconds(int16_t startX, int16_t y, int h, int m, int s, bool blinkColon, uint16_t colTime = 0xFFFF, uint16_t colColon = 0xFFFF, uint16_t colSec = 0x07FF);
void DisplayMatrix_DrawFullTime3x5(int16_t startX, int16_t y, int h, int m, int s, bool blinkColon, uint16_t colTime = 0xFFFF, uint16_t colColon = 0xFFFF, uint16_t colSec = 0x07FF);

inline uint16_t DisplayMatrix_Color(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// Screen Transition Engine
void DisplayMatrix_SuppressShow(bool suppress);
void DisplayMatrix_CaptureCurrentFrame();
void DisplayMatrix_CaptureTargetFrame();
void DisplayMatrix_RunTransition(uint8_t effectType);


// Interactive Canvas Studio API (Companion App / REST)
void DisplayMatrix_SetCanvas(const uint16_t pixels[MATRIX_HEIGHT][MATRIX_WIDTH], uint16_t durationSec);
bool DisplayMatrix_IsCanvasActive();
void DisplayMatrix_DismissCanvas();
void DisplayMatrix_RenderCanvas();
