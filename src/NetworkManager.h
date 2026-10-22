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
void Network_SaveProfile(uint8_t slot, const String& ssid, const String& pass);
void Network_DeleteProfile(uint8_t slot);
String Network_GetProfileSSID(uint8_t slot);
bool Network_HasProfilePass(uint8_t slot);
void Network_ForgetCredentials();

// Settings & Preferences
bool Network_GetAutoRotate();
void Network_SetAutoRotate(bool en);
uint8_t Network_GetActiveApp();
void Network_SetActiveApp(uint8_t app);
uint16_t Network_GetRotateDelaySec();
void Network_SetRotateDelaySec(uint16_t sec);
uint16_t Network_GetCarouselMask();
void Network_SetCarouselMask(uint16_t mask);
bool Network_IsAppInCarousel(uint8_t appIndex);

uint8_t Network_GetTransitionFx();
void Network_SetTransitionFx(uint8_t fx);

bool Network_GetShowSeconds();
void Network_SetShowSeconds(bool en);

const uint8_t* Network_GetCarouselOrder();
String Network_GetCarouselOrderString();
void Network_SetCarouselOrderString(const String& str);

uint8_t Network_GetBatteryMode();
void Network_SetBatteryMode(uint8_t mode);


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

// Clock, Date & Layout Customization (Awtrix Style)
uint8_t Network_GetClockMode();
void Network_SetClockMode(uint8_t mode);

bool Network_GetWeekdayBar();
void Network_SetWeekdayBar(bool en);

bool Network_GetWeekdayStartMonday();
void Network_SetWeekdayStartMonday(bool startMonday);

uint8_t Network_GetWeekdayActiveColorIndex();
void Network_SetWeekdayActiveColorIndex(uint8_t idx);
uint16_t Network_GetWeekdayActiveColor();

uint8_t Network_GetWeekdayInactiveColorIndex();
void Network_SetWeekdayInactiveColorIndex(uint8_t idx);
uint16_t Network_GetWeekdayInactiveColor();

uint8_t Network_GetWeekdayWeekendColorIndex();
void Network_SetWeekdayWeekendColorIndex(uint8_t idx);
uint16_t Network_GetWeekdayWeekendColor();
uint16_t Network_GetWeekdayInactiveWeekendColor();

String Network_GetPortalLanguage();
void Network_SetPortalLanguage(const String& lang);

uint8_t Network_GetDateFormat();
void Network_SetDateFormat(uint8_t fmt);

uint8_t Network_GetClockIcon();
void Network_SetClockIcon(uint8_t icon);

uint8_t Network_GetTextAlign();
void Network_SetTextAlign(uint8_t align);

// Home Assistant & MQTT Integration
bool Network_GetMqttEnabled();
void Network_SetMqttEnabled(bool en);
String Network_GetMqttServer();
void Network_SetMqttServer(const String& srv);
uint16_t Network_GetMqttPort();
void Network_SetMqttPort(uint16_t port);
String Network_GetMqttUser();
void Network_SetMqttUser(const String& user);
String Network_GetMqttPass();
void Network_SetMqttPass(const String& pass);
String Network_GetMqttPrefix();
void Network_SetMqttPrefix(const String& prefix);
