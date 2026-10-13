#include "DisplayMatrix.h"

// TC001 matrix topology: 32 columns wide, 8 rows high
// Column 0 top->down, Column 1 bottom->up (zigzag columns)
static Adafruit_NeoMatrix s_matrix = Adafruit_NeoMatrix(
    MATRIX_WIDTH, MATRIX_HEIGHT, PIN_MATRIX,
    NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
    NEO_GRB + NEO_KHZ800
);

static uint8_t s_brightness = DEFAULT_BRIGHTNESS;

void DisplayMatrix_Init() {
    s_matrix.begin();
    s_matrix.setTextWrap(false);
    s_matrix.setBrightness(s_brightness);
    s_matrix.clear();
    s_matrix.show();
}

void DisplayMatrix_SetBrightness(uint8_t brightness) {
    s_brightness = constrain(brightness, (uint8_t)1, (uint8_t)MAX_BRIGHTNESS);
    s_matrix.setBrightness(s_brightness);
}

uint8_t DisplayMatrix_GetBrightness() {
    return s_brightness;
}

void DisplayMatrix_Clear() {
    s_matrix.clear();
}

void DisplayMatrix_Show() {
    s_matrix.show();
}

void DisplayMatrix_DrawIcon(int16_t x, int16_t y, const uint16_t* bitmap8x8) {
    if (!bitmap8x8) return;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            uint16_t col = bitmap8x8[r * 8 + c];
            if (col != 0) { // 0x0000 = transparent
                s_matrix.drawPixel(x + c, y + r, col);
            }
        }
    }
}

void DisplayMatrix_DrawText(int16_t x, int16_t y, const char* text, uint16_t color) {
    if (!text) return;
    s_matrix.setCursor(x, y);
    s_matrix.setTextColor(color);
    s_matrix.print(text);
}

Adafruit_NeoMatrix& DisplayMatrix_GetMatrix() {
    return s_matrix;
}
