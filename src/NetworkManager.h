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
