#include "NetworkManager.h"
#include <ArduinoJson.h>
#include "YouTubeService.h"
#include "WeatherService.h"
#include "CryptoService.h"
#include "Sensors.h"
#include "OtaManager.h"

static Preferences s_prefs;
static WebServer s_server(80);
static DNSServer s_dnsServer;

static NetworkState s_netState = NET_STATE_IDLE;
static String s_ssid = "";
static String s_pass = "";
static String s_apSSID = "";
static IPAddress s_apIP(192, 168, 4, 1);
static IPAddress s_apNetmask(255, 255, 255, 0);

static uint16_t s_rotateSec = DEFAULT_ROTATE_SEC;
static String s_ytChannel = "";
static String s_ytApiKey = "";

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
  .footer { text-align: center; margin-top: 1.25rem; font-size: 0.75rem; color: var(--text-muted); }
</style>
</head>
<body>
<div class="card">
  <div class="header">
    <div class="badge">Pixel Clock Setup</div>
    <h1>Ulanzi TC001 Evolved</h1>
    <p class="sub">Configure Network, Ambient Screen, Weather, Tickers & Firmware</p>
  </div>
  <form action="/save" method="POST">
    
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

    <div class="field">
      <label for="rotate">Carousel Rotate Delay (seconds, 3 - 120)</label>
      <input type="number" id="rotate" name="rotate" min="3" max="120" value="{{ROTATE}}" required>
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

    <div class="section-title">&#x1F4B0; Crypto Ticker (Binance Live)</div>
    <div class="field">
      <label for="c_sym">Cryptocurrency Pair</label>
      <select id="c_sym" name="c_sym">
        <option value="BTC" {{C_BTC_SEL}}>Bitcoin (BTC / USDT)</option>
        <option value="ETH" {{C_ETH_SEL}}>Ethereum (ETH / USDT)</option>
        <option value="SOL" {{C_SOL_SEL}}>Solana (SOL / USDT)</option>
        <option value="BNB" {{C_BNB_SEL}}>Binance Coin (BNB / USDT)</option>
      </select>
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
</script>
</body>
</html>
)rawliteral");

    html.replace("{{SSID}}", s_ssid);
    html.replace("{{ROTATE}}", String(s_rotateSec));
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

    html.replace("{{YT_CHANNEL}}", s_ytChannel);
    html.replace("{{YT_KEY}}", s_ytApiKey.length() > 0 ? "********" : "");
    html.replace("{{FW_VERSION}}", FW_VERSION);
    return html;
}

static const char PAGE_SAVED[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
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
  <p>Your configuration has been updated and stored in NVS.<br><br>The clock display will update automatically.</p>
</div>
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

    // 2. Process Carousel Rotate Delay
    if (s_server.hasArg("rotate")) {
        int r = s_server.arg("rotate").toInt();
        if (r >= MIN_ROTATE_SEC && r <= MAX_ROTATE_SEC) {
            Network_SetRotateDelaySec((uint16_t)r);
        }
    }

    // 3. Process Ambient, Day/Night Brightness & Night Mode
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

    // 5. Process Crypto Symbol
    if (s_server.hasArg("c_sym")) {
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

    s_server.send_P(200, "text/html", PAGE_SAVED);

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
    s_rotateSec = (uint16_t)s_prefs.getUInt(PREF_KEY_ROTATE_SEC, DEFAULT_ROTATE_SEC);
    if (s_rotateSec < MIN_ROTATE_SEC || s_rotateSec > MAX_ROTATE_SEC) {
        s_rotateSec = DEFAULT_ROTATE_SEC;
    }

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

    WiFi.persistent(false);
    WiFi.mode(WIFI_OFF);

    s_server.on("/", HTTP_GET, []() {
        s_server.send(200, "text/html", buildPortalHtml());
    });
    s_server.on("/save", HTTP_POST, handleSave);
    s_server.on("/scan", HTTP_GET, handleScan);
    s_server.on("/status", HTTP_GET, handleStatus);

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

    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.begin(s_ssid.c_str(), s_pass.c_str());
    s_server.begin();
}

void Network_StartAP() {
    Serial.printf("[WiFi] Starting AP: %s (IP: %s)\n", s_apSSID.c_str(), s_apIP.toString().c_str());
    WiFi.disconnect(true);
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
