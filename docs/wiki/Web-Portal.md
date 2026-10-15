# Web Configuration Portal

The web configuration portal is hosted on port 80 of the device's IP address (`http://<device-ip>/`) or `http://192.168.4.1/` when in SoftAP setup mode.

It features a precision **Tactile Field Instrument** industrial design system (inspired by Braun, Teenage Engineering, and Swiss Grid typography) organized into 4 logical equipment decks, high-density telemetry instrumentation, instant multilingual language support (English and Slovak), and live matrix mirroring.

---

## Top Navigation & Language Switcher

- **GitHub Repository Version Badge**: Displays the current firmware release version (e.g. `v1.5.0 • GitHub`) linking directly to the project GitHub repository.
- **Multilingual Support (EN / SK)**: Switch between English and Slovak with a single click. The choice is saved immediately in client-side `localStorage` and synchronized with device NVS (`ui_lang`).

---

## 📺 Live Display Stream & Virtual Matrix

- **NeoPixel Mirror**: 32×8 canvas rendering the exact pixel colors displayed on the clock in real-time (auto-refreshes every 1.5 seconds).
- **Capture BMP**: One-click download of a standard uncompressed 24-bit `.bmp` screenshot directly from the device (`/api/display/screenshot`).
- **Prev / Next**: Interactive controls to step through active screens directly from the browser.

---

## ⏰ Group 1: Display & Clock Engine

### Clock & Date Layout
- **Clock Display Mode**: Time Only, Combined Time & Date (alternating 5s/3s), or Date Only.
- **Show Seconds**: When enabled, the clock icon is automatically hidden to allow full-width centered `HH:MM:SS` with colons and weekday bars to fit cleanly without crowding or clipping.
- **Unified Typography**: Compact 3×5 bitmap font across all screens with 1px top margin and clear vertical separation from progress and weekday bars.
- **Date Format**: `DD.MM.`, `DD MMM`, `MM/DD`, `DAY DD`.
- **Clock Icon Style**: Dynamic calendar with day of month (1–31), Heart, Clock Face, or None.
- **Text Alignment**: Left-aligned with icon (Awtrix default) or Centered.

### 7 Weekday Bars Under Time
- **Week Starts On**: Monday (ISO 8601) or Sunday (US Standard).
- **Active Day Colors (11 options)**: Cyan (#00F0FF), Orange (#FF8800), Green (#00FF00), Yellow (#FFFF00), Red (#FF2200), White (#FFFFFF), Magenta (#FF00FF), Electric Blue (#0066FF), Lime Green (#76FF03), Amber Gold (#FFC107), Soft Pink (#FF4081).
- **Inactive Day Colors (10 options)**: Dim Active Color (Auto - Slightly Dimmer, scales ~65% of active day so it stays visible even when dimmed), Slate Gray (#708090), Dim Amber (#906810), Dark Blue (#001A40), Deep Wine (#3A000A), Forest Green (#002A0D), Charcoal (#252525), Muted Purple (#2A1035), Dark Cyan (#003038), Off (Black).
- **Weekend Day Color (11 options)**: Same as Active Weekday, Bright Red (#FF2200), Coral Orange (#FF6B4A), Gold (#FFD700), Hot Pink (#FF1493), Purple (#9C27B0), Cyan (#00F0FF), Lime (#76FF03), Pure White (#FFFFFF), Sky Blue (#00BFFF), Emerald (#00E676). Inactive weekend days automatically retain proportional contrast.

### Ambient Brightness & Night Schedule
- **Auto-Brightness via LDR**: Reads the phototransistor, smooths light fluctuations, and dynamically adapts matrix brightness.
- **Day Brightness Ceiling**: Slider range: `5%` to `100%`.
- **Night Mode (Ultra-Dim Red Clock)**: Ultra-dim, glare-free pure deep-red (`0x8000`) centered 3×5 clock time without bright yellow icons or ambient glare.
- **Night Mode Brightness**: Slider range: `1%` to `25%`.
- **Night Schedule Hours**: Configurable start and end hours (e.g. 22:00 – 07:00).

---

## 🎠 Group 2: Screens & Carousel Navigation

- **Auto-Rotate Screens**: Toggles automatic carousel rotation on or off.
- **Carousel Cycle Interval**: Configurable rotation duration in seconds (3 to 120s).
- **Screen Transition Effects (8 options)**:
  - `None (Instant Cut)`: Direct instantaneous screen switch.
  - `Slide Left`: Smooth horizontal scroll to the left.
  - `Slide Right`: Smooth horizontal scroll to the right.
  - `Slide Up`: Vertical upward push animation.
  - `Slide Down`: Vertical downward push animation.
  - `Crossfade / Dissolve`: 10-step linear alpha blending with native 24-bit GRB double-buffering.
  - `Curtain (Center Split Wipe)`: Theatrical opening wipe from center outward.
  - `Random (Shuffle Effects)`: Automatically selects a different dynamic transition on each screen switch.
- **Screen Reorder List**: Move any screen up (`▲`) or down (`▼`) to customize the sequence (`car_ord`).
- **Screen Toggles**: Individual checkboxes to enable or disable any screen in the carousel.
- **Default / Active Screen**: Primary screen on device boot.
- **Battery Display Format**:
  - `Alternating`: Cycles between Percentage (e.g. `93%`) and Voltage (e.g. `4.12V`) every 3 seconds.
  - `Percentage Only`: Displays state of charge (e.g. `93%`).
  - `Voltage Only`: Displays cell voltage (e.g. `4.12V`).

---

## 🌐 Group 3: Data Services & Integrations

- **Real Outdoor Weather (Open-Meteo)**:
  - Predefined city selector covering Slovakia, Czechia, Europe, and Worldwide.
  - Automatic latitude and longitude population.
  - Live WMO weather code mapping to animated 8×8 icons.
- **Financial Markets & Crypto Tickers**:
  - Presets: Bitcoin (`BTC`), Ethereum (`ETH`), Solana (`SOL`), Binance Coin (`BNB`), Apple (`AAPL`), Tesla (`TSLA`), Nvidia (`NVDA`), S&P 500 (`SPX`), Gold Bullion (`GOLD`), `EUR/USD`, `USD/EUR`.
  - Custom ticker symbol input.
- **YouTube Channel Analytics**:
  - Channel ID or `@Handle`.
  - Google Cloud Data API v3 key.
- **Home Assistant & MQTT (Auto-Discovery & Push API)**:
  - Broker IP/hostname, port, credentials, and topic prefix (`ulanzi-tc001`).
  - 12 synchronized entities automatically exposed in Home Assistant.
  - **Custom App Push API**: Push custom screens (e.g. Spotify playing track, 3D printer) via MQTT `<prefix>/custom/<name>` or HTTP `POST /api/custom?name=<name>`.
  - **High-Priority Notifications**: Push alerts via `POST /api/notify` or `<prefix>/notify`.

---

## ⚙️ Group 4: Network & System

- **Wi-Fi Network Name (SSID)**: Target 2.4 GHz wireless network name.
- **Scan Nearby Networks**: Performs a non-blocking 802.11 b/g/n scan with RSSI and lock indicators.
- **Wi-Fi Password**: WPA/WPA2 passphrase.
- **Save & Apply Settings**: AJAX asynchronous submission with toast notification and real-time live preview update.

---

## 🚀 Firmware Update (OTA)

- **Running Version**: Displays active firmware version.
- **Local Binary Upload**: Direct `.bin` file flash via web browser (`/update`) with real-time percentage progress bar.
- **GitHub Cloud OTA**: 1-click update check (`/api/ota/check`) and automatic installation (`/api/ota/install`) from official GitHub releases.
