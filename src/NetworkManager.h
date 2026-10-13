#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include "Config.h"

enum NetworkState {
    NET_STATE_IDLE = 0,
    NET_STATE_CONNECTING,
    NET_STATE_CONNECTED,
    NET_STATE_AP_PORTAL
};

void Network_Init();
void Network_Poll();

NetworkState Network_GetState();
bool Network_IsConnected();
bool Network_IsAPActive();

String Network_GetSSID();
String Network_GetLocalIP();
String Network_GetAPIP();
String Network_GetAPSSID();
int8_t Network_GetRSSI();

bool Network_HasSavedCredentials();
void Network_StartAP();
void Network_ConnectStation();
void Network_SaveCredentials(const String& ssid, const String& pass);
void Network_ForgetCredentials();

// Settings & Preferences
uint16_t Network_GetRotateDelaySec();
void Network_SetRotateDelaySec(uint16_t sec);

String Network_GetYouTubeChannel();
String Network_GetYouTubeApiKey();
void Network_SaveYouTubeConfig(const String& channel, const String& apiKey);

// Ambient & Screen Management
bool Network_GetAutoBrightness();
void Network_SetAutoBrightness(bool en);
bool Network_GetNightMode();
void Network_SetNightMode(bool en);
uint8_t Network_GetNightStartHour();
uint8_t Network_GetNightEndHour();
void Network_SetNightSchedule(uint8_t startHour, uint8_t endHour);

uint8_t Network_GetDayBrightness();
void Network_SetDayBrightness(uint8_t bri);
uint8_t Network_GetNightBrightness();
void Network_SetNightBrightness(uint8_t bri);


// Outdoor Weather
float Network_GetWeatherLat();
float Network_GetWeatherLon();
String Network_GetWeatherCity();
void Network_SetWeatherLocation(float lat, float lon, const String& city);

// Crypto Ticker
String Network_GetCryptoSymbol();
void Network_SetCryptoSymbol(const String& sym);
