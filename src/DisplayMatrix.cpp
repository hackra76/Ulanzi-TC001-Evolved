#include "DisplayMatrix.h"

// TC001 matrix topology: 32 columns wide, 8 rows high
// Column 0 top->down, Column 1 bottom->up (zigzag columns)
static Adafruit_NeoMatrix s_matrix = Adafruit_NeoMatrix(
    MATRIX_WIDTH, MATRIX_HEIGHT, PIN_MATRIX,
    NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
    NEO_GRB + NEO_KHZ800
);

static uint8_t s_brightness = DEFAULT_BRIGHTNESS;
static int16_t s_scrollX = MATRIX_WIDTH;
static uint32_t s_lastScrollMs = 0;

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

int16_t DisplayMatrix_GetTextWidth(const char* text) {
    if (!text) return 0;
    return (int16_t)(strlen(text) * 6);
}

void DisplayMatrix_ResetTicker() {
    s_scrollX = MATRIX_WIDTH;
    s_lastScrollMs = millis();
}

bool DisplayMatrix_ScrollTicker(const char* text, uint16_t color, const uint16_t* pinnedIcon, int speedMs) {
    if (!text) return true;
    uint32_t now = millis();
    int16_t textWidth = DisplayMatrix_GetTextWidth(text);
    int16_t leftBoundary = (pinnedIcon != nullptr) ? 9 : 0;
    int16_t endX = leftBoundary - textWidth;

    if (now - s_lastScrollMs >= (uint32_t)speedMs) {
        s_lastScrollMs = now;
        s_scrollX--;
    }

    bool finished = false;
    if (s_scrollX < endX) {
        s_scrollX = MATRIX_WIDTH;
        finished = true;
    }

    s_matrix.clear();
    // Render text at current scroll offset
    DisplayMatrix_DrawText(s_scrollX, 1, text, color);

    // If an icon is pinned on the left, clear columns 0..8 and redraw icon + spacer column
    if (pinnedIcon != nullptr) {
        for (int16_t x = 0; x <= 8; x++) {
            for (int16_t y = 0; y < MATRIX_HEIGHT; y++) {
                s_matrix.drawPixel(x, y, C_BLACK);
            }
        }
        DisplayMatrix_DrawIcon(0, 0, pinnedIcon);
    }

    DisplayMatrix_Show();
    return finished;
}

Adafruit_NeoMatrix& DisplayMatrix_GetMatrix() {
    return s_matrix;
}
