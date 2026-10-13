#include "YouTubeService.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Config.h"

static String s_channel = "";
static String s_apiKey = "";
static uint64_t s_subs = 125000; // Demo fallback (125K)
static bool s_hasData = true;
static char s_formattedSubs[16] = "125K";
static uint32_t s_lastFetchMs = 0;
static bool s_forceFetch = false;

static void formatSubscriberCount(uint64_t subs, char* out, size_t maxLen) {
    if (subs >= 1000000000ULL) {
        snprintf(out, maxLen, "%.1fB", subs / 1000000000.0);
    } else if (subs >= 10000000ULL) {
        snprintf(out, maxLen, "%lluM", (unsigned long long)(subs / 1000000ULL));
    } else if (subs >= 1000000ULL) {
        snprintf(out, maxLen, "%.1fM", subs / 1000000.0);
    } else if (subs >= 10000ULL) {
        snprintf(out, maxLen, "%lluK", (unsigned long long)(subs / 1000ULL));
    } else if (subs >= 1000ULL) {
        snprintf(out, maxLen, "%.1fK", subs / 1000.0);
    } else {
        snprintf(out, maxLen, "%llu", (unsigned long long)subs);
    }
}

void YouTube_Init() {
    formatSubscriberCount(s_subs, s_formattedSubs, sizeof(s_formattedSubs));
}

void YouTube_SetCredentials(const String& channel, const String& apiKey) {
    s_channel = channel;
    s_channel.trim();
    s_apiKey = apiKey;
    s_apiKey.trim();
    s_forceFetch = true;
}

void YouTube_ForceFetch() {
    s_forceFetch = true;
}

uint64_t YouTube_GetSubscriberCount() {
    return s_subs;
}

bool YouTube_HasData() {
    return s_hasData;
}

const char* YouTube_GetFormattedSubs() {
    return s_formattedSubs;
}

void YouTube_Update() {
    uint32_t now = millis();

    // Check if Wi-Fi is connected
    if (WiFi.status() != WL_CONNECTED) {
        return;
    }

    // Rate limiting: fetch every YT_FETCH_INTERVAL_MS unless force fetch requested
    if (!s_forceFetch && (now - s_lastFetchMs < YT_FETCH_INTERVAL_MS)) {
        return;
    }

    if (s_apiKey.length() == 0 || s_channel.length() == 0) {
        // No credentials configured, keep demo fallback
        return;
    }

    s_forceFetch = false;
    s_lastFetchMs = now;

    // Build URL according to channel ID format or handle
    String channelParam = "";
    if (s_channel.startsWith("@")) {
        channelParam = "forHandle=" + s_channel;
    } else if (s_channel.startsWith("UC") && s_channel.length() == 24) {
        channelParam = "id=" + s_channel;
    } else {
        // If user typed handle without leading @
        channelParam = "forHandle=@" + s_channel;
    }

    String url = "https://www.googleapis.com/youtube/v3/channels?part=statistics&" +
                 channelParam + "&key=" + s_apiKey;

    Serial.printf("[YouTube] Fetching subscriber count for '%s'...\n", s_channel.c_str());

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
                JsonArray items = doc["items"];
                if (items.size() > 0) {
                    const char* subsStr = items[0]["statistics"]["subscriberCount"] | "";
                    if (strlen(subsStr) > 0) {
                        s_subs = strtoull(subsStr, nullptr, 10);
                        s_hasData = true;
                        formatSubscriberCount(s_subs, s_formattedSubs, sizeof(s_formattedSubs));
                        Serial.printf("[YouTube] Success! Subscribers: %llu (%s)\n",
                                      (unsigned long long)s_subs, s_formattedSubs);
                    }
                } else {
                    Serial.println("[YouTube] API response has 0 items (channel not found)");
                }
            } else {
                Serial.printf("[YouTube] JSON parsing error: %s\n", error.c_str());
            }
        } else {
            Serial.printf("[YouTube] HTTP GET failed, code: %d\n", httpCode);
        }
        https.end();
    }
}
