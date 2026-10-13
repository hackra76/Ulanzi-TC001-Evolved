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
