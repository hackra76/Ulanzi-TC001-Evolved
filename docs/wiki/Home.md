# Welcome to Ulanzi TC001 Evolved Wiki

**Ulanzi TC001 Evolved** is a modular, high-performance firmware for the Ulanzi TC001 Smart Pixel Desktop Clock. It replaces legacy firmwares with a resilient, modern ESP32 architecture.

---

## 📚 Table of Contents

- [Quick Setup & Wi-Fi Provisioning](Setup-Guide.md)
- [Web Configuration Portal](Web-Portal.md)
- [Dual OTA Update System](OTA-Updates.md)
- [Hardware Architecture & Pinout](Hardware-Pinout.md)
- [REST API & Captive Endpoints](API-Endpoints.md)

---

## ⚡ Core Highlights

1. **Zero-Configuration SoftAP Captive Portal**: Built-in DNS server automatically redirects mobile and desktop clients to the dark-mode configuration portal.
2. **Ambient Intelligence**: LDR sensor readings are smoothed via exponential moving average (EMA) and passed through a gamma curve for flicker-free auto-brightness.
3. **Custom Day/Night Sliders**: Set your preferred daytime maximum brightness and sleep-safe night brightness.
4. **Worldwide City Presets**: Pre-populated coordinates for 25+ cities across Slovakia, Czechia, Europe, and globally.
5. **Real-time Financial & Creator Tickers**: Live crypto prices from Binance and live subscriber counts from YouTube Data API v3.
6. **Dual OTA Engine**: Browser drag-and-drop `.bin` upload plus automated 1-click cloud updates from GitHub Releases.
