#include "DisplayMatrix.h"

// TC001 matrix topology: 32 columns wide, 8 rows high
// Row 0 left->right, Row 1 right->left (serpentine zigzag rows)
static Adafruit_NeoMatrix s_matrix = Adafruit_NeoMatrix(
    MATRIX_WIDTH, MATRIX_HEIGHT, PIN_MATRIX,
    NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
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

static bool s_suppressShow = false;

void DisplayMatrix_Clear() {
    s_matrix.clear();
}

void DisplayMatrix_Show() {
    if (s_suppressShow) return;
    s_matrix.show();
}

void DisplayMatrix_SuppressShow(bool suppress) {
    s_suppressShow = suppress;
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

uint32_t DisplayMatrix_GetPixel(int16_t x, int16_t y) {
    if (x < 0 || x >= MATRIX_WIDTH || y < 0 || y >= MATRIX_HEIGHT) return 0;
    uint16_t idx = (y & 1) ? ((y + 1) * MATRIX_WIDTH - 1 - x) : (y * MATRIX_WIDTH + x);
    return s_matrix.getPixelColor(idx);
}

void DisplayMatrix_StreamBMP(Client& client, uint8_t scale) {
    if (scale < 1) scale = 1;
    if (scale > 16) scale = 16;
    uint32_t width = MATRIX_WIDTH * scale;
    uint32_t height = MATRIX_HEIGHT * scale;
    uint32_t rowSize = ((width * 3 + 3) / 4) * 4;
    uint32_t imageSize = rowSize * height;
    uint32_t fileSize = 54 + imageSize;

    uint8_t header[54] = {
        'B', 'M',
        (uint8_t)(fileSize & 0xFF), (uint8_t)((fileSize >> 8) & 0xFF), (uint8_t)((fileSize >> 16) & 0xFF), (uint8_t)((fileSize >> 24) & 0xFF),
        0, 0, 0, 0,
        54, 0, 0, 0,
        40, 0, 0, 0,
        (uint8_t)(width & 0xFF), (uint8_t)((width >> 8) & 0xFF), (uint8_t)((width >> 16) & 0xFF), (uint8_t)((width >> 24) & 0xFF),
        (uint8_t)(height & 0xFF), (uint8_t)((height >> 8) & 0xFF), (uint8_t)((height >> 16) & 0xFF), (uint8_t)((height >> 24) & 0xFF),
        1, 0,
        24, 0,
        0, 0, 0, 0,
        (uint8_t)(imageSize & 0xFF), (uint8_t)((imageSize >> 8) & 0xFF), (uint8_t)((imageSize >> 16) & 0xFF), (uint8_t)((imageSize >> 24) & 0xFF),
        0x13, 0x0B, 0, 0,
        0x13, 0x0B, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0
    };

    client.write(header, 54);

    uint8_t rowBuf[960]; // scale=10 -> width=320 -> 960 bytes
    for (int32_t y = (int32_t)height - 1; y >= 0; y--) {
        uint16_t my = y / scale;
        bool gapY = (scale > 2) && ((y % scale) == (scale - 1));
        for (uint32_t x = 0; x < width; x++) {
            uint16_t mx = x / scale;
            bool gapX = (scale > 2) && ((x % scale) == (scale - 1));
            uint32_t pos = x * 3;
            if (gapX || gapY) {
                rowBuf[pos]     = 10; // B
                rowBuf[pos + 1] = 13; // G
                rowBuf[pos + 2] = 18; // R
            } else {
                uint32_t c = DisplayMatrix_GetPixel(mx, my);
                rowBuf[pos]     = (uint8_t)(c & 0xFF);         // B
                rowBuf[pos + 1] = (uint8_t)((c >> 8) & 0xFF);  // G
                rowBuf[pos + 2] = (uint8_t)((c >> 16) & 0xFF); // R
            }
        }
        client.write(rowBuf, rowSize);
    }
}

String DisplayMatrix_GetAsciiArt() {
    String out = "";
    out.reserve(MATRIX_WIDTH * MATRIX_HEIGHT * 2 + MATRIX_HEIGHT);
    for (int16_t y = 0; y < MATRIX_HEIGHT; y++) {
        for (int16_t x = 0; x < MATRIX_WIDTH; x++) {
            uint32_t c = DisplayMatrix_GetPixel(x, y);
            uint8_t r = (c >> 16) & 0xFF;
            uint8_t g = (c >> 8) & 0xFF;
            uint8_t b = c & 0xFF;
            if (r < 15 && g < 15 && b < 15) {
                out += "..";
            } else {
                out += "##";
            }
        }
        out += "\n";
    }
    return out;
}

void DisplayMatrix_GetJson(String& out) {
    out = "{\"width\":32,\"height\":8,\"pixels\":[";
    for (int16_t y = 0; y < MATRIX_HEIGHT; y++) {
        if (y > 0) out += ",";
        out += "[";
        for (int16_t x = 0; x < MATRIX_WIDTH; x++) {
            if (x > 0) out += ",";
            uint32_t c = DisplayMatrix_GetPixel(x, y);
            char hexBuf[10];
            snprintf(hexBuf, sizeof(hexBuf), "\"#%06X\"", (unsigned int)(c & 0xFFFFFF));
            out += hexBuf;
        }
        out += "]";
    }
    out += "]}";
}

// 3x5 Compact Bitmap Font for Digits 0..9
static const uint8_t s_font3x5[10][5] PROGMEM = {
    { 0x07, 0x05, 0x05, 0x05, 0x07 }, // 0
    { 0x02, 0x06, 0x02, 0x02, 0x07 }, // 1
    { 0x07, 0x01, 0x07, 0x04, 0x07 }, // 2
    { 0x07, 0x01, 0x07, 0x01, 0x07 }, // 3
    { 0x05, 0x05, 0x07, 0x01, 0x01 }, // 4
    { 0x07, 0x04, 0x07, 0x01, 0x07 }, // 5
    { 0x07, 0x04, 0x07, 0x05, 0x07 }, // 6
    { 0x07, 0x01, 0x02, 0x02, 0x02 }, // 7
    { 0x07, 0x05, 0x07, 0x05, 0x07 }, // 8
    { 0x07, 0x05, 0x07, 0x01, 0x07 }, // 9
};

// 3x5 Compact Bitmap Font for Uppercase Letters A..Z (ASCII 65..90)
static const uint8_t s_fontAlpha3x5[26][5] PROGMEM = {
    { 0x02, 0x05, 0x07, 0x05, 0x05 }, // 'A'
    { 0x06, 0x05, 0x06, 0x05, 0x06 }, // 'B'
    { 0x07, 0x04, 0x04, 0x04, 0x07 }, // 'C'
    { 0x06, 0x05, 0x05, 0x05, 0x06 }, // 'D'
    { 0x07, 0x04, 0x07, 0x04, 0x07 }, // 'E'
    { 0x07, 0x04, 0x06, 0x04, 0x04 }, // 'F'
    { 0x07, 0x04, 0x05, 0x05, 0x07 }, // 'G'
    { 0x05, 0x05, 0x07, 0x05, 0x05 }, // 'H'
    { 0x07, 0x02, 0x02, 0x02, 0x07 }, // 'I'
    { 0x01, 0x01, 0x01, 0x05, 0x02 }, // 'J'
    { 0x05, 0x06, 0x04, 0x06, 0x05 }, // 'K'
    { 0x04, 0x04, 0x04, 0x04, 0x07 }, // 'L'
    { 0x05, 0x07, 0x05, 0x05, 0x05 }, // 'M'
    { 0x05, 0x06, 0x05, 0x03, 0x05 }, // 'N'
    { 0x07, 0x05, 0x05, 0x05, 0x07 }, // 'O'
    { 0x07, 0x05, 0x07, 0x04, 0x04 }, // 'P'
    { 0x07, 0x05, 0x05, 0x07, 0x01 }, // 'Q'
    { 0x06, 0x05, 0x06, 0x06, 0x05 }, // 'R'
    { 0x03, 0x04, 0x07, 0x01, 0x06 }, // 'S'
    { 0x07, 0x02, 0x02, 0x02, 0x02 }, // 'T'
    { 0x05, 0x05, 0x05, 0x05, 0x07 }, // 'U'
    { 0x05, 0x05, 0x05, 0x05, 0x02 }, // 'V'
    { 0x05, 0x05, 0x05, 0x07, 0x05 }, // 'W'
    { 0x05, 0x05, 0x02, 0x05, 0x05 }, // 'X'
    { 0x05, 0x05, 0x02, 0x02, 0x02 }, // 'Y'
    { 0x07, 0x01, 0x02, 0x04, 0x07 }  // 'Z'
};

int16_t DisplayMatrix_DrawSmallChar(int16_t x, int16_t y, char c, uint16_t color) {
    if (c == ' ') return 2;
    if (c == ':') {
        s_matrix.drawPixel(x, y + 1, color);
        s_matrix.drawPixel(x, y + 3, color);
        return 2;
    }
    if (c == '.') {
        s_matrix.drawPixel(x, y + 4, color);
        return 2;
    }
    if (c == ',') {
        s_matrix.drawPixel(x, y + 4, color);
        s_matrix.drawPixel(x, y + 5, color);
        return 2;
    }
    if (c == '!') {
        s_matrix.drawPixel(x, y,     color);
        s_matrix.drawPixel(x, y + 1, color);
        s_matrix.drawPixel(x, y + 2, color);
        s_matrix.drawPixel(x, y + 4, color);
        return 2;
    }
    if (c == '\'' || c == '^' || (uint8_t)c == 0xB0) {
        s_matrix.drawPixel(x,     y,     color);
        s_matrix.drawPixel(x + 1, y,     color);
        s_matrix.drawPixel(x,     y + 1, color);
        s_matrix.drawPixel(x + 1, y + 1, color);
        return 3;
    }
    if (c == '-') {
        s_matrix.drawPixel(x,     y + 2, color);
        s_matrix.drawPixel(x + 1, y + 2, color);
        s_matrix.drawPixel(x + 2, y + 2, color);
        return 4;
    }
    if (c == '+') {
        s_matrix.drawPixel(x + 1, y + 1, color);
        s_matrix.drawPixel(x,     y + 2, color);
        s_matrix.drawPixel(x + 1, y + 2, color);
        s_matrix.drawPixel(x + 2, y + 2, color);
        s_matrix.drawPixel(x + 1, y + 3, color);
        return 4;
    }
    if (c == '/') {
        s_matrix.drawPixel(x + 2, y,     color);
        s_matrix.drawPixel(x + 2, y + 1, color);
        s_matrix.drawPixel(x + 1, y + 2, color);
        s_matrix.drawPixel(x,     y + 3, color);
        s_matrix.drawPixel(x,     y + 4, color);
        return 4;
    }
    if (c == '%') {
        s_matrix.drawPixel(x,     y,     color);
        s_matrix.drawPixel(x + 2, y,     color);
        s_matrix.drawPixel(x + 2, y + 1, color);
        s_matrix.drawPixel(x + 1, y + 2, color);
        s_matrix.drawPixel(x,     y + 3, color);
        s_matrix.drawPixel(x,     y + 4, color);
        s_matrix.drawPixel(x + 2, y + 4, color);
        return 4;
    }

    const uint8_t* rowData = nullptr;
    if (c >= '0' && c <= '9') {
        rowData = s_font3x5[c - '0'];
    } else if (c >= 'A' && c <= 'Z') {
        rowData = s_fontAlpha3x5[c - 'A'];
    } else if (c >= 'a' && c <= 'z') {
        rowData = s_fontAlpha3x5[c - 'a'];
    }

    if (!rowData) return 3;

    for (int r = 0; r < 5; r++) {
        uint8_t b = pgm_read_byte(&rowData[r]);
        if (b & 4) s_matrix.drawPixel(x,     y + r, color);
        if (b & 2) s_matrix.drawPixel(x + 1, y + r, color);
        if (b & 1) s_matrix.drawPixel(x + 2, y + r, color);
    }
    return 4;
}

int16_t DisplayMatrix_GetSmallTextWidth(const char* text) {
    if (!text) return 0;
    int16_t w = 0;
    while (*text) {
        if ((uint8_t)*text == 0xC2 && (uint8_t)*(text + 1) == 0xB0) {
            w += 3;
            text += 2;
            continue;
        }
        char c = *text;
        if (c == ' ') w += 2;
        else if (c == ':' || c == '.' || c == ',' || c == '!') w += 2;
        else if (c == '\'' || c == '^' || (uint8_t)c == 0xB0) w += 3;
        else w += 4;
        text++;
    }
    return (w > 0) ? (w - 1) : 0;
}

int16_t DisplayMatrix_DrawSmallText(int16_t x, int16_t y, const char* text, uint16_t color) {
    if (!text) return 0;
    int16_t curX = x;
    while (*text) {
        if ((uint8_t)*text == 0xC2 && (uint8_t)*(text + 1) == 0xB0) {
            s_matrix.drawPixel(curX,     y,     color);
            s_matrix.drawPixel(curX + 1, y,     color);
            s_matrix.drawPixel(curX,     y + 1, color);
            s_matrix.drawPixel(curX + 1, y + 1, color);
            curX += 3;
            text += 2;
            continue;
        }
        curX += DisplayMatrix_DrawSmallChar(curX, y, *text, color);
        text++;
    }
    return curX - x;
}


void DisplayMatrix_DrawCalendarIcon(int16_t x, int16_t y, int dayOfMonth) {
    if (dayOfMonth < 1) dayOfMonth = 1;
    if (dayOfMonth > 31) dayOfMonth = 31;

    // 9x8 Dynamic Calendar Icon Base (width 9, height 8)
    // Row 0: Dual mounting rings at cols 2 and 6
    // Row 1: Solid red calendar header bar (cols 0..8)
    // Rows 2..6: White calendar paper body (cols 0..8)
    // Row 7: Rounded bottom white border (cols 1..7)
    static const uint16_t CAL_BASE_9X8[72] = {
        0,       C_RED,   C_WHITE, C_RED,   C_RED,   C_RED,   C_WHITE, C_RED,   0,
        C_RED,   C_RED,   C_RED,   C_RED,   C_RED,   C_RED,   C_RED,   C_RED,   C_RED,
        C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE,
        C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE,
        C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE,
        C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE,
        C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE,
        0,       C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, C_WHITE, 0
    };
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 9; c++) {
            uint16_t col = CAL_BASE_9X8[r * 9 + c];
            if (col != 0) {
                s_matrix.drawPixel(x + c, y + r, col);
            }
        }
    }

    if (dayOfMonth < 10) {
        // Centered single digit at cols 3..5 with symmetrical 3px white margins (cols 0..2 and 6..8)
        for (int r = 0; r < 5; r++) {
            uint8_t row = pgm_read_byte(&s_font3x5[dayOfMonth][r]);
            if (row & 4) s_matrix.drawPixel(x + 3, y + 2 + r, C_BLACK);
            if (row & 2) s_matrix.drawPixel(x + 4, y + 2 + r, C_BLACK);
            if (row & 1) s_matrix.drawPixel(x + 5, y + 2 + r, C_BLACK);
        }
    } else {
        // Two-digit date with guaranteed 1px white separation at column 4
        // Tens digit at cols 1..3 (col 0 is 1px left white margin)
        // Column 4 is white separation gap (digits never collide)
        // Units digit at cols 5..7 (col 8 is 1px right white margin)
        int tens = dayOfMonth / 10;
        int units = dayOfMonth % 10;
        for (int r = 0; r < 5; r++) {
            uint8_t rowT = pgm_read_byte(&s_font3x5[tens][r]);
            if (rowT & 4) s_matrix.drawPixel(x + 1, y + 2 + r, C_BLACK);
            if (rowT & 2) s_matrix.drawPixel(x + 2, y + 2 + r, C_BLACK);
            if (rowT & 1) s_matrix.drawPixel(x + 3, y + 2 + r, C_BLACK);

            uint8_t rowU = pgm_read_byte(&s_font3x5[units][r]);
            if (rowU & 4) s_matrix.drawPixel(x + 5, y + 2 + r, C_BLACK);
            if (rowU & 2) s_matrix.drawPixel(x + 6, y + 2 + r, C_BLACK);
            if (rowU & 1) s_matrix.drawPixel(x + 7, y + 2 + r, C_BLACK);
        }
    }
}

void DisplayMatrix_DrawWeekdayBar(int activeDayIndex, uint16_t activeColor, uint16_t inactiveColor,
                                   uint16_t weekendColor, uint16_t inactiveWeekendColor,
                                   bool isMondayStart, int startX, int y) {
    for (int i = 0; i < 7; i++) {
        bool isWknd = isMondayStart ? (i == 5 || i == 6) : (i == 0 || i == 6);
        uint16_t col;
        if (i == activeDayIndex) {
            col = activeColor;
        } else {
            col = (isWknd && weekendColor != 0) ? weekendColor : inactiveColor;
        }
        int bx = startX + i * 3;
        s_matrix.drawPixel(bx,     y, col);
        s_matrix.drawPixel(bx + 1, y, col);
    }
}

int16_t DisplayMatrix_DrawDigit3x5(int16_t x, int16_t y, uint8_t digit, uint16_t color) {
    if (digit > 9) return 3;
    for (int r = 0; r < 5; r++) {
        uint8_t row = pgm_read_byte(&s_font3x5[digit][r]);
        if (row & 4) s_matrix.drawPixel(x,     y + r, color);
        if (row & 2) s_matrix.drawPixel(x + 1, y + r, color);
        if (row & 1) s_matrix.drawPixel(x + 2, y + r, color);
    }
    return 3;
}

void DisplayMatrix_DrawTime3x5(int16_t x, int16_t y, int h, int m, bool blinkColon, uint16_t colTime, uint16_t colColon) {
    uint8_t h1 = (h / 10) % 10;
    uint8_t h2 = h % 10;
    uint8_t m1 = (m / 10) % 10;
    uint8_t m2 = m % 10;

    int16_t curX = x;
    DisplayMatrix_DrawDigit3x5(curX, y, h1, colTime); curX += 4;
    DisplayMatrix_DrawDigit3x5(curX, y, h2, colTime); curX += 4;
    if (blinkColon) {
        s_matrix.drawPixel(curX, y + 1, colColon);
        s_matrix.drawPixel(curX, y + 3, colColon);
    }
    curX += 2;
    DisplayMatrix_DrawDigit3x5(curX, y, m1, colTime); curX += 4;
    DisplayMatrix_DrawDigit3x5(curX, y, m2, colTime);
}

void DisplayMatrix_DrawTimeWithSeconds(int16_t startX, int16_t y, int h, int m, int s, bool blinkColon, uint16_t colTime, uint16_t colColon, uint16_t colSec) {
    uint8_t h1 = (h / 10) % 10;
    uint8_t h2 = h % 10;
    uint8_t m1 = (m / 10) % 10;
    uint8_t m2 = m % 10;
    uint8_t s1 = (s / 10) % 10;
    uint8_t s2 = s % 10;

    // Fixed column positions when icon is at x=0..7:
    // H1: startX..startX+2 (e.g. 9..11)
    DisplayMatrix_DrawDigit3x5(startX, y, h1, colTime);
    // H2: startX+4..startX+6 (e.g. 13..15)
    DisplayMatrix_DrawDigit3x5(startX + 4, y, h2, colTime);
    // Colon at startX+7 (e.g. 16)
    if (blinkColon) {
        s_matrix.drawPixel(startX + 7, y + 1, colColon);
        s_matrix.drawPixel(startX + 7, y + 3, colColon);
    }
    // M1: startX+8..startX+10 (e.g. 17..19)
    DisplayMatrix_DrawDigit3x5(startX + 8, y, m1, colTime);
    // M2: startX+12..startX+14 (e.g. 21..23)
    DisplayMatrix_DrawDigit3x5(startX + 12, y, m2, colTime);
    // S1: startX+16..startX+18 (e.g. 25..27)
    DisplayMatrix_DrawDigit3x5(startX + 16, y, s1, colSec);
    // S2: startX+20..startX+22 (e.g. 29..31)
    DisplayMatrix_DrawDigit3x5(startX + 20, y, s2, colSec);
}

void DisplayMatrix_DrawFullTime3x5(int16_t startX, int16_t y, int h, int m, int s, bool blinkColon, uint16_t colTime, uint16_t colColon, uint16_t colSec) {
    uint8_t h1 = (h / 10) % 10;
    uint8_t h2 = h % 10;
    uint8_t m1 = (m / 10) % 10;
    uint8_t m2 = m % 10;
    uint8_t s1 = (s / 10) % 10;
    uint8_t s2 = s % 10;

    // Full width layout without icon (width 27, centered at startX = 2):
    // x=2..4: H1
    DisplayMatrix_DrawDigit3x5(startX, y, h1, colTime);
    // x=6..8: H2
    DisplayMatrix_DrawDigit3x5(startX + 4, y, h2, colTime);
    // x=10: Colon 1
    if (blinkColon) {
        s_matrix.drawPixel(startX + 8, y + 1, colColon);
        s_matrix.drawPixel(startX + 8, y + 3, colColon);
    }
    // x=12..14: M1
    DisplayMatrix_DrawDigit3x5(startX + 10, y, m1, colTime);
    // x=16..18: M2
    DisplayMatrix_DrawDigit3x5(startX + 14, y, m2, colTime);
    // x=20: Colon 2
    if (blinkColon) {
        s_matrix.drawPixel(startX + 18, y + 1, colColon);
        s_matrix.drawPixel(startX + 18, y + 3, colColon);
    }
    // x=22..24: S1
    DisplayMatrix_DrawDigit3x5(startX + 20, y, s1, colSec);
    // x=26..28: S2
    DisplayMatrix_DrawDigit3x5(startX + 24, y, s2, colSec);
}

// =============================================================================
//  Screen Transition Engine
// =============================================================================

struct MatrixPixelGRB {
    uint8_t g;
    uint8_t r;
    uint8_t b;
};

static MatrixPixelGRB s_prevFrame[MATRIX_NUM_LEDS];
static MatrixPixelGRB s_nextFrame[MATRIX_NUM_LEDS];

static inline uint16_t getLedIndex(int16_t x, int16_t y) {
    return (y & 1) ? ((y + 1) * MATRIX_WIDTH - 1 - x) : (y * MATRIX_WIDTH + x);
}

static inline void setMatrixRawPixel(int16_t x, int16_t y, const MatrixPixelGRB& px) {
    if (x < 0 || x >= MATRIX_WIDTH || y < 0 || y >= MATRIX_HEIGHT) return;
    uint8_t* raw = s_matrix.getPixels();
    if (!raw) return;
    uint16_t ledIdx = getLedIndex(x, y);
    raw[ledIdx * 3 + 0] = px.g;
    raw[ledIdx * 3 + 1] = px.r;
    raw[ledIdx * 3 + 2] = px.b;
}

void DisplayMatrix_CaptureCurrentFrame() {
    uint8_t* raw = s_matrix.getPixels();
    if (!raw) return;
    for (int16_t y = 0; y < MATRIX_HEIGHT; y++) {
        for (int16_t x = 0; x < MATRIX_WIDTH; x++) {
            uint16_t ledIdx = getLedIndex(x, y);
            s_prevFrame[y * MATRIX_WIDTH + x].g = raw[ledIdx * 3 + 0];
            s_prevFrame[y * MATRIX_WIDTH + x].r = raw[ledIdx * 3 + 1];
            s_prevFrame[y * MATRIX_WIDTH + x].b = raw[ledIdx * 3 + 2];
        }
    }
}

void DisplayMatrix_CaptureTargetFrame() {
    uint8_t* raw = s_matrix.getPixels();
    if (!raw) return;
    for (int16_t y = 0; y < MATRIX_HEIGHT; y++) {
        for (int16_t x = 0; x < MATRIX_WIDTH; x++) {
            uint16_t ledIdx = getLedIndex(x, y);
            s_nextFrame[y * MATRIX_WIDTH + x].g = raw[ledIdx * 3 + 0];
            s_nextFrame[y * MATRIX_WIDTH + x].r = raw[ledIdx * 3 + 1];
            s_nextFrame[y * MATRIX_WIDTH + x].b = raw[ledIdx * 3 + 2];
        }
    }
}

void DisplayMatrix_RunTransition(uint8_t effectType) {
    if (effectType == 7) { // Random
        effectType = 1 + (esp_random() % 6);
    }

    switch (effectType) {
        case 1: { // Slide Left (old moves left, new enters from right)
            for (int step = 1; step <= 16; step++) {
                int dx = step * 2;
                for (int16_t y = 0; y < MATRIX_HEIGHT; y++) {
                    for (int16_t x = 0; x < MATRIX_WIDTH; x++) {
                        if (x + dx < MATRIX_WIDTH) {
                            setMatrixRawPixel(x, y, s_prevFrame[y * MATRIX_WIDTH + (x + dx)]);
                        } else {
                            setMatrixRawPixel(x, y, s_nextFrame[y * MATRIX_WIDTH + (x + dx - MATRIX_WIDTH)]);
                        }
                    }
                }
                s_matrix.show();
                delay(10);
            }
            break;
        }
        case 2: { // Slide Right (old moves right, new enters from left)
            for (int step = 1; step <= 16; step++) {
                int dx = step * 2;
                for (int16_t y = 0; y < MATRIX_HEIGHT; y++) {
                    for (int16_t x = 0; x < MATRIX_WIDTH; x++) {
                        if (x >= dx) {
                            setMatrixRawPixel(x, y, s_prevFrame[y * MATRIX_WIDTH + (x - dx)]);
                        } else {
                            setMatrixRawPixel(x, y, s_nextFrame[y * MATRIX_WIDTH + (x - dx + MATRIX_WIDTH)]);
                        }
                    }
                }
                s_matrix.show();
                delay(10);
            }
            break;
        }
        case 3: { // Slide Up (old moves up, new enters from bottom)
            for (int dy = 1; dy <= 8; dy++) {
                for (int16_t y = 0; y < MATRIX_HEIGHT; y++) {
                    for (int16_t x = 0; x < MATRIX_WIDTH; x++) {
                        if (y + dy < MATRIX_HEIGHT) {
                            setMatrixRawPixel(x, y, s_prevFrame[(y + dy) * MATRIX_WIDTH + x]);
                        } else {
                            setMatrixRawPixel(x, y, s_nextFrame[(y + dy - MATRIX_HEIGHT) * MATRIX_WIDTH + x]);
                        }
                    }
                }
                s_matrix.show();
                delay(20);
            }
            break;
        }
        case 4: { // Slide Down (old moves down, new enters from top)
            for (int dy = 1; dy <= 8; dy++) {
                for (int16_t y = 0; y < MATRIX_HEIGHT; y++) {
                    for (int16_t x = 0; x < MATRIX_WIDTH; x++) {
                        if (y >= dy) {
                            setMatrixRawPixel(x, y, s_prevFrame[(y - dy) * MATRIX_WIDTH + x]);
                        } else {
                            setMatrixRawPixel(x, y, s_nextFrame[(y - dy + MATRIX_HEIGHT) * MATRIX_WIDTH + x]);
                        }
                    }
                }
                s_matrix.show();
                delay(20);
            }
            break;
        }
        case 5: { // Crossfade / Dissolve
            static const uint8_t alphas[] = { 25, 51, 76, 102, 128, 153, 179, 204, 230, 255 };
            for (uint8_t a : alphas) {
                uint8_t inv = 255 - a;
                for (int16_t y = 0; y < MATRIX_HEIGHT; y++) {
                    for (int16_t x = 0; x < MATRIX_WIDTH; x++) {
                        int idx = y * MATRIX_WIDTH + x;
                        MatrixPixelGRB px;
                        px.g = (uint8_t)(((uint16_t)s_prevFrame[idx].g * inv + (uint16_t)s_nextFrame[idx].g * a) / 255);
                        px.r = (uint8_t)(((uint16_t)s_prevFrame[idx].r * inv + (uint16_t)s_nextFrame[idx].r * a) / 255);
                        px.b = (uint8_t)(((uint16_t)s_prevFrame[idx].b * inv + (uint16_t)s_nextFrame[idx].b * a) / 255);
                        setMatrixRawPixel(x, y, px);
                    }
                }
                s_matrix.show();
                delay(18);
            }
            break;
        }
        case 6: { // Curtain (Center Split Wipe)
            for (int step = 1; step <= 16; step++) {
                int leftBound = 16 - step;
                int rightBound = 15 + step;
                for (int16_t y = 0; y < MATRIX_HEIGHT; y++) {
                    for (int16_t x = 0; x < MATRIX_WIDTH; x++) {
                        if (x >= leftBound && x <= rightBound) {
                            setMatrixRawPixel(x, y, s_nextFrame[y * MATRIX_WIDTH + x]);
                        } else {
                            setMatrixRawPixel(x, y, s_prevFrame[y * MATRIX_WIDTH + x]);
                        }
                    }
                }
                s_matrix.show();
                delay(12);
            }
            break;
        }
        case 0: // Instant Cut / None
        default: {
            for (int16_t y = 0; y < MATRIX_HEIGHT; y++) {
                for (int16_t x = 0; x < MATRIX_WIDTH; x++) {
                    setMatrixRawPixel(x, y, s_nextFrame[y * MATRIX_WIDTH + x]);
                }
            }
            s_matrix.show();
            break;
        }
    }
}


