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

## 🖥️ Screen Gallery

The Ulanzi TC001 8×32 WS2812B matrix renders unified 3×5 typography and vibrant 8×8 icons across all screens.

![Ulanzi TC001 Screen Showcase](../media/screens_showcase.png)

| Screen & App | Matrix Live Preview | Summary |
|---|:---:|---|
| **Screen 0: Clock & Calendar** | <img src="../media/screen_0_clock.png" width="320" alt="Clock"> | Dynamic calendar card, 3×5 time, and 7 weekday bars. |
| **Screen 1: Indoor Environment** | <img src="../media/screen_1_indoor.png" width="320" alt="Indoor"> | Sensirion SHT30 precision temperature & humidity telemetry. |
| **Screen 2: Outdoor Weather** | <img src="../media/screen_2_outdoor.png" width="320" alt="Weather"> | Live Open-Meteo forecasts with WMO condition icons. |
| **Screen 3: Financial Markets** | <img src="../media/screen_3_crypto.png" width="320" alt="Markets"> | Bitcoin, Ethereum, Solana, Apple, Tesla, S&P 500, Gold tickers. |
| **Screen 4: YouTube Analytics** | <img src="../media/screen_4_youtube.png" width="320" alt="YouTube"> | Live YouTube subscriber counter via Google Data API v3. |
| **Screen 5: Airspace Radar** | <img src="../media/screen_5_airspace.png" width="320" alt="Radar"> | Live ADS-B overhead flight tracking via OpenSky Network. |
| **Screen 6: Battery Telemetry** | <img src="../media/screen_6_battery.png" width="320" alt="Battery"> | Glowing neon lightning bolt with % or Voltage modes. |
| **Custom App: Spotify & Alerts** | <img src="../media/screen_custom_spotify.png" width="320" alt="Spotify"> | Dynamic MQTT/REST push API with progress bar and notifications. |

---

1. **Zero-Configuration SoftAP Captive Portal**: Built-in DNS server automatically redirects mobile and desktop clients to the dark-mode configuration portal on `192.168.4.1`.
2. **Real-Time Virtual Matrix Stream**: Live 32×8 canvas mirror in the Web Portal, one-click BMP screenshot capture, and terminal ASCII art output.
3. **Awtrix-Style Clock & Layout**: Time/Combined/Date modes, 7 weekday bars with ISO/US start and custom colors, multiple date formats, dynamic calendar icon, and seconds display with auto-hidden icon for clean spacing.
4. **Live Airspace Radar (ADS-B)**: Real-time overhead flight tracking via OpenSky Network cycling active aircraft callsigns within ~50km of local coordinates.
5. **Energy & Battery Telemetry**: Glowing neon lightning bolt icon with selectable Percentage, Voltage, or Alternating display modes.
6. **Financial Markets, Stocks & Crypto**: Live feeds for Bitcoin, Ethereum, Solana, BNB, Apple, Tesla, Nvidia, S&P 500, Gold, EUR/USD, and custom tickers.
7. **Custom Screen Reordering**: Move screens up/down in the web portal to define your exact carousel sequence, with instant AJAX saving and toast notifications.
8. **Screen Transition Effects**: Choice of 8 smooth animations (Instant Cut, Slide Left, Slide Right, Slide Up, Slide Down, Crossfade / Dissolve, Curtain Split Wipe, and Random) with zero brightness loss and native double-buffer rendering.
9. **Home Assistant MQTT Auto-Discovery**: Automatic discovery of 12 synchronized entities (sensors, matrix power/brightness light, switches, buttons, and screen selector).
10. **Custom App Push API & Spotify**: Push dynamic screens via MQTT or HTTP REST with 8×8 icons, progress bars, and high-priority push notifications with buzzer chime.
11. **Dual OTA Engine**: Web browser binary upload plus automated 1-click cloud updates from GitHub Releases with live percentage display on matrix.
