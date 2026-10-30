# Ulanzi TC001 Companion — Android Application Guide 📱

[![Release](https://img.shields.io/github/v/release/hackra76/Ulanzi-TC001-Companion?color=brightgreen&label=Companion%20Release)](https://github.com/hackra76/Ulanzi-TC001-Companion/releases/latest)
[![Platform](https://img.shields.io/badge/Platform-Android%208.0%2B%20(API%2026--36)-3DDC84?logo=android&logoColor=white)](https://github.com/hackra76/Ulanzi-TC001-Companion)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](https://github.com/hackra76/Ulanzi-TC001-Companion/blob/main/LICENSE)
[![GitHub Repo](https://img.shields.io/badge/GitHub-hackra76%2FUlanzi--TC001--Companion-black?logo=github)](https://github.com/hackra76/Ulanzi-TC001-Companion)

The **[Ulanzi TC001 Companion](https://github.com/hackra76/Ulanzi-TC001-Companion)** is the official, native Android application engineered to complement the **Ulanzi TC001 Evolved** firmware. Built with modern Android architecture (**Jetpack Compose**, **Material 3**, **Kotlin Coroutines & Flow**, **OkHttp**, and **DataStore Preferences**), it turns your Android smartphone into a handheld command deck, pixel art studio, and notification bridge for your smart pixel clock.

---

## 📥 Download & Installation

### Requirements
- **Operating System:** Android 8.0 (Oreo, API 26) or newer (fully optimized for Android 14, 15, and 16).
- **Network:** Connected to the same 2.4 GHz Wi-Fi local network (or phone hotspot) as the Ulanzi TC001 clock.

### Installation Steps
1. Navigate to the **[Ulanzi TC001 Companion Releases](https://github.com/hackra76/Ulanzi-TC001-Companion/releases/latest)** page on GitHub.
2. Download the signed production APK file: `Ulanzi-TC001-Companion-vX.Y.Z.apk`.
3. Open the downloaded file on your Android device. If prompted, grant your browser or file manager permission to **Install unknown apps**.
4. Tap **Install** and launch the application.

---

## 📡 Device Discovery & Connection

The Companion app provides zero-configuration discovery to seamlessly connect to your clock without requiring manual IP entry:

1. **mDNS Auto-Discovery:**
   - The app automatically queries the local network via mDNS for `_http._tcp` services and `ulanzi-tc001.local`.
   - When found, the device status pill at the top turns neon green (`CONNECTED`) displaying the clock's IP address, RSSI signal strength, and firmware version.
2. **Fast Subnet Sweep Scanner:**
   - If mDNS is filtered on your router, tap the **Scan Network** button.
   - The app runs a high-speed parallel coroutine sweep across your current `/24` subnet (probing port 80 `/status`) to locate the clock in seconds.
3. **Manual Host / IP Entry:**
   - You can also directly type your clock's IP address (e.g. `192.168.1.150`) or hostname into the connection bar.
4. **Persistent Auto-Reconnect:**
   - Using Android Jetpack DataStore Preferences, the app securely stores the last verified host IP and auto-reconnect flag.
   - Upon launching the app, it instantly reconnects in the background.

---

## 🎛️ Core Feature Hubs & Tabs

The application is structured into 5 dedicated hubs accessible via the bottom navigation bar:

```
┌─────────────────────────────────────────────────────────────┐
│  🟢 CONNECTED • 192.168.1.150  [-48 dBm]   FW: v2.2.0        │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│   [ 1. Live Matrix & Studio ]   [ 2. Controls & Screens ]   │
│   [ 3. Media & Spotify ]        [ 4. Push Alerts ]          │
│   [ 5. Hardware Setup & EEPROM ]                            │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

### 1. 🎨 Live Matrix & Interactive Pixel Canvas Studio

Experience full visual parity with your desk clock on your smartphone screen:

- **Sub-Second Matrix Mirroring:**
  - Dynamic 32×8 LED canvas mirrors the exact pixels currently illuminated on the clock via `GET /api/display` with sub-second polling.
- **Interactive Finger Drawing Studio:**
  - Tap or drag your finger across the 256-pixel matrix canvas to draw custom pixel art in real time.
  - **Pen Tool:** Draw individual pixels with crisp precision.
  - **Eraser Tool:** Clear pixels back to off (black).
  - **Flood-Fill Bucket:** Fill connected areas or the entire canvas with a single tap.
- **13 Curated Neon Retro Colors:**
  - Vibrant palette: Red, Orange, Amber Gold, Yellow, Lime Green, Forest Green, Cyan, Sky Blue, Royal Blue, Magenta, Soft Pink, Purple, and Pure White.
- **7 Instant Retro Presets:**
  - One-tap icons: **Love (Heart)**, **Pac-Man**, **Space Invaders**, **VU Equalizer**, **Rainbow Spectrum**, **Fire Flame**, and **Neon 80s Shades**.
- **Clone Clock Matrix:**
  - Tap **Clone Clock** to fetch the currently active screen from the physical clock directly into your drawing canvas for editing.
- **1-Tap Push to Matrix:**
  - Transmit your artwork directly to the clock display via `/api/custom` with selectable display durations (5s, 10s, 30s, 60s, or persistent).

---

### 2. 🎛️ Screen Switcher & Playback Deck

Take direct control of active display navigation:

- **9-Screen Direct Switcher:**
  - Instantly switch to any screen with a single tap:
    - `Screen 0`: Clock & Calendar
    - `Screen 1`: Indoor Environment (SHT30 Temperature & Humidity)
    - `Screen 2`: Outdoor Weather (Open-Meteo)
    - `Screen 3`: Financial Markets & Crypto Tickers
    - `Screen 4`: YouTube Analytics
    - `Screen 5`: Live Airspace Radar (ADS-B)
    - `Screen 6`: Battery & Energy Telemetry
    - `Screen 7`: Stopwatch & Countdown Timer
    - `Screen 8`: Retro Ambient Screensavers
- **Sequential Stepping:**
  - `Previous` and `Next` action buttons to step through screens along your custom carousel order.
- **Carousel Pause & Resume:**
  - Toggle auto-rotation carousel on or off on the fly without entering the web portal.

---

### 3. 🎵 Now Playing Media Sync & Spotify Live Broadcaster

Turn your Ulanzi TC001 into a dynamic desktop now-playing display:

- **Universal Media Player Integration:**
  - Powered by Android's `MediaSessionManager`, the app listens for active media playback across all modern Android audio apps:
    - **Spotify**, **YouTube Music**, **Apple Music**, **Tidal**, **SoundCloud**, **Pocket Casts**, **Audible**, and local media players.
- **Automatic Metadata Extraction:**
  - Intercepts track title, artist name, album name, total track duration, and current playback position.
- **Dynamic Brand-Adaptive Styling:**
  - Spotify sessions use official Spotify Green (`#1ED760`) with dedicated `spotify` 8×8 icon.
  - YouTube Music sessions adapt with YouTube Red accents.
  - Other media sources adapt with bright Cyan accents.
- **Automated Clock Sync Loop:**
  - Asynchronously streams updates to `POST /api/custom?name=spotify` with configurable polling intervals (3s, 5s, or 10s).
  - Renders a clean 1-pixel progress bar at row 7 displaying real-time song completion percentage (0–100%).
  - Text smoothly scrolls across rows 0–6 masked cleanly behind the pinned 8×8 left icon.
- **In-App Remote Transport Controls:**
  - Play, Pause, Skip Next, Skip Previous, and Instant Push buttons right inside the app.
- **Auto-Eviction on Pause:**
  - Optionally cleans up the custom screen automatically when music is paused via `DELETE /api/custom?name=spotify`.

---

### 4. 🚨 Push Alerts & Intelligent Notification Forwarding Relay

Never miss an urgent phone notification while focusing on your desk:

- **Android Notification Listener Service:**
  - Integrates with Android's secure `NotificationListenerService` to intercept system and app alerts in the background.
- **Supported Notification Sources:**
  - Phone calls & Missed calls
  - SMS & Text messages
  - WhatsApp, Telegram, Signal, Messenger
  - Google Calendar reminders & alarms
  - Custom third-party app notifications
- **Intelligent Flood Prevention & Debouncing:**
  - In-memory thread-safe 2.5-second debounce cache filters out duplicate rapid-fire notifications.
  - Ignores ongoing and persistent system notifications (such as foreground downloads or music notifications).
- **Contextual 8×8 Icons & Chimes:**
  - Maps incoming apps to specialized matrix pixel icons:
    - `bell`: General alerts & alarms
    - `mail`: Messages & emails
    - `clock`: Calendar events & reminders
    - `spotify`: Media alerts
    - `heart`: Loved ones & favorite contacts
    - `warning`: Urgent system warnings
  - Configurable piezo buzzer chime sound (Tone 0: Silent, Tone 1: Gentle beep, Tone 2: Alert chime, Tone 3: Urgent double chirp).
  - Configurable display duration (2 to 20 seconds) before automatically dismissing back to the carousel.
- **Live In-App Notification Feed:**
  - Real-time scrollable log inside the app showing recently intercepted and forwarded notifications with timestamps and dispatch status.

---

### 5. ⏱️ Remote Hardware Countdown Timer & Stopwatch

Remote control the clock's built-in chronometer engine via `/api/timer`:

- **Dual-Mode Chronometer:**
  - **Stopwatch Mode:** Counts up with tenths of seconds (`MM:SS.t`).
  - **Countdown Timer Mode:** Counts down from preset or custom duration (`MM:SS`) with dynamic bottom progress bar and matrix state badge.
- **Quick Preset Buttons:**
  - Instant one-tap presets: **1 min**, **3 min**, **5 min**, **10 min**, **15 min**, and **25 min Pomodoro**.
- **Full Remote Transport:**
  - Start, Pause, Toggle, and Reset actions.
  - Live digital readout simulation card showing remaining time and elapsed milliseconds synchronized with the clock.
- **Acoustic Alarm Dismissal:**
  - When a countdown timer finishes and sounds the piezo buzzer, tap **Dismiss Alarm** from your phone to immediately silence it.

---

### 6. 👾 Ambient Screensaver Remote Engine

Atmospheric visual ambiance at the touch of a button via `/api/screensaver`:

- **5 Ambient Procedural Animation Engines:**
  - **Matrix Digital Rain:** Cascading phosphor green code streams with white spark heads.
  - **Doom Fire / Thermal Lava:** Classic PSX/Doom molten heat propagation simulation.
  - **Starfield Warp:** Hyperspace star streaks.
  - **Pac-Man & Blinky:** 80s arcade chase with animated mouth chomping and energizer pellet.
  - **Conway's Game of Life:** Real-time cellular automata with age-based coloration.
- **Remote Controls:**
  - Trigger active screensaver immediately or exit back to standard rotation.
  - Select inactivity idle timeout (1, 2, 5, 10, 15, or 30 minutes).

---

### 7. 🎛️ Complete Hardware EEPROM Configuration Deck (100% Web Parity)

Configure all internal ESP32 NVS settings directly from the Android app without opening a web browser:

| Category | Available Settings in Companion App | Target API |
|---|---|---|
| **Display & Brightness** | LDR Auto-Brightness toggle, Day Brightness Ceiling (0–100%), Night Brightness (0–100%), Night Mode Quiet Hours Schedule (Start & End hours) | `POST /save` |
| **Clock & Date Layout** | Clock Mode (Time / Alternating / Date), Date Format (`DD.MM.`, `DD MMM`, `MM/DD`, `DAY DD`), Clock Icon (Calendar 1–31, Heart, Clock, None), Alignment (Left / Centered), Seconds switch, Weekday Bar, Week Start (Monday / Sunday), 11 Active/Inactive/Weekend colors | `POST /save` |
| **Reorderable Carousel** | 9-screen rotation sequence bitmask and auto-rotate interval (3 to 120 seconds) | `POST /save` |
| **Multi-Profile Wi-Fi Memory** | 4-slot stored network manager, live connection badges, 1-tap connect, 1-tap clear, and RF 2.4 GHz Wi-Fi scanner | `GET /api/wifi`, `POST /save` |
| **Data Services & APIs** | Weather City preset / GPS Latitude & Longitude, Financial Market Crypto/Stock symbol, YouTube Channel ID or @handle | `POST /save` |
| **Battery Telemetry** | Display mode: Percentage (`93%`), Voltage (`4.12V`), or Alternating (`%` and `V` every 3s) | `POST /save` |
| **Remote Reboot** | Trigger graceful hardware restart with confirmation prompt | `POST /api/reboot`, `POST /reboot` |

---

## 🔗 Firmware REST API Parity Mapping

The companion app communicates directly with the lightweight onboard REST API of Ulanzi TC001 Evolved:

| Companion Action / Screen | HTTP Method & URL | Payload / Parameters |
|---|---|---|
| **Telemetry & Status Sync** | `GET /status` | None (returns JSON with battery, temp, humidity, active screen, etc.) |
| **Live Matrix Mirror** | `GET /api/display` | None (returns 256 RGB hex color array) |
| **Switch Active Screen** | `GET /api/screen` | `?set=<0..8>&auto=<0\|1>` |
| **Send Canvas Pixel Art** | `POST /api/custom` | `?name=canvas` (body: JSON with text, icon, duration) |
| **Stream Spotify Media** | `POST /api/custom` | `?name=spotify` (body: JSON with title, artist, progress 0–100) |
| **Clear Media Screen** | `DELETE /api/custom` | `?name=spotify` |
| **Push Phone Notification** | `POST /api/notify` | JSON: `{"text": "...", "icon": "bell", "sound": 2, "duration": 6}` |
| **Stopwatch & Timer Control** | `POST /api/timer` | `action=start\|pause\|toggle\|reset`, `mode=stopwatch\|countdown`, `duration=<sec>` |
| **Screensaver Control** | `POST /api/screensaver` | `action=start\|stop`, `style=0..4`, `timeout=0..60` |
| **Scan Nearby Wi-Fi** | `GET /scan` | None (returns JSON list of nearby SSIDs & RSSI) |
| **Wi-Fi Multi-Profile List** | `GET /api/wifi` | None (returns 4 stored profile slots) |
| **Save Hardware Settings** | `POST /save` | Form URL-encoded matching ESP32 `NetworkManager` params |
| **Remote Reboot** | `POST /api/reboot` or `POST /reboot` | None (triggers non-blocking ESP32 reboot) |

---

## 💡 Troubleshooting & Best Practices

### 1. Notification Forwarding Not Working
- **Check Notification Access Permission:** Android requires explicit user consent for background notification interception. Go to **Android Settings > Apps > Special App Access > Device & app notifications > Ulanzi Companion** and ensure it is **Allowed**.
- **Battery Optimization Exemption:** Modern Android (Android 12+) aggressively freezes background services. In **Android Settings > Apps > Ulanzi Companion > App battery usage**, set to **Unrestricted** so notification relay remains active when the phone is locked.

### 2. Device Discovery Cannot Find the Clock
- **Wi-Fi Subnet Isolation:** Ensure your phone is connected to the same Wi-Fi SSID and subnet as the clock. If your router has "Client Isolation" or "Guest Network AP Isolation" enabled, devices cannot communicate directly.
- **Phone Hotspot Mode:** If using the clock on a portable mobile hotspot, the clock will connect directly to your phone's subnet (e.g. `192.168.43.x`). Enter the assigned client IP directly in the app.

---

## 📄 License & Attribution

The Ulanzi TC001 Companion Android app is open-source under the **MIT License**.
Source code and issue tracker: **[hackra76/Ulanzi-TC001-Companion](https://github.com/hackra76/Ulanzi-TC001-Companion)**.
