# REST API & Captive Endpoints

The onboard web server exposes several lightweight REST endpoints for status monitoring, configuration, live display streaming, and OTA updating. These endpoints are consumed natively by both the **[Ulanzi TC001 Companion Android App](Android-Companion-App)** and the Web Configuration Portal, as well as Home Assistant and external automation scripts.

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
  "transition_fx": 1,
  "active_app": 0,
  "app_name": "Clock",
  "firmware_version": "2.5.0",
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
  "yt_subscribers": "843",
  "timer_mode": "countdown",
  "timer_state": "stopped",
  "timer_duration": 300,
  "timer_remaining": 300,
  "timer_elapsed_ms": 0,
  "screensaver_style": 0,
  "screensaver_timeout": 5,
  "screensaver_active": false
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


## Interactive Canvas Matrix Endpoint
- **URL**: `POST /api/display` or `POST /api/draw`
- **Content-Type**: `application/json`
- **Request Body**:
```json
{
  "pixels": [
    ["#FF0055", "#00E5FF", ... 32 hex colors],
    ... 8 rows
  ],
  "duration": 60
}
```
*Note: Also accepts flat 256 color arrays or Awtrix `draw` commands `{"draw": [{"dp": [x, y, color]}]}`.*
- **Response**: `{"status": "ok", "message": "Canvas rendered on matrix"}`
- **Dismissal**: Stays active for `duration` seconds, or dismisses instantly when user presses any physical hardware button (`OK`, `LEFT`, `RIGHT`) returning to normal carousel.

## Screen Control Endpoint
- **URL**: `GET /api/screen?set=<app_index>&auto=<0|1>`
  - **Parameters**:
    - `set`: Screen index `0` (Clock), `1` (Indoor Temp), `2` (Outdoor Weather), `3` (Crypto/Markets), `4` (YouTube), `5` (Airspace), `6` (Battery), `7` (Stopwatch & Timer), `8` (Ambient Screensaver).
    - `auto`: `1` to enable carousel auto-rotation, `0` to pause.
  - **Response**: `{"active_app": 0, "auto_rotate": false, "app_name": "Clock"}`

## Stopwatch & Countdown Timer REST API
- **URL**: `GET /api/timer` or `POST /api/timer`
- **Parameters**:
  - `action`: `start`, `pause`, `toggle`, `reset`
  - `mode`: `stopwatch` (or `0`), `countdown` (or `1`)
  - `duration`: Target duration in seconds for countdown mode (e.g. `300` for 5 minutes)
- **Response**: `application/json`
```json
{
  "mode": "countdown",
  "state": "running",
  "duration": 300,
  "remaining": 284,
  "elapsed_ms": 16000
}
```

## Retro Ambient Screensaver REST API
- **URL**: `GET /api/screensaver` or `POST /api/screensaver`
- **Parameters**:
  - `style`: Visual animation style:
    - `0`: Matrix Digital Rain
    - `1`: NeoPixel Fire / Lava
    - `2`: Pac-Man & Blinky Arcade Chase
    - `3`: Conway's Game of Life
    - `4`: Dynamic Shuffle (cycle all styles every 25s)
  - `timeout`: Inactivity auto-activation timeout in minutes (`0` = disabled, `1..60`)
  - `action`: `start` / `activate` (trigger immediately), `stop` / `deactivate` / `wake` (return to active screen)
- **Response**: `application/json`
```json
{
  "style": 0,
  "active_style": 0,
  "timeout": 5,
  "active": true,
  "idle_ms": 312000
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
  - `mqtt_en`, `mqtt_srv`, `mqtt_port`, `mqtt_user`, `mqtt_pass`, `mqtt_pre`: Home Assistant & MQTT broker configuration
  - `ajax`: Set to `1` or pass `X-Requested-With: XMLHttpRequest` to receive JSON `{"status":"success"}`

## Custom App Push API
- **URL**: `POST /api/custom?name=<app_name>`
- **Content-Type**: `application/json`
- **Request Body**:
```json
{
  "text": "Starboy - The Weeknd",
  "icon": "spotify",
  "color": [30, 215, 96],
  "progress": 50,
  "duration": 8,
  "lifetime": 60,
  "scroll": true
}
```
- **Response**: `{"status": "ok", "app": "spotify"}`
- **List Active Custom Apps**: `GET /api/custom`
- **Delete Custom App**: `DELETE /api/custom?name=<app_name>`

## Push Notification API
- **URL**: `POST /api/notify`
- **Content-Type**: `application/json`
- **Request Body**:
```json
{
  "text": "Doorbell Rang!",
  "icon": "bell",
  "sound": 2,
  "duration": 6
}
```
- **Response**: `{"status": "notified"}`

## MQTT Topics & Integration
- **Prefix**: `<prefix>/` (default: `ulanzi-tc001`)
- **Discovery**: `homeassistant/<component>/<node_id>/<object_id>/config` (supports 15 auto-discovered entities including diagnostics)
- **Telemetry**: `<prefix>/sensor/temperature`, `<prefix>/sensor/humidity`, `<prefix>/sensor/battery_percent`, `<prefix>/sensor/battery_voltage`, `<prefix>/sensor/illuminance`, `<prefix>/sensor/active_screen`, `<prefix>/sensor/wifi_rssi`, `<prefix>/sensor/free_heap`
- **Controls**: `<prefix>/light/set`, `<prefix>/light/brightness/set`, `<prefix>/switch/auto_brightness/set`, `<prefix>/switch/auto_rotate/set`, `<prefix>/switch/night_mode/set`, `<prefix>/select/screen/set`, `<prefix>/buzzer/set`, `<prefix>/sound/set`, `<prefix>/button/buzzer/set`, `<prefix>/button/reboot/set`
- **Audio Control via MQTT**: Send named presets (`"click"`, `"alert"`, `"success"`, `"notification"`, `"alarm"`, `"stop"`), numbers (`1` to `5`), RTTTL strings, or JSON objects (`{"rtttl":"..."}`, `{"freq":1000,"duration":150}`) to `<prefix>/buzzer/set` or `<prefix>/sound/set`.
- **Custom Apps**: Subscribe to `<prefix>/custom/+` (payloads match `/api/custom`)
- **Notifications**: Subscribe to `<prefix>/notify` (payloads match `/api/notify`)

## OTA Endpoints
- `POST /update`: Multipart binary file upload (`firmware.bin`)
- `GET /api/ota/check`: Checks latest tag and download URL from GitHub Releases
- `POST /api/ota/install`: Triggers background download and streaming flash from a provided asset URL

## Remote Hardware Reboot API
- **URL**: `POST /api/reboot` or `POST /reboot` (also supports `GET`)
- **Response**: `application/json`
```json
{
  "status": "rebooting",
  "message": "ESP32 rebooting now..."
}
```
*Initiates a graceful hardware restart 500 ms after flushing the HTTP 200 response.*

## Buzzer & Sound Melody API
- **URL**: `POST /api/buzzer` or `GET /api/buzzer` (also supports `/api/sound`)
- **Parameters / JSON Body**:
  - `sound`: Named preset or number (`"click"` / `1`, `"alert"` / `2`, `"success"` / `3`, `"notification"` / `4`, `"alarm"` / `5`)
  - `rtttl`: Standard Ring Tone Text Transfer Language string (e.g. `mario:d=4,o=5,b=100:16e6,16e6,32p,8e6,16c6,8e6,8g6,8p,8g`)
  - `freq` & `duration`: Custom tone frequency in Hz and duration in ms (e.g. `freq=2400&duration=80`)
  - `stop`: Set to `1` or `true` to immediately stop active playback.
- **Response**: `application/json`
```json
{
  "status": "ok",
  "sound": "success"
}
```

## Multi-Network Wi-Fi API
- **URL**: `GET /api/wifi`
- **Response**: `application/json`
```json
{
  "connected": true,
  "active_ssid": "MyHomeNetwork",
  "ip": "192.168.0.4",
  "rssi": -45,
  "profiles": [
    {"slot": 1, "ssid": "MyHomeNetwork", "has_pass": true, "connected": true},
    {"slot": 2, "ssid": "Office_WiFi", "has_pass": true, "connected": false},
    {"slot": 3, "ssid": "iPhone_Hotspot", "has_pass": true, "connected": false},
    {"slot": 4, "ssid": "", "has_pass": false, "connected": false}
  ]
}
```

## Configuration Backup & Restore API
### Export Backup
- **URL**: `GET /api/backup`
- **Headers**: Returns `Content-Disposition: attachment; filename="ulanzi_tc001_backup.json"`
- **Response**: `application/json`
```json
{
  "app": "Ulanzi TC001 Evolved",
  "version": "2.7.0",
  "backup_time": 104523,
  "settings": {
    "wifi_ssid": "MyHomeNetwork",
    "wifi_pass": "secret",
    "wifi_ssid_2": "Office_WiFi",
    "wifi_pass_2": "secret2",
    "wifi_ssid_3": "",
    "wifi_pass_3": "",
    "wifi_ssid_4": "",
    "wifi_pass_4": "",
    "auto_rot": true,
    "act_app": 0,
    "rot_sec": 8,
    "car_mask": 511,
    "car_ord": "0,1,2,3,4,5,6,7,8",
    "trans_fx": 1,
    "bat_mode": 2,
    "clk_mode": 1,
    "show_sec": false,
    "wd_bar": true,
    "wd_mon": true,
    "wd_acol": 0,
    "wd_icol": 0,
    "wd_wcol": 0,
    "ui_lang": "en",
    "date_fmt": 0,
    "clk_icon": 0,
    "txt_align": 0,
    "auto_bri": true,
    "night_en": true,
    "night_st": 23,
    "night_end": 7,
    "day_bri": 35,
    "ngt_bri": 2,
    "ss_style": 0,
    "ss_time": 0,
    "w_lat": 48.1486,
    "w_lon": 17.1077,
    "w_city": "Bratislava",
    "c_sym": "BTC",
    "yt_chan": "",
    "yt_key": "",
    "mqtt_en": false,
    "mqtt_srv": "",
    "mqtt_port": 1883,
    "mqtt_user": "",
    "mqtt_pass": "",
    "mqtt_pre": "ulanzi-tc001"
  }
}
```

### Restore Backup
- **URL**: `POST /api/restore`
- **Content-Type**: `application/json`
- **Body**: Exported backup JSON or flat settings object
- **Response**: `application/json`
```json
{
  "status": "ok",
  "message": "Settings restored successfully"
}
```
*Applies all received parameters to NVS flash memory, reconfigures active services dynamically, and sounds an acoustic confirmation click.*
