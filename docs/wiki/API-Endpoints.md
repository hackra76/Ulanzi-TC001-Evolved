# REST API & Captive Endpoints

The onboard web server exposes several REST endpoints for status monitoring, configuration, and OTA updating.

## Status Endpoint
- **URL**: `GET /status`
- **Response**: `application/json`
```json
{
  "state": "connected",
  "ip": "192.168.1.150",
  "ap_ip": "192.168.4.1",
  "ssid": "MyHomeNetwork",
  "rssi": -55,
  "rotate_sec": 8,
  "firmware_version": "1.2.0",
  "temperature_in": 22.4,
  "humidity_in": 46.2,
  "battery_volts": 4.12,
  "battery_pct": 92,
  "ldr_raw": 1820,
  "auto_bri": true,
  "day_bri": 35,
  "night_bri": 2,
  "night_mode": true,
  "weather_temp": "18.5C",
  "weather_city": "Bratislava",
  "crypto_symbol": "BTC",
  "crypto_price": "68.5K",
  "yt_subscribers": "1.4M"
}
```

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
- **Parameters**: `ssid`, `password`, `rotate`, `auto_bri`, `night_mode`, `night_start`, `night_end`, `day_bri`, `night_bri`, `w_lat`, `w_lon`, `w_city`, `c_sym`, `yt_channel`, `yt_key`

## OTA Endpoints
- `POST /update`: Multipart binary file upload (`firmware.bin`)
- `GET /api/ota/check`: Checks latest tag and download URL from GitHub Releases
- `POST /api/ota/install`: Triggers background download and streaming flash from a provided asset URL
