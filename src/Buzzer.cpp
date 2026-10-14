#include "Buzzer.h"
#include "Config.h"

void Buzzer_Init() {
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);
}

void Buzzer_Beep(uint16_t freqHz, uint16_t durationMs) {
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcAttach(PIN_BUZZER, freqHz, BUZZER_LEDC_RES);
    ledcWriteTone(PIN_BUZZER, freqHz);
    delay(durationMs);
    ledcWriteTone(PIN_BUZZER, 0);
    ledcDetach(PIN_BUZZER);
#else
    ledcSetup(BUZZER_LEDC_CH, freqHz, BUZZER_LEDC_RES);
    ledcAttachPin(PIN_BUZZER, BUZZER_LEDC_CH);
    ledcWriteTone(BUZZER_LEDC_CH, freqHz);
    delay(durationMs);
    ledcWriteTone(BUZZER_LEDC_CH, 0);
    ledcDetachPin(PIN_BUZZER);
#endif
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);
}

void Buzzer_Click() {
    Buzzer_Beep(2400, 20);
}

void Buzzer_Alert() {
    Buzzer_Beep(3200, 80);
    delay(40);
    Buzzer_Beep(3200, 80);
}
