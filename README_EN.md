# Ulanzi TC001 Evolved 🚀 (English Documentation)

[![PlatformIO](https://img.shields.io/badge/PlatformIO-Compatible-orange?logo=platformio)](https://platformio.org/)
[![ESP32](https://img.shields.io/badge/ESP32-WROOM--32-blue?logo=espressif)](https://www.espressif.com/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Release](https://img.shields.io/github/v/release/hackra76/Ulanzi-TC001-Evolved?color=brightgreen)](https://github.com/hackra76/Ulanzi-TC001-Evolved/releases)

Next-generation firmware for the **Ulanzi TC001 Smart Pixel Desktop Clock** running on ESP32 with an **8×32 WS2812B RGB LED matrix**, Sensirion SHT30 sensor, ambient intelligence, live crypto tickers, outdoor weather, YouTube statistics, and dual OTA update system.

🌐 **Languages:** [English (Default)](README.md) • [English Mirror](README_EN.md) • [Slovenčina](README_SK.md)

---

## ✨ Features

- **📱 Captive Portal Wi-Fi Provisioning:**
  - Automatic fallback to SoftAP (`TC001-Setup-XXXX`) with captive portal redirection on `192.168.4.1`.
  - Non-blocking Wi-Fi credential scanner with signal strength indicators.
  - Seamless background station reconnection and credential storage in NVS (`Preferences`).
- **💡 Ambient Intelligence & Screen Management:**
  - Dynamic auto-brightness via LDR sensor using exponential moving average filtering and non-linear gamma curves.
  - **Day Brightness Ceiling:** Configurable slider (5% – 100%) in web portal.
  - **Night Mode Schedule:** Configurable quiet hours (e.g., 23:00 to 07:00) with ultra-dim red crescent moon clock.
  - **Night Brightness Slider:** Configurable slider (1% – 25%).
  - **Quick Wake:** Any button press temporarily wakes the screen for 12 seconds during night mode.
- **🌤️ Real Outdoor Weather:**
  - Live data from Open-Meteo API (zero API keys required).
  - WMO weather code mapping to animated 8×8 icons (Sun, Clouds, Rain, Snow, Storm).
  - **Predefined City Selector:** 25+ presets across Slovakia, Czechia, Europe, and Worldwide that auto-fill coordinates.
- **📈 Real-Time Crypto Ticker:**
  - Direct integration with Binance public market API.
  - Supports `BTC`, `ETH`, `SOL`, and `BNB` with dynamic compact formatting (`68.5K`, `2.6K`).
- **▶️ YouTube Channel Telemetry:**
  - Real-time subscriber count via Google Data API v3 with compact number notation (`1.4M`, `250K`).
- **🔄 Dual Over-The-Air (OTA) Updates:**
  - Direct web browser `.bin` binary upload (`/update`) with real-time percentage progress bar.
  - 1-Click cloud updates from GitHub Releases (`/api/ota/check`, `/api/ota/install`).
  - Active update progress percentage rendered live on the 8×32 matrix display.
- **⏱️ Smart Carousel:**
  - Non-blocking horizontal ticker engine with pinned 8×8 status icons.
  - Configurable rotation delay (3 to 120 seconds).
  - Physical button controls to toggle auto-rotation and cycle manually.

---

## 🎛️ Physical Controls

| Button | Action | Function |
|---|---|---|
| **Left** (GPIO 26) | Short Click | Switch to Previous Screen (pauses carousel) |
| **Right** (GPIO 14) | Short Click | Switch to Next Screen (pauses carousel) |
| **OK / Center** (GPIO 27) | Short Click | Cycle Manual Brightness (Low / Med / High) |
| **OK / Center** (GPIO 27) | Long Press (>800ms) | Toggle Auto-Rotation Carousel On/Off |
| **Any Button** | Click (in Night Mode) | Temporarily wake display at Day Brightness for 12s |

---

## 🌐 Web Configuration Portal

Once connected to your local network, access the web configuration portal at `http://<device-ip>/` or `http://192.168.4.1/` when in setup mode:

1. **Wi-Fi Network:** Scan nearby networks and save credentials.
2. **Ambient & Screen:** Toggle auto-brightness, set Day and Night brightness sliders, configure quiet hours schedule, and set carousel rotate delay.
3. **Weather Settings:** Choose your city from the preset list or enter custom latitude/longitude coordinates.
4. **Financial Ticker:** Choose your preferred cryptocurrency pair (`BTC`, `ETH`, `SOL`, `BNB`).
5. **YouTube Analytics:** Set channel ID or handle and Google API key.
6. **Firmware Update (OTA):** Upload a new firmware binary or click to install the latest GitHub release.

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
