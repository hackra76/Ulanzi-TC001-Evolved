#pragma once
#include <Arduino.h>

void YouTube_Init();
void YouTube_Update();
void YouTube_ForceFetch();

uint64_t YouTube_GetSubscriberCount();
bool YouTube_HasData();
const char* YouTube_GetFormattedSubs();
void YouTube_SetCredentials(const String& channel, const String& apiKey);
