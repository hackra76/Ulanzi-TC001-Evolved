#include "NetworkManager.h"
#include <ArduinoJson.h>
#include "YouTubeService.h"
#include "WeatherService.h"
#include "CryptoService.h"
#include "Sensors.h"
#include "OtaManager.h"
#include "DisplayMatrix.h"
#include "MqttService.h"
#include "CustomAppService.h"

static Preferences s_prefs;
static WebServer s_server(80);
static DNSServer s_dnsServer;

static NetworkState s_netState = NET_STATE_IDLE;
static String s_ssid = "";
static String s_pass = "";
static String s_apSSID = "";
static IPAddress s_apIP(192, 168, 4, 1);
static IPAddress s_apNetmask(255, 255, 255, 0);

static bool s_autoRotate = true;
static uint8_t s_activeApp = 0;
static uint16_t s_rotateSec = DEFAULT_ROTATE_SEC;
static uint8_t s_carouselMask = 0x7F; // Bitmask of enabled carousel screens (default: all 7 active)
static uint8_t s_carouselOrder[7] = {0, 1, 2, 3, 4, 5, 6}; // Screen display sequence
static uint8_t s_transitionFx = DEFAULT_TRANSITION_FX; // Screen transition animation effect
static uint8_t s_batteryMode = 2; // 0: Percentage, 1: Voltage, 2: Alternating
static String s_ytChannel = "";
static String s_ytApiKey = "";

// Home Assistant & MQTT
static bool s_mqttEn = false;
static String s_mqttServer = "";
static uint16_t s_mqttPort = DEFAULT_MQTT_PORT;
static String s_mqttUser = "";
static String s_mqttPass = "";
static String s_mqttPrefix = DEFAULT_MQTT_PREFIX;

// Ambient & Screen
static bool s_autoBri = true;
static bool s_nightMode = true;
static uint8_t s_nightStart = DEFAULT_NIGHT_START;
static uint8_t s_nightEnd = DEFAULT_NIGHT_END;
static uint8_t s_dayBri = DEFAULT_DAY_BRI;
static uint8_t s_nightBri = DEFAULT_NIGHT_BRI;

// Weather
static float s_wLat = DEFAULT_WEATHER_LAT;
static float s_wLon = DEFAULT_WEATHER_LON;
static String s_wCity = DEFAULT_WEATHER_CITY;

// Crypto
static String s_cryptoSym = DEFAULT_CRYPTO_SYM;

// Clock, Date & Layout Settings (Awtrix Style)
static uint8_t s_clockMode = 1;       // 0: Time Only, 1: Combined, 2: Date Only
static bool s_showSeconds = false;    // Option to show seconds in time display
static bool s_weekdayBar = true;      // 7 bars under time
static bool s_weekdayStartMon = true; // Start Monday
static uint8_t s_weekdayActiveColIdx = 0;   // 0: Cyan, 1: Orange, 2: Green, 3: Yellow, 4: Red, 5: White
static uint8_t s_weekdayInactColIdx = 0;    // 0: Dim Slate, 1: Dark Gray, 2: Dim Amber, 3: Off
static uint8_t s_dateFormat = 0;      // 0: DD.MM., 1: DD MMM, 2: MM/DD, 3: DAY DD
static uint8_t s_clockIcon = 0;       // 0: Dynamic Calendar, 1: Heart, 2: Clock Face, 3: None
static uint8_t s_textAlign = 0;       // 0: Left-Aligned with Icon (Awtrix), 1: Centered

static const uint16_t s_wdActiveColors[] = {
    0x07FF, // 0: Cyan (#00F0FF)
    0xFD20, // 1: Orange (#FF8800)
    0x07E0, // 2: Green (#00FF00)
    0xFFE0, // 3: Yellow (#FFFF00)
    0xF800, // 4: Red (#FF2200)
    0xFFFF  // 5: White (#FFFFFF)
};

static const uint16_t s_wdInactiveColors[] = {
    0x0000, // 0: Auto (computed dynamically from active color at ~65% intensity)
    0x7432, // 1: Slate Gray (#708090)
    0x9342, // 2: Dim Amber (#906810)
    0x0000  // 3: Off (Black)
};

static uint32_t s_connectStartMs = 0;
static uint32_t s_lastWifiCheckMs = 0;
static bool s_pendingReconnect = false;
static uint32_t s_reconnectTriggerMs = 0;

static String buildPortalHtml() {
    String html = F(R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Ulanzi TC001 Evolved &bull; Settings Portal</title>
<style>
  :root {
    --bg: #090d16;
    --card: rgba(22, 30, 49, 0.92);
    --border: rgba(255, 255, 255, 0.1);
    --accent: #38bdf8;
    --accent-glow: rgba(56, 189, 248, 0.25);
    --text: #f1f5f9;
    --text-muted: #94a3b8;
  }
  * { box-sizing: border-box; margin: 0; padding: 0; }
  body {
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
    background: var(--bg);
    color: var(--text);
    min-height: 100vh;
    display: flex;
    align-items: center;
    justify-content: center;
    padding: 1.5rem 1rem;
    background-image: radial-gradient(circle at 50% 0%, #1e293b 0%, #090d16 100%);
  }
  .card {
    width: 100%;
    max-width: 480px;
    background: var(--card);
    border: 1px solid var(--border);
    border-radius: 1.25rem;
    padding: 2rem;
    backdrop-filter: blur(14px);
    box-shadow: 0 20px 40px rgba(0,0,0,0.5), 0 0 40px var(--accent-glow);
  }
  .header { text-align: center; margin-bottom: 1.5rem; }
  .badge {
    display: inline-block;
    padding: 0.25rem 0.75rem;
    font-size: 0.75rem;
    font-weight: 700;
    text-transform: uppercase;
    letter-spacing: 0.05em;
    color: var(--accent);
    background: rgba(56, 189, 248, 0.12);
    border: 1px solid rgba(56, 189, 248, 0.3);
    border-radius: 9999px;
    margin-bottom: 0.5rem;
  }
  h1 { font-size: 1.45rem; font-weight: 700; margin-bottom: 0.25rem; }
  p.sub { font-size: 0.85rem; color: var(--text-muted); }
  .section-title {
    font-size: 0.8rem;
    font-weight: 700;
    text-transform: uppercase;
    letter-spacing: 0.05em;
    color: var(--accent);
    margin: 1.25rem 0 0.75rem 0;
    padding-bottom: 0.35rem;
    border-bottom: 1px solid var(--border);
  }
  .field { margin-bottom: 1rem; }
  .row { display: flex; gap: 0.75rem; }
  .row .field { flex: 1; }
  label {
    display: block;
    font-size: 0.8rem;
    font-weight: 600;
    margin-bottom: 0.35rem;
    color: var(--text-muted);
  }
  .chk-label {
    display: flex;
    align-items: center;
    gap: 0.6rem;
    font-size: 0.85rem;
    color: var(--text);
    cursor: pointer;
    margin-bottom: 0.6rem;
  }
  .chk-label input[type="checkbox"] {
    width: 1.1rem;
    height: 1.1rem;
    accent-color: var(--accent);
    cursor: pointer;
  }
  input[type="text"], input[type="password"], input[type="number"], select {
    width: 100%;
    padding: 0.75rem 0.85rem;
    background: rgba(15, 23, 42, 0.8);
    border: 1px solid var(--border);
    border-radius: 0.65rem;
    color: var(--text);
    font-size: 0.9rem;
    outline: none;
    transition: all 0.2s ease;
  }
  input[type="range"] {
    width: 100%;
    height: 6px;
    background: rgba(15, 23, 42, 0.9);
    border-radius: 9999px;
    outline: none;
    accent-color: var(--accent);
    cursor: pointer;
    margin: 0.5rem 0;
  }
  input:focus, select:focus {
    border-color: var(--accent);
    box-shadow: 0 0 0 3px var(--accent-glow);
  }
  .scan-btn {
    display: inline-flex;
    align-items: center;
    gap: 0.4rem;
    background: transparent;
    border: 1px solid var(--border);
    color: var(--accent);
    padding: 0.4rem 0.85rem;
    border-radius: 0.5rem;
    font-size: 0.75rem;
    font-weight: 600;
    cursor: pointer;
    margin-top: 0.4rem;
    transition: background 0.2s;
  }
  .scan-btn:hover { background: rgba(56, 189, 248, 0.12); }
  .btn-submit {
    width: 100%;
    padding: 0.85rem;
    margin-top: 1.5rem;
    background: linear-gradient(135deg, #0284c7 0%, #38bdf8 100%);
    border: none;
    border-radius: 0.75rem;
    color: #fff;
    font-size: 0.95rem;
    font-weight: 700;
    cursor: pointer;
    box-shadow: 0 8px 20px rgba(56, 189, 248, 0.35);
  }
  .btn-submit:hover { opacity: 0.95; }
  #toast {
    position: fixed;
    top: 1.25rem;
    left: 50%;
    transform: translateX(-50%);
    background: #0284c7;
    color: white;
    padding: 0.65rem 1.4rem;
    border-radius: 9999px;
    box-shadow: 0 10px 30px rgba(0,0,0,0.6), 0 0 20px rgba(56,189,248,0.4);
    z-index: 1000;
    font-weight: 700;
    font-size: 0.85rem;
    display: none;
    transition: opacity 0.3s;
    pointer-events: none;
  }
  .reorder-list {
    display: flex;
    flex-direction: column;
    gap: 0.35rem;
    margin-bottom: 0.85rem;
  }
  .reorder-item {
    display: flex;
    align-items: center;
    justify-content: space-between;
    background: rgba(15, 23, 42, 0.7);
    border: 1px solid var(--border);
    border-radius: 0.6rem;
    padding: 0.45rem 0.75rem;
    transition: background 0.2s;
  }
  .reorder-item:hover {
    background: rgba(30, 41, 59, 0.85);
  }
  .reorder-btns {
    display: flex;
    gap: 0.25rem;
  }
  .btn-sm {
    background: rgba(255, 255, 255, 0.08);
    border: 1px solid var(--border);
    color: var(--text);
    border-radius: 0.35rem;
    width: 1.7rem;
    height: 1.7rem;
    display: flex;
    align-items: center;
    justify-content: center;
    cursor: pointer;
    font-size: 0.75rem;
    transition: all 0.2s;
  }
  .btn-sm:hover {
    background: var(--accent);
    color: #000;
    border-color: var(--accent);
  }
  .footer { text-align: center; margin-top: 1.25rem; font-size: 0.75rem; color: var(--text-muted); }
</style>
</head>
<body>
<div id="toast"></div>
<div class="card">
  <div class="header">
    <div class="badge">Pixel Clock Setup</div>
    <h1>Ulanzi TC001 Evolved</h1>
    <p class="sub">Configure Network, Ambient Screen, Weather, Tickers & Firmware</p>
  </div>

  <div style="background:rgba(15,23,42,0.9); border:1px solid rgba(56,189,248,0.3); border-radius:1rem; padding:1rem; margin-bottom:1.5rem; text-align:center; box-shadow:0 0 25px rgba(56,189,248,0.15);">
    <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:0.6rem;">
      <span style="font-size:0.75rem; font-weight:700; color:var(--accent); text-transform:uppercase; letter-spacing:0.05em;">&#x1F4FA; Live Display Stream</span>
      <a href="/api/display/screenshot" target="_blank" download="tc001_screenshot.bmp" style="font-size:0.75rem; color:var(--accent); text-decoration:none; border:1px solid rgba(56,189,248,0.4); padding:0.2rem 0.5rem; border-radius:9999px;">&#x1F4F7; Capture BMP</a>
    </div>
    <div style="overflow-x:auto; padding:4px 0;">
      <canvas id="virtualMatrix" width="320" height="80" style="background:#05070c; border-radius:6px; box-shadow:inset 0 0 10px #000; max-width:100%; height:auto; display:inline-block; vertical-align:middle;"></canvas>
    </div>
    <div style="display:flex; justify-content:center; align-items:center; gap:0.5rem; margin-top:0.6rem;">
      <button type="button" class="scan-btn" style="padding:0.25rem 0.6rem; margin:0;" onclick="switchScreen(-1)">&larr; Prev</button>
      <span id="screenNameBadge" style="font-size:0.75rem; font-weight:600; color:var(--text); min-width:140px; text-align:center;">{{APP_NAME}}</span>
      <button type="button" class="scan-btn" style="padding:0.25rem 0.6rem; margin:0;" onclick="switchScreen(1)">Next &rarr;</button>
    </div>
    <div style="font-size:0.7rem; color:var(--text-muted); margin-top:0.4rem;">Real-time 32&times;8 NeoPixel mirror &bull; Auto-refresh 1.5s</div>
  </div>

  <form id="settingsForm" action="/save" method="POST">
    
    <div class="section-title">&#x1F4F6; Wi-Fi Network</div>
    <div class="field">
      <label for="ssid">Network Name (SSID)</label>
      <input type="text" id="ssid" name="ssid" value="{{SSID}}" placeholder="Enter Wi-Fi SSID" required autocomplete="off">
      <button type="button" class="scan-btn" onclick="scanNetworks()">&#x21bb; Scan Nearby Networks</button>
      <select id="scanSelect" style="display:none; margin-top:0.4rem;" onchange="selectNetwork(this.value)">
        <option value="">-- Detected Networks --</option>
      </select>
    </div>
    <div class="field">
      <label for="password">Wi-Fi Password</label>
      <input type="password" id="password" name="password" placeholder="Leave empty if unchanged">
    </div>

    <div class="section-title">&#x1F319; Ambient Intelligence & Display</div>
    <label class="chk-label">
      <input type="checkbox" name="auto_bri" value="1" {{AUTO_BRI_CHK}}>
      Auto-Brightness via LDR Light Sensor
    </label>
    
    <div class="field">
      <div style="display:flex; justify-content:space-between; margin-bottom:0.25rem;">
        <label for="day_bri" style="margin:0;">Day Brightness Ceiling</label>
        <span id="day_bri_val" style="color:var(--accent); font-weight:700;">{{DAY_BRI}}%</span>
      </div>
      <input type="range" id="day_bri" name="day_bri" min="5" max="100" value="{{DAY_BRI}}" oninput="document.getElementById('day_bri_val').innerText = this.value + '%'">
    </div>

    <label class="chk-label" style="margin-top:0.75rem;">
      <input type="checkbox" name="night_mode" value="1" {{NIGHT_MODE_CHK}}>
      Night Mode (Quiet Dim Red Clock)
    </label>

    <div class="field">
      <div style="display:flex; justify-content:space-between; margin-bottom:0.25rem;">
        <label for="night_bri" style="margin:0;">Night Mode Brightness</label>
        <span id="night_bri_val" style="color:var(--accent); font-weight:700;">{{NIGHT_BRI}}%</span>
      </div>
      <input type="range" id="night_bri" name="night_bri" min="1" max="25" value="{{NIGHT_BRI}}" oninput="document.getElementById('night_bri_val').innerText = this.value + '%'">
    </div>

    <div class="row">
      <div class="field">
        <label for="night_start">Night Start Hour</label>
        <input type="number" id="night_start" name="night_start" min="0" max="23" value="{{NIGHT_START}}">
      </div>
      <div class="field">
        <label for="night_end">Night End Hour</label>
        <input type="number" id="night_end" name="night_end" min="0" max="23" value="{{NIGHT_END}}">
      </div>
    </div>

    <div class="section-title">&#x1F3A0; Screen Carousel & Navigation</div>
    <label class="chk-label">
      <input type="checkbox" id="auto_rotate" name="auto_rotate" value="1" {{AUTO_ROTATE_CHK}} onchange="document.getElementById('rotate_delay_field').style.display = this.checked ? 'block' : 'none'">
      Auto-Rotate Screens (Carousel)
    </label>

    <div style="font-size:0.8rem; font-weight:600; color:var(--text-muted); margin:0.75rem 0 0.35rem 0;">Screens in Carousel (Check to Enable &bull; Use &#x25B2;&#x25BC; to Reorder):</div>
    <input type="hidden" id="car_ord" name="car_ord" value="{{CAR_ORD}}">
    <div class="reorder-list" id="carouselScreens">
      {{REORDER_ITEMS}}
    </div>

    <div class="field">
      <label for="active_app">Default / Active Screen</label>
      <select id="active_app" name="active_app">
        <option value="0" {{APP_0_SEL}}>Clock (Heart / Time)</option>
        <option value="1" {{APP_1_SEL}}>Indoor Environment (SHT30 Temp)</option>
        <option value="2" {{APP_2_SEL}}>Outdoor Weather (Open-Meteo)</option>
        <option value="3" {{APP_3_SEL}}>Crypto & Financial Markets</option>
        <option value="4" {{APP_4_SEL}}>YouTube Subscriber Count</option>
        <option value="5" {{APP_5_SEL}}>Live Airspace Radar (ADS-B)</option>
        <option value="6" {{APP_6_SEL}}>Battery & Energy Telemetry</option>
      </select>
    </div>

    <div class="field" id="rotate_delay_field" style="{{ROTATE_DELAY_DISPLAY}}">
      <label for="rotate">Carousel Cycle Interval (seconds, 3 - 120)</label>
      <input type="number" id="rotate" name="rotate" min="3" max="120" value="{{ROTATE}}" required>
    </div>

    <div class="field">
      <label for="trans_fx">Screen Transition Effect</label>
      <select id="trans_fx" name="trans_fx">
        <option value="0" {{TRANS_0_SEL}}>None (Instant Cut)</option>
        <option value="1" {{TRANS_1_SEL}}>Slide Left</option>
        <option value="2" {{TRANS_2_SEL}}>Slide Right</option>
        <option value="3" {{TRANS_3_SEL}}>Slide Up</option>
        <option value="4" {{TRANS_4_SEL}}>Slide Down</option>
        <option value="5" {{TRANS_5_SEL}}>Crossfade / Dissolve</option>
        <option value="6" {{TRANS_6_SEL}}>Curtain (Center Split Wipe)</option>
        <option value="7" {{TRANS_7_SEL}}>Random (Shuffle Effects)</option>
      </select>
    </div>

    <div class="section-title">&#x26A1; Battery & Energy Telemetry</div>
    <div class="field">
      <label for="bat_mode">Battery Display Format</label>
      <select id="bat_mode" name="bat_mode">
        <option value="2" {{BAT_MODE_2_SEL}}>Alternating (Percentage &bull; Voltage every 3s)</option>
        <option value="0" {{BAT_MODE_0_SEL}}>Percentage Only (e.g. 94%)</option>
        <option value="1" {{BAT_MODE_1_SEL}}>Voltage Only (e.g. 4.12V)</option>
      </select>
    </div>

    <div class="section-title">&#x23F0; Clock & Date Layout (Awtrix Style)</div>
    
    <div class="field">
      <label for="clk_mode">Clock Display Mode</label>
      <select id="clk_mode" name="clk_mode">
        <option value="0" {{CLK_MODE_0_SEL}}>Time Only</option>
        <option value="1" {{CLK_MODE_1_SEL}}>Combined Time & Date (Alternating)</option>
        <option value="2" {{CLK_MODE_2_SEL}}>Date Only</option>
      </select>
    </div>

    <label class="chk-label">
      <input type="checkbox" name="show_sec" value="1" {{SHOW_SEC_CHK}}>
      Show Seconds in Time Display
    </label>

    <label class="chk-label">
      <input type="checkbox" name="wd_bar" value="1" {{WD_BAR_CHK}}>
      Show 7 Weekday Bars Under Time
    </label>

    <div class="row">
      <div class="field">
        <label for="wd_mon">Week Starts On</label>
        <select id="wd_mon" name="wd_mon">
          <option value="1" {{WD_MON_1_SEL}}>Monday (ISO 8601)</option>
          <option value="0" {{WD_MON_0_SEL}}>Sunday (US Standard)</option>
        </select>
      </div>
      <div class="field">
        <label for="wd_acol">Active Day Color</label>
        <select id="wd_acol" name="wd_acol">
          <option value="0" {{WD_ACOL_0_SEL}}>Cyan (#00F0FF)</option>
          <option value="1" {{WD_ACOL_1_SEL}}>Orange (#FF8800)</option>
          <option value="2" {{WD_ACOL_2_SEL}}>Green (#00FF00)</option>
          <option value="3" {{WD_ACOL_3_SEL}}>Yellow (#FFFF00)</option>
          <option value="4" {{WD_ACOL_4_SEL}}>Red (#FF2200)</option>
          <option value="5" {{WD_ACOL_5_SEL}}>White (#FFFFFF)</option>
        </select>
      </div>
    </div>

    <div class="row">
      <div class="field">
        <label for="wd_icol">Inactive Day Color</label>
        <select id="wd_icol" name="wd_icol">
          <option value="0" {{WD_ICOL_0_SEL}}>Dim Active Color (Auto - Slightly Dimmer)</option>
          <option value="1" {{WD_ICOL_1_SEL}}>Slate Gray (#708090)</option>
          <option value="2" {{WD_ICOL_2_SEL}}>Dim Amber (#906810)</option>
          <option value="3" {{WD_ICOL_3_SEL}}>Off (Black)</option>
        </select>
      </div>
      <div class="field">
        <label for="date_fmt">Date Format</label>
        <select id="date_fmt" name="date_fmt">
          <option value="0" {{DATE_FMT_0_SEL}}>DD.MM. (e.g. 14.10.)</option>
          <option value="1" {{DATE_FMT_1_SEL}}>DD MMM (e.g. 14 OCT)</option>
          <option value="2" {{DATE_FMT_2_SEL}}>MM/DD (e.g. 10/14)</option>
          <option value="3" {{DATE_FMT_3_SEL}}>DAY DD (e.g. WED 14)</option>
        </select>
      </div>
    </div>

    <div class="row">
      <div class="field">
        <label for="clk_icon">Clock Icon Style</label>
        <select id="clk_icon" name="clk_icon">
          <option value="0" {{CLK_ICON_0_SEL}}>Dynamic Calendar (Date 1-31)</option>
          <option value="1" {{CLK_ICON_1_SEL}}>Heart (Classic)</option>
          <option value="2" {{CLK_ICON_2_SEL}}>Clock Face</option>
          <option value="3" {{CLK_ICON_3_SEL}}>None (Full Width)</option>
        </select>
      </div>
      <div class="field">
        <label for="txt_align">Text Alignment</label>
        <select id="txt_align" name="txt_align">
          <option value="0" {{TXT_ALIGN_0_SEL}}>Left-Aligned with Icon (Awtrix)</option>
          <option value="1" {{TXT_ALIGN_1_SEL}}>Centered in Window</option>
        </select>
      </div>
    </div>

    <div class="section-title">&#x26C5; Real Outdoor Weather (Open-Meteo)</div>
    <div class="field">
      <label for="city_preset">Select Predefined City</label>
      <select id="city_preset" onchange="applyCityPreset(this.value)">
        <option value="">-- Choose City Preset --</option>
        <optgroup label="Slovakia">
          <option value="Bratislava|48.1486|17.1077">Bratislava</option>
          <option value="Košice|48.7164|21.2611">Košice</option>
          <option value="Banská Bystrica|48.7363|19.1462">Banská Bystrica</option>
          <option value="Žilina|49.2232|18.7394">Žilina</option>
          <option value="Nitra|48.3061|18.0764">Nitra</option>
          <option value="Prešov|48.9984|21.2339">Prešov</option>
          <option value="Trnava|48.3774|17.5883">Trnava</option>
          <option value="Trenčín|48.8945|18.0444">Trenčín</option>
          <option value="Poprad|49.0512|20.2970">Poprad</option>
        </optgroup>
        <optgroup label="Czech Republic">
          <option value="Praha|50.0755|14.4378">Praha</option>
          <option value="Brno|49.1951|16.6068">Brno</option>
          <option value="Ostrava|49.8209|18.2625">Ostrava</option>
          <option value="Plzeň|49.7384|13.3736">Plzeň</option>
          <option value="Olomouc|49.5938|17.2509">Olomouc</option>
        </optgroup>
        <optgroup label="Europe">
          <option value="Vienna|48.2082|16.3738">Vienna (Wien)</option>
          <option value="Budapest|47.4979|19.0402">Budapest</option>
          <option value="Warsaw|52.2297|21.0122">Warsaw (Warszawa)</option>
          <option value="Berlin|52.5200|13.4050">Berlin</option>
          <option value="London|51.5074|-0.1278">London</option>
          <option value="Paris|48.8566|2.3522">Paris</option>
          <option value="Rome|41.9028|12.4964">Rome</option>
          <option value="Madrid|40.4168|-3.7038">Madrid</option>
          <option value="Amsterdam|52.3676|4.9041">Amsterdam</option>
          <option value="Zurich|47.3769|8.5417">Zurich</option>
        </optgroup>
        <optgroup label="North America & Worldwide">
          <option value="New York|40.7128|-74.0060">New York</option>
          <option value="Los Angeles|34.0522|-118.2437">Los Angeles</option>
          <option value="Toronto|43.6532|-79.3832">Toronto</option>
          <option value="Tokyo|35.6762|139.6503">Tokyo</option>
          <option value="Dubai|25.2048|55.2708">Dubai</option>
          <option value="Sydney|-33.8688|151.2093">Sydney</option>
        </optgroup>
      </select>
    </div>

    <div class="field">
      <label for="w_city">City Name</label>
      <input type="text" id="w_city" name="w_city" value="{{W_CITY}}" placeholder="e.g. Bratislava">
    </div>
    <div class="row">
      <div class="field">
        <label for="w_lat">Latitude</label>
        <input type="text" id="w_lat" name="w_lat" value="{{W_LAT}}" placeholder="48.1486">
      </div>
      <div class="field">
        <label for="w_lon">Longitude</label>
        <input type="text" id="w_lon" name="w_lon" value="{{W_LON}}" placeholder="17.1077">
      </div>
    </div>

    <div class="section-title">&#x1F4B0; Financial Markets & Crypto (Live Tickers)</div>
    <div class="field">
      <label for="c_sym">Popular Asset / Index Preset</label>
      <select id="c_sym" name="c_sym" onchange="document.getElementById('c_sym_custom').value = ''">
        <optgroup label="Cryptocurrencies">
          <option value="BTC" {{C_BTC_SEL}}>Bitcoin (BTC / USDT)</option>
          <option value="ETH" {{C_ETH_SEL}}>Ethereum (ETH / USDT)</option>
          <option value="SOL" {{C_SOL_SEL}}>Solana (SOL / USDT)</option>
          <option value="BNB" {{C_BNB_SEL}}>Binance Coin (BNB / USDT)</option>
        </optgroup>
        <optgroup label="Stocks & Indices">
          <option value="AAPL" {{C_AAPL_SEL}}>Apple Inc. (AAPL - Nasdaq)</option>
          <option value="TSLA" {{C_TSLA_SEL}}>Tesla Inc. (TSLA - Nasdaq)</option>
          <option value="NVDA" {{C_NVDA_SEL}}>Nvidia Corp. (NVDA - Nasdaq)</option>
          <option value="SPX" {{C_SPX_SEL}}>S&P 500 Index (^GSPC)</option>
        </optgroup>
        <optgroup label="Commodities & Forex">
          <option value="GOLD" {{C_GOLD_SEL}}>Gold Bullion (GC=F / USD)</option>
          <option value="EUR" {{C_EUR_SEL}}>EUR / USD (Euro Forex)</option>
          <option value="USD" {{C_USD_SEL}}>USD / EUR (US Dollar Forex)</option>
        </optgroup>
      </select>
    </div>
    <div class="field">
      <label for="c_sym_custom">Or Custom Ticker / Symbol</label>
      <input type="text" id="c_sym_custom" name="c_sym_custom" value="{{C_SYM_CUSTOM}}" placeholder="e.g. AAPL, GOLD, EUR, MSFT, BTC">
    </div>

    <div class="section-title">&#x25B6; YouTube Channel Analytics</div>
    <div class="field">
      <label for="yt_channel">Channel ID or @Handle</label>
      <input type="text" id="yt_channel" name="yt_channel" value="{{YT_CHANNEL}}" placeholder="e.g. @MKBHD or UCxxxxxxxx">
    </div>
    <div class="field">
      <label for="yt_key">YouTube Data API v3 Key</label>
      <input type="password" id="yt_key" name="yt_key" value="{{YT_KEY}}" placeholder="Google Cloud API key">
    </div>

    <div class="section-title">&#x1F3E0; Home Assistant &amp; MQTT (Auto-Discovery &amp; Push API)</div>
    <div class="field">
      <label class="chk-label">
        <input type="checkbox" name="mqtt_en" id="mqtt_en" {{MQTT_EN_CHK}} onchange="document.getElementById('mqtt_settings').style.display = this.checked ? 'block' : 'none';">
        Enable MQTT Client &amp; Home Assistant Auto-Discovery
      </label>
    </div>
    <div id="mqtt_settings" style="{{MQTT_DISPLAY}}">
      <div class="row">
        <div class="field" style="flex:2;">
          <label for="mqtt_srv">Broker IP or Hostname</label>
          <input type="text" id="mqtt_srv" name="mqtt_srv" value="{{MQTT_SRV}}" placeholder="e.g. 192.168.0.100 or homeassistant.local">
        </div>
        <div class="field" style="flex:1;">
          <label for="mqtt_port">Port</label>
          <input type="number" id="mqtt_port" name="mqtt_port" value="{{MQTT_PORT}}" placeholder="1883">
        </div>
      </div>
      <div class="row">
        <div class="field">
          <label for="mqtt_user">Username (Optional)</label>
          <input type="text" id="mqtt_user" name="mqtt_user" value="{{MQTT_USER}}" placeholder="Home Assistant MQTT user">
        </div>
        <div class="field">
          <label for="mqtt_pass">Password (Optional)</label>
          <input type="password" id="mqtt_pass" name="mqtt_pass" value="{{MQTT_PASS}}" placeholder="Password">
        </div>
      </div>
      <div class="field">
        <label for="mqtt_pre">Topic Prefix</label>
        <input type="text" id="mqtt_pre" name="mqtt_pre" value="{{MQTT_PRE}}" placeholder="ulanzi-tc001">
      </div>
      <div style="font-size:0.75rem; color:var(--text-muted); line-height:1.4; margin-top:0.25rem; background:rgba(0,0,0,0.2); padding:0.6rem; border-radius:0.5rem; border:1px solid rgba(255,255,255,0.05);">
        &#x1F4A1; <strong>Push Spotify &amp; Custom Screens:</strong><br>
        MQTT: Publish JSON to <code>{{MQTT_PRE}}/custom/spotify</code> or <code>{{MQTT_PRE}}/notify</code><br>
        HTTP REST: <code>POST /api/custom?name=spotify</code> or <code>POST /api/notify</code><br>
        Payload: <code>{"text":"Song - Artist","icon":"spotify","progress":45,"duration":8}</code>
      </div>
    </div>

    <button type="submit" class="btn-submit">Save & Apply Settings</button>
  </form>

  <div class="section-title" style="margin-top:1.75rem;">&#x1F680; Firmware Update (OTA)</div>
  <div style="font-size:0.8rem; color:var(--text-muted); margin-bottom:0.75rem;">
    Running Version: <strong style="color:var(--accent);">v{{FW_VERSION}}</strong>
  </div>
  <div class="field">
    <label for="ota_file">Flash Local Binary (.bin)</label>
    <input type="file" id="ota_file" accept=".bin" style="padding:0.4rem;">
    <button type="button" class="scan-btn" style="margin-top:0.5rem; width:100%; justify-content:center;" onclick="uploadFirmware()">&#x21E7; Flash Firmware File</button>
    <div id="ota_progress" style="display:none; margin-top:0.75rem;">
      <div style="background:rgba(15,23,42,0.8); border-radius:9999px; height:8px; overflow:hidden;">
        <div id="ota_bar" style="background:var(--accent); width:0%; height:100%; transition:width 0.2s;"></div>
      </div>
      <div id="ota_msg" style="font-size:0.75rem; color:var(--text-muted); text-align:center; margin-top:0.35rem;"></div>
    </div>
  </div>
  <div style="margin-top:0.75rem; padding-top:0.75rem; border-top:1px dashed var(--border);">
    <button type="button" class="scan-btn" style="width:100%; justify-content:center;" onclick="checkCloudOta()">&#x2601; Check GitHub Releases</button>
    <div id="cloud_ota_res" style="font-size:0.8rem; margin-top:0.5rem; text-align:center;"></div>
  </div>

  <div class="footer">Ulanzi TC001 Evolved &bull; Firmware v{{FW_VERSION}}</div>
</div>
<script>
function scanNetworks() {
  const sel = document.getElementById('scanSelect');
  sel.style.display = 'block';
  sel.innerHTML = '<option>Scanning...</option>';
  fetch('/scan')
    .then(r => r.json())
    .then(data => {
      sel.innerHTML = '<option value="">-- Detected Networks (' + data.length + ') --</option>';
      data.forEach(net => {
        const opt = document.createElement('option');
        opt.value = net.ssid;
        opt.textContent = net.ssid + ' (' + net.rssi + ' dBm' + (net.secure ? ' 🔒' : '') + ')';
        sel.appendChild(opt);
      });
    })
    .catch(() => {
      sel.innerHTML = '<option value="">Scan failed, enter SSID manually</option>';
    });
}
function selectNetwork(val) {
  if (val) document.getElementById('ssid').value = val;
}
function applyCityPreset(val) {
  if (!val) return;
  const parts = val.split('|');
  if (parts.length === 3) {
    document.getElementById('w_city').value = parts[0];
    document.getElementById('w_lat').value = parts[1];
    document.getElementById('w_lon').value = parts[2];
  }
}
function uploadFirmware() {
  const fileInput = document.getElementById('ota_file');
  if (!fileInput.files.length) {
    alert('Please select a .bin firmware file first');
    return;
  }
  const file = fileInput.files[0];
  const formData = new FormData();
  formData.append('update', file);

  const prog = document.getElementById('ota_progress');
  const bar = document.getElementById('ota_bar');
  const msg = document.getElementById('ota_msg');
  prog.style.display = 'block';
  msg.textContent = 'Uploading: 0%';

  const xhr = new XMLHttpRequest();
  xhr.open('POST', '/update', true);
  xhr.upload.onprogress = function(e) {
    if (e.lengthComputable) {
      const pct = Math.round((e.loaded / e.total) * 100);
      bar.style.width = pct + '%';
      msg.textContent = 'Uploading & Flashing: ' + pct + '%';
    }
  };
  xhr.onload = function() {
    if (xhr.status === 200) {
      bar.style.width = '100%';
      bar.style.background = '#10b981';
      msg.innerHTML = '<strong style="color:#10b981;">✓ Flash Complete! Rebooting clock...</strong>';
      setTimeout(() => location.reload(), 6000);
    } else {
      bar.style.background = '#ef4444';
      msg.innerHTML = '<strong style="color:#ef4444;">✗ Update Error: ' + xhr.responseText + '</strong>';
    }
  };
  xhr.onerror = function() {
    bar.style.background = '#ef4444';
    msg.innerHTML = '<strong style="color:#ef4444;">✗ Network connection error</strong>';
  };
  xhr.send(formData);
}
function checkCloudOta() {
  const res = document.getElementById('cloud_ota_res');
  res.innerHTML = '<span style="color:var(--text-muted);">Checking GitHub Releases...</span>';
  fetch('/api/ota/check')
    .then(r => r.json())
    .then(data => {
      if (data.has_update) {
        res.innerHTML = '<div style="color:#10b981; font-weight:700; margin-bottom:0.35rem;">★ New Version Available: v' + data.latest_version + '</div>' +
          '<button type="button" class="scan-btn" style="background:#0284c7; color:#fff; border:none;" onclick="installCloudOta(\'' + data.download_url + '\')">Install v' + data.latest_version + ' Now</button>';
      } else {
        res.innerHTML = '<span style="color:#10b981;">✓ Firmware is up to date (v' + data.current_version + ')</span>';
      }
    })
    .catch(() => {
      res.innerHTML = '<span style="color:#ef4444;">Cannot reach GitHub releases</span>';
    });
}
function installCloudOta(url) {
  const res = document.getElementById('cloud_ota_res');
  res.innerHTML = '<span style="color:var(--accent);">Downloading and flashing from GitHub... Watch clock display!</span>';
  const fd = new FormData();
  fd.append('url', url);
  fetch('/api/ota/install', { method: 'POST', body: fd })
    .then(r => r.json())
    .then(d => {
      setTimeout(() => location.reload(), 10000);
    });
}
let currentAppIndex = {{ACTIVE_APP}};
const appNames = ["Clock", "Indoor Temp", "Outdoor Weather", "Crypto", "YouTube", "Airspace", "Battery"];
function switchScreen(dir) {
  currentAppIndex = (currentAppIndex + dir + 7) % 7;
  fetch('/api/screen?set=' + currentAppIndex)
    .then(r => r.json())
    .then(d => {
      document.getElementById('screenNameBadge').innerText = d.app_name || appNames[currentAppIndex];
      updateVirtualMatrix();
    });
}
function updateVirtualMatrix() {
  fetch('/api/display')
    .then(r => r.json())
    .then(data => {
      const cvs = document.getElementById('virtualMatrix');
      if (!cvs) return;
      const ctx = cvs.getContext('2d');
      ctx.fillStyle = '#0a0d14';
      ctx.fillRect(0, 0, 320, 80);
      const rows = data.pixels;
      for (let y = 0; y < 8; y++) {
        for (let x = 0; x < 32; x++) {
          ctx.fillStyle = rows[y][x];
          ctx.beginPath();
          ctx.arc(x * 10 + 5, y * 10 + 5, 3.8, 0, Math.PI * 2);
          ctx.fill();
        }
      }
    })
    .catch(() => {});
}
setInterval(updateVirtualMatrix, 1500);
updateVirtualMatrix();

function moveScreen(btn, dir) {
  const item = btn.closest('.reorder-item');
  if (dir === -1 && item.previousElementSibling) {
    item.parentNode.insertBefore(item, item.previousElementSibling);
  } else if (dir === 1 && item.nextElementSibling) {
    item.parentNode.insertBefore(item.nextElementSibling, item);
  }
  updateOrderInput();
}

function updateOrderInput() {
  const items = document.querySelectorAll('#carouselScreens .reorder-item');
  const order = Array.from(items).map(el => el.getAttribute('data-app')).join(',');
  document.getElementById('car_ord').value = order;
}

function showToast(msg) {
  const t = document.getElementById('toast');
  t.innerText = msg;
  t.style.display = 'block';
  t.style.opacity = '1';
  setTimeout(() => {
    t.style.opacity = '0';
    setTimeout(() => { t.style.display = 'none'; }, 300);
  }, 2500);
}

document.getElementById('settingsForm').addEventListener('submit', function(e) {
  e.preventDefault();
  updateOrderInput();
  const form = this;
  const btn = form.querySelector('.btn-submit');
  const origText = btn.innerText;
  btn.innerText = 'Saving...';
  btn.disabled = true;

  const params = new URLSearchParams(new FormData(form));
  params.append('ajax', '1');

  fetch('/save', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/x-www-form-urlencoded',
      'X-Requested-With': 'XMLHttpRequest'
    },
    body: params.toString()
  })
  .then(res => {
    btn.innerText = origText;
    btn.disabled = false;
    showToast('✓ Settings Saved Automatically!');
    updateVirtualMatrix();
  })
  .catch(err => {
    btn.innerText = origText;
    btn.disabled = false;
    form.submit();
  });
});
</script>
</body>
</html>
)rawliteral");

    html.replace("{{SSID}}", s_ssid);
    html.replace("{{ROTATE}}", String(s_rotateSec));
    html.replace("{{AUTO_ROTATE_CHK}}", s_autoRotate ? "checked" : "");
    html.replace("{{ROTATE_DELAY_DISPLAY}}", s_autoRotate ? "" : "display:none;");
    html.replace("{{ACTIVE_APP}}", String(s_activeApp));
    static const char* kAppNames[] = {
        "Clock", "Indoor Temp", "Outdoor Weather", "Crypto", "YouTube", "Airspace", "Battery"
    };
    html.replace("{{APP_NAME}}", kAppNames[s_activeApp % 7]);
    for (int i = 0; i < 7; i++) {
        char tag[20];
        snprintf(tag, sizeof(tag), "{{APP_%d_SEL}}", i);
        html.replace(tag, (s_activeApp == i) ? "selected" : "");
    }

    String reorderHtml = "";
    static const char* kAppIconsNames[] = {
        "&#x23F0; Clock", "&#x1F321; Indoor Temp", "&#x26C5; Outdoor Weather", "&#x1F4B0; Crypto & Markets", "&#x25B6; YouTube", "&#x2708; Live Airspace", "&#x26A1; Battery & Energy"
    };
    for (int i = 0; i < 7; i++) {
        uint8_t appIdx = s_carouselOrder[i];
        if (appIdx >= 7) appIdx = i;
        bool isChecked = (s_carouselMask & (1 << appIdx)) != 0;

        reorderHtml += "<div class=\"reorder-item\" data-app=\"" + String(appIdx) + "\">";
        reorderHtml += "<label class=\"chk-label\" style=\"margin:0;\">";
        reorderHtml += "<input type=\"checkbox\" name=\"car_app_" + String(appIdx) + "\" value=\"1\" " + (isChecked ? "checked " : "") + "onchange=\"updateOrderInput()\">";
        reorderHtml += kAppIconsNames[appIdx];
        reorderHtml += "</label>";
        reorderHtml += "<div style=\"display:flex; gap:0.25rem;\">";
        reorderHtml += "<button type=\"button\" class=\"btn-sm\" onclick=\"moveScreen(this, -1)\">&#x25B2;</button>";
        reorderHtml += "<button type=\"button\" class=\"btn-sm\" onclick=\"moveScreen(this, 1)\">&#x25BC;</button>";
        reorderHtml += "</div></div>";
    }
    html.replace("{{REORDER_ITEMS}}", reorderHtml);
    html.replace("{{CAR_ORD}}", Network_GetCarouselOrderString());

    html.replace("{{BAT_MODE_0_SEL}}", (s_batteryMode == 0) ? "selected" : "");
    html.replace("{{BAT_MODE_1_SEL}}", (s_batteryMode == 1) ? "selected" : "");
    html.replace("{{BAT_MODE_2_SEL}}", (s_batteryMode == 2) ? "selected" : "");

    // Screen transition effects
    for (int i = 0; i <= 7; i++) {
        char tag[20];
        snprintf(tag, sizeof(tag), "{{TRANS_%d_SEL}}", i);
        html.replace(tag, (s_transitionFx == i) ? "selected" : "");
    }

    // Clock & Layout replacements
    for (int i = 0; i < 3; i++) {
        char tag[20];
        snprintf(tag, sizeof(tag), "{{CLK_MODE_%d_SEL}}", i);
        html.replace(tag, (s_clockMode == i) ? "selected" : "");
    }
    html.replace("{{SHOW_SEC_CHK}}", s_showSeconds ? "checked" : "");
    html.replace("{{WD_BAR_CHK}}", s_weekdayBar ? "checked" : "");
    html.replace("{{WD_MON_1_SEL}}", s_weekdayStartMon ? "selected" : "");
    html.replace("{{WD_MON_0_SEL}}", !s_weekdayStartMon ? "selected" : "");
    for (int i = 0; i < 6; i++) {
        char tag[20];
        snprintf(tag, sizeof(tag), "{{WD_ACOL_%d_SEL}}", i);
        html.replace(tag, (s_weekdayActiveColIdx == i) ? "selected" : "");
    }
    for (int i = 0; i < 4; i++) {
        char tag[20];
        snprintf(tag, sizeof(tag), "{{WD_ICOL_%d_SEL}}", i);
        html.replace(tag, (s_weekdayInactColIdx == i) ? "selected" : "");
    }
    for (int i = 0; i < 4; i++) {
        char tag[20];
        snprintf(tag, sizeof(tag), "{{DATE_FMT_%d_SEL}}", i);
        html.replace(tag, (s_dateFormat == i) ? "selected" : "");
    }
    for (int i = 0; i < 4; i++) {
        char tag[20];
        snprintf(tag, sizeof(tag), "{{CLK_ICON_%d_SEL}}", i);
        html.replace(tag, (s_clockIcon == i) ? "selected" : "");
    }
    html.replace("{{TXT_ALIGN_0_SEL}}", (s_textAlign == 0) ? "selected" : "");
    html.replace("{{TXT_ALIGN_1_SEL}}", (s_textAlign == 1) ? "selected" : "");

    html.replace("{{AUTO_BRI_CHK}}", s_autoBri ? "checked" : "");
    html.replace("{{NIGHT_MODE_CHK}}", s_nightMode ? "checked" : "");
    html.replace("{{NIGHT_START}}", String(s_nightStart));
    html.replace("{{NIGHT_END}}", String(s_nightEnd));
    html.replace("{{DAY_BRI}}", String(s_dayBri));
    html.replace("{{NIGHT_BRI}}", String(s_nightBri));

    html.replace("{{W_CITY}}", s_wCity);
    html.replace("{{W_LAT}}", String(s_wLat, 4));
    html.replace("{{W_LON}}", String(s_wLon, 4));

    html.replace("{{C_BTC_SEL}}", s_cryptoSym == "BTC" ? "selected" : "");
    html.replace("{{C_ETH_SEL}}", s_cryptoSym == "ETH" ? "selected" : "");
    html.replace("{{C_SOL_SEL}}", s_cryptoSym == "SOL" ? "selected" : "");
    html.replace("{{C_BNB_SEL}}", s_cryptoSym == "BNB" ? "selected" : "");
    html.replace("{{C_AAPL_SEL}}", s_cryptoSym == "AAPL" ? "selected" : "");
    html.replace("{{C_GOLD_SEL}}", (s_cryptoSym == "GOLD" || s_cryptoSym == "GC=F") ? "selected" : "");
    html.replace("{{C_EUR_SEL}}", (s_cryptoSym == "EUR" || s_cryptoSym == "EURUSD" || s_cryptoSym == "EURUSD=X") ? "selected" : "");
    html.replace("{{C_USD_SEL}}", (s_cryptoSym == "USD" || s_cryptoSym == "USDEUR" || s_cryptoSym == "USDEUR=X") ? "selected" : "");
    html.replace("{{C_SPX_SEL}}", (s_cryptoSym == "SPX" || s_cryptoSym == "^GSPC") ? "selected" : "");
    html.replace("{{C_TSLA_SEL}}", s_cryptoSym == "TSLA" ? "selected" : "");
    html.replace("{{C_NVDA_SEL}}", s_cryptoSym == "NVDA" ? "selected" : "");
    html.replace("{{C_SYM_CUSTOM}}", s_cryptoSym);

    html.replace("{{YT_CHANNEL}}", s_ytChannel);
    html.replace("{{YT_KEY}}", s_ytApiKey.length() > 0 ? "********" : "");

    html.replace("{{MQTT_EN_CHK}}", s_mqttEn ? "checked" : "");
    html.replace("{{MQTT_DISPLAY}}", s_mqttEn ? "" : "display:none;");
    html.replace("{{MQTT_SRV}}", s_mqttServer);
    html.replace("{{MQTT_PORT}}", String(s_mqttPort));
    html.replace("{{MQTT_USER}}", s_mqttUser);
    html.replace("{{MQTT_PASS}}", s_mqttPass.length() > 0 ? "********" : "");
    html.replace("{{MQTT_PRE}}", s_mqttPrefix);

    html.replace("{{FW_VERSION}}", FW_VERSION);
    return html;
}

static const char PAGE_SAVED[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta http-equiv="refresh" content="1;url=/">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Settings Saved &bull; Ulanzi TC001 Evolved</title>
<style>
  body {
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
    background: #090d16;
    color: #f1f5f9;
    min-height: 100vh;
    display: flex;
    align-items: center;
    justify-content: center;
    padding: 1.5rem;
    text-align: center;
  }
  .card {
    max-width: 400px;
    background: rgba(22, 30, 49, 0.9);
    border: 1px solid rgba(255,255,255,0.1);
    border-radius: 1.25rem;
    padding: 2.5rem 1.75rem;
    box-shadow: 0 20px 40px rgba(0,0,0,0.5);
  }
  .icon { font-size: 3rem; margin-bottom: 1rem; }
  h2 { font-size: 1.5rem; margin-bottom: 0.5rem; color: #38bdf8; }
  p { color: #94a3b8; font-size: 0.95rem; line-height: 1.5; }
</style>
</head>
<body>
<div class="card">
  <div class="icon">&#x2714;</div>
  <h2>Settings Saved!</h2>
  <p>Your configuration has been updated and stored in NVS.<br><br>Returning to settings in a moment...</p>
</div>
<script>setTimeout(() => { window.location.href = '/'; }, 1000);</script>
</body>
</html>
)rawliteral";

static bool isIp(const String& str) {
    for (size_t i = 0; i < str.length(); i++) {
        char c = str.charAt(i);
        if (c != '.' && (c < '0' || c > '9')) return false;
    }
    return true;
}

static void handleCaptiveRedirect() {
    String host = s_server.hostHeader();
    if (!isIp(host) && host != "192.168.4.1") {
        s_server.sendHeader("Location", String("http://192.168.4.1/"), true);
        s_server.send(302, "text/plain", "");
        return;
    }
    s_server.send(200, "text/html", buildPortalHtml());
}

static void handleScan() {
    int n = WiFi.scanNetworks();
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();

    for (int i = 0; i < n; i++) {
        String curSsid = WiFi.SSID(i);
        if (curSsid.length() == 0) continue;
        JsonObject item = arr.add<JsonObject>();
        item["ssid"] = curSsid;
        item["rssi"] = WiFi.RSSI(i);
        item["secure"] = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
    }
    String out;
    serializeJson(doc, out);
    s_server.send(200, "application/json", out);
}

static void handleSave() {
    bool wifiChanged = false;

    // 1. Process Wi-Fi credentials
    if (s_server.hasArg("ssid")) {
        String newSsid = s_server.arg("ssid");
        String newPass = s_server.arg("password");
        newSsid.trim();
        newPass.trim();

        if (newSsid.length() > 0 && (newSsid != s_ssid || newPass.length() > 0)) {
            Network_SaveCredentials(newSsid, (newPass.length() > 0) ? newPass : s_pass);
            wifiChanged = true;
        }
    }

    // 2. Process Carousel & Screen Navigation
    Network_SetAutoRotate(s_server.hasArg("auto_rotate"));
    if (s_server.hasArg("car_ord")) {
        Network_SetCarouselOrderString(s_server.arg("car_ord"));
    }
    uint8_t carMask = 0;
    for (int i = 0; i < 7; i++) {
        char argName[16];
        snprintf(argName, sizeof(argName), "car_app_%d", i);
        if (s_server.hasArg(argName)) {
            carMask |= (1 << i);
        }
    }
    if (carMask == 0) carMask = 0x01; // Ensure at least clock is selected
    Network_SetCarouselMask(carMask);

    if (s_server.hasArg("bat_mode")) {
        Network_SetBatteryMode((uint8_t)s_server.arg("bat_mode").toInt());
    }

    if (s_server.hasArg("active_app")) {
        int app = s_server.arg("active_app").toInt();
        if (app >= 0 && app < 7) {
            Network_SetActiveApp((uint8_t)app);
        }
    }
    if (s_server.hasArg("rotate")) {
        int r = s_server.arg("rotate").toInt();
        if (r >= MIN_ROTATE_SEC && r <= MAX_ROTATE_SEC) {
            Network_SetRotateDelaySec((uint16_t)r);
        }
    }
    if (s_server.hasArg("trans_fx")) {
        Network_SetTransitionFx((uint8_t)s_server.arg("trans_fx").toInt());
    }

    // 3. Process Clock & Date Layout (Awtrix Style)
    if (s_server.hasArg("clk_mode")) {
        Network_SetClockMode((uint8_t)s_server.arg("clk_mode").toInt());
    }
    Network_SetShowSeconds(s_server.hasArg("show_sec"));
    Network_SetWeekdayBar(s_server.hasArg("wd_bar"));
    if (s_server.hasArg("wd_mon")) {
        Network_SetWeekdayStartMonday(s_server.arg("wd_mon").toInt() == 1);
    }
    if (s_server.hasArg("wd_acol")) {
        Network_SetWeekdayActiveColorIndex((uint8_t)s_server.arg("wd_acol").toInt());
    }
    if (s_server.hasArg("wd_icol")) {
        Network_SetWeekdayInactiveColorIndex((uint8_t)s_server.arg("wd_icol").toInt());
    }
    if (s_server.hasArg("date_fmt")) {
        Network_SetDateFormat((uint8_t)s_server.arg("date_fmt").toInt());
    }
    if (s_server.hasArg("clk_icon")) {
        Network_SetClockIcon((uint8_t)s_server.arg("clk_icon").toInt());
    }
    if (s_server.hasArg("txt_align")) {
        Network_SetTextAlign((uint8_t)s_server.arg("txt_align").toInt());
    }

    // 4. Process Ambient, Day/Night Brightness & Night Mode
    Network_SetAutoBrightness(s_server.hasArg("auto_bri"));
    Network_SetNightMode(s_server.hasArg("night_mode"));
    if (s_server.hasArg("night_start") && s_server.hasArg("night_end")) {
        uint8_t st = (uint8_t)constrain(s_server.arg("night_start").toInt(), 0, 23);
        uint8_t en = (uint8_t)constrain(s_server.arg("night_end").toInt(), 0, 23);
        Network_SetNightSchedule(st, en);
    }
    if (s_server.hasArg("day_bri")) {
        uint8_t db = (uint8_t)constrain(s_server.arg("day_bri").toInt(), 5, 100);
        Network_SetDayBrightness(db);
    }
    if (s_server.hasArg("night_bri")) {
        uint8_t nb = (uint8_t)constrain(s_server.arg("night_bri").toInt(), 1, 25);
        Network_SetNightBrightness(nb);
    }

    // 4. Process Outdoor Weather
    if (s_server.hasArg("w_lat") && s_server.hasArg("w_lon")) {
        float lat = s_server.arg("w_lat").toFloat();
        float lon = s_server.arg("w_lon").toFloat();
        String city = s_server.hasArg("w_city") ? s_server.arg("w_city") : "";
        Network_SetWeatherLocation(lat, lon, city);
    }

    // 5. Process Markets / Crypto Symbol
    if (s_server.hasArg("c_sym_custom") && s_server.arg("c_sym_custom").length() > 0) {
        Network_SetCryptoSymbol(s_server.arg("c_sym_custom"));
    } else if (s_server.hasArg("c_sym")) {
        Network_SetCryptoSymbol(s_server.arg("c_sym"));
    }

    // 6. Process YouTube Analytics
    if (s_server.hasArg("yt_channel")) {
        String ch = s_server.arg("yt_channel");
        String key = s_server.arg("yt_key");
        ch.trim();
        key.trim();
        if (key == "********") {
            key = s_ytApiKey;
        }
        Network_SaveYouTubeConfig(ch, key);
    }

    // 7. Process Home Assistant & MQTT
    Network_SetMqttEnabled(s_server.hasArg("mqtt_en"));
    if (s_server.hasArg("mqtt_srv")) {
        Network_SetMqttServer(s_server.arg("mqtt_srv"));
    }
    if (s_server.hasArg("mqtt_port")) {
        Network_SetMqttPort((uint16_t)s_server.arg("mqtt_port").toInt());
    }
    if (s_server.hasArg("mqtt_user")) {
        Network_SetMqttUser(s_server.arg("mqtt_user"));
    }
    if (s_server.hasArg("mqtt_pass")) {
        String p = s_server.arg("mqtt_pass");
        if (p != "********") {
            Network_SetMqttPass(p);
        }
    }
    if (s_server.hasArg("mqtt_pre")) {
        String pre = s_server.arg("mqtt_pre");
        pre.trim();
        if (pre.length() > 0) Network_SetMqttPrefix(pre);
    }

    s_server.sendHeader("Access-Control-Allow-Origin", "*");
    if (s_server.hasArg("ajax") || s_server.header("X-Requested-With") == "XMLHttpRequest") {
        s_server.send(200, "application/json", "{\"status\":\"success\"}");
    } else {
        s_server.send_P(200, "text/html", PAGE_SAVED);
    }

    if (wifiChanged || s_netState == NET_STATE_AP_PORTAL) {
        s_pendingReconnect = true;
        s_reconnectTriggerMs = millis() + 1500;
    }
}

static void handleStatus() {
    JsonDocument doc;
    doc["state"] = (s_netState == NET_STATE_CONNECTED) ? "connected" : 
                   (s_netState == NET_STATE_AP_PORTAL ? "ap_portal" : "connecting");
    doc["ip"] = Network_GetLocalIP();
    doc["ap_ip"] = Network_GetAPIP();
    doc["ssid"] = Network_GetSSID();
    doc["rssi"] = Network_GetRSSI();
    doc["rotate_sec"] = s_rotateSec;
    doc["auto_rotate"] = s_autoRotate;
    doc["carousel_mask"] = s_carouselMask;
    doc["transition_fx"] = s_transitionFx;
    doc["active_app"] = s_activeApp;
    doc["firmware_version"] = FW_VERSION;

    // Ambient & Sensor telemetry
    const SensorData& s = Sensors_GetData();
    doc["temperature_in"] = s.temperature;
    doc["humidity_in"] = s.humidity;
    doc["battery_volts"] = s.batteryVolts;
    doc["battery_pct"] = s.batteryPercent;
    doc["ldr_raw"] = s.ldrRaw;
    doc["auto_bri"] = s_autoBri;
    doc["day_bri"] = s_dayBri;
    doc["night_bri"] = s_nightBri;
    doc["night_mode"] = s_nightMode;

    // Services
    doc["weather_temp"] = Weather_GetFormattedTemp();
    doc["weather_city"] = Weather_GetCity();
    doc["crypto_symbol"] = Crypto_GetSymbol();
    doc["crypto_price"] = Crypto_GetFormattedPrice();
    doc["yt_subscribers"] = YouTube_GetFormattedSubs();
    doc["mqtt_connected"] = MqttService_IsConnected();

    String out;
    serializeJson(doc, out);
    s_server.send(200, "application/json", out);
}

void Network_Init() {
    uint32_t chipId = 0;
    for (int i = 0; i < 17; i = i + 8) {
        chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }
    char buf[32];
    snprintf(buf, sizeof(buf), "%s%04X", WIFI_AP_SSID_PREFIX, (uint16_t)(chipId & 0xFFFF));
    s_apSSID = String(buf);

    s_prefs.begin(PREF_NAMESPACE, false);
    s_ssid = s_prefs.getString(PREF_KEY_SSID, "");
    s_pass = s_prefs.getString(PREF_KEY_PASS, "");
    s_autoRotate = s_prefs.getBool(PREF_KEY_AUTO_ROTATE, true);
    s_activeApp = (uint8_t)s_prefs.getUChar(PREF_KEY_ACTIVE_APP, 0);
    if (s_activeApp >= 7) s_activeApp = 0;
    s_rotateSec = (uint16_t)s_prefs.getUInt(PREF_KEY_ROTATE_SEC, DEFAULT_ROTATE_SEC);
    if (s_rotateSec < MIN_ROTATE_SEC || s_rotateSec > MAX_ROTATE_SEC) {
        s_rotateSec = DEFAULT_ROTATE_SEC;
    }
    s_carouselMask = (uint8_t)s_prefs.getUChar(PREF_KEY_CAROUSEL_MASK, 0x7F);
    if (s_carouselMask == 0) s_carouselMask = 0x7F;

    String savedOrder = s_prefs.getString(PREF_KEY_CAROUSEL_ORDER, "0,1,2,3,4,5,6");
    Network_SetCarouselOrderString(savedOrder);
    s_transitionFx = (uint8_t)s_prefs.getUChar(PREF_KEY_TRANSITION_FX, DEFAULT_TRANSITION_FX);
    if (s_transitionFx > 7) s_transitionFx = DEFAULT_TRANSITION_FX;
    s_batteryMode = (uint8_t)s_prefs.getUChar(PREF_KEY_BATTERY_MODE, 2);
    if (s_batteryMode > 2) s_batteryMode = 2;

    s_clockMode = (uint8_t)s_prefs.getUChar(PREF_KEY_CLOCK_MODE, 1);
    if (s_clockMode > 2) s_clockMode = 0;
    s_showSeconds = s_prefs.getBool(PREF_KEY_SHOW_SECONDS, false);
    s_weekdayBar = s_prefs.getBool(PREF_KEY_WEEKDAY_BAR, true);
    s_weekdayStartMon = s_prefs.getBool(PREF_KEY_WD_START_MON, true);
    s_weekdayActiveColIdx = (uint8_t)s_prefs.getUChar(PREF_KEY_WD_ACTIVE_COL, 0);
    if (s_weekdayActiveColIdx > 5) s_weekdayActiveColIdx = 0;
    s_weekdayInactColIdx = (uint8_t)s_prefs.getUChar(PREF_KEY_WD_INACT_COL, 0);
    if (s_weekdayInactColIdx > 3) s_weekdayInactColIdx = 0;
    s_dateFormat = (uint8_t)s_prefs.getUChar(PREF_KEY_DATE_FORMAT, 0);
    if (s_dateFormat > 3) s_dateFormat = 0;
    s_clockIcon = (uint8_t)s_prefs.getUChar(PREF_KEY_CLOCK_ICON, 0);
    if (s_clockIcon > 3) s_clockIcon = 0;
    s_textAlign = (uint8_t)s_prefs.getUChar(PREF_KEY_TEXT_ALIGN, 0);
    if (s_textAlign > 1) s_textAlign = 0;

    s_autoBri = s_prefs.getBool(PREF_KEY_AUTO_BRI, true);
    s_nightMode = s_prefs.getBool(PREF_KEY_NIGHT_MODE, true);
    s_nightStart = (uint8_t)s_prefs.getUChar(PREF_KEY_NIGHT_START, DEFAULT_NIGHT_START);
    s_nightEnd = (uint8_t)s_prefs.getUChar(PREF_KEY_NIGHT_END, DEFAULT_NIGHT_END);
    s_dayBri = (uint8_t)s_prefs.getUChar(PREF_KEY_DAY_BRI, DEFAULT_DAY_BRI);
    s_nightBri = (uint8_t)s_prefs.getUChar(PREF_KEY_NIGHT_BRI, DEFAULT_NIGHT_BRI);

    s_wLat = s_prefs.getFloat(PREF_KEY_WEATHER_LAT, DEFAULT_WEATHER_LAT);
    s_wLon = s_prefs.getFloat(PREF_KEY_WEATHER_LON, DEFAULT_WEATHER_LON);
    s_wCity = s_prefs.getString(PREF_KEY_WEATHER_CITY, DEFAULT_WEATHER_CITY);
    Weather_SetConfig(s_wLat, s_wLon, s_wCity);

    s_cryptoSym = s_prefs.getString(PREF_KEY_CRYPTO_SYM, DEFAULT_CRYPTO_SYM);
    Crypto_SetSymbol(s_cryptoSym);

    s_ytChannel = s_prefs.getString(PREF_KEY_YT_CHANNEL, "");
    s_ytApiKey = s_prefs.getString(PREF_KEY_YT_APIKEY, "");
    YouTube_SetCredentials(s_ytChannel, s_ytApiKey);

    s_mqttEn = s_prefs.getBool(PREF_KEY_MQTT_ENABLED, false);
    s_mqttServer = s_prefs.getString(PREF_KEY_MQTT_SERVER, "");
    s_mqttPort = s_prefs.getUShort(PREF_KEY_MQTT_PORT, DEFAULT_MQTT_PORT);
    s_mqttUser = s_prefs.getString(PREF_KEY_MQTT_USER, "");
    s_mqttPass = s_prefs.getString(PREF_KEY_MQTT_PASS, "");
    s_mqttPrefix = s_prefs.getString(PREF_KEY_MQTT_PREFIX, DEFAULT_MQTT_PREFIX);

    WiFi.persistent(false);
    WiFi.mode(WIFI_OFF);

    s_server.on("/", HTTP_GET, []() {
        s_server.send(200, "text/html", buildPortalHtml());
    });
    s_server.on("/save", HTTP_POST, handleSave);
    s_server.on("/scan", HTTP_GET, handleScan);
    s_server.on("/status", HTTP_GET, handleStatus);

    s_server.on("/api/display", HTTP_GET, []() {
        String json;
        DisplayMatrix_GetJson(json);
        s_server.send(200, "application/json", json);
    });

    s_server.on("/api/display/screenshot", HTTP_GET, []() {
        s_server.sendHeader("Content-Disposition", "inline; filename=\"tc001_screenshot.bmp\"");
        s_server.setContentLength(76854);
        s_server.send(200, "image/bmp", "");
        WiFiClient client = s_server.client();
        DisplayMatrix_StreamBMP(client, 10);
    });

    s_server.on("/api/display/text", HTTP_GET, []() {
        String ascii = DisplayMatrix_GetAsciiArt();
        s_server.send(200, "text/plain", ascii);
    });

    s_server.on("/api/screen", HTTP_GET, []() {
        if (s_server.hasArg("auto")) {
            Network_SetAutoRotate(s_server.arg("auto").toInt() != 0);
        }
        if (s_server.hasArg("set")) {
            int app = s_server.arg("set").toInt();
            if (app >= 0 && app < 7) {
                Network_SetActiveApp((uint8_t)app);
                if (!s_server.hasArg("auto")) {
                    Network_SetAutoRotate(false);
                }
            }
        }
        JsonDocument doc;
        doc["active_app"] = Network_GetActiveApp();
        doc["auto_rotate"] = Network_GetAutoRotate();
        static const char* kNames[] = {
            "Clock", "Indoor Environment", "Outdoor Weather",
            "Crypto Ticker", "YouTube Analytics", "Live Airspace", "Battery & Power"
        };
        doc["app_name"] = kNames[Network_GetActiveApp() % 7];
        String out;
        serializeJson(doc, out);
        s_server.send(200, "application/json", out);
    });

    // Custom App & Notification REST Push API
    s_server.on("/api/custom", HTTP_GET, []() {
        String json;
        CustomApp_GetJsonList(json);
        s_server.send(200, "application/json", json);
    });

    s_server.on("/api/custom", HTTP_POST, []() {
        String appName = s_server.hasArg("name") ? s_server.arg("name") : "custom";
        if (s_server.hasArg("delete") && s_server.arg("delete").toInt() != 0) {
            CustomApp_Delete(appName);
            s_server.send(200, "application/json", "{\"status\":\"deleted\"}");
            return;
        }

        String body = s_server.arg("plain");
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, body);
        if (err) {
            if (s_server.hasArg("text")) {
                doc["text"] = s_server.arg("text");
                if (s_server.hasArg("icon")) doc["icon"] = s_server.arg("icon");
                if (s_server.hasArg("progress")) doc["progress"] = s_server.arg("progress").toInt();
                if (s_server.hasArg("duration")) doc["duration"] = s_server.arg("duration").toInt();
                if (s_server.hasArg("lifetime")) doc["lifetime"] = s_server.arg("lifetime").toInt();
            } else {
                s_server.send(400, "application/json", "{\"error\":\"Invalid JSON or missing text\"}");
                return;
            }
        }
        if (doc["name"].is<const char*>()) {
            appName = doc["name"].as<const char*>();
        }

        bool ok = CustomApp_Push(appName, doc.as<JsonVariantConst>());
        if (ok) {
            s_server.send(200, "application/json", "{\"status\":\"ok\",\"app\":\"" + appName + "\"}");
        } else {
            s_server.send(500, "application/json", "{\"error\":\"Failed to store app\"}");
        }
    });

    s_server.on("/api/custom", HTTP_DELETE, []() {
        String appName = s_server.hasArg("name") ? s_server.arg("name") : "";
        if (appName.length() > 0) {
            CustomApp_Delete(appName);
            s_server.send(200, "application/json", "{\"status\":\"deleted\"}");
        } else {
            CustomApp_Clear();
            s_server.send(200, "application/json", "{\"status\":\"all_cleared\"}");
        }
    });

    s_server.on("/api/notify", HTTP_POST, []() {
        String body = s_server.arg("plain");
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, body);
        if (err) {
            if (s_server.hasArg("text")) {
                doc["text"] = s_server.arg("text");
                if (s_server.hasArg("icon")) doc["icon"] = s_server.arg("icon");
                if (s_server.hasArg("sound")) doc["sound"] = s_server.arg("sound").toInt();
                if (s_server.hasArg("duration")) doc["duration"] = s_server.arg("duration").toInt();
            } else {
                s_server.send(400, "application/json", "{\"error\":\"Invalid JSON or missing text\"}");
                return;
            }
        }
        CustomApp_PushNotification(doc.as<JsonVariantConst>());
        s_server.send(200, "application/json", "{\"status\":\"notified\"}");
    });

    // Initialize Over-The-Air Update System
    OTA_Init(s_server);

    // Captive Portal OS detection probes
    s_server.on("/generate_204", HTTP_GET, handleCaptiveRedirect);
    s_server.on("/hotspot-detect.html", HTTP_GET, handleCaptiveRedirect);
    s_server.on("/ncsi.txt", HTTP_GET, handleCaptiveRedirect);
    s_server.on("/connecttest.txt", HTTP_GET, handleCaptiveRedirect);
    s_server.on("/redirect", HTTP_GET, handleCaptiveRedirect);
    s_server.on("/canonical.html", HTTP_GET, handleCaptiveRedirect);

    s_server.onNotFound([]() {
        if (s_netState == NET_STATE_AP_PORTAL) {
            handleCaptiveRedirect();
        } else {
            s_server.send(404, "text/plain", "Not Found");
        }
    });

    if (s_ssid.length() > 0) {
        Network_ConnectStation();
    } else {
        Network_StartAP();
    }
}

void Network_ConnectStation() {
    Serial.printf("[WiFi] Connecting to saved SSID '%s'...\n", s_ssid.c_str());
    s_netState = NET_STATE_CONNECTING;
    s_connectStartMs = millis();

    if (WiFi.getMode() == WIFI_STA) {
        WiFi.disconnect(true);
    }
    WiFi.mode(WIFI_STA);
    WiFi.begin(s_ssid.c_str(), s_pass.c_str());
    s_server.begin();
}

void Network_StartAP() {
    Serial.printf("[WiFi] Starting AP: %s (IP: %s)\n", s_apSSID.c_str(), s_apIP.toString().c_str());
    if (WiFi.getMode() == WIFI_STA) {
        WiFi.disconnect(true);
    }
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(s_apIP, s_apIP, s_apNetmask);
    WiFi.softAP(s_apSSID.c_str());

    s_dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    s_dnsServer.start(53, "*", s_apIP);

    s_server.begin();
    s_netState = NET_STATE_AP_PORTAL;
}

void Network_Poll() {
    uint32_t now = millis();

    // Service OTA Reboot Timer
    OTA_Poll();

    if (s_pendingReconnect && (now >= s_reconnectTriggerMs)) {
        s_pendingReconnect = false;
        s_dnsServer.stop();
        Network_ConnectStation();
        return;
    }

    if (s_netState == NET_STATE_AP_PORTAL) {
        s_dnsServer.processNextRequest();
        s_server.handleClient();
        return;
    }

    if (s_netState == NET_STATE_CONNECTING) {
        s_server.handleClient();
        if (WiFi.status() == WL_CONNECTED) {
            s_netState = NET_STATE_CONNECTED;
            Serial.printf("[WiFi] Connected! IP: %s, RSSI: %d dBm\n",
                          WiFi.localIP().toString().c_str(), WiFi.RSSI());
        } else if (now - s_connectStartMs >= WIFI_CONNECT_TIMEOUT_MS) {
            Serial.printf("[WiFi] Connection timeout. Falling back to AP mode.\n");
            Network_StartAP();
        }
        return;
    }

    if (s_netState == NET_STATE_CONNECTED) {
        s_server.handleClient();

        if (now - s_lastWifiCheckMs >= 10000) {
            s_lastWifiCheckMs = now;
            if (WiFi.status() != WL_CONNECTED) {
                Serial.printf("[WiFi] Connection lost. Reconnecting...\n");
                Network_ConnectStation();
            }
        }
    }
}

NetworkState Network_GetState() {
    return s_netState;
}

bool Network_IsConnected() {
    return (s_netState == NET_STATE_CONNECTED && WiFi.status() == WL_CONNECTED);
}

bool Network_IsAPActive() {
    return (s_netState == NET_STATE_AP_PORTAL);
}

String Network_GetSSID() {
    return (s_netState == NET_STATE_CONNECTED) ? WiFi.SSID() : s_ssid;
}

String Network_GetLocalIP() {
    return (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : "";
}

String Network_GetAPIP() {
    return s_apIP.toString();
}

String Network_GetAPSSID() {
    return s_apSSID;
}

int8_t Network_GetRSSI() {
    return (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0;
}

bool Network_HasSavedCredentials() {
    return (s_ssid.length() > 0);
}

void Network_SaveCredentials(const String& ssid, const String& pass) {
    s_ssid = ssid;
    s_pass = pass;
    s_prefs.putString(PREF_KEY_SSID, s_ssid);
    s_prefs.putString(PREF_KEY_PASS, s_pass);
    Serial.printf("[WiFi] Saved credentials for SSID '%s' to NVS\n", s_ssid.c_str());
}

void Network_ForgetCredentials() {
    s_ssid = "";
    s_pass = "";
    s_prefs.remove(PREF_KEY_SSID);
    s_prefs.remove(PREF_KEY_PASS);
    Serial.printf("[WiFi] Cleared Wi-Fi credentials from NVS\n");
    Network_StartAP();
}

uint16_t Network_GetRotateDelaySec() {
    return s_rotateSec;
}

bool Network_GetAutoRotate() {
    return s_autoRotate;
}

void Network_SetAutoRotate(bool en) {
    if (s_autoRotate != en) {
        s_autoRotate = en;
        s_prefs.putBool(PREF_KEY_AUTO_ROTATE, s_autoRotate);
        Serial.printf("[Settings] Carousel auto-rotate set to %s\n", s_autoRotate ? "ENABLED" : "DISABLED");
    }
}

uint8_t Network_GetActiveApp() {
    return s_activeApp;
}

void Network_SetActiveApp(uint8_t app) {
    if (app < 7) {
        s_activeApp = app;
        s_prefs.putUChar(PREF_KEY_ACTIVE_APP, s_activeApp);
        Serial.printf("[Settings] Active screen set to %u\n", s_activeApp);
    }
}

void Network_SetRotateDelaySec(uint16_t sec) {
    s_rotateSec = constrain(sec, (uint16_t)MIN_ROTATE_SEC, (uint16_t)MAX_ROTATE_SEC);
    s_prefs.putUInt(PREF_KEY_ROTATE_SEC, s_rotateSec);
    Serial.printf("[Settings] Carousel rotate delay set to %u sec\n", s_rotateSec);
}

String Network_GetYouTubeChannel() {
    return s_ytChannel;
}

String Network_GetYouTubeApiKey() {
    return s_ytApiKey;
}

void Network_SaveYouTubeConfig(const String& channel, const String& apiKey) {
    s_ytChannel = channel;
    s_ytApiKey = apiKey;
    s_prefs.putString(PREF_KEY_YT_CHANNEL, s_ytChannel);
    s_prefs.putString(PREF_KEY_YT_APIKEY, s_ytApiKey);
    YouTube_SetCredentials(s_ytChannel, s_ytApiKey);
}

bool Network_GetAutoBrightness() {
    return s_autoBri;
}

void Network_SetAutoBrightness(bool en) {
    s_autoBri = en;
    s_prefs.putBool(PREF_KEY_AUTO_BRI, s_autoBri);
}

bool Network_GetNightMode() {
    return s_nightMode;
}

void Network_SetNightMode(bool en) {
    s_nightMode = en;
    s_prefs.putBool(PREF_KEY_NIGHT_MODE, s_nightMode);
}

uint8_t Network_GetNightStartHour() {
    return s_nightStart;
}

uint8_t Network_GetNightEndHour() {
    return s_nightEnd;
}

void Network_SetNightSchedule(uint8_t startHour, uint8_t endHour) {
    s_nightStart = startHour;
    s_nightEnd = endHour;
    s_prefs.putUChar(PREF_KEY_NIGHT_START, s_nightStart);
    s_prefs.putUChar(PREF_KEY_NIGHT_END, s_nightEnd);
}

uint8_t Network_GetDayBrightness() {
    return s_dayBri;
}

void Network_SetDayBrightness(uint8_t bri) {
    s_dayBri = constrain(bri, (uint8_t)5, (uint8_t)100);
    s_prefs.putUChar(PREF_KEY_DAY_BRI, s_dayBri);
}

uint8_t Network_GetNightBrightness() {
    return s_nightBri;
}

void Network_SetNightBrightness(uint8_t bri) {
    s_nightBri = constrain(bri, (uint8_t)1, (uint8_t)25);
    s_prefs.putUChar(PREF_KEY_NIGHT_BRI, s_nightBri);
}

float Network_GetWeatherLat() {
    return s_wLat;
}

float Network_GetWeatherLon() {
    return s_wLon;
}

String Network_GetWeatherCity() {
    return s_wCity;
}

void Network_SetWeatherLocation(float lat, float lon, const String& city) {
    s_wLat = lat;
    s_wLon = lon;
    s_wCity = city;
    s_prefs.putFloat(PREF_KEY_WEATHER_LAT, s_wLat);
    s_prefs.putFloat(PREF_KEY_WEATHER_LON, s_wLon);
    s_prefs.putString(PREF_KEY_WEATHER_CITY, s_wCity);
    Weather_SetConfig(s_wLat, s_wLon, s_wCity);
}

String Network_GetCryptoSymbol() {
    return s_cryptoSym;
}

void Network_SetCryptoSymbol(const String& sym) {
    s_cryptoSym = sym;
    s_prefs.putString(PREF_KEY_CRYPTO_SYM, s_cryptoSym);
    Crypto_SetSymbol(s_cryptoSym);
}

uint8_t Network_GetClockMode() { return s_clockMode; }
void Network_SetClockMode(uint8_t mode) {
    s_clockMode = (mode <= 2) ? mode : 0;
    s_prefs.putUChar(PREF_KEY_CLOCK_MODE, s_clockMode);
}

bool Network_GetWeekdayBar() { return s_weekdayBar; }
void Network_SetWeekdayBar(bool en) {
    s_weekdayBar = en;
    s_prefs.putBool(PREF_KEY_WEEKDAY_BAR, s_weekdayBar);
}

bool Network_GetWeekdayStartMonday() { return s_weekdayStartMon; }
void Network_SetWeekdayStartMonday(bool startMonday) {
    s_weekdayStartMon = startMonday;
    s_prefs.putBool(PREF_KEY_WD_START_MON, s_weekdayStartMon);
}

uint8_t Network_GetWeekdayActiveColorIndex() { return s_weekdayActiveColIdx; }
void Network_SetWeekdayActiveColorIndex(uint8_t idx) {
    s_weekdayActiveColIdx = (idx <= 5) ? idx : 0;
    s_prefs.putUChar(PREF_KEY_WD_ACTIVE_COL, s_weekdayActiveColIdx);
}
uint16_t Network_GetWeekdayActiveColor() {
    return s_wdActiveColors[(s_weekdayActiveColIdx <= 5) ? s_weekdayActiveColIdx : 0];
}

uint8_t Network_GetWeekdayInactiveColorIndex() { return s_weekdayInactColIdx; }
void Network_SetWeekdayInactiveColorIndex(uint8_t idx) {
    s_weekdayInactColIdx = (idx <= 3) ? idx : 0;
    s_prefs.putUChar(PREF_KEY_WD_INACT_COL, s_weekdayInactColIdx);
}
uint16_t Network_GetWeekdayInactiveColor() {
    if (s_weekdayInactColIdx == 0) {
        // Slightly dimmer than active day color (~65% intensity)
        uint16_t ac = Network_GetWeekdayActiveColor();
        uint8_t r = (ac >> 11) & 0x1F;
        uint8_t g = (ac >> 5) & 0x3F;
        uint8_t b = ac & 0x1F;
        r = (uint8_t)(((uint16_t)r * 170) >> 8);
        g = (uint8_t)(((uint16_t)g * 170) >> 8);
        b = (uint8_t)(((uint16_t)b * 170) >> 8);
        return (r << 11) | (g << 5) | b;
    }
    return s_wdInactiveColors[(s_weekdayInactColIdx <= 3) ? s_weekdayInactColIdx : 0];
}

uint8_t Network_GetDateFormat() { return s_dateFormat; }
void Network_SetDateFormat(uint8_t fmt) {
    s_dateFormat = (fmt <= 3) ? fmt : 0;
    s_prefs.putUChar(PREF_KEY_DATE_FORMAT, s_dateFormat);
}

uint8_t Network_GetClockIcon() { return s_clockIcon; }
void Network_SetClockIcon(uint8_t icon) {
    s_clockIcon = (icon <= 3) ? icon : 0;
    s_prefs.putUChar(PREF_KEY_CLOCK_ICON, s_clockIcon);
}

uint8_t Network_GetTextAlign() { return s_textAlign; }
void Network_SetTextAlign(uint8_t align) {
    s_textAlign = (align <= 1) ? align : 0;
    s_prefs.putUChar(PREF_KEY_TEXT_ALIGN, s_textAlign);
}

uint8_t Network_GetCarouselMask() { return s_carouselMask; }
void Network_SetCarouselMask(uint8_t mask) {
    s_carouselMask = (mask == 0) ? 0x01 : (mask & 0x7F);
    s_prefs.putUChar(PREF_KEY_CAROUSEL_MASK, s_carouselMask);
}

bool Network_IsAppInCarousel(uint8_t appIndex) {
    if (appIndex >= 7) return false;
    return (s_carouselMask & (1 << appIndex)) != 0;
}

uint8_t Network_GetTransitionFx() {
    return s_transitionFx;
}

void Network_SetTransitionFx(uint8_t fx) {
    s_transitionFx = (fx <= 7) ? fx : DEFAULT_TRANSITION_FX;
    s_prefs.putUChar(PREF_KEY_TRANSITION_FX, s_transitionFx);
    Serial.printf("[Settings] Transition effect set to %u\n", s_transitionFx);
}

bool Network_GetShowSeconds() { return s_showSeconds; }
void Network_SetShowSeconds(bool en) {
    s_showSeconds = en;
    s_prefs.putBool(PREF_KEY_SHOW_SECONDS, s_showSeconds);
}

const uint8_t* Network_GetCarouselOrder() {
    return s_carouselOrder;
}

String Network_GetCarouselOrderString() {
    String res = "";
    for (int i = 0; i < 7; i++) {
        if (i > 0) res += ",";
        res += String(s_carouselOrder[i]);
    }
    return res;
}

void Network_SetCarouselOrderString(const String& str) {
    int idx = 0;
    int start = 0;
    bool seen[7] = {false};
    uint8_t newOrder[7];
    
    while (idx < 7 && start < (int)str.length()) {
        int comma = str.indexOf(',', start);
        String token = (comma == -1) ? str.substring(start) : str.substring(start, comma);
        token.trim();
        int val = token.toInt();
        if (val >= 0 && val < 7 && !seen[val]) {
            newOrder[idx++] = (uint8_t)val;
            seen[val] = true;
        }
        if (comma == -1) break;
        start = comma + 1;
    }
    for (int i = 0; i < 7; i++) {
        if (!seen[i] && idx < 7) {
            newOrder[idx++] = (uint8_t)i;
            seen[i] = true;
        }
    }
    for (int i = 0; i < 7; i++) {
        s_carouselOrder[i] = newOrder[i];
    }
    s_prefs.putString(PREF_KEY_CAROUSEL_ORDER, Network_GetCarouselOrderString());
}

uint8_t Network_GetBatteryMode() {
    return s_batteryMode;
}

void Network_SetBatteryMode(uint8_t mode) {
    s_batteryMode = (mode <= 2) ? mode : 2;
    s_prefs.putUChar(PREF_KEY_BATTERY_MODE, s_batteryMode);
}

// Home Assistant & MQTT Getters & Setters
bool Network_GetMqttEnabled() { return s_mqttEn; }
void Network_SetMqttEnabled(bool en) {
    s_mqttEn = en;
    s_prefs.putBool(PREF_KEY_MQTT_ENABLED, s_mqttEn);
    MqttService_SetEnabled(en);
}

String Network_GetMqttServer() { return s_mqttServer; }
void Network_SetMqttServer(const String& srv) {
    s_mqttServer = srv;
    s_prefs.putString(PREF_KEY_MQTT_SERVER, s_mqttServer);
    MqttService_SetServer(srv);
}

uint16_t Network_GetMqttPort() { return s_mqttPort; }
void Network_SetMqttPort(uint16_t port) {
    s_mqttPort = port;
    s_prefs.putUShort(PREF_KEY_MQTT_PORT, s_mqttPort);
    MqttService_SetPort(port);
}

String Network_GetMqttUser() { return s_mqttUser; }
void Network_SetMqttUser(const String& user) {
    s_mqttUser = user;
    s_prefs.putString(PREF_KEY_MQTT_USER, s_mqttUser);
    MqttService_SetUser(user);
}

String Network_GetMqttPass() { return s_mqttPass; }
void Network_SetMqttPass(const String& pass) {
    s_mqttPass = pass;
    s_prefs.putString(PREF_KEY_MQTT_PASS, s_mqttPass);
    MqttService_SetPass(pass);
}

String Network_GetMqttPrefix() { return s_mqttPrefix; }
void Network_SetMqttPrefix(const String& prefix) {
    s_mqttPrefix = prefix;
    s_prefs.putString(PREF_KEY_MQTT_PREFIX, s_mqttPrefix);
    MqttService_SetPrefix(prefix);
}
