#pragma once
#include <Arduino.h>

void Weather_Init();
void Weather_Update();
void Weather_ForceFetch();

bool Weather_HasData();
float Weather_GetTemperature();
int Weather_GetHumidity();
int Weather_GetWeatherCode();
const char* Weather_GetCity();
const uint16_t* Weather_GetConditionIcon();
const char* Weather_GetFormattedTemp();

void Weather_SetConfig(float lat, float lon, const String& city);
