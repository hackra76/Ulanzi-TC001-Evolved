# Ulanzi TC001 Evolved 🚀

[![PlatformIO](https://img.shields.io/badge/PlatformIO-Compatible-orange?logo=platformio)](https://platformio.org/)
[![ESP32](https://img.shields.io/badge/ESP32-WROOM--32-blue?logo=espressif)](https://www.espressif.com/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Release](https://img.shields.io/github/v/release/hackra76/Ulanzi-TC001-Evolved?color=brightgreen)](https://github.com/hackra76/Ulanzi-TC001-Evolved/releases)
[![Android Companion](https://img.shields.io/badge/Android_Companion-v1.0.10-3DDC84?logo=android&logoColor=white)](https://github.com/hackra76/Ulanzi-TC001-Companion)
[![Companion Releases](https://img.shields.io/github/v/release/hackra76/Ulanzi-TC001-Companion?label=APK%20Download&color=blue&logo=github)](https://github.com/hackra76/Ulanzi-TC001-Companion/releases/latest)
[![YouTube Short](https://img.shields.io/badge/YouTube_Short-Watch_Video-red?logo=youtube)](https://youtube.com/shorts/nju1VtkFuw0?feature=share)

Next-generation firmware for the original **[Ulanzi TC001 Smart Pixel Desktop Clock](https://www.ulanzi.com/products/ulanzi-pixel-smart-clock-2882?_pos=1&_psq=clock&_psid=1e85ab0a9&_ss=e)** (manufactured by Shenzhen Ulanzi Technology Co., Ltd.) running on ESP32 with an **8×32 WS2812B RGB LED matrix**, Sensirion SHT30 sensor, ambient intelligence, live crypto tickers, financial markets & stocks, live overhead airspace radar, outdoor weather, YouTube statistics, Awtrix-style layout customization, dual OTA update system, and official native **[Android Companion App](https://github.com/hackra76/Ulanzi-TC001-Companion)**.

🌐 **Languages:** [English (Default)](README.md) • [English Mirror](README_EN.md) • [Slovenčina](README_SK.md)

<a href="https://buymeacoffee.com/hackra" target="_blank"><img src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png" alt="Buy Me A Coffee" style="height: 48px !important;width: 174px !important;" ></a>

---

## 🎬 Video Showcase

<div align="center">
  <a href="https://youtube.com/shorts/nju1VtkFuw0?feature=share" target="_blank">
    <img src="https://img.youtube.com/vi/nju1VtkFuw0/maxresdefault.jpg" width="600" alt="Watch Ulanzi TC001 Evolved on YouTube Shorts">
  </a>
  <p>▶️ <b><a href="https://youtube.com/shorts/nju1VtkFuw0?feature=share">Watch on YouTube Shorts: Why Everyone is Hacking This $40 Smart Clock</a></b></p>
</div>

---

## 📱 Official Android Companion App (Ulanzi TC001 Companion)

Take full wireless control of your clock right from your pocket with the dedicated **[Ulanzi TC001 Companion Android App](https://github.com/hackra76/Ulanzi-TC001-Companion)** (built with Kotlin, Jetpack Compose, and Material 3 design).

[![Companion App GitHub](https://img.shields.io/badge/GitHub-hackra76%2FUlanzi--TC001--Companion-black?logo=github)](https://github.com/hackra76/Ulanzi-TC001-Companion)
[![Download APK](https://img.shields.io/badge/Direct_APK-Download_Latest_(v1.0.10)-3DDC84?logo=android&logoColor=white)](https://github.com/hackra76/Ulanzi-TC001-Companion/releases/latest)
[![Wiki Documentation](https://img.shields.io/badge/Wiki-Android_Companion_Guide-blue?logo=gitbook)](https://github.com/hackra76/Ulanzi-TC001-Evolved/wiki/Android-Companion-App)

### 🌟 Key Companion App Capabilities:

1. **🎨 Interactive 32×8 Pixel Canvas Studio:**
   - Real-time 32×8 LED matrix stream mirror directly inside the app.
   - Interactive touch canvas: draw pixel art directly with your finger using **Pen**, **Eraser**, and **Flood-Fill Bucket**.
   - 13 vibrant retro neon colors + 7 instant presets (Love, Pac-Man, Space Invaders, VU Equalizer, Rainbow, Fire Flame, Neon Shades).
   - **Clone Clock Matrix**: Read current matrix pixels into your canvas with a single tap.
   - **1-Tap Push**: Send drawings directly to the clock display with custom duration options.
2. **🎵 Now Playing Media Sync & Spotify Broadcaster:**
   - Background `MediaSessionManager` automatically detects active music playback (Spotify, YouTube Music, Podcasts, SoundCloud).
   - Extracts live track metadata (title, artist, album, duration, playback position) with brand-adaptive theming (Spotify Green `#1ED760`, YouTube Red, Cyan).
   - Synchronizes directly to the clock via `/api/custom?name=spotify` with scrolling marquee and 1-pixel bottom progress bar.
   - In-app playback controls: Play, Pause, Skip Next, Skip Previous, and Instant Push.
3. **🚨 Push Alerts & Intelligent Notification Forwarding Relay:**
   - Intercepts incoming Android notifications (incoming calls, SMS, WhatsApp, Telegram, calendar reminders, app alerts) via `NotificationListenerService`.
   - Intelligent debouncing (2.5s window) prevents notification floods.
   - Contextual 8×8 pixel icons (bell, mail, clock, spotify, heart, warning) with configurable piezo chime sound (0–3) and display duration (2–20s) pushed instantly via `/api/notify`.
4. **⏱️ Remote Hardware Stopwatch & Countdown Timer:**
   - Remote control for the clock's onboard chronometer via `/api/timer`.
   - Quick presets for 1m, 3m, 5m, 10m, 15m, and 25m Pomodoro sessions.
   - Start, pause, toggle, and reset controls with live digital readout simulation.
5. **👾 Retro Screensaver Controller:**
   - Trigger and stop all 5 procedural ambient screensavers (Matrix Rain, Starfield, Doom Fire, Retro Clock, Plasma Flow).
   - Configure idle timeout (1 to 30 minutes) via `/api/screensaver`.
6. **🎛️ Complete Hardware EEPROM Configuration Deck (100% Web Parity):**
   - **Display & Brightness**: LDR auto-brightness toggle, Day Brightness (0–100%), Night Brightness (0–100%), Night Mode quiet hours schedule.
   - **Clock & Date Layout**: Clock display mode, Date format, 7 weekday bars with ISO/US start day and 11-color palette selectors.
   - **Reorderable Carousel**: 9-screen rotation sequence bitmask and auto-rotate interval (3–120s).
   - **Multi-Profile Wi-Fi Memory**: Manage 4 stored network profiles with live connection badges, 1-tap connect, and RF Wi-Fi scanner.
   - **Data Feeds & Telemetry**: Weather city/coordinates, Financial market crypto/stock tickers, YouTube channel ID, battery telemetry mode (% / V / alternating), and remote reboot.
7. **📡 Zero-Configuration Discovery & Instant Auto-Reconnect:**
   - Automatic mDNS discovery (`ulanzi-tc001.local`) and parallel subnet IP scanning.
   - Jetpack DataStore persistent memory automatically reconnects to your clock upon app launch.

👉 **Download & Install:** Grab the latest release APK directly from **[Ulanzi TC001 Companion Releases](https://github.com/hackra76/Ulanzi-TC001-Companion/releases/latest)**.

---

## 🖥️ Screen Gallery & Display Capabilities

The 8×32 WS2812B NeoPixel RGB LED matrix renders information with high visual contrast, unified 3×5 typography, and dedicated 8×8 icons.

![Ulanzi TC001 Screen Showcase](docs/media/screens_showcase.png)

| Screen & Function | Matrix Live Preview | Details & Capabilities |
|---|:---:|---|
| **Screen 0: Clock & Calendar** | <img src="docs/media/screen_0_clock.png" width="340" alt="Clock and Calendar Screen"> | Dynamic calendar card (Awtrix day 1–31), 3×5 digital time, and 7 weekday bars with custom active, inactive, and weekend color accents. |
| **Screen 1: Indoor Environment** | <img src="docs/media/screen_1_indoor.png" width="340" alt="Indoor Environment Screen"> | Real-time Sensirion SHT30 precision telemetry (`°C` temperature and humidity) via hardware I2C bus. |
| **Screen 2: Outdoor Weather** | <img src="docs/media/screen_2_outdoor.png" width="340" alt="Outdoor Weather Screen"> | Live Open-Meteo forecasts with WMO condition icons (Sun, Clouds, Rain, Snow, Storm) and 25+ city presets. |
| **Screen 3: Financial Markets & Crypto** | <img src="docs/media/screen_3_crypto.png" width="340" alt="Financial Markets Screen"> | Zero-key live price ticker for Bitcoin, Ethereum, Solana, Apple, Tesla, Nvidia, S&P 500, Gold Bullion, and custom symbols. |
| **Screen 4: YouTube Analytics** | <img src="docs/media/screen_4_youtube.png" width="340" alt="YouTube Subscriber Counter Screen"> | Live channel subscriber counter with redesigned centered play button icon via Google Data API v3. |
| **Screen 5: Live Airspace Radar** | <img src="docs/media/screen_5_airspace.png" width="340" alt="Airspace Radar Screen"> | Real-time overhead flight tracking (ADS-B) via OpenSky Network within ~50km radius with cyan plane icon. |
| **Screen 6: Battery & Energy Telemetry** | <img src="docs/media/screen_6_battery.png" width="340" alt="Battery Telemetry Screen"> | Distinctive neon yellow/white glowing lightning bolt with Percentage, Voltage, or Alternating display modes. |
| **Screen 7: Stopwatch & Countdown Timer** | <img src="docs/media/screen_7_timer.png" width="340" alt="Stopwatch & Timer Screen"> | Dual-mode chronometer (Stopwatch with tenths of seconds & Countdown with acoustic alarm, dynamic bottom progress bar, and state badge). |
| **Screen 8: Retro Ambient Screensavers** | <img src="docs/media/screen_8_screensaver.png" width="340" alt="Retro Ambient Screensaver Screen"> | Zero-heap procedural animation suite: Matrix Digital Rain, Doom Fire / Thermal Lava, Pac-Man & Blinky Arcade Chase, and Conway's Game of Life. |
| **Custom App: Spotify & Push Alerts** | <img src="docs/media/screen_custom_spotify.png" width="340" alt="Spotify and Custom App Screen"> | Dynamic MQTT / REST push API with Spotify green icon, smooth scrolling track ticker, and bottom progress bar. |

---

## ✨ Features

- **📱 Native Android Companion App Ecosystem:**
  - Dedicated modern Android app built with **Jetpack Compose**, **Material 3**, and target **Android 15/14/16** ([hackra76/Ulanzi-TC001-Companion](https://github.com/hackra76/Ulanzi-TC001-Companion)).
  - **Interactive 32×8 Pixel Canvas Studio**: Draw pixel art on your phone with Pen, Eraser, Bucket fill, 13 retro colors, 7 presets, clone matrix, and instant push to clock.
  - **Spotify & Now Playing Live Broadcaster**: Intercepts active media sessions, extracts metadata & album colors, and streams progress bars via `/api/custom`.
  - **Intelligent Phone Notification Relay**: Background alert interception with duplicate debouncing, 8×8 contextual pixel icons, and acoustic piezo chimes via `/api/notify`.
  - **Remote Chronometer & Pomodoro**: Remote trigger and live digital readout for stopwatch and countdown timers.
  - **Screensaver Remote**: Ambient animation switcher (5 styles) with idle timeout controls.
  - **Hardware EEPROM Configuration Deck**: 100% web parity for brightness, quiet hours, 9-screen carousel order, multi-network Wi-Fi profiles, coordinates, tickers, and reboot.
  - **Zero-Config Discovery & Reconnect**: Instant mDNS auto-detection (`ulanzi-tc001.local`) and persistent DataStore auto-reconnection.
- **🎛️ Tactile Field Instrument Web Portal (Industrial Design):**
  - Engineered with a modern **Tactile Field Instrument** industrial design system (Braun / Teenage Engineering / Swiss Grid aesthetic).
  - High-density telemetry deck: real-time Wi-Fi RSSI gauge, flash memory meter, and uptime clock in monospace data headers.
  - Tactile physical-feel controls: milled mechanical switchgear, knurled thumb sliders, recessed bezel cards, and live 32×8 NeoPixel stream mirror.
  - **Categorized into 4 Logical Decks**: **Display & Clock Engine**, **Screens & Carousel Navigation**, **Data Services & Integrations**, and **Network & System**.
  - **Multilingual Support**: Instant toggle between **English** and **Slovak** (`[EN | SK]`), persisted in client storage and device NVS (`ui_lang`).
  - **Direct GitHub Repo Link**: Header version badge (`v2.7.0 • GitHub`) linking directly to the latest releases and source code.
  - **Configuration Backup & Restore**: One-click JSON backup export (`GET /api/backup`) and file restore (`POST /api/restore`) with validation, immediate NVS flash commit, and acoustic buzzer confirmation.
  - **Remote Hardware Reboot REST API**: Graceful remote restart via `POST /api/reboot` and `POST /reboot` with automated connection handshake for the Companion App and home automation.
  - **Developer Support Integration**: Interactive Buy Me a Coffee floating widget and header/footer action badges.
  - Automatic fallback to SoftAP (`Ulanzi-Setup-XXXX`) with captive portal redirection on `192.168.4.1`.
  - Non-blocking Wi-Fi credential scanner with signal strength indicators and AJAX instant save.
- **📶 Multi-Network Wi-Fi Memory & Intelligent Auto-Roaming:**
  - Remembers up to 4 wireless network profiles in NVS (Profile 01 Primary, Profile 02 Secondary, Profile 03 Hotspot, Profile 04 Auxiliary).
  - Fast 2.4 GHz RF scan on startup (~1.5s) automatically matches nearby SSIDs and connects to whichever saved network has the strongest RSSI (effortless roaming between home, office, and mobile hotspots).
  - **Self-Healing Wi-Fi AP Fallback & Auto-Reconnection**: If Wi-Fi is lost and connection to all saved networks times out, the device enters fallback SoftAP (`Ulanzi-Setup-XXXX`) in concurrent `WIFI_AP_STA` mode. It automatically attempts background reconnection to saved profiles every 30 seconds (postponing retries if a user is actively connected to the portal) and seamlessly resumes normal clock operation as soon as the network returns.
  - Full REST API support via `GET /api/wifi` and one-click slot assignment in Web Portal Deck 04.
- **📺 Live Display Stream & Virtual Matrix:**
  - Real-time 32×8 NeoPixel mirror canvas directly in the Web Portal with 1.5s auto-refresh.
  - One-click BMP screenshot export (`/api/display/screenshot`).
  - Terminal ASCII art representation (`/api/display/text`).
  - Web-based Next/Previous screen testing buttons.
- **⏰ Awtrix-Style Clock & Date Layout:**
  - **Continuous Real-Time NTP & Automatic DST Synchronization**: Continuous non-blocking sync with ESP-IDF's POSIX timezone engine (`CET-1CEST,M3.5.0,M10.5.0/3`). Automatically applies daylight saving time transitions (summer/winter time in Slovakia & Europe) without requiring reboots and eliminates crystal clock drift.
  - 3 Display Modes: Time Only, Combined Time & Date (alternating 5s/3s), or Date Only.
  - **7 Weekday Bars with Weekend Customization**:
    - 11 Active Day colors (Cyan, Orange, Green, Yellow, Red, White, Magenta, Electric Blue, Lime, Amber Gold, Pink).
    - 10 Inactive Day colors (Dim Active Auto, Slate Gray, Amber, Dark Blue, Deep Wine, Forest Green, Charcoal, Purple, Dark Cyan, Off Black).
    - Dedicated Weekend Day color selector (11 options) with automatic proportional inactive weekend dimming.
    - Monday (ISO 8601) or Sunday week start.
  - Date formats: `DD.MM.`, `DD MMM`, `MM/DD`, `DAY DD`.
  - Clock icons: Dynamic 9×8 calendar (shows day 1–31 with dedicated 1px separation gap and symmetrical borders, eliminating digit collision on double-digit dates like 22/23), Heart, Clock face, or Full-Width.
  - Seconds Display: Full-width centered `HH:MM:SS` with dividers; automatically hides icon when seconds are enabled for clean spacing.
  - Unified compact 3×5 typography system across all display screens (Clock, Weather, Tickers, Radar, Battery, Custom Apps) with 1px top margin and clean vertical separation from bars.
- **✈️ Live Airspace Radar (ADS-B):**
  - Real-time overhead flight tracking via OpenSky Network based on local weather GPS coordinates (~50km bounding radius).
  - Automatically cycles active aircraft callsigns every 4 seconds with cyan airplane icon.
  - Displays `"CLEAR"` in green when airspace is empty.
- **⚡ Energy & Battery Telemetry:**
  - Distinctive neon yellow and white glowing lightning bolt icon (`ICON_LIGHTNING`).
  - Configurable display format in web portal: Percentage only (`93%`), Voltage only (`4.12V`), or Alternating every 3 seconds.
- **📈 Financial Markets, Stocks & Crypto Tickers:**
  - Public market feeds with zero API keys required.
  - Cryptocurrencies: `BTC`, `ETH`, `SOL`, `BNB`.
  - Stocks & Indices: `AAPL`, `TSLA`, `NVDA`, `SPX` (S&P 500).
  - Forex & Commodities: Gold Bullion (`GOLD`), `EUR/USD`, `USD/EUR`.
  - Custom ticker input with dynamic currency color accents.
- **▶️ YouTube Channel Analytics:**
  - Real-time subscriber counter via Google Data API v3.
  - Redesigned bold, centered white triangle play button on rounded red pill icon.
- **🌤️ Real Outdoor Weather:**
  - Live data from Open-Meteo API (zero API keys required).
  - WMO weather code mapping to animated 8×8 icons (Sun, Clouds, Rain, Snow, Storm).
  - **Predefined City Selector:** 25+ presets across Slovakia, Czechia, Europe, and Worldwide.
- **💡 Ambient Intelligence & Screen Management:**
  - Dynamic auto-brightness via LDR sensor using exponential moving average filtering and gamma curves.
  - Day Brightness Ceiling slider (5% – 100%) and Night Brightness slider (1% – 25%).
  - Configurable Night Mode quiet hours schedule with ultra-dim, distraction-free pure deep-red centered 3×5 clock time (zero yellow moon glare for bedroom rest).
  - Quick Wake: Any button press temporarily wakes display for 12 seconds during night mode.
- **🎛️ Reorderable Screen Carousel:**
  - Drag-and-drop style `▲` / `▼` controls in web portal to customize screen display order.
  - Individual checkboxes to enable or disable any screen.
  - Sequence order persists in NVS (`car_ord`) and governs auto-rotation and hardware button cycling.
  - Configurable rotation interval (3 to 120 seconds).
  - Web portal AJAX instant save with toast notifications and automatic return.
- **🎬 Screen Transition Effects:**
  - 8 selectable animations for smooth screen switching in Web Portal:
    - **None (Instant Cut)**: Instant screen switch.
    - **Slide Left**: Smooth leftwards scroll (2px stride, 16 steps).
    - **Slide Right**: Smooth rightwards scroll.
    - **Slide Up**: Vertical push from bottom to top (8 steps).
    - **Slide Down**: Vertical push from top to bottom.
    - **Crossfade / Dissolve**: 10-step linear alpha blending with native 24-bit GRB double-buffering.
    - **Curtain (Center Split Wipe)**: Theatrical opening wipe from the center outward.
    - **Random**: Automatically selects a dynamic effect on each screen transition.
  - Direct hardware buffer manipulation eliminates flicker and preserves full LED brightness without double-attenuation.
- **⚡ Asynchronous Dual-Core Background Architecture:**
  - Dedicated FreeRTOS worker task pinned to **Core 0 (PRO_CPU)** handles all blocking HTTP/TLS network requests for Weather, Crypto, YouTube, and Live Airspace.
  - Completely frees **Core 1 (APP_CPU)** for uninterrupted 60+ FPS LED matrix rendering, smooth screen transitions, and instant button handling without network stalls.
- **🎵 Melodic Tone Sequencer & Retro RTTTL Player:**
  - Non-blocking hardware tone sequencer with 5 built-in melodic audio presets: `Click`, `Alert`, `Success`, `Notification`, and `Alarm`.
  - Integrated **Ring Tone Text Transfer Language (RTTTL)** parser for playing custom retro 8-bit melodies.
  - Full REST API (`POST /api/buzzer`, `POST /api/sound`) and MQTT control topics (`<prefix>/buzzer/set`, `<prefix>/sound/set`).
  - Interactive **Acoustic Signal & Buzzer Testbench** in the Web Portal with a real-time **AUDIO** status indicator diode.
- **🏠 Home Assistant MQTT Auto-Discovery (15 Entities):**
  - Native zero-configuration integration: automatically creates 15 synchronized entities in Home Assistant when connected to your broker.
  - Sensors: SHT30 Temperature (`°C`), Humidity (`%`), Battery (`%`), Battery Voltage (`V`), Ambient Light, Active Screen, Wi-Fi Signal RSSI (`dBm`), Free Heap Memory (`B`).
  - Controls: Display Matrix Light (Power & Brightness slider 0–255), Auto-Brightness Switch, Carousel Auto-Rotate Switch, Night Mode Switch, Screen Select Dropdown, Buzzer Chime Button, Restart Button.
  - Audio Control via MQTT: Send named presets, frequencies, or RTTTL strings directly to `<prefix>/buzzer/set` or `<prefix>/sound/set`.
  - Live connection status and broker credentials configurable directly in the Web Portal.
- **🎵 Custom App Push API & Spotify Integration:**
  - **Dynamic Screens**: Push custom apps via MQTT (`<prefix>/custom/<name>`) or HTTP REST (`POST /api/custom?name=<name>`).
  - Pushed screens (e.g. Spotify currently playing track, 3D printer status, washer progress) dynamically join the carousel rotation.
  - **Payload Features**: Text, 8×8 icons (built-in `spotify`, `bell`, `lightning`, `youtube`, etc., or 64 RGB565 array), colors, 1-pixel bottom progress bar (0–100%), custom duration, and lifetime auto-eviction.
  - **Clean Typography & Layout**: Text rendered at rows 0–6 when progress bar is active at row 7, eliminating vertical collisions.
  - **Pinned Icons**: Left icon is pinned with columns 0–8 masked so marquee text scrolls cleanly behind the icon without pixel bleeding.
  - **🚨 High-Priority Notifications**: Push instant alerts via `POST /api/notify` or `<prefix>/notify` that interrupt the active screen with buzzer chime and auto-dismiss back to the carousel.
- **⏱️ Precision Stopwatch & Countdown Timer:**
  - **Dual Chronometer Engine**: Full-featured Stopwatch (counts up `MM:SS.t` with tenths of seconds) and Countdown Timer (`MM:SS` with real-time bottom progress bar and state badge).
  - **Acoustic & Visual Alarm**: Pulsing 2.4 kHz piezo buzzer chime and flashing matrix bell alarm upon timer expiration, instantly dismissible with any physical button press.
  - **Carousel Pinning**: Auto-rotation automatically locks in place while the timer is actively running, preventing the display from rotating away.
  - **Tactile Web Portal Remote**: Interactive chronometer control deck in the Web Portal with quick duration buttons (1m, 3m, 5m, 10m, 15m), mode switcher, and live state reflection.
  - **Remote REST API**: Remote automation via `GET /api/timer` or `POST /api/timer` (`action=start|pause|toggle|reset`, `mode=stopwatch|countdown`, `duration=<sec>`).
- **👾 Retro Ambient Screensavers:**
  - **Zero-Heap Procedural Engines**:
    - **Matrix Digital Rain**: Cyberpunk cascading green phosphor code streams with glowing white spark heads.
    - **Doom Fire / Thermal Lava**: Classic PSX/Doom molten heat propagation simulation with dynamic bottom ember generation.
    - **Pac-Man & Blinky Arcade Chase**: 80s arcade chase with animated mouth chomping, pellet dots, energizer, and reversible blue ghost vulnerability mode.
    - **Conway's Game of Life**: Real-time cellular automata with age-based coloration (Cyan -> Emerald -> Yellow -> Magenta) and automatic stagnation / extinction re-seeding.
    - **Dynamic Shuffle**: Cycles through all four animations every 25 seconds for an ever-evolving desk ambiance.
  - **Two Operational Triggers**:
    - **Carousel Screen (Screen 8)**: Seamlessly rotates as part of the scheduled sequence alongside Clock, Weather, and Tickers.
    - **Inactivity Idle Mode**: Automatically activates screensaver after a configurable timeout (1, 2, 5, 10, 15, or 30 minutes of no button interaction) with instant zero-lag wake on any physical button press.
  - **Dedicated REST API**: Full automation and external control via `GET /api/screensaver` and `POST /api/screensaver` (`style=0..4`, `timeout=0..60`, `action=start|stop`).
- **🔄 Dual Over-The-Air (OTA) Updates:**
  - Direct web browser `.bin` binary upload (`/update`) with real-time percentage progress bar.
  - 1-Click cloud updates from GitHub Releases (`/api/ota/check`, `/api/ota/install`).
  - Active update progress percentage rendered live on the 8×32 matrix display.

---

## 🎛️ Physical Controls

| Button | Action | Function |
|---|---|---|
| **Left** (GPIO 26) | Short Click | Switch to Previous Screen along custom order (pauses carousel) |
| **Right** (GPIO 14) | Short Click | Switch to Next Screen along custom order (pauses carousel) |
| **OK / Center** (Screensaver Screen) | Short Click | Cycle Screensaver Styles (Matrix Rain, Fire, Pac-Man, Life, Shuffle) |
| **OK / Center** (Battery Screen) | Short Click | Toggle Telemetry Display between Voltage (V) and Percentage (%) |
| **OK / Center** (Crypto Screen) | Short Click | Cycle Market Asset Presets (BTC, ETH, SOL, BNB, AAPL, TSLA, NVDA, SPX, GOLD, EUR, USD) |
| **OK / Center** (Timer Screen) | Short Click | Start / Pause (Toggle) Stopwatch or Countdown Timer |
| **OK / Center** (Other Screens) | Short Click | Cycle Manual Brightness (10% → 35% → 80%) |
| **OK / Center** (Global Long Press) | Long Press (>700ms) | Toggle Auto-Rotation Carousel On/Off (on Timer screen: reset / toggle mode) |
| **Left** (Timer Screen) | Long Press (>700ms) | Adjust Countdown Duration -1 minute (when stopped/paused) |
| **Right** (Timer Screen) | Long Press (>700ms) | Adjust Countdown Duration +1 minute (when stopped/paused) |
| **Any Button** | Click (during Timer Alarm) | Instantly silence and dismiss countdown alarm |
| **Any Button** | Click (in Idle Screensaver) | Instantly wake device back to active rotation |
| **Any Button** | Click (in Night Mode) | Temporarily wake display at Day Brightness for 12s |

---

## 🌐 Web Configuration Portal

Access the web portal at `http://<device-ip>/` (or `http://192.168.4.1/` in setup mode):

1. **Live Display Stream:** Real-time 32×8 canvas mirror and BMP screenshot capture button.
2. **Wi-Fi Network:** Scan nearby networks and save credentials.
3. **Ambient & Screen:** Toggle auto-brightness, Day/Night brightness ceilings, and quiet hours.
4. **Screen Carousel & Transitions:** Reorder screens using `▲`/`▼` buttons, toggle individual screens, and select transition effect (Slide, Fade, Curtain, Random).
5. **Battery Display:** Select Percentage, Voltage, or Alternating every 3s.
6. **Clock & Date Layout:** Select display mode, seconds display, weekday bar, colors, date format, and icon style.
7. **Weather Settings:** Choose city preset or input custom coordinates.
8. **Financial Markets:** Select popular stock, index, commodity, or crypto preset, or enter custom symbol.
9. **YouTube Analytics:** Set channel ID or @handle and Google API key.
10. **Home Assistant & MQTT:** Configure broker IP, port, credentials, and topic prefix with live connection indicator.
11. **Firmware Update (OTA):** Upload local `.bin` or install new releases from GitHub.

---

## 🔌 Hardware Architecture & Pinout

| Peripheral | Component | GPIO Pin | Details |
|---|---|---|---|
| **Display** | WS2812B NeoPixel | **GPIO 32** | 8×32 matrix (256 LEDs, zigzag columns) |
| **Sensor (I2C)** | Sensirion SHT30 | **GPIO 21 (SDA), GPIO 22 (SCL)** | Ambient Temp & Humidity, address `0x44` |
| **Light Sensor** | LDR Photoresistor | **GPIO 33** | ADC1 CH5 with EMA filtering |
| **Battery Sense** | Li-ion Telemetry | **GPIO 34** | ADC1 CH6 with 1:1 voltage divider |
| **Buzzer** | Piezo Element | **GPIO 15** | Non-blocking tone generation |
| **Left Button** | Tactile Switch | **GPIO 26** | Active LOW, internal pull-up |
| **Center Button**| Tactile Switch | **GPIO 27** | Active LOW, internal pull-up |
| **Right Button** | Tactile Switch | **GPIO 14** | Active LOW, internal pull-up |
| **Power** | Li-ion Battery | — | 4400 mAh capacity with USB-C charge |

---

## 🛠️ Building & Flashing

### Requirements
- [PlatformIO Core](https://platformio.org/) or PlatformIO IDE (VS Code extension)

### Build Firmware
```powershell
pio run -e ulanzi-tc001
```

### USB Flash (Windows PowerShell)
Always use UTF-8 console encoding to avoid Python UnicodeEncodeError crashes:
```powershell
$env:PYTHONIOENCODING="utf8"; pio run -e ulanzi-tc001 -t upload
```

### Monitor Serial Output
```powershell
pio device monitor -b 115200
```

---

## 📦 Releases & Binaries

Pre-compiled binaries for each release are published under [Releases](https://github.com/hackra76/Ulanzi-TC001-Evolved/releases):
- `firmware.bin`: OTA update file for flashing via Web Portal or GitHub 1-click update.
- `firmware.factory.bin`: Full factory image for initial USB flashing starting at flash offset `0x0000`.

---

## 🙏 Credits & Hardware Attribution

This project is an independent custom firmware engineered for the **Ulanzi TC001 Smart Pixel Clock**, originally designed and manufactured by **[Shenzhen Ulanzi Technology Co., Ltd. (Ulanzi)](https://www.ulanzi.com/)**.

- **Original Hardware Device**: [Ulanzi Pixel Smart Clock (TC001)](https://www.ulanzi.com/products/ulanzi-pixel-smart-clock-2882?_pos=1&_psq=clock&_psid=1e85ab0a9&_ss=e)
- **Hardware Engineering**: Special thanks to **Ulanzi** for creating such an accessible, beautifully engineered, and hackable ESP32 desktop device featuring an 8×32 addressable RGB matrix, Sensirion sensor, and portable Li-ion battery.
- **Inspiration & Ecosystem**: Gratitude to the broader open-source pixel clock community and the **Awtrix** ecosystem for pioneering desktop matrix aesthetics.

> [!NOTE]
> *Disclaimer: Ulanzi TC001 Evolved is an independent, community-driven open-source project and is not officially affiliated with, endorsed by, or sponsored by Shenzhen Ulanzi Technology Co., Ltd. "Ulanzi" and the TC001 model designation are trademarks of their respective owners.*

---

## 📄 License
This project is open-source under the MIT License.
