#pragma once
#include <Arduino.h>

void Buzzer_Init();
void Buzzer_Poll();
void Buzzer_Beep(uint16_t freqHz, uint16_t durationMs);
void Buzzer_Click();
void Buzzer_Alert();
void Buzzer_Success();
void Buzzer_Notification();
void Buzzer_Alarm();
void Buzzer_Stop();
void Buzzer_PlayRTTTL(const char* rtttl);
