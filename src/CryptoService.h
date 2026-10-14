#pragma once
#include <Arduino.h>

void Crypto_Init();
void Crypto_Update();
void Crypto_ForceFetch();

const char* Crypto_GetSymbol();
float Crypto_GetPrice();
const char* Crypto_GetFormattedPrice();
const uint16_t* Crypto_GetIcon();
uint16_t Crypto_GetColor();

void Crypto_SetSymbol(const String& symbol);
