# Web Configuration Portal

The web configuration portal is hosted on port 80 of the device's IP address (`http://<device-ip>/`) or `http://192.168.4.1/` when in SoftAP setup mode.

## Sections

### 1. Live Display Stream & Virtual Matrix
- **NeoPixel Mirror**: 32×8 canvas rendering the exact pixel colors displayed on the clock in real-time (auto-refreshes every 1.5 seconds).
- **Capture BMP**: One-click download of a standard uncompressed 24-bit `.bmp` screenshot directly from the device.
- **Prev / Next**: Interactive controls to step through active screens directly from the browser.

### 2. Wi-Fi Network
- **SSID**: Target 2.4 GHz wireless network name.
- **Password**: WPA/WPA2/WPA3-Personal passphrase.
- **Scan Nearby Networks**: Performs a non-blocking 802.11 b/g/n channel scan and lists SSIDs with signal strength (RSSI) and security flags.

### 3. Ambient Intelligence & Display
- **Auto-Brightness via LDR**: Reads the phototransistor, smooths light fluctuations, and dynamically adapts matrix brightness.
- **Day Brightness Ceiling**: Slider range: `5%` to `100%`.
- **Night Mode**: Enables quiet mode during defined schedule hours (e.g. 23:00 – 07:00).
- **Night Mode Brightness**: Slider range: `1%` to `25%`.

### 4. Screen Carousel & Reordering
- **Auto-Rotate Screens**: Toggles automatic carousel rotation on or off.
- **Carousel Cycle Interval**: Configurable rotation duration in seconds (3 to 120s).
- **Screen Reorder List**: Move any screen up (`▲`) or down (`▼`) to customize the sequence. The order is stored in NVS (`car_ord`) and respected by both the carousel timer and the hardware left/right buttons.
- **Screen Toggles**: Individual checkboxes to enable or disable any screen in the carousel.
- **Default / Active Screen**: Dropdown to select the primary boot and active screen.

### 5. Battery & Energy Telemetry
- **Battery Display Format**:
  - `Alternating`: Automatically cycles between Percentage (e.g. `93%`) and Voltage (e.g. `4.12V`) every 3 seconds.
  - `Percentage Only`: Displays state of charge (e.g. `93%`).
  - `Voltage Only`: Displays cell voltage (e.g. `4.12V`).
- Rendered with glowing neon yellow/white lightning bolt (`ICON_LIGHTNING`).

### 6. Clock & Date Layout (Awtrix Style)
- **Clock Display Mode**: Time Only, Combined Time & Date (alternating 5s/3s), or Date Only.
- **Show Seconds**: When enabled, the clock icon is automatically hidden to allow full-width centered `HH:MM:SS` with colons and weekday bars to fit cleanly without crowding or clipping.
- **7 Weekday Bars**: Horizontal bar indicators under time.
- **Week Starts On**: Monday (ISO 8601) or Sunday (US Standard).
- **Active / Inactive Day Colors**: Cyan, Orange, Green, Yellow, Red, White, Dim Slate, Dark Gray, Dim Amber, Off.
- **Date Format**: `DD.MM.`, `DD MMM`, `MM/DD`, `DAY DD`.
- **Clock Icon Style**: Dynamic calendar with day of month (1–31), Heart, Clock Face, or None.
- **Text Alignment**: Left-aligned with icon (Awtrix default) or Centered.

### 7. Real Outdoor Weather
- **Predefined City Presets**: Dropdown covering cities across Slovakia, Czechia, Europe, and Worldwide that automatically populates latitude, longitude, and city name.
- **Live Open-Meteo Integration**: Zero API key required.

### 8. Financial Markets & Crypto Tickers
- **Popular Asset / Index Presets**:
  - Cryptocurrencies: `BTC`, `ETH`, `SOL`, `BNB`.
  - Stocks & Indices: Apple (`AAPL`), Tesla (`TSLA`), Nvidia (`NVDA`), S&P 500 (`SPX`).
  - Forex & Commodities: Gold Bullion (`GOLD`), `EUR/USD`, `USD/EUR`.
- **Custom Ticker Symbol**: Text input for any symbol or commodity.

### 9. YouTube Channel Analytics
- **Channel ID / Handle**: Supports `@Handle` or raw `UCxxxxxxxx` channel IDs.
- **API Key**: Google Cloud Data API v3 key.
- **Icon**: Redesigned bold, centered white triangle play button.

### 10. Home Assistant & MQTT Integration
- **Enable MQTT Client**: Connects to your local MQTT broker (e.g. Home Assistant Mosquitto).
- **Broker Host & Port**: IP address or hostname and port (default `1883`).
- **Credentials**: Optional username and password.
- **Topic Prefix**: Root topic prefix (default `ulanzi-tc001`).
- **Auto-Discovery**: Automatically exposes 12 entities to Home Assistant.
- **Custom Apps & Notifications**: Push custom payloads via MQTT `<prefix>/custom/<name>` or `<prefix>/notify`.

### 11. Instant AJAX Save & Apply
- Clicking **Save & Apply Settings** executes an AJAX `fetch('/save')` in the background.
- A floating toast notification (`✓ Settings Saved Automatically!`) displays confirmation without leaving or reloading the page.
- The virtual matrix stream updates instantly with the applied settings.
- Includes automatic fallback to standard POST with 1-second meta-refresh.

### 12. Firmware Update (OTA)
- **Local Flash**: File input for `.bin` files with live percentage progress bar.
- **GitHub Cloud Update**: One-click check and update from GitHub Releases.
