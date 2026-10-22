#include "CryptoService.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Config.h"
#include "Icons.h"
#include "NetworkManager.h"

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
    } else if (price >= 100.0f) {
        snprintf(out, maxLen, "%.1f", price);
    } else if (price >= 10.0f) {
        snprintf(out, maxLen, "%.2f", price);
    } else {
        snprintf(out, maxLen, "%.3f", price);
    }
}

void Crypto_Init() {
    formatPrice(s_price, s_formattedPrice, sizeof(s_formattedPrice));
}

void Crypto_SetSymbol(const String& symbol) {
    if (symbol.length() > 0) {
        String upper = symbol;
        upper.toUpperCase();
        upper.trim();
        if (upper != s_symbol) {
            s_symbol = upper;
            snprintf(s_formattedPrice, sizeof(s_formattedPrice), "...");
            s_forceFetch = true;
        }
    }
}

void Crypto_CycleAsset() {
    static const char* const kAssets[] = {
        "BTC", "ETH", "SOL", "BNB", "AAPL", "TSLA", "NVDA", "SPX", "GOLD", "EUR", "USD"
    };
    static const size_t kAssetCount = sizeof(kAssets) / sizeof(kAssets[0]);

    int curIdx = -1;
    for (size_t i = 0; i < kAssetCount; i++) {
        if (s_symbol.equalsIgnoreCase(kAssets[i])) {
            curIdx = (int)i;
            break;
        }
    }
    int nextIdx = (curIdx + 1) % kAssetCount;
    Network_SetCryptoSymbol(kAssets[nextIdx]);
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

static bool isStockOrForex(const String& sym) {
    if (sym == "AAPL" || sym == "GOLD" || sym == "GC=F" || sym == "EUR" || 
        sym == "EURUSD" || sym == "EURUSD=X" || sym == "USD" || sym == "USDEUR" || 
        sym == "USDEUR=X" || sym == "SPX" || sym == "^GSPC" || sym == "TSLA" || 
        sym == "NVDA" || sym.indexOf('^') >= 0 || sym.indexOf('=') >= 0) {
        return true;
    }
    return false;
}

static String getYahooTicker(const String& sym) {
    if (sym == "GOLD") return "GC=F";
    if (sym == "EUR" || sym == "EURUSD") return "EURUSD=X";
    if (sym == "USD" || sym == "USDEUR") return "USDEUR=X";
    if (sym == "SPX") return "^GSPC";
    return sym;
}

const uint16_t* Crypto_GetIcon() {
    if (s_symbol == "BTC") return ICON_BTC;
    if (s_symbol == "ETH") return ICON_ETH;
    if (s_symbol == "SOL") return ICON_SOL;
    if (s_symbol == "BNB") return ICON_BNB;
    if (s_symbol == "AAPL") return ICON_APPLE;
    if (s_symbol == "TSLA") return ICON_TESLA;
    if (s_symbol == "NVDA") return ICON_NVIDIA;
    if (s_symbol == "SPX" || s_symbol == "^GSPC") return ICON_SPX;
    if (s_symbol == "GOLD" || s_symbol == "GC=F") return ICON_GOLD;
    if (s_symbol == "EUR" || s_symbol == "EURUSD" || s_symbol == "EURUSD=X") return ICON_EUR;
    if (s_symbol == "USD" || s_symbol == "USDEUR" || s_symbol == "USDEUR=X") return ICON_USD;
    return ICON_CHART;
}

uint16_t Crypto_GetColor() {
    if (s_symbol == "BTC") return C_ORANGE;
    if (s_symbol == "ETH") return C_CYAN;
    if (s_symbol == "SOL") return C_MAGENTA;
    if (s_symbol == "BNB") return C_YELLOW;
    if (s_symbol == "AAPL") return C_WHITE;
    if (s_symbol == "TSLA") return C_RED;
    if (s_symbol == "NVDA") return C_GREEN;
    if (s_symbol == "SPX" || s_symbol == "^GSPC") return C_CYAN;
    if (s_symbol == "GOLD" || s_symbol == "GC=F") return C_YELLOW;
    if (s_symbol == "EUR" || s_symbol == "EURUSD" || s_symbol == "EURUSD=X") return C_CYAN;
    if (s_symbol == "USD" || s_symbol == "USDEUR" || s_symbol == "USDEUR=X") return C_GREEN;
    return C_GREEN;
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

    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(6000);

    HTTPClient https;

    if (isStockOrForex(s_symbol)) {
        String ticker = getYahooTicker(s_symbol);
        String url = "https://query1.finance.yahoo.com/v8/finance/chart/" + ticker + "?interval=1d&range=1d";
        https.setUserAgent("Mozilla/5.0 (Windows NT 10.0; Win64; x64)");
        if (https.begin(client, url)) {
            int httpCode = https.GET();
            if (httpCode == HTTP_CODE_OK) {
                JsonDocument filter;
                filter["chart"]["result"][0]["meta"]["regularMarketPrice"] = true;
                JsonDocument doc;
                DeserializationError error = deserializeJson(doc, https.getStream(), DeserializationOption::Filter(filter));
                if (!error) {
                    float price = doc["chart"]["result"][0]["meta"]["regularMarketPrice"] | 0.0f;
                    if (price > 0.0f) {
                        s_price = price;
                        formatPrice(s_price, s_formattedPrice, sizeof(s_formattedPrice));
                        Serial.printf("[Markets] Success! %s: %s (%.4f)\n", s_symbol.c_str(), s_formattedPrice, s_price);
                    }
                } else {
                    Serial.printf("[Markets] JSON error for %s: %s\n", s_symbol.c_str(), error.c_str());
                }
            } else {
                Serial.printf("[Markets] HTTP error %d for %s\n", httpCode, s_symbol.c_str());
            }
            https.end();
        }
    } else {
        String pair = s_symbol;
        if (!pair.endsWith("USDT") && !pair.endsWith("USD") && !pair.endsWith("BUSD")) {
            pair += "USDT";
        }
        String url = "https://api.binance.com/api/v3/ticker/price?symbol=" + pair;
        if (https.begin(client, url)) {
            int httpCode = https.GET();
            if (httpCode == HTTP_CODE_OK) {
                JsonDocument doc;
                DeserializationError error = deserializeJson(doc, https.getStream());
                if (!error) {
                    const char* priceStr = doc["price"] | "";
                    if (strlen(priceStr) > 0) {
                        s_price = atof(priceStr);
                        formatPrice(s_price, s_formattedPrice, sizeof(s_formattedPrice));
                        Serial.printf("[Crypto] Success! %s: %s ($%.2f)\n", pair.c_str(), s_formattedPrice, s_price);
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
}
