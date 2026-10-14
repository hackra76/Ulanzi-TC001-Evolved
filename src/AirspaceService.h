#pragma once
#include <Arduino.h>

void Airspace_Init();
void Airspace_Update();
void Airspace_ForceFetch();

const char* Airspace_GetCallsign();
int Airspace_GetFlightCount();
