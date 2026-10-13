#include "WeatherService.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Config.h"
#include "Icons.h"

static float s_lat = DEFAULT_WEATHER_LAT;
static float s_lon = DEFAULT_WEATHER_LON;
static String s_city = DEFAULT_WEATHER_CITY;

static float s_temperature = 19.5f;
static int s_humidity = 55;
static int s_weatherCode = 0;
static bool s_hasData = true;
static char s_formattedTemp[12] = "19C";

static uint32_t s_lastFetchMs = 0;
static bool s_forceFetch = false;

void Weather_Init() {
    snprintf(s_formattedTemp, sizeof(s_formattedTemp), "%dC", (int)roundf(s_temperature));
}

void Weather_SetConfig(float lat, float lon, const String& city) {
    s_lat = lat;
    s_lon = lon;
    if (city.length() > 0) s_city = city;
    s_forceFetch = true;
}

void Weather_ForceFetch() {
    s_forceFetch = true;
}

bool Weather_HasData() {
    return s_hasData;
}

float Weather_GetTemperature() {
    return s_temperature;
}

int Weather_GetHumidity() {
    return s_humidity;
}

int Weather_GetWeatherCode() {
    return s_weatherCode;
}

const char* Weather_GetCity() {
    return s_city.c_str();
}

const char* Weather_GetFormattedTemp() {
    return s_formattedTemp;
}

const uint16_t* Weather_GetConditionIcon() {
    // WMO Weather interpretation codes (WW)
    if (s_weatherCode == 0 || s_weatherCode == 1) {
        return ICON_SUN; // Clear sky, Mainly clear
    } else if (s_weatherCode == 2 || s_weatherCode == 3 || s_weatherCode == 45 || s_weatherCode == 48) {
        return ICON_CLOUD; // Partly cloudy, Overcast, Fog
    } else if ((s_weatherCode >= 51 && s_weatherCode <= 67) || (s_weatherCode >= 80 && s_weatherCode <= 82)) {
        return ICON_RAIN; // Drizzle, Rain, Rain showers
    } else if ((s_weatherCode >= 71 && s_weatherCode <= 77) || s_weatherCode == 85 || s_weatherCode == 86) {
        return ICON_SNOW; // Snow fall, Snow grains, Snow showers
    } else if (s_weatherCode >= 95 && s_weatherCode <= 99) {
        return ICON_STORM; // Thunderstorm
    }
    return ICON_SUN;
}

void Weather_Update() {
    uint32_t now = millis();

    if (WiFi.status() != WL_CONNECTED) {
        return;
    }

    if (!s_forceFetch && (now - s_lastFetchMs < WEATHER_FETCH_INTERVAL_MS)) {
        return;
    }

    s_forceFetch = false;
    s_lastFetchMs = now;

    String url = "https://api.open-meteo.com/v1/forecast?latitude=" +
                 String(s_lat, 4) + "&longitude=" + String(s_lon, 4) +
                 "&current=temperature_2m,relative_humidity_2m,weather_code";

    Serial.printf("[Weather] Querying Open-Meteo for %s (%.4f, %.4f)...\n",
                  s_city.c_str(), s_lat, s_lon);

    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(6000);

    HTTPClient https;
    if (https.begin(client, url)) {
        int httpCode = https.GET();
        if (httpCode == HTTP_CODE_OK) {
            String payload = https.getString();
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, payload);
            if (!error) {
                JsonObject current = doc["current"];
                s_temperature = current["temperature_2m"] | s_temperature;
                s_humidity = current["relative_humidity_2m"] | s_humidity;
                s_weatherCode = current["weather_code"] | s_weatherCode;
                s_hasData = true;

                snprintf(s_formattedTemp, sizeof(s_formattedTemp), "%dC", (int)roundf(s_temperature));
                Serial.printf("[Weather] Success! %s: %s, %d%% RH, WMO code %d\n",
                              s_city.c_str(), s_formattedTemp, s_humidity, s_weatherCode);
            } else {
                Serial.printf("[Weather] JSON parse error: %s\n", error.c_str());
            }
        } else {
            Serial.printf("[Weather] HTTP error code %d\n", httpCode);
        }
        https.end();
    }
}
