# Web Configuration Portal

The web configuration portal is hosted on port 80 of the device's IP address (`http://<device-ip>/`).

## Sections

### 1. Wi-Fi Network
- **SSID**: Target 2.4 GHz wireless network name.
- **Password**: WPA/WPA2/WPA3-Personal passphrase.
- **Scan Nearby Networks**: Performs a non-blocking 802.11 b/g/n channel scan and lists SSIDs with signal strength (RSSI) and security flags.

### 2. Ambient Intelligence & Display
- **Auto-Brightness via LDR**: Reads the phototransistor, smooths light fluctuations, and dynamically adapts matrix brightness.
- **Day Brightness Ceiling**: Slider range: `5%` to `100%` (Default: `35%`).
- **Night Mode**: Enables quiet mode during defined schedule hours (e.g. 23:00 – 07:00).
- **Night Mode Brightness**: Slider range: `1%` to `25%` (Default: `2%`).
- **Carousel Rotate Delay**: Interval in seconds between automatic screen rotations (3 to 120 seconds).

### 3. Real Outdoor Weather
- **Predefined City Presets**: Dropdown covering cities across Slovakia (Bratislava, Košice, Banská Bystrica, Žilina, etc.), Czechia (Praha, Brno, Ostrava, etc.), Europe (Vienna, Budapest, Warsaw, Berlin, London, Paris, Rome), and Worldwide (New York, Tokyo, Sydney).
- **Coordinates**: Automatically populated latitude and longitude.

### 4. Crypto Ticker
- **Cryptocurrency Pair**: Select between `BTC`, `ETH`, `SOL`, or `BNB`. Fetches live spot market prices from Binance.

### 5. YouTube Channel Analytics
- **Channel ID / Handle**: Supports `@Handle` or raw `UCxxxxxxxx` channel IDs.
- **API Key**: Google Cloud Data API v3 key.

### 6. Firmware Update (OTA)
- **Local Flash**: File input for `.bin` files with live percentage bar.
- **GitHub Cloud Update**: One-click check and update from GitHub Releases.
