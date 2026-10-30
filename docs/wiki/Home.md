# Welcome to Ulanzi TC001 Evolved Wiki

[![YouTube Short](https://img.shields.io/badge/YouTube_Short-Watch_Video-red?logo=youtube)](https://youtube.com/shorts/nju1VtkFuw0?feature=share)
[![Android Companion](https://img.shields.io/badge/Android_Companion-v1.0.4-3DDC84?logo=android&logoColor=white)](https://github.com/hackra76/Ulanzi-TC001-Companion)
[![Companion Releases](https://img.shields.io/github/v/release/hackra76/Ulanzi-TC001-Companion?label=APK%20Download&color=blue&logo=github)](https://github.com/hackra76/Ulanzi-TC001-Companion/releases/latest)

**Ulanzi TC001 Evolved** is a modular, high-performance firmware for the Ulanzi TC001 Smart Pixel Desktop Clock. It transforms the clock with a modern, resilient ESP32 architecture featuring live display streaming, Awtrix-style layout customization, live overhead airspace radar, stock/crypto markets, custom carousel reordering, and official native **[Android Companion App](Android-Companion-App)**.

---

## 🎬 Video Showcase

<div align="center">
  <a href="https://youtube.com/shorts/nju1VtkFuw0?feature=share" target="_blank">
    <img src="https://img.youtube.com/vi/nju1VtkFuw0/maxresdefault.jpg" width="600" alt="Watch Ulanzi TC001 Evolved on YouTube Shorts">
  </a>
  <p>▶️ <b><a href="https://youtube.com/shorts/nju1VtkFuw0?feature=share">Watch on YouTube Shorts: Why Everyone is Hacking This $40 Smart Clock</a></b></p>
</div>

---

## 📚 Table of Contents

- [Android Companion App Guide](Android-Companion-App)
- [Quick Setup & Wi-Fi Provisioning](Setup-Guide)
- [Web Configuration Portal](Web-Portal)
- [Dual OTA Update System](OTA-Updates)
- [Hardware Architecture & Pinout](Hardware-Pinout)
- [REST API & Captive Endpoints](API-Endpoints)

---

## 🖥️ Screen Gallery

The Ulanzi TC001 8×32 WS2812B matrix renders unified 3×5 typography and vibrant 8×8 icons across all screens.

![Ulanzi TC001 Screen Showcase](https://raw.githubusercontent.com/hackra76/Ulanzi-TC001-Evolved/main/docs/media/screens_showcase.png)

| Screen & App | Matrix Live Preview | Summary |
|---|:---:|---|
| **Screen 0: Clock & Calendar** | <img src="https://raw.githubusercontent.com/hackra76/Ulanzi-TC001-Evolved/main/docs/media/screen_0_clock.png" width="320" alt="Clock"> | Dynamic 9×8 calendar card with dedicated 1px separation gap, 3×5 time, and 7 weekday bars. |
| **Screen 1: Indoor Environment** | <img src="https://raw.githubusercontent.com/hackra76/Ulanzi-TC001-Evolved/main/docs/media/screen_1_indoor.png" width="320" alt="Indoor"> | Sensirion SHT30 precision temperature & humidity telemetry. |
| **Screen 2: Outdoor Weather** | <img src="https://raw.githubusercontent.com/hackra76/Ulanzi-TC001-Evolved/main/docs/media/screen_2_outdoor.png" width="320" alt="Weather"> | Live Open-Meteo forecasts with WMO condition icons. |
| **Screen 3: Financial Markets** | <img src="https://raw.githubusercontent.com/hackra76/Ulanzi-TC001-Evolved/main/docs/media/screen_3_crypto.png" width="320" alt="Markets"> | Bitcoin, Ethereum, Solana, Apple, Tesla, S&P 500, Gold tickers. |
| **Screen 4: YouTube Analytics** | <img src="https://raw.githubusercontent.com/hackra76/Ulanzi-TC001-Evolved/main/docs/media/screen_4_youtube.png" width="320" alt="YouTube"> | Live YouTube subscriber counter via Google Data API v3. |
| **Screen 5: Airspace Radar** | <img src="https://raw.githubusercontent.com/hackra76/Ulanzi-TC001-Evolved/main/docs/media/screen_5_airspace.png" width="320" alt="Radar"> | Live ADS-B overhead flight tracking via OpenSky Network. |
| **Screen 6: Battery Telemetry** | <img src="https://raw.githubusercontent.com/hackra76/Ulanzi-TC001-Evolved/main/docs/media/screen_6_battery.png" width="320" alt="Battery"> | Glowing neon lightning bolt with % or Voltage modes. |
| **Screen 7: Stopwatch & Timer** | <img src="https://raw.githubusercontent.com/hackra76/Ulanzi-TC001-Evolved/main/docs/media/screen_7_timer.png" width="320" alt="Timer"> | Dual-mode stopwatch with tenths of seconds & countdown with acoustic alarm. |
| **Screen 8: Retro Screensavers** | <img src="https://raw.githubusercontent.com/hackra76/Ulanzi-TC001-Evolved/main/docs/media/screen_8_screensaver.png" width="320" alt="Screensaver"> | Zero-heap procedural animations: Matrix Digital Rain, Doom Fire, Pac-Man, and Game of Life. |
| **Custom App: Spotify & Alerts** | <img src="https://raw.githubusercontent.com/hackra76/Ulanzi-TC001-Evolved/main/docs/media/screen_custom_spotify.png" width="320" alt="Spotify"> | Dynamic MQTT/REST push API with progress bar and notifications. |

---

1. **Zero-Configuration SoftAP Captive Portal**: Built-in DNS server automatically redirects mobile and desktop clients to the dark-mode configuration portal on `192.168.4.1`.
2. **Real-Time Virtual Matrix Stream**: Live 32×8 canvas mirror in the Web Portal, one-click BMP screenshot capture, and terminal ASCII art output.
3. **Awtrix-Style Clock & Layout**: Time/Combined/Date modes, 7 weekday bars with ISO/US start and custom colors, multiple date formats, dynamic 9×8 calendar icon with dedicated 1px separation gap, and seconds display with auto-hidden icon for clean spacing.
4. **Live Airspace Radar (ADS-B)**: Real-time overhead flight tracking via OpenSky Network cycling active aircraft callsigns within ~50km of local coordinates.
5. **Energy & Battery Telemetry**: Glowing neon lightning bolt icon with selectable Percentage, Voltage, or Alternating display modes.
6. **Financial Markets, Stocks & Crypto**: Live feeds for Bitcoin, Ethereum, Solana, BNB, Apple, Tesla, Nvidia, S&P 500, Gold, EUR/USD, and custom tickers.
7. **Custom Screen Reordering**: Move screens up/down in the web portal to define your exact carousel sequence, with instant AJAX saving and toast notifications.
8. **Screen Transition Effects**: Choice of 8 smooth animations (Instant Cut, Slide Left, Slide Right, Slide Up, Slide Down, Crossfade / Dissolve, Curtain Split Wipe, and Random) with zero brightness loss and native double-buffer rendering.
9. **Precision Stopwatch & Countdown Timer**: Dual-mode chronometer (tenths of seconds in stopwatch, real-time bottom progress bar in countdown, auto-rotation freeze lock during runs, and 2.4 kHz audible buzzer alarm).
10. **Retro Ambient Screensavers**: Procedural zero-heap animations (Matrix Digital Rain, Doom Fire / Thermal Lava, Pac-Man & Blinky Arcade Chase, and Conway's Game of Life) with carousel rotation and inactivity idle auto-trigger.
11. **Home Assistant MQTT Auto-Discovery**: Automatic discovery of 15 synchronized entities (including Wi-Fi RSSI and Free Memory diagnostics) (sensors, matrix power/brightness light, switches, buttons, and screen selector).
12. **Custom App Push API & Spotify**: Push dynamic screens via MQTT or HTTP REST with 8×8 icons, progress bars, and high-priority push notifications with buzzer chime.
13. **Dual OTA Engine**: Web browser binary upload plus automated 1-click cloud updates from GitHub Releases with live percentage display on matrix.
14. **Developer Support Integration**: Interactive Buy Me a Coffee floating widget and header/footer action badges seamlessly embedded in the Web Configuration Portal.
15. **Native Android Companion App**: Official Material 3 Android app with interactive 32×8 Pixel Canvas Studio, background Spotify media broadcaster, intelligent phone notification forwarding relay, Pomodoro timer remote, screensaver switcher, and complete hardware EEPROM configuration parity ([Android Companion Guide](Android-Companion-App)).
16. **Asynchronous Dual-Core Background Architecture**: Pinned FreeRTOS worker task on Core 0 (PRO_CPU) executing all HTTP/TLS queries, guaranteeing 60+ FPS matrix rendering and zero UI latency on Core 1 (APP_CPU).
17. **Melodic Audio Sequencer & RTTTL Player**: Non-blocking tone engine with 5 audio presets, custom frequency tones, full RTTTL ringtone player, REST and MQTT audio control, and interactive Web Portal testbench with real-time AUDIO indicator diode.
18. **Stabilized Battery & LDR Sensing**: 8x battery ADC oversampling with EMA filtering to eliminate Wi-Fi burst voltage noise, and 0.8-unit deadband LDR hysteresis with 1 step / 40ms slew-rate limiter.

---

## 🙏 Hardware Credits & Attribution

This open-source firmware runs on the **[Ulanzi TC001 Smart Pixel Desktop Clock](https://www.ulanzi.com/products/ulanzi-pixel-smart-clock-2882?_pos=1&_psq=clock&_psid=1e85ab0a9&_ss=e)** manufactured by **[Shenzhen Ulanzi Technology Co., Ltd. (Ulanzi)](https://www.ulanzi.com/)**. We express our sincere appreciation to Ulanzi for producing an open, durable, and highly capable ESP32 hardware platform for makers and developers.
