#include "Buttons.h"
#include "Config.h"

static const uint8_t s_pins[BTN_COUNT] = { PIN_BTN_LEFT, PIN_BTN_OK, PIN_BTN_RIGHT };
static bool s_lastState[BTN_COUNT] = { HIGH, HIGH, HIGH };
static uint32_t s_pressTime[BTN_COUNT] = { 0, 0, 0 };
static ButtonEvent s_events[BTN_COUNT] = { BTN_NONE, BTN_NONE, BTN_NONE };

void Buttons_Init() {
    for (int i = 0; i < BTN_COUNT; i++) {
        pinMode(s_pins[i], INPUT_PULLUP);
        s_lastState[i] = digitalRead(s_pins[i]);
    }
}

void Buttons_Poll() {
    uint32_t now = millis();
    for (int i = 0; i < BTN_COUNT; i++) {
        bool raw = digitalRead(s_pins[i]);
        if (raw == LOW && s_lastState[i] == HIGH) {
            // Button pressed down
            s_pressTime[i] = now;
        } else if (raw == HIGH && s_lastState[i] == LOW) {
            // Button released
            uint32_t duration = now - s_pressTime[i];
            if (duration >= BTN_DEBOUNCE_MS && duration < BTN_LONG_PRESS_MS) {
                s_events[i] = BTN_SHORT_CLICK;
            }
        } else if (raw == LOW && s_lastState[i] == LOW) {
            // Holding button
            if (s_pressTime[i] > 0 && (now - s_pressTime[i]) >= BTN_LONG_PRESS_MS) {
                s_events[i] = BTN_LONG_PRESS;
                s_pressTime[i] = 0; // prevent repeat until released
            }
        }
        s_lastState[i] = raw;
    }
}

ButtonEvent Buttons_GetEvent(ButtonId id) {
    if (id >= BTN_COUNT) return BTN_NONE;
    ButtonEvent ev = s_events[id];
    s_events[id] = BTN_NONE; // consume event
    return ev;
}
