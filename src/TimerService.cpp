#include "TimerService.h"
#include "DisplayMatrix.h"
#include "Icons.h"
#include "Buzzer.h"

static TimerMode s_mode = TIMER_MODE_COUNTDOWN;
static TimerState s_state = TIMER_STATE_STOPPED;

static uint32_t s_durationSec = 300;     // Default 5 minutes (300s)
static uint32_t s_remainingSec = 300;
static uint32_t s_startMs = 0;
static uint32_t s_accumulatedMs = 0;

static uint32_t s_lastBeepMs = 0;
static uint8_t s_beepStep = 0;
static uint32_t s_expiredSinceMs = 0;

void Timer_Init() {
    s_mode = TIMER_MODE_COUNTDOWN;
    s_state = TIMER_STATE_STOPPED;
    s_durationSec = 300;
    s_remainingSec = 300;
    s_accumulatedMs = 0;
    s_startMs = 0;
}

void Timer_Start() {
    if (s_state == TIMER_STATE_RUNNING) return;

    if (s_state == TIMER_STATE_EXPIRED || s_state == TIMER_STATE_STOPPED) {
        if (s_mode == TIMER_MODE_COUNTDOWN && s_remainingSec == 0) {
            s_remainingSec = (s_durationSec > 0) ? s_durationSec : 300;
        }
        s_accumulatedMs = 0;
    }

    s_startMs = millis();
    s_state = TIMER_STATE_RUNNING;
}

void Timer_Pause() {
    if (s_state != TIMER_STATE_RUNNING) return;

    s_accumulatedMs += (millis() - s_startMs);
    s_state = TIMER_STATE_PAUSED;
}

void Timer_Toggle() {
    if (s_state == TIMER_STATE_RUNNING) {
        Timer_Pause();
    } else if (s_state == TIMER_STATE_PAUSED || s_state == TIMER_STATE_STOPPED) {
        Timer_Start();
    } else if (s_state == TIMER_STATE_EXPIRED) {
        Timer_Reset();
    }
}

void Timer_Reset() {
    Buzzer_Stop();
    s_state = TIMER_STATE_STOPPED;
    s_accumulatedMs = 0;
    s_startMs = 0;
    s_remainingSec = s_durationSec;
    s_beepStep = 0;
}

void Timer_SetMode(TimerMode mode) {
    if (s_mode == mode) return;
    s_mode = mode;
    Timer_Reset();
}

TimerMode Timer_GetMode() {
    return s_mode;
}

TimerState Timer_GetState() {
    return s_state;
}

void Timer_SetDuration(uint32_t seconds) {
    if (seconds == 0) seconds = 60;
    if (seconds > 5999) seconds = 5999; // Cap at 99:59
    s_durationSec = seconds;
    if (s_state == TIMER_STATE_STOPPED) {
        s_remainingSec = s_durationSec;
    }
}

uint32_t Timer_GetDuration() {
    return s_durationSec;
}

uint32_t Timer_GetRemainingSeconds() {
    return s_remainingSec;
}

uint32_t Timer_GetElapsedMillis() {
    if (s_state == TIMER_STATE_RUNNING) {
        return s_accumulatedMs + (millis() - s_startMs);
    }
    return s_accumulatedMs;
}

void Timer_AdjustDuration(int32_t deltaSeconds) {
    int32_t next = (int32_t)s_durationSec + deltaSeconds;
    if (next < 60) next = 60;
    if (next > 5999) next = 5999;
    Timer_SetDuration((uint32_t)next);
    if (s_state == TIMER_STATE_STOPPED) {
        s_remainingSec = s_durationSec;
    }
}

void Timer_Update() {
    uint32_t now = millis();

    if (s_state == TIMER_STATE_RUNNING) {
        uint32_t totalElapsedMs = s_accumulatedMs + (now - s_startMs);

        if (s_mode == TIMER_MODE_COUNTDOWN) {
            uint32_t elapsedSec = totalElapsedMs / 1000;
            if (elapsedSec >= s_durationSec) {
                s_remainingSec = 0;
                s_state = TIMER_STATE_EXPIRED;
                s_expiredSinceMs = now;
                s_lastBeepMs = now;
                s_beepStep = 0;
                Buzzer_Beep(2400, 120);
            } else {
                s_remainingSec = s_durationSec - elapsedSec;
            }
        }
    } else if (s_state == TIMER_STATE_EXPIRED) {
        // Sound pulsing buzzer alarm for up to 10 seconds
        if (now - s_expiredSinceMs < 10000) {
            if (now - s_lastBeepMs >= 300) {
                s_lastBeepMs = now;
                s_beepStep++;
                if ((s_beepStep % 3) != 0) {
                    Buzzer_Beep(2400, 80);
                }
            }
        } else {
            // Auto-stop alarm after 10 seconds
            s_state = TIMER_STATE_STOPPED;
            s_remainingSec = s_durationSec;
        }
    }
}

void Timer_Render() {
    uint32_t now = millis();
    DisplayMatrix_Clear();

    bool isExpired = (s_state == TIMER_STATE_EXPIRED);
    bool flashState = ((now / 250) % 2) == 0;

    // 1. Draw Left Icon (8x8)
    if (isExpired) {
        if (flashState) {
            DisplayMatrix_DrawIcon(0, 0, ICON_BELL);
        } else {
            DisplayMatrix_DrawIcon(0, 0, ICON_TIMER);
        }
    } else if (s_mode == TIMER_MODE_STOPWATCH) {
        DisplayMatrix_DrawIcon(0, 0, ICON_STOPWATCH);
    } else {
        DisplayMatrix_DrawIcon(0, 0, ICON_TIMER);
    }

    // 2. Compute Digits
    int m = 0;
    int s = 0;
    int tenths = 0;

    if (s_mode == TIMER_MODE_STOPWATCH) {
        uint32_t ms = Timer_GetElapsedMillis();
        m = (ms / 60000) % 100;
        s = (ms / 1000) % 60;
        tenths = (ms % 1000) / 100;
    } else {
        m = (s_remainingSec / 60) % 100;
        s = s_remainingSec % 60;
    }

    // 3. Draw 3x5 Time Digits
    uint16_t colTime = isExpired ? (flashState ? C_RED : C_WHITE) :
                       (s_state == TIMER_STATE_RUNNING ? C_WHITE :
                       (s_state == TIMER_STATE_PAUSED ? C_YELLOW : C_GRAY));
    uint16_t colColon = (s_state == TIMER_STATE_RUNNING) ? (flashState ? C_WHITE : 0x4208) : colTime;

    DisplayMatrix_DrawDigit3x5(9, 1, m / 10, colTime);
    DisplayMatrix_DrawDigit3x5(13, 1, m % 10, colTime);

    // Colon
    DisplayMatrix_GetMatrix().drawPixel(17, 2, colColon);
    DisplayMatrix_GetMatrix().drawPixel(17, 4, colColon);

    DisplayMatrix_DrawDigit3x5(19, 1, s / 10, colTime);
    DisplayMatrix_DrawDigit3x5(23, 1, s % 10, colTime);

    // 4. Right side indicator
    if (s_mode == TIMER_MODE_STOPWATCH) {
        // Draw tenths of second '.T'
        DisplayMatrix_GetMatrix().drawPixel(27, 5, C_GRAY);
        DisplayMatrix_DrawDigit3x5(29, 1, tenths, s_state == TIMER_STATE_RUNNING ? C_CYAN : C_GRAY);
    } else {
        // State badge (Play / Pause / Stop)
        if (s_state == TIMER_STATE_RUNNING) {
            // Play green triangle
            DisplayMatrix_GetMatrix().drawPixel(28, 1, C_GREEN);
            DisplayMatrix_GetMatrix().drawPixel(28, 2, C_GREEN);
            DisplayMatrix_GetMatrix().drawPixel(28, 3, C_GREEN);
            DisplayMatrix_GetMatrix().drawPixel(28, 4, C_GREEN);
            DisplayMatrix_GetMatrix().drawPixel(28, 5, C_GREEN);
            DisplayMatrix_GetMatrix().drawPixel(29, 2, C_GREEN);
            DisplayMatrix_GetMatrix().drawPixel(29, 3, C_GREEN);
            DisplayMatrix_GetMatrix().drawPixel(29, 4, C_GREEN);
            DisplayMatrix_GetMatrix().drawPixel(30, 3, C_GREEN);
        } else if (s_state == TIMER_STATE_PAUSED) {
            // Pause yellow double bars
            DisplayMatrix_GetMatrix().drawFastVLine(28, 1, 5, C_YELLOW);
            DisplayMatrix_GetMatrix().drawFastVLine(30, 1, 5, C_YELLOW);
        } else if (s_state == TIMER_STATE_STOPPED) {
            // Stopped small gray box
            DisplayMatrix_GetMatrix().fillRect(28, 2, 3, 3, C_GRAY);
        }

        // 5. Bottom Progress Bar (Row 7, x=9..31 -> 23 pixels max)
        if (s_durationSec > 0) {
            int progressW = (int)((s_remainingSec * 23) / s_durationSec);
            if (progressW > 23) progressW = 23;
            uint16_t colBar = (s_remainingSec <= 10) ? C_RED :
                              (s_remainingSec <= 60 ? C_YELLOW : C_CYAN);
            for (int px = 0; px < progressW; px++) {
                DisplayMatrix_GetMatrix().drawPixel(9 + px, 7, colBar);
            }
        }
    }

    DisplayMatrix_Show();
}
