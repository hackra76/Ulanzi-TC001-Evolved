# Ulanzi TC001 Evolved 🚀

[![PlatformIO](https://img.shields.io/badge/PlatformIO-Compatible-orange?logo=platformio)](https://platformio.org/)
[![ESP32](https://img.shields.io/badge/ESP32-WROOM--32-blue?logo=espressif)](https://www.espressif.com/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Release](https://img.shields.io/github/v/release/hackra76/Ulanzi-TC001-Evolved?color=brightgreen)](https://github.com/hackra76/Ulanzi-TC001-Evolved/releases)

Next-generation firmware for the **Ulanzi TC001 Smart Pixel Desktop Clock** running on ESP32 with an **8×32 WS2812B RGB LED matrix**, Sensirion SHT30 sensor, ambient intelligence, live crypto tickers, financial markets & stocks, live overhead airspace radar, outdoor weather, YouTube statistics, Awtrix-style layout customization, and dual OTA update system.

🌐 **Languages:** [English (Default)](README.md) • [English Mirror](README_EN.md) • [Slovenčina](README_SK.md)

---

## ✨ Features

- **📱 Captive Portal Wi-Fi Provisioning:**
  - Automatic fallback to SoftAP (`Ulanzi-Setup-XXXX`) with captive portal redirection on `192.168.4.1`.
  - Non-blocking Wi-Fi credential scanner with signal strength indicators.
  - Seamless background station reconnection and credential storage in NVS (`Preferences`).
- **📺 Live Display Stream & Virtual Matrix:**
  - Real-time 32×8 NeoPixel mirror canvas directly in the Web Portal with 1.5s auto-refresh.
  - One-click BMP screenshot export (`/api/display/screenshot`).
  - Terminal ASCII art representation (`/api/display/text`).
  - Web-based Next/Previous screen testing buttons.
- **⏰ Awtrix-Style Clock & Date Layout:**
  - 3 Display Modes: Time Only, Combined Time & Date (alternating 5s/3s), or Date Only.
  - 7 Weekday Bars under time with configurable active and inactive day colors, and Monday (ISO 8601) or Sunday week start.
  - Date formats: `DD.MM.`, `DD MMM`, `MM/DD`, `DAY DD`.
  - Clock icons: Dynamic calendar (shows day 1–31), Heart, Clock face, or Full-Width.
  - Seconds Display: Full-width centered `HH:MM:SS` with dividers; automatically hides icon when seconds are enabled for clean spacing.
  - Proportional 3×5 font at row 1 and weekday bars at row 7, eliminating vertical overlapping.
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
  - Configurable Night Mode quiet hours schedule with ultra-dim red crescent moon clock.
  - Quick Wake: Any button press temporarily wakes display for 12 seconds during night mode.
- **🎛️ Reorderable Screen Carousel:**
  - Drag-and-drop style `▲` / `▼` controls in web portal to customize screen display order.
  - Individual checkboxes to enable or disable any screen.
  - Sequence order persists in NVS (`car_ord`) and governs auto-rotation and hardware button cycling.
  - Configurable rotation interval (3 to 120 seconds).
  - Web portal AJAX instant save with toast notifications and automatic return.
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
| **OK / Center** (GPIO 27) | Short Click | Cycle Manual Brightness (Low / Med / High) |
| **OK / Center** (GPIO 27) | Long Press (>700ms) | Toggle Auto-Rotation Carousel On/Off |
| **Any Button** | Click (in Night Mode) | Temporarily wake display at Day Brightness for 12s |

---

## 🌐 Web Configuration Portal

Access the web portal at `http://<device-ip>/` (or `http://192.168.4.1/` in setup mode):

1. **Live Display Stream:** Real-time 32×8 canvas mirror and BMP screenshot capture button.
2. **Wi-Fi Network:** Scan nearby networks and save credentials.
3. **Ambient & Screen:** Toggle auto-brightness, Day/Night brightness ceilings, and quiet hours.
4. **Screen Carousel & Reordering:** Reorder screens using `▲`/`▼` buttons and toggle individual screens.
5. **Battery Display:** Select Percentage, Voltage, or Alternating every 3s.
6. **Clock & Date Layout:** Select display mode, seconds display, weekday bar, colors, date format, and icon style.
7. **Weather Settings:** Choose city preset or input custom coordinates.
8. **Financial Markets:** Select popular stock, index, commodity, or crypto preset, or enter custom symbol.
9. **YouTube Analytics:** Set channel ID or @handle and Google API key.
10. **Firmware Update (OTA):** Upload local `.bin` or install new releases from GitHub.

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

## 📄 License
This project is open-source under the MIT License.
