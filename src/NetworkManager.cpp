#include "NetworkManager.h"
#include <ArduinoJson.h>
#include "YouTubeService.h"

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
    --card: rgba(22, 30, 49, 0.88);
    --border: rgba(255, 255, 255, 0.1);
    --accent: #38bdf8;
    --accent-glow: rgba(56, 189, 248, 0.25);
    --text: #f1f5f9;
    --text-muted: #94a3b8;
    --red: #ef4444;
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
    padding: 1.25rem;
    background-image: radial-gradient(circle at 50% 0%, #1e293b 0%, #090d16 100%);
  }
  .card {
    width: 100%;
    max-width: 440px;
    background: var(--card);
    border: 1px solid var(--border);
    border-radius: 1.25rem;
    padding: 2rem;
    backdrop-filter: blur(12px);
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
    padding-bottom: 0.25rem;
    border-bottom: 1px solid var(--border);
  }
  .field { margin-bottom: 1rem; }
  label {
    display: block;
    font-size: 0.8rem;
    font-weight: 600;
    margin-bottom: 0.4rem;
    color: var(--text-muted);
  }
  input, select {
    width: 100%;
    padding: 0.75rem 0.9rem;
    background: rgba(15, 23, 42, 0.8);
    border: 1px solid var(--border);
    border-radius: 0.65rem;
    color: var(--text);
    font-size: 0.9rem;
    outline: none;
    transition: all 0.2s ease;
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
    padding: 0.35rem 0.75rem;
    border-radius: 0.5rem;
    font-size: 0.75rem;
    font-weight: 600;
    cursor: pointer;
    margin-top: 0.4rem;
  }
  .scan-btn:hover { background: rgba(56, 189, 248, 0.1); }
  .btn-submit {
    width: 100%;
    padding: 0.85rem;
    margin-top: 1.25rem;
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
    <p class="sub">Configure Wi-Fi, Screen Rotation, and YouTube Analytics</p>
  </div>
  <form action="/save" method="POST">
    <div class="section-title">&#x1F4F6; Wi-Fi Connection</div>
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

    <div class="section-title">&#x23F1; Carousel Display</div>
    <div class="field">
      <label for="rotate">Rotate Delay (seconds, 3 - 120)</label>
      <input type="number" id="rotate" name="rotate" min="3" max="120" value="{{ROTATE}}" required>
    </div>

    <div class="section-title">&#x25B6; YouTube Analytics</div>
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
  <div class="footer">Ulanzi TC001 Evolved &bull; Firmware v1.0.0</div>
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
</script>
</body>
</html>
)rawliteral");

    html.replace("{{SSID}}", s_ssid);
    html.replace("{{ROTATE}}", String(s_rotateSec));
    html.replace("{{YT_CHANNEL}}", s_ytChannel);
    html.replace("{{YT_KEY}}", s_ytApiKey.length() > 0 ? "********" : "");
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

    // 3. Process YouTube Analytics
    if (s_server.hasArg("yt_channel")) {
        String ch = s_server.arg("yt_channel");
        String key = s_server.arg("yt_key");
        ch.trim();
        key.trim();
        if (key == "********") {
            key = s_ytApiKey; // Preserve existing key
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
    doc["yt_channel"] = s_ytChannel;
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
    Serial.printf("[Settings] Saved YouTube channel '%s' to NVS\n", s_ytChannel.c_str());
}
