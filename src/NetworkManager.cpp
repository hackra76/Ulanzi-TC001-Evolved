#include "NetworkManager.h"
#include <ArduinoJson.h>

static Preferences s_prefs;
static WebServer s_server(80);
static DNSServer s_dnsServer;

static NetworkState s_netState = NET_STATE_IDLE;
static String s_ssid = "";
static String s_pass = "";
static String s_apSSID = "";
static IPAddress s_apIP(192, 168, 4, 1);
static IPAddress s_apNetmask(255, 255, 255, 0);

static uint32_t s_connectStartMs = 0;
static uint32_t s_lastWifiCheckMs = 0;
static bool s_pendingReconnect = false;
static uint32_t s_reconnectTriggerMs = 0;

// Embedded Portal HTML Page (Mobile responsive, Dark Glassmorphism)
static const char PAGE_PORTAL[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Ulanzi TC001 Evolved &bull; Wi-Fi Setup</title>
<style>
  :root {
    --bg: #090d16;
    --card: rgba(22, 30, 49, 0.85);
    --border: rgba(255, 255, 255, 0.1);
    --accent: #38bdf8;
    --accent-glow: rgba(56, 189, 248, 0.25);
    --text: #f1f5f9;
    --text-muted: #94a3b8;
    --success: #10b981;
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
    max-width: 420px;
    background: var(--card);
    border: 1px solid var(--border);
    border-radius: 1.25rem;
    padding: 2rem;
    backdrop-filter: blur(12px);
    box-shadow: 0 20px 40px rgba(0,0,0,0.5), 0 0 40px var(--accent-glow);
  }
  .header {
    text-align: center;
    margin-bottom: 1.75rem;
  }
  .header .badge {
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
    margin-bottom: 0.75rem;
  }
  h1 { font-size: 1.5rem; font-weight: 700; margin-bottom: 0.25rem; }
  p.sub { font-size: 0.875rem; color: var(--text-muted); }
  .field { margin-bottom: 1.25rem; }
  label {
    display: block;
    font-size: 0.8125rem;
    font-weight: 600;
    margin-bottom: 0.5rem;
    color: var(--text-muted);
    text-transform: uppercase;
    letter-spacing: 0.04em;
  }
  input, select {
    width: 100%;
    padding: 0.85rem 1rem;
    background: rgba(15, 23, 42, 0.8);
    border: 1px solid var(--border);
    border-radius: 0.75rem;
    color: var(--text);
    font-size: 0.95rem;
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
    gap: 0.5rem;
    background: transparent;
    border: 1px solid var(--border);
    color: var(--accent);
    padding: 0.4rem 0.85rem;
    border-radius: 0.5rem;
    font-size: 0.8rem;
    font-weight: 600;
    cursor: pointer;
    margin-top: 0.4rem;
    transition: background 0.2s;
  }
  .scan-btn:hover { background: rgba(56, 189, 248, 0.1); }
  .btn-submit {
    width: 100%;
    padding: 0.9rem;
    margin-top: 0.75rem;
    background: linear-gradient(135deg, #0284c7 0%, #38bdf8 100%);
    border: none;
    border-radius: 0.75rem;
    color: #fff;
    font-size: 1rem;
    font-weight: 700;
    cursor: pointer;
    box-shadow: 0 8px 20px rgba(56, 189, 248, 0.35);
    transition: transform 0.1s, opacity 0.2s;
  }
  .btn-submit:hover { opacity: 0.95; }
  .btn-submit:active { transform: scale(0.98); }
  .footer {
    text-align: center;
    margin-top: 1.5rem;
    font-size: 0.75rem;
    color: var(--text-muted);
  }
</style>
</head>
<body>
<div class="card">
  <div class="header">
    <div class="badge">ESP32 Pixel Matrix Clock</div>
    <h1>Ulanzi TC001 Evolved</h1>
    <p class="sub">Select a 2.4 GHz Wi-Fi network to connect your clock.</p>
  </div>
  <form action="/save" method="POST">
    <div class="field">
      <label for="ssid">Wi-Fi Network (SSID)</label>
      <input type="text" id="ssid" name="ssid" placeholder="Enter network name" required autocomplete="off">
      <button type="button" class="scan-btn" onclick="scanNetworks()">&#x21bb; Scan Networks</button>
      <select id="scanSelect" style="display:none; margin-top:0.5rem;" onchange="selectNetwork(this.value)">
        <option value="">-- Choose detected network --</option>
      </select>
    </div>
    <div class="field">
      <label for="password">Wi-Fi Password</label>
      <input type="password" id="password" name="password" placeholder="Enter network password">
    </div>
    <button type="submit" class="btn-submit">Connect Device</button>
  </form>
  <div class="footer">Ulanzi TC001 Evolved &bull; Firmware v1.0.0</div>
</div>
<script>
function scanNetworks() {
  const sel = document.getElementById('scanSelect');
  sel.style.display = 'block';
  sel.innerHTML = '<option>Scanning nearby networks...</option>';
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
)rawliteral";

static const char PAGE_SAVED[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Saved &bull; Ulanzi TC001 Evolved</title>
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
  <h2>Credentials Saved</h2>
  <p>The Ulanzi TC001 is now connecting to your Wi-Fi network.<br><br>Watch the LED matrix on the clock for the scrolling IP address.</p>
</div>
</body>
</html>
)rawliteral";

// Captive Portal DNS Redirection
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
    s_server.send_P(200, "text/html", PAGE_PORTAL);
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
    if (s_server.hasArg("ssid")) {
        String newSsid = s_server.arg("ssid");
        String newPass = s_server.arg("password");
        newSsid.trim();
        newPass.trim();

        if (newSsid.length() > 0) {
            Network_SaveCredentials(newSsid, newPass);
            s_server.send_P(200, "text/html", PAGE_SAVED);
            s_pendingReconnect = true;
            s_reconnectTriggerMs = millis() + 1500;
            return;
        }
    }
    s_server.send(400, "text/plain", "Missing SSID");
}

static void handleStatus() {
    JsonDocument doc;
    doc["state"] = (s_netState == NET_STATE_CONNECTED) ? "connected" : 
                   (s_netState == NET_STATE_AP_PORTAL ? "ap_portal" : "connecting");
    doc["ip"] = Network_GetLocalIP();
    doc["ap_ip"] = Network_GetAPIP();
    doc["ssid"] = Network_GetSSID();
    doc["rssi"] = Network_GetRSSI();
    String out;
    serializeJson(doc, out);
    s_server.send(200, "application/json", out);
}

void Network_Init() {
    // Generate chip-unique AP SSID
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

    WiFi.persistent(false);
    WiFi.mode(WIFI_OFF);

    // Setup web server routes
    s_server.on("/", HTTP_GET, []() {
        s_server.send_P(200, "text/html", PAGE_PORTAL);
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

    // Check for delayed reconnect after user submitted credentials
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

        // Periodic link monitor (every 10 seconds)
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
