#pragma once
#include <Arduino.h>
#include <WebServer.h>

enum OtaStatus {
    OTA_STATUS_IDLE = 0,
    OTA_STATUS_UPLOADING,
    OTA_STATUS_FLASHING,
    OTA_STATUS_SUCCESS,
    OTA_STATUS_ERROR
};

void OTA_Init(WebServer& server);
void OTA_Poll();

bool OTA_IsBusy();
uint8_t OTA_GetProgress();
OtaStatus OTA_GetStatus();
const char* OTA_GetStatusString();

bool OTA_CheckGitHubRelease(String& latestTag, String& downloadUrl);
bool OTA_StartGitHubUpdate(const String& downloadUrl);
