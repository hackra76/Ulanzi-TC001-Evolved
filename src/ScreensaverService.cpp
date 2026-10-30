#include "ScreensaverService.h"
#include "DisplayMatrix.h"
#include "Icons.h"
#include "Config.h"
#include <Preferences.h>

static ScreensaverStyle s_style = SCREENSAVER_MATRIX;
static uint8_t s_timeoutMin = 0; // 0 = disabled
static bool s_isActive = false;
static uint32_t s_lastActivityMs = 0;

static uint32_t s_lastRenderMs = 0;
static uint32_t s_lastShuffleMs = 0;
static uint8_t s_shuffleStyle = 0;

// --- Engine 1: Matrix Digital Rain ---
struct RainCol {
    int8_t headY;
    uint8_t length;
    uint8_t speed;
    uint8_t counter;
};
static RainCol s_rainCols[32];

static void initMatrixRain() {
    for (int c = 0; c < 32; c++) {
        s_rainCols[c].headY = -(int8_t)(random(1, 10));
        s_rainCols[c].length = random(3, 7);
        s_rainCols[c].speed = random(1, 4);
        s_rainCols[c].counter = 0;
    }
}

static void renderMatrixRain() {
    DisplayMatrix_Clear();
    auto& matrix = DisplayMatrix_GetMatrix();

    for (int c = 0; c < 32; c++) {
        s_rainCols[c].counter++;
        if (s_rainCols[c].counter >= s_rainCols[c].speed) {
            s_rainCols[c].counter = 0;
            s_rainCols[c].headY++;
            if (s_rainCols[c].headY - s_rainCols[c].length > 8) {
                s_rainCols[c].headY = -(int8_t)(random(1, 8));
                s_rainCols[c].length = random(3, 7);
                s_rainCols[c].speed = random(1, 4);
            }
        }

        int8_t h = s_rainCols[c].headY;
        for (int l = 0; l <= s_rainCols[c].length; l++) {
            int8_t y = h - l;
            if (y >= 0 && y < 8) {
                uint16_t col;
                if (l == 0) {
                    col = C_WHITE; // Bright phosphor leading spark
                } else if (l == 1) {
                    col = 0x07E0;  // Neon Green
                } else if (l <= 3) {
                    col = 0x04C0;  // Emerald
                } else if (l <= 5) {
                    col = 0x0280;  // Dim Green
                } else {
                    col = 0x0160;  // Dark phosphor trail
                }
                matrix.drawPixel(c, y, col);
            }
        }
    }
    DisplayMatrix_Show();
}

// --- Engine 2: NeoPixel Fire / Lava ---
static uint8_t s_fireHeat[8][32];

static uint16_t fireColor(uint8_t h) {
    if (h < 25) return 0x0000;
    if (h < 75) {
        // Black to Deep Red
        uint8_t r = (h - 25) * 31 / 50;
        return (r << 11);
    }
    if (h < 150) {
        // Deep Red to Orange
        uint8_t g = (h - 75) * 45 / 75;
        return (31 << 11) | (g << 5);
    }
    if (h < 220) {
        // Orange to Yellow
        uint8_t g = 45 + (h - 150) * 18 / 70;
        return (31 << 11) | (g << 5);
    }
    // Yellow to Pure White
    uint8_t b = (h - 220) * 31 / 35;
    return (31 << 11) | (63 << 5) | b;
}

static void initFire() {
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 32; x++) {
            s_fireHeat[y][x] = 0;
        }
    }
}

static void renderFire() {
    DisplayMatrix_Clear();
    auto& matrix = DisplayMatrix_GetMatrix();

    // 1. Generate bottom row embers
    for (int x = 0; x < 32; x++) {
        if (random(0, 100) > 25) {
            s_fireHeat[7][x] = random(170, 255);
        } else {
            s_fireHeat[7][x] = random(40, 130);
        }
    }

    // 2. Propagate heat upwards with decay and wind
    for (int y = 0; y < 7; y++) {
        for (int x = 0; x < 32; x++) {
            int srcX = (x + random(0, 3) - 1 + 32) % 32;
            int decay = random(1, 4);
            int below = s_fireHeat[y + 1][srcX];
            s_fireHeat[y][x] = (below > decay) ? (below - decay) : 0;
        }
    }

    // 3. Render pixel heat
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 32; x++) {
            uint16_t c = fireColor(s_fireHeat[y][x]);
            if (c != 0) matrix.drawPixel(x, y, c);
        }
    }
    DisplayMatrix_Show();
}

// --- Engine 3: Pac-Man & Blinky Arcade ---
static int16_t s_pacX = -6;
static int8_t s_pacDir = 1; // 1 = right, -1 = left
static uint8_t s_pacMouth = 0;
static bool s_superPac = false;
static uint32_t s_dotsMask = 0xFFFFFFFF; // 32 dots across matrix

static void initPacman() {
    s_pacX = -6;
    s_pacDir = 1;
    s_pacMouth = 0;
    s_superPac = false;
    s_dotsMask = 0xFFFFFFFF;
}

static void renderPacman() {
    DisplayMatrix_Clear();
    auto& matrix = DisplayMatrix_GetMatrix();

    s_pacMouth = (s_pacMouth + 1) % 4;
    s_pacX += s_pacDir;

    // Turn around at borders
    if (s_pacDir > 0 && s_pacX > 38) {
        s_pacDir = -1;
        s_superPac = true; // Turn tables: Pac-Man chases ghost!
    } else if (s_pacDir < 0 && s_pacX < -12) {
        s_pacDir = 1;
        s_superPac = false;
        s_dotsMask = 0xFFFFFFFF; // Replenish pellets
    }

    // 1. Draw Dot Pellets along Row 4
    for (int col = 0; col < 32; col += 3) {
        if ((s_dotsMask & (1UL << (col % 32))) && (!s_superPac || (col % 6 == 0))) {
            matrix.drawPixel(col, 4, (col == 15 || col == 30) ? C_WHITE : 0xFD20); // Energizer
        }
    }
    // Eat dot under Pac-Man
    if (s_pacX >= 0 && s_pacX < 32) {
        s_dotsMask &= ~(1UL << s_pacX);
    }

    // 2. Draw Pac-Man (6x6 bitmap)
    static const uint8_t PAC_OPEN_R[6] = { 0x1E, 0x3E, 0x3C, 0x30, 0x3C, 0x1E };
    static const uint8_t PAC_MID_R[6]  = { 0x1E, 0x3F, 0x3E, 0x3C, 0x3E, 0x1E };
    static const uint8_t PAC_CLOSE[6]  = { 0x1E, 0x3F, 0x3F, 0x3F, 0x3F, 0x1E };
    static const uint8_t PAC_OPEN_L[6] = { 0x1E, 0x1F, 0x0F, 0x03, 0x0F, 0x1F };
    static const uint8_t PAC_MID_L[6]  = { 0x1E, 0x3F, 0x1F, 0x0F, 0x1F, 0x3F };

    const uint8_t* pacMap = PAC_CLOSE;
    if (s_pacDir > 0) {
        pacMap = (s_pacMouth == 0 || s_pacMouth == 2) ? PAC_OPEN_R : ((s_pacMouth == 1) ? PAC_MID_R : PAC_CLOSE);
    } else {
        pacMap = (s_pacMouth == 0 || s_pacMouth == 2) ? PAC_OPEN_L : ((s_pacMouth == 1) ? PAC_MID_L : PAC_CLOSE);
    }

    for (int r = 0; r < 6; r++) {
        uint8_t row = pacMap[r];
        for (int c = 0; c < 6; c++) {
            if (row & (1 << (5 - c))) {
                int px = s_pacX + c;
                int py = 1 + r;
                if (px >= 0 && px < 32) {
                    matrix.drawPixel(px, py, C_YELLOW);
                }
            }
        }
    }

    // 3. Draw Blinky / Blue Ghost (6x6 bitmap)
    int16_t ghostX = (s_pacDir > 0) ? (s_pacX - 10) : (s_pacX + 10);
    static const uint8_t GHOST_BODY[6] = { 0x1E, 0x3F, 0x3F, 0x3F, 0x3F, 0x2D };
    uint16_t ghostCol = s_superPac ? (((s_pacX % 4) == 0) ? C_WHITE : 0x001F) : C_RED;

    for (int r = 0; r < 6; r++) {
        uint8_t row = GHOST_BODY[r];
        for (int c = 0; c < 6; c++) {
            if (row & (1 << (5 - c))) {
                int px = ghostX + c;
                int py = 1 + r;
                if (px >= 0 && px < 32) {
                    // Eye pupils
                    if (r == 1 && (c == 1 || c == 4)) {
                        matrix.drawPixel(px, py, s_superPac ? C_YELLOW : C_WHITE);
                    } else if (r == 2 && (c == (s_pacDir > 0 ? 2 : 1) || c == (s_pacDir > 0 ? 5 : 4))) {
                        matrix.drawPixel(px, py, s_superPac ? C_ORANGE : C_BLUE);
                    } else {
                        matrix.drawPixel(px, py, ghostCol);
                    }
                }
            }
        }
    }

    DisplayMatrix_Show();
}

// --- Engine 4: Conway's Game of Life ---
static uint8_t s_lifeGrid[8][32];
static uint8_t s_lifeAge[8][32];
static uint16_t s_lifeStagnantCount = 0;
static uint32_t s_lastGridHash = 0;

static void reseedLife() {
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 32; x++) {
            if (random(0, 100) < 22) {
                s_lifeGrid[y][x] = 1;
                s_lifeAge[y][x] = 1;
            } else {
                s_lifeGrid[y][x] = 0;
                s_lifeAge[y][x] = 0;
            }
        }
    }
    s_lifeStagnantCount = 0;
}

static void initLife() {
    reseedLife();
}

static void renderLife() {
    DisplayMatrix_Clear();
    auto& matrix = DisplayMatrix_GetMatrix();

    uint8_t next[8][32] = {0};
    uint32_t liveCount = 0;
    uint32_t gridHash = 0;

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 32; x++) {
            int neighbors = 0;
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    if (dx == 0 && dy == 0) continue;
                    int ny = (y + dy + 8) % 8;
                    int nx = (x + dx + 32) % 32;
                    if (s_lifeGrid[ny][nx]) neighbors++;
                }
            }

            if (s_lifeGrid[y][x]) {
                if (neighbors == 2 || neighbors == 3) {
                    next[y][x] = 1;
                    s_lifeAge[y][x] = (s_lifeAge[y][x] < 250) ? (s_lifeAge[y][x] + 1) : 250;
                    liveCount++;
                    gridHash = (gridHash << 1) ^ (x * 8 + y);
                } else {
                    next[y][x] = 0;
                    s_lifeAge[y][x] = 0;
                }
            } else {
                if (neighbors == 3) {
                    next[y][x] = 1;
                    s_lifeAge[y][x] = 1;
                    liveCount++;
                    gridHash = (gridHash << 1) ^ (x * 8 + y);
                } else {
                    next[y][x] = 0;
                    s_lifeAge[y][x] = 0;
                }
            }

            // Draw living cells with color based on longevity
            if (next[y][x]) {
                uint8_t age = s_lifeAge[y][x];
                uint16_t c;
                if (age <= 1) {
                    c = C_CYAN;   // Newborn
                } else if (age <= 5) {
                    c = 0x07E0;   // Youth emerald
                } else if (age <= 12) {
                    c = C_YELLOW; // Mature
                } else {
                    c = C_MAGENTA;// Ancient
                }
                matrix.drawPixel(x, y, c);
            }
        }
    }

    // Copy to active grid
    memcpy(s_lifeGrid, next, sizeof(next));

    // Stagnation and extinction detection
    if (gridHash == s_lastGridHash || liveCount < 4) {
        s_lifeStagnantCount++;
    } else {
        s_lifeStagnantCount = 0;
    }
    s_lastGridHash = gridHash;

    if (s_lifeStagnantCount > 10 || liveCount == 0) {
        reseedLife();
    }

    DisplayMatrix_Show();
}

// --- Public Interface ---
void Screensaver_Init() {
    Preferences prefs;
    prefs.begin(PREF_NAMESPACE, true);
    s_style = (ScreensaverStyle)prefs.getUChar(PREF_KEY_SS_STYLE, (uint8_t)DEFAULT_SS_STYLE);
    if (s_style >= SCREENSAVER_STYLE_COUNT) s_style = (ScreensaverStyle)DEFAULT_SS_STYLE;
    s_timeoutMin = prefs.getUChar(PREF_KEY_SS_TIMEOUT, (uint8_t)DEFAULT_SS_TIMEOUT);
    prefs.end();

    s_isActive = false;
    s_lastActivityMs = millis();
    s_lastRenderMs = 0;
    s_lastShuffleMs = millis();
    s_shuffleStyle = 0;

    initMatrixRain();
    initFire();
    initPacman();
    initLife();
}

void Screensaver_SetStyle(ScreensaverStyle style) {
    if (style >= SCREENSAVER_STYLE_COUNT) style = SCREENSAVER_MATRIX;
    s_style = style;
    Preferences prefs;
    prefs.begin(PREF_NAMESPACE, false);
    prefs.putUChar(PREF_KEY_SS_STYLE, (uint8_t)s_style);
    prefs.end();

    if (s_style == SCREENSAVER_MATRIX) initMatrixRain();
    else if (s_style == SCREENSAVER_FIRE) initFire();
    else if (s_style == SCREENSAVER_PACMAN) initPacman();
    else if (s_style == SCREENSAVER_LIFE) initLife();
}

ScreensaverStyle Screensaver_GetStyle() {
    return s_style;
}

ScreensaverStyle Screensaver_GetActiveStyle() {
    if (s_style == SCREENSAVER_SHUFFLE) {
        return (ScreensaverStyle)(s_shuffleStyle % 4);
    }
    return s_style;
}

void Screensaver_SetTimeoutMin(uint8_t minutes) {
    s_timeoutMin = minutes;
    if (s_timeoutMin == 0 && s_isActive) {
        Screensaver_Deactivate();
    }
    Preferences prefs;
    prefs.begin(PREF_NAMESPACE, false);
    prefs.putUChar(PREF_KEY_SS_TIMEOUT, s_timeoutMin);
    prefs.end();
}

uint8_t Screensaver_GetTimeoutMin() {
    return s_timeoutMin;
}

void Screensaver_Activate() {
    if (s_isActive) return;
    s_isActive = true;
    ScreensaverStyle active = Screensaver_GetActiveStyle();
    if (active == SCREENSAVER_MATRIX) initMatrixRain();
    else if (active == SCREENSAVER_FIRE) initFire();
    else if (active == SCREENSAVER_PACMAN) initPacman();
    else if (active == SCREENSAVER_LIFE) initLife();
}

void Screensaver_Deactivate() {
    s_isActive = false;
    s_lastActivityMs = millis();
}

bool Screensaver_IsActive() {
    return s_isActive;
}

void Screensaver_ResetActivity() {
    s_lastActivityMs = millis();
    if (s_isActive) {
        Screensaver_Deactivate();
    }
}

uint32_t Screensaver_GetIdleMs() {
    return millis() - s_lastActivityMs;
}

void Screensaver_Update() {
    uint32_t now = millis();

    // 1. Shuffle cycle (every 25 seconds)
    if (s_style == SCREENSAVER_SHUFFLE && (now - s_lastShuffleMs >= 25000)) {
        s_lastShuffleMs = now;
        s_shuffleStyle = (s_shuffleStyle + 1) % 4;
        ScreensaverStyle cur = (ScreensaverStyle)s_shuffleStyle;
        if (cur == SCREENSAVER_MATRIX) initMatrixRain();
        else if (cur == SCREENSAVER_FIRE) initFire();
        else if (cur == SCREENSAVER_PACMAN) initPacman();
        else if (cur == SCREENSAVER_LIFE) initLife();
    }

    // 2. Inactivity trigger
    if (s_timeoutMin > 0 && !s_isActive) {
        uint32_t idleMs = now - s_lastActivityMs;
        if (idleMs >= (uint32_t)s_timeoutMin * 60000) {
            Screensaver_Activate();
        }
    }
}

void Screensaver_Render() {
    uint32_t now = millis();
    ScreensaverStyle active = Screensaver_GetActiveStyle();

    // Frame rate limiter
    uint32_t targetInterval = (active == SCREENSAVER_LIFE) ? 110 :
                              (active == SCREENSAVER_FIRE) ? 35 : 40;
    if (now - s_lastRenderMs < targetInterval) {
        return;
    }
    s_lastRenderMs = now;

    switch (active) {
        case SCREENSAVER_MATRIX:
            renderMatrixRain();
            break;
        case SCREENSAVER_FIRE:
            renderFire();
            break;
        case SCREENSAVER_PACMAN:
            renderPacman();
            break;
        case SCREENSAVER_LIFE:
            renderLife();
            break;
        default:
            renderMatrixRain();
            break;
    }
}
