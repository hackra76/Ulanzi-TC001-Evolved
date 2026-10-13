#pragma once
#include <Arduino.h>

enum ButtonId {
    BTN_ID_LEFT = 0,
    BTN_ID_OK   = 1,
    BTN_ID_RIGHT= 2,
    BTN_COUNT   = 3
};

enum ButtonEvent {
    BTN_NONE = 0,
    BTN_SHORT_CLICK,
    BTN_LONG_PRESS
};

void Buttons_Init();
void Buttons_Poll();
ButtonEvent Buttons_GetEvent(ButtonId id);
