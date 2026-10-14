#pragma once
#include <Arduino.h>

void MqttService_Init();
void MqttService_Update();
bool MqttService_IsConnected();

// Configuration
bool MqttService_IsEnabled();
void MqttService_SetEnabled(bool en);
String MqttService_GetServer();
void MqttService_SetServer(const String& srv);
uint16_t MqttService_GetPort();
void MqttService_SetPort(uint16_t port);
String MqttService_GetUser();
void MqttService_SetUser(const String& user);
String MqttService_GetPass();
void MqttService_SetPass(const String& pass);
String MqttService_GetPrefix();
void MqttService_SetPrefix(const String& prefix);

// Immediate telemetry / state push
void MqttService_PublishState();
void MqttService_PublishTelemetry();
