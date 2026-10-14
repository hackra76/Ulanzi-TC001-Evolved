# REST API & Captive Endpoints

The onboard web server exposes several REST endpoints for status monitoring, configuration, live display streaming, and OTA updating.

## Status Endpoint
- **URL**: `GET /status`
- **Response**: `application/json`
```json
{
  "state": "connected",
  "ip": "192.168.0.4",
  "ap_ip": "192.168.4.1",
  "ssid": "MyHomeNetwork",
  "rssi": -45,
  "rotate_sec": 8,
  "auto_rotate": true,
  "carousel_mask": 127,
  "active_app": 0,
  "firmware_version": "1.3.0",
  "temperature_in": 22.4,
  "humidity_in": 46.2,
  "battery_volts": 4.12,
  "battery_pct": 92,
  "ldr_raw": 1820,
  "auto_bri": true,
  "day_bri": 60,
  "night_bri": 9,
  "night_mode": true,
  "weather_temp": "14°C",
  "weather_city": "Prešov",
  "crypto_symbol": "GOLD",
  "crypto_price": "4.2K",
  "yt_subscribers": "843"
}
```

## Live Display Streaming & Screenshot Endpoints
- **URL**: `GET /api/display`
  - **Response**: `application/json`
  - Returns the live 8×32 matrix color buffer as an array of hex colors: `{"pixels": [["#000000", ...], ...]}`.
- **URL**: `GET /api/display/screenshot`
  - **Response**: `image/bmp`
  - Streams an instant 24-bit uncompressed standard BMP image file of the current NeoPixel matrix directly to the browser.
- **URL**: `GET /api/display/text`
  - **Response**: `text/plain`
  - Returns an ASCII art character representation of the active display buffer (`#` for lit pixels, `.` for off).

## Screen Control Endpoint
- **URL**: `GET /api/screen?set=<app_index>&auto=<0|1>`
  - **Parameters**:
    - `set`: Screen index `0` (Clock), `1` (Indoor Temp), `2` (Outdoor Weather), `3` (Crypto/Markets), `4` (YouTube), `5` (Airspace), `6` (Battery).
    - `auto`: `1` to enable carousel auto-rotation, `0` to pause.
  - **Response**: `{"active_app": 0, "auto_rotate": false, "app_name": "Clock"}`

## Wi-Fi Scan Endpoint
- **URL**: `GET /scan`
- **Response**: `application/json`
```json
[
  { "ssid": "MyHomeNetwork", "rssi": -48, "secure": true },
  { "ssid": "Guest_Network", "rssi": -72, "secure": false }
]
```

## Settings Save Endpoint
- **URL**: `POST /save`
- **Content-Type**: `application/x-www-form-urlencoded`
- **Parameters**:
  - `ssid`, `password`: Wi-Fi credentials
  - `auto_rotate`, `rotate`: Carousel interval
  - `car_ord`: Comma-separated screen sequence, e.g. `0,1,2,3,4,5,6`
  - `car_app_0` ... `car_app_6`: Individual screen enable flags
  - `bat_mode`: Battery format (`0`: Percentage, `1`: Voltage, `2`: Alternating every 3s)
  - `clk_mode`: Clock display mode (`0`: Time Only, `1`: Combined 5s/3s, `2`: Date Only)
  - `show_sec`: Show seconds flag (`1` or `0`, automatically hides icon for clean full-width centered layout)
  - `wd_bar`: Weekday bars flag (`1` or `0`)
  - `wd_mon`: Week start (`1` for Monday, `0` for Sunday)
  - `wd_acol`: Active weekday bar color index (0–5)
  - `wd_icol`: Inactive weekday bar color index (0–3)
  - `date_fmt`: Date format index (0–3)
  - `clk_icon`: Clock icon style index (0–3)
  - `txt_align`: Text alignment (`0`: Left-aligned with icon, `1`: Centered)
  - `auto_bri`, `night_mode`, `night_start`, `night_end`, `day_bri`, `night_bri`: Ambient settings
  - `w_lat`, `w_lon`, `w_city`: Open-Meteo GPS location
  - `c_sym`, `c_sym_custom`: Market / stock / crypto ticker symbol
  - `yt_channel`, `yt_key`: YouTube channel ID / handle and Data API v3 key
  - `ajax`: Set to `1` or pass `X-Requested-With: XMLHttpRequest` to receive JSON `{"status":"success"}`

## OTA Endpoints
- `POST /update`: Multipart binary file upload (`firmware.bin`)
- `GET /api/ota/check`: Checks latest tag and download URL from GitHub Releases
- `POST /api/ota/install`: Triggers background download and streaming flash from a provided asset URL
