#include "CryptoService.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Config.h"
#include "Icons.h"

static String s_symbol = DEFAULT_CRYPTO_SYM;
static float s_price = 68500.0f; // Default demo BTC price
static char s_formattedPrice[12] = "68.5K";
static uint32_t s_lastFetchMs = 0;
static bool s_forceFetch = false;

static void formatPrice(float price, char* out, size_t maxLen) {
    if (price >= 100000.0f) {
        snprintf(out, maxLen, "%dK", (int)roundf(price / 1000.0f));
    } else if (price >= 10000.0f) {
        snprintf(out, maxLen, "%.1fK", price / 1000.0f);
    } else if (price >= 1000.0f) {
        snprintf(out, maxLen, "%.1fK", price / 1000.0f);
    } else {
        snprintf(out, maxLen, "%.1f", price);
    }
}

void Crypto_Init() {
    formatPrice(s_price, s_formattedPrice, sizeof(s_formattedPrice));
}

void Crypto_SetSymbol(const String& symbol) {
    if (symbol.length() > 0) {
        s_symbol = symbol;
        s_symbol.toUpperCase();
        s_symbol.trim();
        s_forceFetch = true;
    }
}

void Crypto_ForceFetch() {
    s_forceFetch = true;
}

const char* Crypto_GetSymbol() {
    return s_symbol.c_str();
}

float Crypto_GetPrice() {
    return s_price;
}

const char* Crypto_GetFormattedPrice() {
    return s_formattedPrice;
}

const uint16_t* Crypto_GetIcon() {
    if (s_symbol == "ETH") {
        return ICON_ETH;
    }
    return ICON_BTC;
}

void Crypto_Update() {
    uint32_t now = millis();

    if (WiFi.status() != WL_CONNECTED) {
        return;
    }

    if (!s_forceFetch && (now - s_lastFetchMs < CRYPTO_FETCH_INTERVAL_MS)) {
        return;
    }

    s_forceFetch = false;
    s_lastFetchMs = now;

    String pair = s_symbol;
    if (!pair.endsWith("USDT") && !pair.endsWith("USD")) {
        pair += "USDT";
    }

    String url = "https://api.binance.com/api/v3/ticker/price?symbol=" + pair;

    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(5000);

    HTTPClient https;
    if (https.begin(client, url)) {
        int httpCode = https.GET();
        if (httpCode == HTTP_CODE_OK) {
            String payload = https.getString();
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, payload);
            if (!error) {
                const char* priceStr = doc["price"] | "";
                if (strlen(priceStr) > 0) {
                    s_price = atof(priceStr);
                    formatPrice(s_price, s_formattedPrice, sizeof(s_formattedPrice));
                    Serial.printf("[Crypto] Success! %s: %s ($%.2f)\n",
                                  pair.c_str(), s_formattedPrice, s_price);
                }
            } else {
                Serial.printf("[Crypto] JSON parse error: %s\n", error.c_str());
            }
        } else {
            Serial.printf("[Crypto] HTTP error code %d\n", httpCode);
        }
        https.end();
    }
}
