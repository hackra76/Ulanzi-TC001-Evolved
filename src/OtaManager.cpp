#include "OtaManager.h"
#include <WiFi.h>
#include <Update.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Config.h"

static OtaStatus s_status = OTA_STATUS_IDLE;
static uint8_t s_progress = 0;
static String s_errorMsg = "";
static bool s_rebootPending = false;
static uint32_t s_rebootMs = 0;

static void handleUpload(WebServer& server) {
    HTTPUpload& upload = server.upload();

    if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("[OTA] Web Upload Started: %s\n", upload.filename.c_str());
        s_status = OTA_STATUS_UPLOADING;
        s_progress = 0;
        s_errorMsg = "";

        // Stop services / maximize free heap
        if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) {
            Update.printError(Serial);
            s_status = OTA_STATUS_ERROR;
            s_errorMsg = "Update.begin failed";
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (s_status != OTA_STATUS_ERROR) {
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                Update.printError(Serial);
                s_status = OTA_STATUS_ERROR;
                s_errorMsg = "Flash write failed";
            }
            if (upload.totalSize > 0) {
                s_progress = (uint8_t)((upload.currentSize * 100) / upload.totalSize);
            }
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (s_status != OTA_STATUS_ERROR) {
            if (Update.end(true)) {
                Serial.printf("[OTA] Success! Flashed %u bytes.\n", upload.totalSize);
                s_status = OTA_STATUS_SUCCESS;
                s_progress = 100;
                s_rebootPending = true;
                s_rebootMs = millis() + 1200;
            } else {
                Update.printError(Serial);
                s_status = OTA_STATUS_ERROR;
                s_errorMsg = "Update.end verification failed";
            }
        }
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
        Update.end();
        s_status = OTA_STATUS_ERROR;
        s_errorMsg = "Upload aborted";
        Serial.println("[OTA] Upload aborted");
    }
}

void OTA_Init(WebServer& server) {
    // 1. Direct Web Browser File Upload Endpoint
    server.on("/update", HTTP_POST, [&server]() {
        server.sendHeader("Connection", "close");
        if (s_status == OTA_STATUS_SUCCESS) {
            server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Update installed! Rebooting...\"}");
        } else {
            String err = s_errorMsg.length() > 0 ? s_errorMsg : "OTA verification error";
            server.send(500, "application/json", "{\"status\":\"error\",\"message\":\"" + err + "\"}");
        }
    }, [&server]() {
        handleUpload(server);
    });

    // 2. GitHub Releases Check Endpoint
    server.on("/api/ota/check", HTTP_GET, [&server]() {
        String latestTag = "";
        String downloadUrl = "";
        bool hasUpdate = OTA_CheckGitHubRelease(latestTag, downloadUrl);

        JsonDocument doc;
        doc["current_version"] = FW_VERSION;
        doc["latest_version"] = latestTag.length() > 0 ? latestTag : FW_VERSION;
        doc["has_update"] = hasUpdate;
        doc["download_url"] = downloadUrl;

        String out;
        serializeJson(doc, out);
        server.send(200, "application/json", out);
    });

    // 3. GitHub 1-Click Cloud Install Endpoint
    server.on("/api/ota/install", HTTP_POST, [&server]() {
        if (server.hasArg("url")) {
            String url = server.arg("url");
            server.send(200, "application/json", "{\"status\":\"started\",\"message\":\"Downloading update from GitHub...\"}");
            OTA_StartGitHubUpdate(url);
        } else {
            server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing URL argument\"}");
        }
    });
}

void OTA_Poll() {
    if (s_rebootPending && (millis() >= s_rebootMs)) {
        Serial.println("[OTA] Rebooting now into updated firmware...");
        ESP.restart();
    }
}

bool OTA_IsBusy() {
    return (s_status == OTA_STATUS_UPLOADING || s_status == OTA_STATUS_FLASHING);
}

uint8_t OTA_GetProgress() {
    return s_progress;
}

OtaStatus OTA_GetStatus() {
    return s_status;
}

const char* OTA_GetStatusString() {
    switch (s_status) {
        case OTA_STATUS_IDLE:      return "Idle";
        case OTA_STATUS_UPLOADING: return "Uploading...";
        case OTA_STATUS_FLASHING:  return "Flashing...";
        case OTA_STATUS_SUCCESS:   return "Success! Rebooting...";
        case OTA_STATUS_ERROR:     return "Error";
        default:                   return "Unknown";
    }
}

bool OTA_CheckGitHubRelease(String& latestTag, String& downloadUrl) {
    if (WiFi.status() != WL_CONNECTED) return false;

    String url = "https://api.github.com/repos/" + String(GITHUB_REPO) + "/releases/latest";
    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(6000);

    HTTPClient https;
    https.setUserAgent("ESP32-Ulanzi-TC001");
    if (https.begin(client, url)) {
        int code = https.GET();
        if (code == HTTP_CODE_OK) {
            String payload = https.getString();
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, payload);
            if (!err) {
                const char* tag = doc["tag_name"] | "";
                latestTag = String(tag);
                if (latestTag.startsWith("v")) latestTag = latestTag.substring(1);

                // Look for .bin asset
                JsonArray assets = doc["assets"];
                for (JsonObject asset : assets) {
                    const char* name = asset["name"] | "";
                    const char* dl = asset["browser_download_url"] | "";
                    if (String(name).endsWith(".bin")) {
                        downloadUrl = String(dl);
                        break;
                    }
                }

                https.end();
                // Compare versions (e.g. "1.2.0" vs FW_VERSION)
                return (latestTag != FW_VERSION && downloadUrl.length() > 0);
            }
        }
        https.end();
    }
    return false;
}

bool OTA_StartGitHubUpdate(const String& downloadUrl) {
    if (WiFi.status() != WL_CONNECTED || downloadUrl.length() == 0) return false;

    Serial.printf("[OTA] Downloading Cloud Update from: %s\n", downloadUrl.c_str());
    s_status = OTA_STATUS_FLASHING;
    s_progress = 0;

    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(8000);

    HTTPClient https;
    https.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    if (!https.begin(client, downloadUrl)) {
        s_status = OTA_STATUS_ERROR;
        s_errorMsg = "Cannot connect to update asset";
        return false;
    }

    int code = https.GET();
    if (code != HTTP_CODE_OK) {
        https.end();
        s_status = OTA_STATUS_ERROR;
        s_errorMsg = "HTTP error code " + String(code);
        return false;
    }

    int totalSize = https.getSize();
    if (totalSize <= 0) {
        https.end();
        s_status = OTA_STATUS_ERROR;
        s_errorMsg = "Invalid asset size";
        return false;
    }

    if (!Update.begin(totalSize, U_FLASH)) {
        https.end();
        s_status = OTA_STATUS_ERROR;
        s_errorMsg = "Not enough space in OTA partition";
        return false;
    }

    WiFiClient* stream = https.getStreamPtr();
    size_t written = Update.writeStream(*stream);

    if (written == (size_t)totalSize && Update.end(true)) {
        Serial.println("[OTA] Cloud Update Success! Rebooting...");
        s_status = OTA_STATUS_SUCCESS;
        s_progress = 100;
        s_rebootPending = true;
        s_rebootMs = millis() + 1500;
        https.end();
        return true;
    } else {
        Update.printError(Serial);
        s_status = OTA_STATUS_ERROR;
        s_errorMsg = "Stream write failure";
        https.end();
        return false;
    }
}
