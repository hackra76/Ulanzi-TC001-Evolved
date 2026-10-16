#pragma once
#include <Arduino.h>

enum ScreensaverStyle {
    SCREENSAVER_MATRIX = 0,   // Matrix Digital Rain (Cyberpunk cascading code)
    SCREENSAVER_FIRE   = 1,   // NeoPixel Fire / Lava (Molten thermal simulation)
    SCREENSAVER_PACMAN = 2,   // Pac-Man & Blinky (Retro 80s arcade chase)
    SCREENSAVER_LIFE   = 3,   // Conway's Game of Life (Cellular automata)
    SCREENSAVER_SHUFFLE= 4,   // Periodically cycle through all styles
    SCREENSAVER_STYLE_COUNT
};

void Screensaver_Init();
void Screensaver_Update();
void Screensaver_Render();

// Configuration & State
void Screensaver_SetStyle(ScreensaverStyle style);
ScreensaverStyle Screensaver_GetStyle();
ScreensaverStyle Screensaver_GetActiveStyle();

void Screensaver_SetTimeoutMin(uint8_t minutes); // 0 = disabled, 1..60
uint8_t Screensaver_GetTimeoutMin();

void Screensaver_Activate();
void Screensaver_Deactivate();
bool Screensaver_IsActive();

void Screensaver_ResetActivity();
uint32_t Screensaver_GetIdleMs();
