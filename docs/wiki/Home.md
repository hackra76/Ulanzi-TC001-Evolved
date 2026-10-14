# Welcome to Ulanzi TC001 Evolved Wiki

**Ulanzi TC001 Evolved** is a modular, high-performance firmware for the Ulanzi TC001 Smart Pixel Desktop Clock. It transforms the clock with a modern, resilient ESP32 architecture featuring live display streaming, Awtrix-style layout customization, live overhead airspace radar, stock/crypto markets, and custom carousel reordering.

---

## 📚 Table of Contents

- [Quick Setup & Wi-Fi Provisioning](Setup-Guide.md)
- [Web Configuration Portal](Web-Portal.md)
- [Dual OTA Update System](OTA-Updates.md)
- [Hardware Architecture & Pinout](Hardware-Pinout.md)
- [REST API & Captive Endpoints](API-Endpoints.md)

---

## ⚡ Core Highlights

1. **Zero-Configuration SoftAP Captive Portal**: Built-in DNS server automatically redirects mobile and desktop clients to the dark-mode configuration portal on `192.168.4.1`.
2. **Real-Time Virtual Matrix Stream**: Live 32×8 canvas mirror in the Web Portal, one-click BMP screenshot capture, and terminal ASCII art output.
3. **Awtrix-Style Clock & Layout**: Time/Combined/Date modes, 7 weekday bars with ISO/US start and custom colors, multiple date formats, dynamic calendar icon, and seconds display with auto-hidden icon for clean spacing.
4. **Live Airspace Radar (ADS-B)**: Real-time overhead flight tracking via OpenSky Network cycling active aircraft callsigns within ~50km of local coordinates.
5. **Energy & Battery Telemetry**: Glowing neon lightning bolt icon with selectable Percentage, Voltage, or Alternating display modes.
6. **Financial Markets, Stocks & Crypto**: Live feeds for Bitcoin, Ethereum, Solana, BNB, Apple, Tesla, Nvidia, S&P 500, Gold, EUR/USD, and custom tickers.
7. **Custom Screen Reordering**: Move screens up/down in the web portal to define your exact carousel sequence, with instant AJAX saving and toast notifications.
8. **Home Assistant MQTT Auto-Discovery**: Automatic discovery of 12 synchronized entities (sensors, matrix power/brightness light, switches, buttons, and screen selector).
9. **Custom App Push API & Spotify**: Push dynamic screens via MQTT or HTTP REST with 8×8 icons, progress bars, and high-priority push notifications with buzzer chime.
10. **Dual OTA Engine**: Web browser binary upload plus automated 1-click cloud updates from GitHub Releases with live percentage display on matrix.
