#include "AirspaceService.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "NetworkManager.h"

#define AIRSPACE_FETCH_INTERVAL_MS 60000 // 60s
#define MAX_TRACKED_FLIGHTS 6

static char s_flights[MAX_TRACKED_FLIGHTS][10];
static int s_flightCount = 0;
static int s_currentFlightIndex = 0;
static uint32_t s_lastFetchMs = 0;
static uint32_t s_lastCycleMs = 0;
static bool s_forceFetch = false;
static char s_displayCallsign[12] = "RADAR";

void Airspace_Init() {
    snprintf(s_displayCallsign, sizeof(s_displayCallsign), "RADAR");
    s_forceFetch = true;
}

void Airspace_ForceFetch() {
    s_forceFetch = true;
}

const char* Airspace_GetCallsign() {
    return s_displayCallsign;
}

int Airspace_GetFlightCount() {
    return s_flightCount;
}

void Airspace_Update() {
    uint32_t now = millis();

    // Cycle between detected overhead flights every 4 seconds
    if (s_flightCount > 0 && (now - s_lastCycleMs >= 4000)) {
        s_lastCycleMs = now;
        s_currentFlightIndex = (s_currentFlightIndex + 1) % s_flightCount;
        snprintf(s_displayCallsign, sizeof(s_displayCallsign), "%s", s_flights[s_currentFlightIndex]);
    }

    if (WiFi.status() != WL_CONNECTED) {
        return;
    }

    if (!s_forceFetch && (now - s_lastFetchMs < AIRSPACE_FETCH_INTERVAL_MS)) {
        return;
    }

    s_forceFetch = false;
    s_lastFetchMs = now;

    float lat = Network_GetWeatherLat();
    float lon = Network_GetWeatherLon();
    // Bounding box: ~50km radius
    float lamin = lat - 0.5f;
    float lamax = lat + 0.5f;
    float lomin = lon - 0.5f;
    float lomax = lon + 0.5f;

    char url[160];
    snprintf(url, sizeof(url), "https://opensky-network.org/api/states/all?lamin=%.3f&lomin=%.3f&lamax=%.3f&lomax=%.3f",
             lamin, lomin, lamax, lomax);

    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(6000);

    HTTPClient https;
    https.setUserAgent("Mozilla/5.0 (ESP32-Ulanzi-Evolved)");

    if (https.begin(client, url)) {
        int httpCode = https.GET();
        if (httpCode == HTTP_CODE_OK) {
            String payload = https.getString();
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, payload);
            if (!error) {
                JsonArray states = doc["states"].as<JsonArray>();
                int count = 0;
                for (JsonArray state : states) {
                    if (count >= MAX_TRACKED_FLIGHTS) break;
                    const char* cs = state[1] | "";
                    String csStr = String(cs);
                    csStr.trim();
                    if (csStr.length() > 0) {
                        snprintf(s_flights[count], sizeof(s_flights[count]), "%s", csStr.c_str());
                        count++;
                    }
                }
                s_flightCount = count;
                if (s_flightCount > 0) {
                    s_currentFlightIndex = 0;
                    snprintf(s_displayCallsign, sizeof(s_displayCallsign), "%s", s_flights[0]);
                    Serial.printf("[Airspace] Found %d live flights overhead! Active: %s\n", s_flightCount, s_displayCallsign);
                } else {
                    snprintf(s_displayCallsign, sizeof(s_displayCallsign), "CLEAR");
                    Serial.println("[Airspace] No flights overhead (airspace clear)");
                }
            } else {
                Serial.printf("[Airspace] JSON parse error: %s\n", error.c_str());
            }
        } else {
            Serial.printf("[Airspace] HTTP error code %d\n", httpCode);
        }
        https.end();
    }
}
