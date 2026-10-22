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
#include "TimerService.h"
#include "ScreensaverService.h"

static Preferences s_prefs;
static WebServer s_server(80);
static DNSServer s_dnsServer;

static NetworkState s_netState = NET_STATE_IDLE;
static String s_ssid = "";
static String s_pass = "";

struct WifiProfile {
    String ssid;
    String pass;
};
static WifiProfile s_wifiProfiles[MAX_WIFI_PROFILES];
static int s_activeProfileIdx = 0;

static const char* const s_wifiSsidKeys[MAX_WIFI_PROFILES] = {
    PREF_KEY_SSID, PREF_KEY_SSID_2, PREF_KEY_SSID_3, PREF_KEY_SSID_4
};
static const char* const s_wifiPassKeys[MAX_WIFI_PROFILES] = {
    PREF_KEY_PASS, PREF_KEY_PASS_2, PREF_KEY_PASS_3, PREF_KEY_PASS_4
};

static String s_apSSID = "";
static IPAddress s_apIP(192, 168, 4, 1);
static IPAddress s_apNetmask(255, 255, 255, 0);

static bool s_autoRotate = true;
static uint8_t s_activeApp = 0;
static uint16_t s_rotateSec = DEFAULT_ROTATE_SEC;
static uint16_t s_carouselMask = 0x1FF; // Bitmask of enabled carousel screens (default: all 9 active)
static uint8_t s_carouselOrder[9] = {0, 1, 2, 3, 4, 5, 6, 7, 8}; // Screen display sequence
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
static uint8_t s_weekdayActiveColIdx = 0;   // Active day color
static uint8_t s_weekdayInactColIdx = 0;    // Inactive day color
static uint8_t s_weekdayWeekendColIdx = 0;  // Weekend day color (0: Same as active)
static String s_portalLang = "en";          // Portal language ("en" or "sk")
static uint8_t s_dateFormat = 0;      // 0: DD.MM., 1: DD MMM, 2: MM/DD, 3: DAY DD
static uint8_t s_clockIcon = 0;       // 0: Dynamic Calendar, 1: Heart, 2: Clock Face, 3: None
static uint8_t s_textAlign = 0;       // 0: Left-Aligned with Icon (Awtrix), 1: Centered

static const uint16_t s_wdActiveColors[] = {
    0x07FF, // 0: Cyan (#00F0FF)
    0xFD20, // 1: Orange (#FF8800)
    0x07E0, // 2: Green (#00FF00)
    0xFFE0, // 3: Yellow (#FFFF00)
    0xF800, // 4: Red (#FF2200)
    0xFFFF, // 5: White (#FFFFFF)
    0xF81F, // 6: Magenta (#FF00FF)
    0x033F, // 7: Electric Blue (#0066FF)
    0x77E0, // 8: Lime Green (#76FF03)
    0xFE00, // 9: Amber Gold (#FFC107)
    0xFA10  // 10: Soft Pink (#FF4081)
};
static const size_t s_numWdActiveColors = sizeof(s_wdActiveColors) / sizeof(s_wdActiveColors[0]);

static const uint16_t s_wdInactiveColors[] = {
    0x0000, // 0: Auto (computed dynamically from active color at ~65% intensity)
    0x7412, // 1: Slate Gray (#708090)
    0x9342, // 2: Dim Amber (#906810)
    0x00C8, // 3: Dark Blue (#001A40)
    0x3801, // 4: Deep Wine (#3A000A)
    0x0141, // 5: Forest Green (#002A0D)
    0x2124, // 6: Charcoal (#252525)
    0x2886, // 7: Muted Purple (#2A1035)
    0x0187, // 8: Dark Cyan (#003038)
    0x0000  // 9: Off (Black)
};
static const size_t s_numWdInactiveColors = sizeof(s_wdInactiveColors) / sizeof(s_wdInactiveColors[0]);

static const uint16_t s_wdWeekendColors[] = {
    0x0000, // 0: Same as Active Day Color
    0xF800, // 1: Bright Red (#FF2200)
    0xFB49, // 2: Coral Orange (#FF6B4A)
    0xFEA0, // 3: Gold (#FFD700)
    0xF8B2, // 4: Hot Pink (#FF1493)
    0x9936, // 5: Purple (#9C27B0)
    0x07FF, // 6: Cyan (#00F0FF)
    0x77E0, // 7: Lime (#76FF03)
    0xFFFF, // 8: Pure White (#FFFFFF)
    0x05FF, // 9: Sky Blue (#00BFFF)
    0x072E  // 10: Emerald (#00E676)
};
static const size_t s_numWdWeekendColors = sizeof(s_wdWeekendColors) / sizeof(s_wdWeekendColors[0]);

static uint32_t s_connectStartMs = 0;
static uint32_t s_lastWifiCheckMs = 0;
static bool s_pendingReconnect = false;
static uint32_t s_reconnectTriggerMs = 0;

static const char PAGE_PORTAL[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover">
<title>ULANZI TC001 // FIELD INSTRUMENT TERMINAL</title>
<style>
/* =============================================================================
   TACTILE FIELD INSTRUMENT DESIGN SYSTEM
   Enforcing Anti-Slop: Zero slate blues, zero floating bento cards, zero glass,
   zero soft pill badges, zero generic sans readouts.
   Palette: Warm Carbon (#121211), Milled Plate (#171716), Inset (#0B0B0A),
   Hairlines (#2A2A28), Primary Ink (#F4F3EF), Signal Orange (#FF4F00),
   Lab Ochre (#E5A93C), Phosphor Green (#00D062).
   ============================================================================= */

:root {
  --chassis-base:       #121211;
  --chassis-plate:      #171716;
  --chassis-inset:      #0B0B0A;
  --chassis-well:       #1D1D1B;
  --chassis-hover:      #242422;
  --chassis-active:     #2D2D2A;

  --hairline-main:      #2A2A28;
  --hairline-dim:       #1E1E1C;
  --hairline-bright:    #3E3E3A;

  --ink-primary:        #F4F3EF;
  --ink-secondary:      #B2B1AC;
  --ink-muted:          #6A6965;
  --ink-dim:            #42413E;

  --signal-orange:      #FF4F00;
  --signal-orange-dim:  #3D1402;
  --lab-ochre:          #E5A93C;
  --phosphor-green:     #00D062;
  --phosphor-dim:       #062A14;
  --signal-red:         #E53935;

  --font-header:        'Space Grotesk', -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
  --font-mono:          'JetBrains Mono', 'IBM Plex Mono', 'SF Mono', Consolas, ui-monospace, monospace;
}

/* Base Reset & Industrial Substrate */
*, *::before, *::after {
  box-sizing: border-box;
  margin: 0;
  padding: 0;
  border-radius: 0 !important; /* Anti-Slop: ZERO rounded corners on base */
}

html, body {
  min-height: 100vh;
  background-color: var(--chassis-base);
  color: var(--ink-primary);
  font-family: var(--font-mono);
  font-size: 13px;
  line-height: 1.4;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  overflow-x: hidden;
}

/* Custom Industrial Hairline Scrollbar */
::-webkit-scrollbar {
  width: 6px;
  height: 6px;
}
::-webkit-scrollbar-track {
  background: var(--chassis-inset);
  border-left: 1px solid var(--hairline-dim);
}
::-webkit-scrollbar-thumb {
  background: var(--hairline-bright);
}
::-webkit-scrollbar-thumb:hover {
  background: var(--ink-muted);
}

/* Terminal Shell */
.terminal-shell {
  width: 100%;
  max-width: 980px;
  margin: 0 auto;
  border-left: 1px solid var(--hairline-main);
  border-right: 1px solid var(--hairline-main);
  background: var(--chassis-base);
  min-height: 100vh;
  display: flex;
  flex-direction: column;
}

/* Machinery Bar (Sticky Header) */
header.machinery-bar {
  display: flex;
  justify-content: space-between;
  align-items: center;
  flex-wrap: wrap;
  gap: 12px;
  padding: 10px 16px;
  background: var(--chassis-plate);
  border-bottom: 2px solid var(--hairline-main);
  position: sticky;
  top: 0;
  z-index: 50;
}

.brand-spec {
  display: flex;
  align-items: center;
  gap: 10px;
}

.chassis-tag {
  background: var(--signal-orange);
  color: #000;
  font-family: var(--font-header);
  font-weight: 800;
  font-size: 11px;
  letter-spacing: 0.12em;
  padding: 3px 6px;
  line-height: 1;
}

.unit-id {
  font-family: var(--font-header);
  font-weight: 700;
  font-size: 14px;
  letter-spacing: 0.08em;
  color: var(--ink-primary);
}

.unit-sub {
  font-size: 9px;
  color: var(--ink-muted);
  letter-spacing: 0.12em;
  text-transform: uppercase;
}

/* Diode Status Cluster */
.status-cluster {
  display: flex;
  align-items: center;
  gap: 14px;
}

.diode-group {
  display: flex;
  align-items: center;
  gap: 5px;
}

.diode {
  width: 6px;
  height: 6px;
  background: var(--ink-dim);
  border: 1px solid var(--hairline-main);
  display: inline-block;
}
.diode.active-green {
  background: var(--phosphor-green);
  box-shadow: 0 0 5px rgba(0, 208, 98, 0.7);
}
.diode.active-orange {
  background: var(--signal-orange);
  box-shadow: 0 0 5px rgba(255, 79, 0, 0.7);
}
.diode.active-ochre {
  background: var(--lab-ochre);
  box-shadow: 0 0 5px rgba(229, 169, 60, 0.7);
}

.diode-label {
  font-size: 9px;
  font-weight: 700;
  letter-spacing: 0.1em;
  color: var(--ink-secondary);
}

.header-actions {
  display: flex;
  align-items: center;
  gap: 10px;
}

.fw-badge {
  display: inline-flex;
  align-items: center;
  gap: 6px;
  padding: 4px 8px;
  font-size: 11px;
  font-weight: 700;
  color: var(--ink-primary);
  background: var(--chassis-well);
  border: 1px solid var(--hairline-bright);
  text-decoration: none;
  letter-spacing: 0.05em;
  transition: all 0.1s;
}
.fw-badge:hover {
  background: var(--chassis-hover);
  border-color: var(--signal-orange);
}
.coffee-badge {
  display: inline-flex;
  align-items: center;
  gap: 5px;
  padding: 4px 8px;
  font-size: 11px;
  font-weight: 700;
  color: #FFDD00;
  background: var(--chassis-well);
  border: 1px solid var(--hairline-bright);
  text-decoration: none;
  letter-spacing: 0.05em;
  transition: all 0.1s;
}
.coffee-badge:hover {
  background: var(--chassis-hover);
  border-color: #FFDD00;
  color: #FFE640;
}
.footer-coffee-link {
  color: #FFDD00;
  text-decoration: none;
  font-weight: 700;
  transition: opacity 0.15s;
}
.footer-coffee-link:hover {
  opacity: 0.8;
  text-decoration: underline;
}
/* Buy Me a Coffee Widget Overrides against Global 0px Border-Radius */
#bmc-wbtn {
  border-radius: 50% !important;
  box-shadow: 0 4px 14px rgba(0,0,0,0.4) !important;
}
#bmc-close-btn {
  border-radius: 50% !important;
}
#bmc-iframe {
  border-radius: 8px !important;
}
.badge-dot {
  width: 5px;
  height: 5px;
  background: var(--phosphor-green);
  box-shadow: 0 0 4px var(--phosphor-green);
  display: inline-block;
}

/* Language Segmented Rocker */
.rocker-switch {
  display: inline-flex;
  border: 1px solid var(--hairline-bright);
  background: var(--chassis-inset);
}
.rocker-option {
  background: transparent;
  border: none;
  color: var(--ink-muted);
  padding: 4px 9px;
  font-family: var(--font-mono);
  font-size: 11px;
  font-weight: 700;
  letter-spacing: 0.08em;
  cursor: pointer;
  transition: all 0.1s;
}
.rocker-option.selected {
  background: var(--chassis-plate);
  color: var(--ink-primary);
  box-shadow: inset 0 -2px 0 var(--signal-orange);
}

/* Corner Crosshairs (+) */
.with-crosshair {
  position: relative;
}
.with-crosshair::before {
  content: "+";
  position: absolute;
  top: -6px;
  left: -5px;
  font-family: var(--font-mono);
  font-size: 10px;
  line-height: 1;
  color: var(--ink-muted);
  pointer-events: none;
  z-index: 2;
}
.with-crosshair::after {
  content: "+";
  position: absolute;
  bottom: -6px;
  right: -5px;
  font-family: var(--font-mono);
  font-size: 10px;
  line-height: 1;
  color: var(--ink-muted);
  pointer-events: none;
  z-index: 2;
}

/* Master Hero Telemetry Row */
.hero-telemetry-row {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
  border-bottom: 1px solid var(--hairline-main);
  background: var(--chassis-plate);
}

.instrument-cell {
  padding: 12px 14px;
  border-right: 1px solid var(--hairline-main);
  background: var(--chassis-plate);
}
.instrument-cell:last-child {
  border-right: none;
}

.tech-tag {
  display: flex;
  justify-content: space-between;
  align-items: baseline;
  margin-bottom: 4px;
}
.tech-tag .name {
  font-family: var(--font-header);
  font-size: 9px;
  font-weight: 700;
  letter-spacing: 0.14em;
  color: var(--ink-muted);
  text-transform: uppercase;
}
.tech-tag .code {
  font-size: 9px;
  color: var(--signal-orange);
  letter-spacing: 0.08em;
  font-weight: 700;
}

.hero-readout {
  display: flex;
  align-items: baseline;
  gap: 4px;
  margin: 2px 0;
}
.hero-num {
  font-family: var(--font-mono);
  font-size: 30px;
  font-weight: 800;
  line-height: 1.1;
  color: var(--ink-primary);
  font-variant-numeric: tabular-nums;
  letter-spacing: -0.03em;
}
.hero-unit {
  font-size: 11px;
  font-weight: 700;
  color: var(--lab-ochre);
  letter-spacing: 0.08em;
  text-transform: uppercase;
}

.cell-subtext {
  font-size: 10px;
  color: var(--ink-secondary);
  display: flex;
  justify-content: space-between;
  border-top: 1px solid var(--hairline-dim);
  padding-top: 4px;
  margin-top: 6px;
  letter-spacing: 0.04em;
}
.cell-subtext span.sub-val {
  color: var(--ink-primary);
  font-weight: 700;
}

/* Virtual 32x8 LED Matrix Instrument Deck */
.matrix-instrument-bay {
  padding: 14px 16px;
  background: var(--chassis-base);
  border-bottom: 1px solid var(--hairline-main);
}
.matrix-bay-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 10px;
}
.canvas-viewport {
  background: var(--chassis-inset);
  border: 1px solid var(--hairline-bright);
  padding: 12px;
  text-align: center;
  overflow-x: auto;
}
#virtualMatrix {
  background: #05070A;
  border: 1px solid #1A1A18;
  max-width: 100%;
  height: auto;
  display: inline-block;
  vertical-align: middle;
}
.matrix-controls-bar {
  display: flex;
  justify-content: space-between;
  align-items: center;
  gap: 12px;
  margin-top: 10px;
}
.screen-indicator {
  display: flex;
  align-items: baseline;
  gap: 6px;
  background: var(--chassis-plate);
  border: 1px solid var(--hairline-main);
  padding: 6px 14px;
}
.screen-indicator .ind-tag {
  font-size: 9px;
  color: var(--signal-orange);
  letter-spacing: 0.1em;
  font-weight: 700;
}
.screen-indicator .ind-name {
  font-size: 12px;
  font-weight: 700;
  color: var(--ink-primary);
  letter-spacing: 0.06em;
  text-transform: uppercase;
}

/* Tactile Mechanical Buttons */
.tactile-btn {
  background: var(--chassis-plate);
  color: var(--ink-primary);
  border: 1px solid var(--hairline-bright);
  padding: 7px 14px;
  font-family: var(--font-mono);
  font-size: 11px;
  font-weight: 700;
  letter-spacing: 0.08em;
  text-transform: uppercase;
  cursor: pointer;
  display: inline-flex;
  align-items: center;
  gap: 6px;
  text-decoration: none;
  box-shadow: inset 0 1px 0 rgba(255,255,255,0.06), 0 2px 0 #000;
  transition: all 0.1s;
}
.tactile-btn:hover {
  background: var(--chassis-hover);
  border-color: var(--ink-secondary);
}
.tactile-btn:active {
  transform: translateY(1px);
  box-shadow: inset 0 1px 0 rgba(0,0,0,0.4), 0 1px 0 #000;
}
.tactile-btn.primary {
  background: var(--signal-orange);
  color: #000;
  border-color: #E04400;
}
.tactile-btn.primary:hover {
  background: #FF661A;
}
.tactile-btn.btn-sm {
  padding: 4px 8px;
  font-size: 10px;
}
.tactile-btn.btn-xs {
  padding: 2px 6px;
  font-size: 9px;
  height: 22px;
}

/* Swiss Index Tab Plates */
.index-plates-bar {
  display: flex;
  flex-wrap: wrap;
  gap: 0;
  background: var(--chassis-inset);
  border-bottom: 1px solid var(--hairline-main);
  overflow-x: auto;
}
.plate-btn {
  background: var(--chassis-plate);
  color: var(--ink-secondary);
  border: none;
  border-right: 1px solid var(--hairline-main);
  border-bottom: 2px solid transparent;
  padding: 10px 14px;
  font-family: var(--font-mono);
  font-size: 11px;
  font-weight: 700;
  letter-spacing: 0.08em;
  cursor: pointer;
  display: flex;
  align-items: center;
  gap: 6px;
  white-space: nowrap;
  transition: background 0.1s, color 0.1s;
}
.plate-btn:hover {
  background: var(--chassis-hover);
  color: var(--ink-primary);
}
.plate-btn.active {
  background: var(--chassis-base);
  color: var(--ink-primary);
  border-bottom: 2px solid var(--signal-orange);
}
.plate-btn .p-idx {
  font-size: 9px;
  color: var(--signal-orange);
  font-weight: 800;
}

/* Panel Sections & Param Tables */
.control-panel-section {
  display: none;
  padding: 18px 16px;
  background: var(--chassis-base);
}
.control-panel-section.active {
  display: block;
}
.section-legend {
  font-family: var(--font-header);
  font-size: 12px;
  font-weight: 700;
  letter-spacing: 0.12em;
  color: var(--ink-primary);
  text-transform: uppercase;
  margin: 16px 0 12px 0;
  display: flex;
  align-items: center;
  gap: 8px;
  border-bottom: 1px solid var(--hairline-main);
  padding-bottom: 6px;
}
.section-legend:first-child {
  margin-top: 0;
}
.section-legend::before {
  content: "";
  display: inline-block;
  width: 4px;
  height: 12px;
  background: var(--signal-orange);
}
.param-table {
  width: 100%;
  border-collapse: collapse;
  margin-bottom: 16px;
}
.param-table tr {
  border-bottom: 1px solid var(--hairline-dim);
}
.param-table tr:hover {
  background: var(--chassis-plate);
}
.param-table td {
  padding: 8px 6px;
  vertical-align: middle;
}
.param-table td.label-cell {
  width: 44%;
}
.param-name {
  font-size: 12px;
  font-weight: 600;
  color: var(--ink-primary);
}
.param-desc {
  font-size: 10px;
  color: var(--ink-muted);
  margin-top: 2px;
  letter-spacing: 0.02em;
}

/* Inputs, Selects & Sliders */
input[type="text"], input[type="password"], input[type="number"], select {
  width: 100%;
  padding: 6px 10px;
  background: var(--chassis-inset);
  border: 1px solid var(--hairline-main);
  color: var(--ink-primary);
  font-family: var(--font-mono);
  font-size: 12px;
  outline: none;
  transition: border-color 0.15s;
}
input[type="text"]:focus, input[type="password"]:focus, input[type="number"]:focus, select:focus {
  border-color: var(--signal-orange);
}
input[type="range"] {
  -webkit-appearance: none;
  appearance: none;
  width: 100%;
  height: 6px;
  background: var(--chassis-inset);
  border: 1px solid var(--hairline-main);
  outline: none;
}
input[type="range"]::-webkit-slider-thumb {
  -webkit-appearance: none;
  appearance: none;
  width: 14px;
  height: 18px;
  background: var(--ink-primary);
  border: 1px solid #000;
  cursor: pointer;
}
input[type="range"]::-moz-range-thumb {
  width: 14px;
  height: 18px;
  background: var(--ink-primary);
  border: 1px solid #000;
  cursor: pointer;
}

/* Physical Actuator Switches */
.visually-hidden {
  position: absolute;
  clip-path: inset(50%);
  overflow: hidden;
  width: 1px;
  height: 1px;
  margin: -1px;
  padding: 0;
  border: 0;
  white-space: nowrap;
}
.actuator-label {
  display: inline-flex;
  align-items: center;
  gap: 10px;
  cursor: pointer;
  user-select: none;
  font-size: 12px;
  font-weight: 600;
  color: var(--ink-primary);
}
.actuator-box {
  width: 36px;
  height: 20px;
  background: var(--chassis-inset);
  border: 1px solid var(--hairline-main);
  position: relative;
  display: inline-block;
  vertical-align: middle;
  flex-shrink: 0;
  transition: border-color 0.15s;
}
.actuator-box::after {
  content: "";
  position: absolute;
  top: 2px;
  left: 2px;
  width: 14px;
  height: 14px;
  background: var(--ink-dim);
  transition: transform 0.15s, background 0.15s;
}
input:checked + .actuator-label .actuator-box,
.visually-hidden:checked + .actuator-box {
  border-color: var(--signal-orange);
  background: var(--signal-orange-dim);
}
input:checked + .actuator-label .actuator-box::after,
.visually-hidden:checked + .actuator-box::after {
  transform: translateX(16px);
  background: var(--signal-orange);
}
.visually-hidden:focus-visible + .actuator-box {
  outline: 1px solid var(--signal-orange);
}

/* Save Action Bar */
.save-bar {
  display: flex;
  justify-content: space-between;
  align-items: center;
  flex-wrap: wrap;
  gap: 12px;
  padding: 12px 16px;
  background: var(--chassis-plate);
  border-top: 1px solid var(--hairline-main);
  margin-top: 16px;
}
.save-info {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 10px;
  color: var(--ink-secondary);
  letter-spacing: 0.05em;
}
.save-dot {
  width: 6px;
  height: 6px;
  background: var(--phosphor-green);
  box-shadow: 0 0 4px var(--phosphor-green);
  display: inline-block;
}

/* Floating Status Toast */
#toast {
  position: fixed;
  top: 16px;
  left: 50%;
  transform: translateX(-50%);
  background: var(--chassis-plate);
  border: 1px solid var(--signal-orange);
  color: var(--ink-primary);
  padding: 8px 18px;
  font-family: var(--font-mono);
  font-size: 11px;
  font-weight: 700;
  letter-spacing: 0.08em;
  box-shadow: 0 6px 20px rgba(0,0,0,0.8);
  z-index: 1000;
  display: none;
  pointer-events: none;
}

/* Reorder List */
.reorder-list {
  display: flex;
  flex-direction: column;
  gap: 4px;
  margin-bottom: 12px;
}
.reorder-item {
  display: flex;
  align-items: center;
  justify-content: space-between;
  background: var(--chassis-plate);
  border: 1px solid var(--hairline-dim);
  padding: 6px 10px;
}
.reorder-item:hover {
  background: var(--chassis-hover);
  border-color: var(--hairline-bright);
}
.reorder-name {
  font-size: 11px;
  font-weight: 700;
  letter-spacing: 0.05em;
  color: var(--ink-primary);
}
.reorder-step-btns {
  display: flex;
  gap: 4px;
}

/* Meter Bar for OTA */
.ota-meter-wrap {
  background: var(--chassis-inset);
  border: 1px solid var(--hairline-main);
  height: 12px;
  overflow: hidden;
  margin-top: 8px;
}
#ota_bar {
  background: var(--signal-orange);
  width: 0%;
  height: 100%;
  transition: width 0.15s;
}

/* Terminal Footer */
.terminal-footer {
  padding: 12px 16px;
  border-top: 1px solid var(--hairline-main);
  background: var(--chassis-plate);
  font-size: 10px;
  color: var(--ink-muted);
  display: flex;
  justify-content: space-between;
  align-items: center;
  letter-spacing: 0.08em;
  margin-top: auto;
}
</style>
</head>
<body>
<div id="toast"></div>

<div class="terminal-shell">
  <!-- MACHINERY TOP BAR -->
  <header class="machinery-bar">
    <div class="brand-spec">
      <span class="chassis-tag">TC-001</span>
      <div>
        <div class="unit-id" data-i18n="app_title">ULANZI TC001 EVOLVED</div>
        <div class="unit-sub" data-i18n="app_sub">TACTILE FIELD INSTRUMENT // 8×32 MATRIX TELEMETRY</div>
      </div>
    </div>

    <div class="status-cluster">
      <div class="diode-group" title="Wi-Fi Link Status">
        <span class="diode active-green" id="diodeWifi"></span>
        <span class="diode-label">WIFI</span>
      </div>
      <div class="diode-group" title="MQTT Broker Status">
        <span class="diode" id="diodeMqtt"></span>
        <span class="diode-label">MQTT</span>
      </div>
      <div class="diode-group" title="LDR Ambient Light Sensor">
        <span class="diode active-green" id="diodeLdr"></span>
        <span class="diode-label">LDR</span>
      </div>
      <div class="diode-group" title="Matrix Render Engine">
        <span class="diode active-green" id="diodeMatrix"></span>
        <span class="diode-label">MATRIX</span>
      </div>
    </div>

    <div class="header-actions">
      <a href="https://buymeacoffee.com/hackra" target="_blank" rel="noopener" class="coffee-badge" onclick="openBmc(event)" title="Buy Me a Coffee (@hackra)">
        <svg viewBox="0 0 24 24" width="12" height="12" fill="currentColor" style="display:inline-block;vertical-align:middle;"><path d="M20.216 6.415l-.132-.666c-.119-.597-.384-1.144-.766-1.583-.564-.647-1.355-.985-2.288-.985H4.07c-.477 0-.917.184-1.25.518-.337.338-.52.782-.516 1.25.045 5.344.408 10.22 1.077 14.502.585 3.738 2.658 5.464 6.16 5.464 3.513 0 5.586-1.726 6.171-5.464.444-2.836.733-5.99.883-9.452.484-.047.935-.187 1.309-.434.577-.381.94-.972 1.022-1.666l.006-.051c.074-.593-.053-1.182-.366-1.668-.31-.482-.78-.813-1.32-.931l-.38-.083zm-2.846-1.815c.571 0 1.05.195 1.385.579.222.255.374.571.442.915l.094.475c-.65.178-1.353.284-2.1.317l-.208-1.84c.129-.278.267-.446.387-.446zm-2.903 16.985c-.482 3.08-2.094 4.38-5.018 4.38-2.934 0-4.545-1.3-5.027-4.38-.636-4.07-.985-8.73-1.03-13.88h12.11c-.13 4.254-.42 8.784-1.035 13.88z"/></svg>
        <span>COFFEE</span>
      </a>
      <a href="https://github.com/hackra76/Ulanzi-TC001-Evolved" target="_blank" rel="noopener" class="fw-badge" title="GitHub Repository">
        <span class="badge-dot"></span> <span id="topFwVer">v2.0.0</span>
      </a>
      <div class="rocker-switch">
        <button type="button" class="rocker-option selected" id="btn-en" onclick="setLang('en')">EN</button>
        <button type="button" class="rocker-option" id="btn-sk" onclick="setLang('sk')">SK</button>
      </div>
    </div>
  </header>

  <!-- MASTER TELEMETRY ROW -->
  <div class="hero-telemetry-row with-crosshair">
    <div class="instrument-cell">
      <div class="tech-tag">
        <span class="name" data-i18n="tele_pwr">POWER // BATT</span>
        <span class="code" id="teleBattStatus">CH-A</span>
      </div>
      <div class="hero-readout">
        <span class="hero-num" id="teleBatteryVolts">--.--</span>
        <span class="hero-unit">V</span>
      </div>
      <div class="cell-subtext">
        <span data-i18n="tele_cap">EST. CAPACITY</span>
        <span class="sub-val" id="teleBatteryPct">--%</span>
      </div>
    </div>

    <div class="instrument-cell">
      <div class="tech-tag">
        <span class="name" data-i18n="tele_env">ENVIRONMENT</span>
        <span class="code">SHT30</span>
      </div>
      <div class="hero-readout">
        <span class="hero-num" id="teleTemp">--.-</span>
        <span class="hero-unit">°C</span>
      </div>
      <div class="cell-subtext">
        <span data-i18n="tele_hum">RELATIVE HUMIDITY</span>
        <span class="sub-val" id="teleHum">--% RH</span>
      </div>
    </div>

    <div class="instrument-cell">
      <div class="tech-tag">
        <span class="name" data-i18n="tele_opt">OPTICAL AMBIENT</span>
        <span class="code">LDR-ADC</span>
      </div>
      <div class="hero-readout">
        <span class="hero-num" id="teleLdr">---</span>
        <span class="hero-unit">RAW</span>
      </div>
      <div class="cell-subtext">
        <span data-i18n="tele_bri">AUTO BRIGHTNESS</span>
        <span class="sub-val" id="teleBriLevel">35% CEIL</span>
      </div>
    </div>

    <div class="instrument-cell">
      <div class="tech-tag">
        <span class="name" data-i18n="tele_net">RF CARRIER / SCREEN</span>
        <span class="code" id="teleNetState">STA</span>
      </div>
      <div class="hero-readout">
        <span class="hero-num" id="teleRssi">--</span>
        <span class="hero-unit">dBm</span>
      </div>
      <div class="cell-subtext">
        <span data-i18n="tele_act">ACTIVE SCREEN</span>
        <span class="sub-val" id="teleScreenName">Clock</span>
      </div>
    </div>
  </div>

  <!-- VIRTUAL MATRIX INSTRUMENT BAY -->
  <div class="matrix-instrument-bay with-crosshair">
    <div class="matrix-bay-header">
      <div class="tech-tag" style="margin:0;">
        <span class="name" data-i18n="live_stream">REAL-TIME DISPLAY MATRIX // 32×8 NEOPIXEL MIRROR</span>
        <span class="code">1.5s SYNC</span>
      </div>
      <a href="/api/display/screenshot" target="_blank" download="tc001_screenshot.bmp" class="tactile-btn btn-sm" data-i18n="cap_bmp">
        &#x2399; CAPTURE BMP
      </a>
    </div>

    <div class="canvas-viewport">
      <canvas id="virtualMatrix" width="320" height="80"></canvas>
    </div>

    <div class="matrix-controls-bar">
      <button type="button" class="tactile-btn" onclick="switchScreen(-1)" data-i18n="btn_prev">&larr; PREV</button>
      <div class="screen-indicator">
        <span class="ind-tag" data-i18n="lbl_channel">CHANNEL:</span>
        <span id="screenNameBadge" class="ind-name">Clock</span>
      </div>
      <button type="button" class="tactile-btn" onclick="switchScreen(1)" data-i18n="btn_next">NEXT &rarr;</button>
    </div>
  </div>

  <!-- SWISS INDEX TAB PLATES -->
  <div class="index-plates-bar">
    <button type="button" class="plate-btn active" data-plate="clock" onclick="switchPlate('clock')">
      <span class="p-idx">01</span> <span data-i18n="tab_clock">DISPLAY &amp; CLOCK</span>
    </button>
    <button type="button" class="plate-btn" data-plate="carousel" onclick="switchPlate('carousel')">
      <span class="p-idx">02</span> <span data-i18n="tab_carousel">CAROUSEL &amp; SCREENS</span>
    </button>
    <button type="button" class="plate-btn" data-plate="services" onclick="switchPlate('services')">
      <span class="p-idx">03</span> <span data-i18n="tab_services">INTEGRATIONS &amp; FEEDS</span>
    </button>
    <button type="button" class="plate-btn" data-plate="network" onclick="switchPlate('network')">
      <span class="p-idx">04</span> <span data-i18n="tab_network">NETWORK &amp; SYSTEM</span>
    </button>
    <button type="button" class="plate-btn" data-plate="ota" onclick="switchPlate('ota')">
      <span class="p-idx">05</span> <span data-i18n="tab_ota">FIRMWARE OTA</span>
    </button>
  </div>

  <!-- MAIN CONFIGURATION FORM -->
  <form id="settingsForm" action="/save" method="POST">
    <input type="hidden" id="ui_lang" name="ui_lang" value="en">

    <!-- PANEL 01: DISPLAY & CLOCK -->
    <div class="control-panel-section active" id="panel-clock">
      <div class="section-legend" data-i18n="sub_clock_layout">Clock &amp; Date Architecture</div>
      <table class="param-table">
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="lbl_clk_mode">Clock Display Mode</div>
            <div class="param-desc">Time, Date or Alternating interval</div>
          </td>
          <td>
            <select id="clk_mode" name="clk_mode">
              <option value="0"  data-i18n="opt_clk_time">Time Only</option>
              <option value="1"  data-i18n="opt_clk_both">Combined Time &amp; Date (Alternating)</option>
              <option value="2"  data-i18n="opt_clk_date">Date Only</option>
            </select>
          </td>
        </tr>
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="lbl_date_fmt">Date Format</div>
            <div class="param-desc">European or international calendar notation</div>
          </td>
          <td>
            <select id="date_fmt" name="date_fmt">
              <option value="0" >DD.MM. (e.g. 14.10.)</option>
              <option value="1" >DD MMM (e.g. 14 OCT)</option>
              <option value="2" >MM/DD (e.g. 10/14)</option>
              <option value="3" >DAY DD (e.g. WED 14)</option>
            </select>
          </td>
        </tr>
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="lbl_clk_icon">Clock Icon Style</div>
            <div class="param-desc">Dynamic 8x8 day glyph, heart or full width</div>
          </td>
          <td>
            <select id="clk_icon" name="clk_icon">
              <option value="0"  data-i18n="opt_icon_cal">Dynamic Calendar (Date 1-31)</option>
              <option value="1"  data-i18n="opt_icon_heart">Heart (Classic)</option>
              <option value="2"  data-i18n="opt_icon_clock">Clock Face</option>
              <option value="3"  data-i18n="opt_icon_none">None (Full Width)</option>
            </select>
          </td>
        </tr>
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="lbl_txt_align">Text Alignment</div>
            <div class="param-desc">Left margin with icon spacer or centered</div>
          </td>
          <td>
            <select id="txt_align" name="txt_align">
              <option value="0"  data-i18n="opt_align_left">Left-Aligned with Icon (Awtrix)</option>
              <option value="1"  data-i18n="opt_align_center">Centered in Window</option>
            </select>
          </td>
        </tr>
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="chk_show_sec">Show Seconds</div>
            <div class="param-desc">High-precision 3x5 seconds counter</div>
          </td>
          <td>
            <label class="actuator-label">
              <input type="checkbox" class="visually-hidden" name="show_sec" value="1" >
              <span class="actuator-box"></span>
              <span data-i18n="chk_show_sec">Show Seconds in Time Display</span>
            </label>
          </td>
        </tr>
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="chk_wd_bar">Weekday Indicators</div>
            <div class="param-desc">7 micro bars along row 7</div>
          </td>
          <td>
            <label class="actuator-label">
              <input type="checkbox" class="visually-hidden" name="wd_bar" value="1" >
              <span class="actuator-box"></span>
              <span data-i18n="chk_wd_bar">Show 7 Weekday Bars Under Time</span>
            </label>
          </td>
        </tr>
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="lbl_wd_mon">Week Starting Day</div>
            <div class="param-desc">Standard ISO 8601 or US Sunday</div>
          </td>
          <td>
            <select id="wd_mon" name="wd_mon">
              <option value="1"  data-i18n="opt_mon_iso">Monday (ISO 8601)</option>
              <option value="0"  data-i18n="opt_sun_us">Sunday (US Standard)</option>
            </select>
          </td>
        </tr>
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="lbl_wd_acol">Active Day Color</div>
            <div class="param-desc">Phosphor tone for current day of week</div>
          </td>
          <td>
            <select id="wd_acol" name="wd_acol">
              <option value="0" >Cyan (#00F0FF)</option>
              <option value="1" >Orange (#FF8800)</option>
              <option value="2" >Green (#00FF00)</option>
              <option value="3" >Yellow (#FFFF00)</option>
              <option value="4" >Red (#FF2200)</option>
              <option value="5" >White (#FFFFFF)</option>
              <option value="6" >Magenta (#FF00FF)</option>
              <option value="7" >Electric Blue (#0066FF)</option>
              <option value="8" >Lime Green (#76FF03)</option>
              <option value="9" >Amber Gold (#FFC107)</option>
              <option value="10" >Soft Pink (#FF4081)</option>
            </select>
          </td>
        </tr>
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="lbl_wd_icol">Inactive Day Color</div>
            <div class="param-desc">Dimmable contrast for non-current weekdays</div>
          </td>
          <td>
            <select id="wd_icol" name="wd_icol">
              <option value="0"  data-i18n="opt_inact_auto">Dim Active Color (Auto - Slightly Dimmer)</option>
              <option value="1" >Slate Gray (#708090)</option>
              <option value="2" >Dim Amber (#906810)</option>
              <option value="3" >Dark Blue (#001A40)</option>
              <option value="4" >Deep Wine (#3A000A)</option>
              <option value="5" >Forest Green (#002A0D)</option>
              <option value="6" >Charcoal (#252525)</option>
              <option value="7" >Muted Purple (#2A1035)</option>
              <option value="8" >Dark Cyan (#003038)</option>
              <option value="9"  data-i18n="opt_inact_off">Off (Black)</option>
            </select>
          </td>
        </tr>
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="lbl_wd_wcol">Weekend Color</div>
            <div class="param-desc">Saturday and Sunday distinctive tone</div>
          </td>
          <td>
            <select id="wd_wcol" name="wd_wcol">
              <option value="0"  data-i18n="opt_wknd_active">Same as Active Weekday</option>
              <option value="1" >Bright Red (#FF2200)</option>
              <option value="2" >Coral Orange (#FF6B4A)</option>
              <option value="3" >Gold (#FFD700)</option>
              <option value="4" >Hot Pink (#FF1493)</option>
              <option value="5" >Purple (#9C27B0)</option>
              <option value="6" >Cyan (#00F0FF)</option>
              <option value="7" >Lime (#76FF03)</option>
              <option value="8" >Pure White (#FFFFFF)</option>
              <option value="9" >Sky Blue (#00BFFF)</option>
              <option value="10" >Emerald (#00E676)</option>
            </select>
          </td>
        </tr>
      </table>

      <div class="section-legend" data-i18n="sub_ambient">Ambient Photometry &amp; Night Cycle</div>
      <table class="param-table">
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="chk_auto_bri">Auto-Brightness</div>
            <div class="param-desc">Dynamic LDR light sensor gamma adaptation</div>
          </td>
          <td>
            <label class="actuator-label">
              <input type="checkbox" class="visually-hidden" name="auto_bri" value="1" >
              <span class="actuator-box"></span>
              <span data-i18n="chk_auto_bri">Auto-Brightness via LDR Light Sensor</span>
            </label>
          </td>
        </tr>
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="lbl_day_bri">Day Brightness Ceiling</div>
            <div class="param-desc">Maximum output during daytime</div>
          </td>
          <td>
            <div style="display:flex; align-items:center; gap:12px;">
              <input type="range" id="day_bri" name="day_bri" min="5" max="100" value="35" oninput="document.getElementById('day_bri_val').innerText = this.value + '%'">
              <span id="day_bri_val" style="font-weight:700; color:var(--signal-orange); min-width:44px; text-align:right;">35%</span>
            </div>
          </td>
        </tr>
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="chk_night_mode">Night Mode</div>
            <div class="param-desc">Ultra-dim distraction-free red clock</div>
          </td>
          <td>
            <label class="actuator-label">
              <input type="checkbox" class="visually-hidden" name="night_mode" value="1" >
              <span class="actuator-box"></span>
              <span data-i18n="chk_night_mode">Night Mode (Quiet Dim Red Clock)</span>
            </label>
          </td>
        </tr>
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="lbl_night_bri">Night Brightness Floor</div>
            <div class="param-desc">Minimum output for dark environment</div>
          </td>
          <td>
            <div style="display:flex; align-items:center; gap:12px;">
              <input type="range" id="night_bri" name="night_bri" min="1" max="25" value="2" oninput="document.getElementById('night_bri_val').innerText = this.value + '%'">
              <span id="night_bri_val" style="font-weight:700; color:var(--signal-orange); min-width:44px; text-align:right;">2%</span>
            </div>
          </td>
        </tr>
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="lbl_night_start">Quiet Hours Schedule</div>
            <div class="param-desc">Start and end hours (24h format)</div>
          </td>
          <td>
            <div style="display:flex; gap:10px; align-items:center;">
              <input type="number" id="night_start" name="night_start" min="0" max="23" value="23" style="max-width:80px;">
              <span style="color:var(--ink-muted);">&rarr;</span>
              <input type="number" id="night_end" name="night_end" min="0" max="23" value="7" style="max-width:80px;">
            </div>
          </td>
        </tr>
      </table>

      <!-- Stopwatch & Countdown Timer Remote Controller -->
      <div class="section-legend" data-i18n="grp_timer">&#x23F1; Stopwatch &amp; Countdown Timer Remote</div>
      <table class="param-table">
        <tr>
          <td class="label-cell">
            <div class="param-name" id="timerModeLabel">COUNTDOWN TIMER</div>
            <div class="param-desc" id="timerStateLabel">State: Stopped</div>
          </td>
          <td>
            <div style="display:flex; align-items:center; justify-content:space-between; flex-wrap:wrap; gap:10px;">
              <div id="timerDisplay" style="font-family:var(--font-mono, monospace); font-size:26px; font-weight:800; color:var(--signal-orange); letter-spacing:0.06em;">05:00</div>
              <div style="display:flex; gap:6px; flex-wrap:wrap;">
                <button type="button" class="tactile-btn btn-sm" onclick="apiTimer('toggle')" data-i18n="btn_timer_toggle">&#x25B6; / &#x23F8; Toggle</button>
                <button type="button" class="tactile-btn btn-sm" onclick="apiTimer('reset')" data-i18n="btn_timer_reset">&#x21BB; Reset</button>
                <button type="button" class="tactile-btn btn-sm" onclick="switchTimerMode()"><span data-i18n="lbl_timer_mode">Mode:</span> <span id="timerModeBtn">Countdown</span></button>
              </div>
            </div>
            <div style="margin-top:10px; display:flex; align-items:center; gap:6px; flex-wrap:wrap;">
              <span style="font-size:10px; font-weight:700; color:var(--ink-muted); text-transform:uppercase;">PRESETS:</span>
              <button type="button" class="tactile-btn btn-xs" onclick="setTimerSec(60)">1m</button>
              <button type="button" class="tactile-btn btn-xs" onclick="setTimerSec(180)">3m</button>
              <button type="button" class="tactile-btn btn-xs" onclick="setTimerSec(300)">5m</button>
              <button type="button" class="tactile-btn btn-xs" onclick="setTimerSec(600)">10m</button>
              <button type="button" class="tactile-btn btn-xs" onclick="setTimerSec(1500)">25m (Pomodoro)</button>
            </div>
          </td>
        </tr>
      </table>
    </div>

    <!-- PANEL 02: CAROUSEL & SCREENS -->
    <div class="control-panel-section" id="panel-carousel">
      <div class="section-legend" data-i18n="grp_carousel">Carousel Engine &amp; Transitions</div>
      <table class="param-table">
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="chk_auto_rotate">Auto-Rotate Carousel</div>
            <div class="param-desc">Automatically cycles through active screens</div>
          </td>
          <td>
            <label class="actuator-label">
              <input type="checkbox" class="visually-hidden" id="auto_rotate" name="auto_rotate" value="1"  onchange="document.getElementById('rotate_delay_field').style.display = this.checked ? 'table-row' : 'none'">
              <span class="actuator-box"></span>
              <span data-i18n="chk_auto_rotate">Auto-Rotate Screens (Carousel)</span>
            </label>
          </td>
        </tr>
        <tr id="rotate_delay_field" style="">
          <td class="label-cell">
            <div class="param-name" data-i18n="lbl_rotate_interval">Cycle Interval</div>
            <div class="param-desc">Dwell time per screen (3 - 120 seconds)</div>
          </td>
          <td>
            <input type="number" id="rotate" name="rotate" min="3" max="120" value="8" required style="max-width:120px;">
          </td>
        </tr>
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="lbl_trans_fx">Screen Transition Effect</div>
            <div class="param-desc">Direct matrix raw frame animation</div>
          </td>
          <td>
            <select id="trans_fx" name="trans_fx">
              <option value="0"  data-i18n="opt_trans_0">None (Instant Cut)</option>
              <option value="1"  data-i18n="opt_trans_1">Slide Left</option>
              <option value="2"  data-i18n="opt_trans_2">Slide Right</option>
              <option value="3"  data-i18n="opt_trans_3">Slide Up</option>
              <option value="4"  data-i18n="opt_trans_4">Slide Down</option>
              <option value="5"  data-i18n="opt_trans_5">Crossfade / Dissolve</option>
              <option value="6"  data-i18n="opt_trans_6">Curtain (Center Split Wipe)</option>
              <option value="7"  data-i18n="opt_trans_7">Random (Shuffle Effects)</option>
            </select>
          </td>
        </tr>
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="lbl_active_app">Default / Active Screen</div>
            <div class="param-desc">Target screen when carousel paused</div>
          </td>
          <td>
            <select id="active_app" name="active_app">
              <option value="0"  data-i18n="app_name_0">Clock (Heart / Time)</option>
              <option value="1"  data-i18n="app_name_1">Indoor Environment (SHT30 Temp)</option>
              <option value="2"  data-i18n="app_name_2">Outdoor Weather (Open-Meteo)</option>
              <option value="3"  data-i18n="app_name_3">Crypto &amp; Financial Markets</option>
              <option value="4"  data-i18n="app_name_4">YouTube Subscriber Count</option>
              <option value="5"  data-i18n="app_name_5">Live Airspace Radar (ADS-B)</option>
              <option value="6"  data-i18n="app_name_6">Battery &amp; Energy Telemetry</option>
              <option value="7"  data-i18n="app_name_7">Stopwatch &amp; Countdown Timer</option>
              <option value="8"  data-i18n="app_name_8">Retro Ambient Screensaver</option>
            </select>
          </td>
        </tr>
      </table>

      <div class="section-legend" data-i18n="lbl_carousel_screens">Active Rotation Sequence (Check to Enable &bull; Use &#x25B2;&#x25BC; to Reorder)</div>
      <input type="hidden" id="car_ord" name="car_ord" value="0,1,2,3,4,5,6,7,8">
      <div class="reorder-list" id="carouselScreens">
        
      </div>

      <div class="section-legend" data-i18n="sub_battery">Battery Telemetry Format</div>
      <table class="param-table">
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="lbl_bat_mode">Battery Display Format</div>
            <div class="param-desc">Percentage, precise voltage or alternating</div>
          </td>
          <td>
            <select id="bat_mode" name="bat_mode">
              <option value="2"  data-i18n="opt_bat_alt">Alternating (Percentage &bull; Voltage every 3s)</option>
              <option value="0"  data-i18n="opt_bat_pct">Percentage Only (e.g. 94%)</option>
              <option value="1"  data-i18n="opt_bat_volt">Voltage Only (e.g. 4.12V)</option>
            </select>
          </td>
        </tr>
      </table>

      <!-- Retro Ambient Screensaver -->
      <div class="section-legend" data-i18n="grp_screensaver">&#x1F47E; Retro Ambient Screensaver</div>
      <table class="param-table">
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="lbl_ss_style">Screensaver Style</div>
            <div class="param-desc">Zero-heap procedural animation engine</div>
          </td>
          <td>
            <select id="ss_style" name="ss_style">
              <option value="0" data-i18n="opt_ss_0">Matrix Digital Rain (Cyberpunk Phosphor)</option>
              <option value="1" data-i18n="opt_ss_1">NeoPixel Fire / Lava (Thermal Simulation)</option>
              <option value="2" data-i18n="opt_ss_2">Pac-Man &amp; Blinky (80s Arcade Chase)</option>
              <option value="3" data-i18n="opt_ss_3">Conway's Game of Life (Cellular Automata)</option>
              <option value="4" data-i18n="opt_ss_4">Dynamic Shuffle (Cycle All Styles)</option>
            </select>
          </td>
        </tr>
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="lbl_ss_timeout">Idle Inactivity Timeout</div>
            <div class="param-desc">Activates screensaver after no button activity</div>
          </td>
          <td>
            <select id="ss_time" name="ss_time">
              <option value="0" data-i18n="opt_ss_to_0">Disabled (Carousel Only)</option>
              <option value="1">1 Minute</option>
              <option value="2">2 Minutes</option>
              <option value="5">5 Minutes</option>
              <option value="10">10 Minutes</option>
              <option value="15">15 Minutes</option>
              <option value="30">30 Minutes</option>
            </select>
          </td>
        </tr>
      </table>
    </div>

    <!-- PANEL 03: INTEGRATIONS & FEEDS -->
    <div class="control-panel-section" id="panel-services">
      <div class="section-legend" data-i18n="sub_weather">Meteorological Telemetry (Open-Meteo)</div>
      <table class="param-table">
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="lbl_city_preset">Predefined City Preset</div>
            <div class="param-desc">Auto-populates coordinates and station name</div>
          </td>
          <td>
            <select id="city_preset" onchange="applyCityPreset(this.value)">
              <option value="" data-i18n="opt_choose_city">-- Choose City Preset --</option>
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
              <optgroup label="North America &amp; Worldwide">
                <option value="New York|40.7128|-74.0060">New York</option>
                <option value="Los Angeles|34.0522|-118.2437">Los Angeles</option>
                <option value="Toronto|43.6532|-79.3832">Toronto</option>
                <option value="Tokyo|35.6762|139.6503">Tokyo</option>
                <option value="Dubai|25.2048|55.2708">Dubai</option>
                <option value="Sydney|-33.8688|151.2093">Sydney</option>
              </optgroup>
            </select>
          </td>
        </tr>
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="lbl_city_name">City Name</div>
            <div class="param-desc">Station title for weather reports</div>
          </td>
          <td>
            <input type="text" id="w_city" name="w_city" value="Bratislava" placeholder="e.g. Bratislava">
          </td>
        </tr>
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="lbl_lat">Geographic Coordinates</div>
            <div class="param-desc">Decimal latitude and longitude</div>
          </td>
          <td>
            <div style="display:flex; gap:10px;">
              <input type="text" id="w_lat" name="w_lat" value="48.1486" placeholder="Lat: 48.1486">
              <input type="text" id="w_lon" name="w_lon" value="17.1077" placeholder="Lon: 17.1077">
            </div>
          </td>
        </tr>
      </table>

      <div class="section-legend" data-i18n="sub_finance">Financial Markets &amp; Cryptocurrency</div>
      <table class="param-table">
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="lbl_asset_preset">Asset / Index Preset</div>
            <div class="param-desc">Binance Crypto or Yahoo Finance Index</div>
          </td>
          <td>
            <select id="c_sym" name="c_sym" onchange="document.getElementById('c_sym_custom').value = ''">
              <optgroup label="Cryptocurrencies">
                <option value="BTC" >Bitcoin (BTC / USDT)</option>
                <option value="ETH" >Ethereum (ETH / USDT)</option>
                <option value="SOL" >Solana (SOL / USDT)</option>
                <option value="BNB" >Binance Coin (BNB / USDT)</option>
              </optgroup>
              <optgroup label="Stocks &amp; Indices">
                <option value="AAPL" >Apple Inc. (AAPL - Nasdaq)</option>
                <option value="TSLA" >Tesla Inc. (TSLA - Nasdaq)</option>
                <option value="NVDA" >Nvidia Corp. (NVDA - Nasdaq)</option>
                <option value="SPX" >S&amp;P 500 Index (^GSPC)</option>
              </optgroup>
              <optgroup label="Commodities &amp; Forex">
                <option value="GOLD" >Gold Bullion (GC=F / USD)</option>
                <option value="EUR" >EUR / USD (Euro Forex)</option>
                <option value="USD" >USD / EUR (US Dollar Forex)</option>
              </optgroup>
            </select>
          </td>
        </tr>
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="lbl_custom_ticker">Custom Symbol</div>
            <div class="param-desc">Overrides preset with any valid market ticker</div>
          </td>
          <td>
            <input type="text" id="c_sym_custom" name="c_sym_custom" value="" placeholder="e.g. AAPL, GOLD, EUR, MSFT, BTC">
          </td>
        </tr>
      </table>

      <div class="section-legend" data-i18n="sub_youtube">YouTube Channel Telemetry</div>
      <table class="param-table">
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="lbl_yt_channel">Channel ID or @Handle</div>
            <div class="param-desc">Public YouTube creator channel</div>
          </td>
          <td>
            <input type="text" id="yt_channel" name="yt_channel" value="" placeholder="e.g. @MKBHD or UCxxxxxxxx">
          </td>
        </tr>
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="lbl_yt_key">YouTube Data API v3 Key</div>
            <div class="param-desc">Google Cloud API authentication key</div>
          </td>
          <td>
            <input type="password" id="yt_key" name="yt_key" value="" placeholder="Google Cloud API key">
          </td>
        </tr>
      </table>

      <div class="section-legend" data-i18n="sub_ha_mqtt">Home Assistant &amp; MQTT Telemetry</div>
      <table class="param-table">
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="chk_mqtt_en">Enable MQTT Client</div>
            <div class="param-desc">Enables Auto-Discovery and Push API</div>
          </td>
          <td>
            <label class="actuator-label">
              <input type="checkbox" class="visually-hidden" name="mqtt_en" id="mqtt_en"  onchange="document.getElementById('mqtt_settings').style.display = this.checked ? 'block' : 'none';">
              <span class="actuator-box"></span>
              <span data-i18n="chk_mqtt_en">Enable MQTT Client &amp; Home Assistant Auto-Discovery</span>
            </label>
          </td>
        </tr>
      </table>

      <div id="mqtt_settings" style="display:none;">
        <table class="param-table">
          <tr>
            <td class="label-cell">
              <div class="param-name" data-i18n="lbl_mqtt_srv">Broker IP or Hostname</div>
              <div class="param-desc">MQTT broker address</div>
            </td>
            <td>
              <div style="display:flex; gap:10px;">
                <input type="text" id="mqtt_srv" name="mqtt_srv" value="" placeholder="192.168.0.100 or homeassistant.local" style="flex:2;">
                <input type="number" id="mqtt_port" name="mqtt_port" value="1883" placeholder="1883" style="flex:1; max-width:90px;">
              </div>
            </td>
          </tr>
          <tr>
            <td class="label-cell">
              <div class="param-name" data-i18n="lbl_mqtt_user">Authentication</div>
              <div class="param-desc">Optional broker username and password</div>
            </td>
            <td>
              <div style="display:flex; gap:10px;">
                <input type="text" id="mqtt_user" name="mqtt_user" value="" placeholder="Username">
                <input type="password" id="mqtt_pass" name="mqtt_pass" value="" placeholder="Password">
              </div>
            </td>
          </tr>
          <tr>
            <td class="label-cell">
              <div class="param-name" data-i18n="lbl_mqtt_pre">Topic Prefix</div>
              <div class="param-desc">Base topic for sensors and notifications</div>
            </td>
            <td>
              <input type="text" id="mqtt_pre" name="mqtt_pre" value="ulanzi-tc001" placeholder="ulanzi-tc001">
            </td>
          </tr>
        </table>
        <div style="font-size:10px; color:var(--ink-secondary); line-height:1.5; background:var(--chassis-inset); padding:10px 12px; border:1px solid var(--hairline-main); margin-bottom:14px;" data-i18n="box_custom_push">
          &#x1F4A1; <strong>Push Spotify &amp; Custom Screens:</strong><br>
          MQTT: Publish JSON to <code>ulanzi-tc001/custom/spotify</code> or <code>ulanzi-tc001/notify</code><br>
          HTTP REST: <code>POST /api/custom?name=spotify</code> or <code>POST /api/notify</code><br>
          Payload: <code>{"text":"Song - Artist","icon":"spotify","progress":45,"duration":8}</code>
        </div>
      </div>
    </div>

    <!-- PANEL 04: NETWORK & SYSTEM -->
    <div class="control-panel-section" id="panel-network">
      <div class="section-legend" data-i18n="grp_network">Wi-Fi RF Link &amp; Multi-Network Memory</div>
      <div style="font-size:11px; color:var(--ink-secondary); line-height:1.4; margin-bottom:12px;" data-i18n="desc_wifi_roam">
        Stores up to 4 Wi-Fi networks. Automatically detects and connects to whichever saved network is within RF range with the highest signal strength (ideal for home / office roaming).
      </div>

      <div style="margin-bottom:12px; display:flex; gap:8px; align-items:center; flex-wrap:wrap;">
        <button type="button" class="tactile-btn btn-sm" onclick="scanNetworks()" data-i18n="btn_scan">&#x21BB; Scan Nearby Networks</button>
        <select id="scanSelect" style="display:none; flex:1; min-width:200px;" onchange="selectNetwork(this.value)">
          <option value="" data-i18n="opt_scan_detect">-- Detected Networks --</option>
        </select>
        <span id="scanSlotButtons" style="display:none; gap:4px;">
          <button type="button" class="tactile-btn btn-sm" onclick="fillSlotFromScan(1)" title="Assign to Slot 1">&#x2192; Slot 1</button>
          <button type="button" class="tactile-btn btn-sm" onclick="fillSlotFromScan(2)" title="Assign to Slot 2">&#x2192; Slot 2</button>
          <button type="button" class="tactile-btn btn-sm" onclick="fillSlotFromScan(3)" title="Assign to Slot 3">&#x2192; Slot 3</button>
          <button type="button" class="tactile-btn btn-sm" onclick="fillSlotFromScan(4)" title="Assign to Slot 4">&#x2192; Slot 4</button>
        </span>
      </div>

      <table class="param-table">
        <!-- PROFILE 1 -->
        <tr>
          <td class="label-cell">
            <div class="param-name"><span data-i18n="lbl_slot_1">Profile 01 (Primary)</span> <span id="wifi_badge_1" style="font-size:9px; margin-left:4px;"></span></div>
            <div class="param-desc" data-i18n="desc_slot_1">Primary Wi-Fi access point (e.g. Home)</div>
          </td>
          <td>
            <div style="display:flex; flex-direction:column; gap:6px;">
              <input type="text" id="ssid" name="ssid" value="" placeholder="Network 1 SSID" autocomplete="off">
              <input type="password" id="password" name="password" placeholder="Password (leave empty if unchanged)" data-i18n-ph="ph_pass">
            </div>
          </td>
        </tr>

        <!-- PROFILE 2 -->
        <tr>
          <td class="label-cell">
            <div class="param-name"><span data-i18n="lbl_slot_2">Profile 02 (Secondary)</span> <span id="wifi_badge_2" style="font-size:9px; margin-left:4px;"></span></div>
            <div class="param-desc" data-i18n="desc_slot_2">Second location (e.g. Office / Work / Cottage)</div>
          </td>
          <td>
            <div style="display:flex; flex-direction:column; gap:6px;">
              <div style="display:flex; gap:6px;">
                <input type="text" id="ssid_2" name="ssid_2" value="" placeholder="Network 2 SSID (e.g. Office)" autocomplete="off" style="flex:1;">
                <button type="button" class="tactile-btn btn-sm" onclick="clearWifiSlot(2)" style="color:var(--signal-orange);" title="Clear Slot 2">&#x2715;</button>
              </div>
              <input type="password" id="password_2" name="password_2" placeholder="Password (leave empty if unchanged)" data-i18n-ph="ph_pass">
              <input type="hidden" id="del_wifi_2" name="del_wifi_2" value="0">
            </div>
          </td>
        </tr>

        <!-- PROFILE 3 -->
        <tr>
          <td class="label-cell">
            <div class="param-name"><span data-i18n="lbl_slot_3">Profile 03 (Hotspot)</span> <span id="wifi_badge_3" style="font-size:9px; margin-left:4px;"></span></div>
            <div class="param-desc" data-i18n="desc_slot_3">Mobile or travel hotspot (e.g. Smartphone)</div>
          </td>
          <td>
            <div style="display:flex; flex-direction:column; gap:6px;">
              <div style="display:flex; gap:6px;">
                <input type="text" id="ssid_3" name="ssid_3" value="" placeholder="Network 3 SSID (e.g. Hotspot)" autocomplete="off" style="flex:1;">
                <button type="button" class="tactile-btn btn-sm" onclick="clearWifiSlot(3)" style="color:var(--signal-orange);" title="Clear Slot 3">&#x2715;</button>
              </div>
              <input type="password" id="password_3" name="password_3" placeholder="Password (leave empty if unchanged)" data-i18n-ph="ph_pass">
              <input type="hidden" id="del_wifi_3" name="del_wifi_3" value="0">
            </div>
          </td>
        </tr>

        <!-- PROFILE 4 -->
        <tr>
          <td class="label-cell">
            <div class="param-name"><span data-i18n="lbl_slot_4">Profile 04 (Auxiliary)</span> <span id="wifi_badge_4" style="font-size:9px; margin-left:4px;"></span></div>
            <div class="param-desc" data-i18n="desc_slot_4">Backup or guest network</div>
          </td>
          <td>
            <div style="display:flex; flex-direction:column; gap:6px;">
              <div style="display:flex; gap:6px;">
                <input type="text" id="ssid_4" name="ssid_4" value="" placeholder="Network 4 SSID (Optional)" autocomplete="off" style="flex:1;">
                <button type="button" class="tactile-btn btn-sm" onclick="clearWifiSlot(4)" style="color:var(--signal-orange);" title="Clear Slot 4">&#x2715;</button>
              </div>
              <input type="password" id="password_4" name="password_4" placeholder="Password (leave empty if unchanged)" data-i18n-ph="ph_pass">
              <input type="hidden" id="del_wifi_4" name="del_wifi_4" value="0">
            </div>
          </td>
        </tr>
      </table>
    </div>

    <!-- PERMANENT SAVE BAR -->
    <div class="save-bar with-crosshair">
      <div class="save-info">
        <span class="save-dot"></span>
        <span data-i18n="save_desc">Parameters are stored to NVS Flash memory and committed immediately.</span>
      </div>
      <button type="submit" class="tactile-btn primary" data-i18n="btn_save">Save &amp; Apply Settings</button>
    </div>
  </form>

  <!-- PANEL 05: FIRMWARE OTA (OUTSIDE FORM) -->
  <div class="control-panel-section" id="panel-ota">
    <div class="section-legend" data-i18n="grp_ota">Field Firmware Upgrade (OTA)</div>
    <div style="font-size:11px; color:var(--ink-secondary); margin-bottom:12px;">
      <span data-i18n="lbl_running_ver">Running Version:</span> <strong id="otaRunningVer" style="color:var(--signal-orange);">v2.0.0</strong>
    </div>

    <table class="param-table">
      <tr>
        <td class="label-cell">
          <div class="param-name" data-i18n="lbl_flash_local">Flash Local Binary (.bin)</div>
          <div class="param-desc">Direct multipart upload to flash memory</div>
        </td>
        <td>
          <input type="file" id="ota_file" accept=".bin" style="padding:4px;">
          <div style="margin-top:6px;">
            <button type="button" class="tactile-btn btn-sm" onclick="uploadFirmware()" data-i18n="btn_flash">&#x21E7; Flash Firmware File</button>
          </div>
          <div id="ota_progress" style="display:none; margin-top:8px;">
            <div class="ota-meter-wrap">
              <div id="ota_bar"></div>
            </div>
            <div id="ota_msg" style="font-size:10px; color:var(--ink-secondary); margin-top:4px;"></div>
          </div>
        </td>
      </tr>
      <tr>
        <td class="label-cell">
          <div class="param-name" data-i18n="btn_check_ota">Cloud Releases (GitHub)</div>
          <div class="param-desc">Direct OTA download and flash from official releases</div>
        </td>
        <td>
          <button type="button" class="tactile-btn btn-sm" onclick="checkCloudOta()" data-i18n="btn_check_ota">&#x2601; Check GitHub Releases</button>
          <div id="cloud_ota_res" style="font-size:11px; margin-top:8px;"></div>
        </td>
      </tr>
    </table>
  </div>

  <!-- TERMINAL FOOTER -->
  <footer class="terminal-footer">
    <div>ULANZI TC001 EVOLVED // TACTILE FIELD INSTRUMENT &bull; <a href="https://buymeacoffee.com/hackra" target="_blank" rel="noopener" class="footer-coffee-link" onclick="openBmc(event)">&#x2615; BUY ME A COFFEE</a></div>
    <div id="footerFwVer">FIRMWARE v2.0.0 &bull; NVS ARMED</div>
  </footer>
</div>

<script>
const I18N = {
  en: {
    app_title: "ULANZI TC001 EVOLVED",
    app_sub: "TACTILE FIELD INSTRUMENT // 8×32 MATRIX TELEMETRY",
    tab_clock: "DISPLAY & CLOCK",
    tab_carousel: "CAROUSEL & SCREENS",
    tab_services: "INTEGRATIONS & FEEDS",
    tab_network: "NETWORK & SYSTEM",
    tab_ota: "FIRMWARE OTA",
    tele_pwr: "POWER // BATT",
    tele_cap: "EST. CAPACITY",
    tele_env: "ENVIRONMENT",
    tele_hum: "RELATIVE HUMIDITY",
    tele_opt: "OPTICAL AMBIENT",
    tele_bri: "AUTO BRIGHTNESS",
    tele_net: "RF CARRIER / SCREEN",
    tele_act: "ACTIVE SCREEN",
    lbl_channel: "CHANNEL:",
    save_desc: "Parameters are stored to NVS Flash memory and committed immediately.",
    live_stream: "REAL-TIME DISPLAY MATRIX // 32×8 NEOPIXEL MIRROR",
    cap_bmp: "\u2399 CAPTURE BMP",
    btn_prev: "\u2190 PREV",
    btn_next: "NEXT \u2192",
    grp_clock: "Display & Clock Engine",
    sub_clock_layout: "Clock & Date Architecture",
    lbl_clk_mode: "Clock Display Mode",
    opt_clk_time: "Time Only",
    opt_clk_both: "Combined Time & Date (Alternating)",
    opt_clk_date: "Date Only",
    lbl_date_fmt: "Date Format",
    lbl_clk_icon: "Clock Icon Style",
    opt_icon_cal: "Dynamic Calendar (Date 1-31)",
    opt_icon_heart: "Heart (Classic)",
    opt_icon_clock: "Clock Face",
    opt_icon_none: "None (Full Width)",
    lbl_txt_align: "Text Alignment",
    opt_align_left: "Left-Aligned with Icon (Awtrix)",
    opt_align_center: "Centered in Window",
    chk_show_sec: "Show Seconds in Time Display",
    chk_wd_bar: "Show 7 Weekday Bars Under Time",
    lbl_wd_mon: "Week Starts On",
    opt_mon_iso: "Monday (ISO 8601)",
    opt_sun_us: "Sunday (US Standard)",
    lbl_wd_acol: "Active Day Color",
    lbl_wd_icol: "Inactive Day Color",
    opt_inact_auto: "Dim Active Color (Auto - Slightly Dimmer)",
    opt_inact_off: "Off (Black)",
    lbl_wd_wcol: "Weekend Color (Sat & Sun)",
    opt_wknd_active: "Same as Active Weekday",
    sub_ambient: "Ambient Photometry & Night Cycle",
    chk_auto_bri: "Auto-Brightness via LDR Light Sensor",
    lbl_day_bri: "Day Brightness Ceiling",
    chk_night_mode: "Night Mode (Quiet Dim Red Clock)",
    lbl_night_bri: "Night Mode Brightness Floor",
    lbl_night_start: "Quiet Hours Schedule",
    lbl_night_end: "Quiet Hours Schedule",
    grp_carousel: "Carousel Engine & Transitions",
    chk_auto_rotate: "Auto-Rotate Screens (Carousel)",
    lbl_rotate_interval: "Cycle Interval",
    lbl_trans_fx: "Screen Transition Effect",
    opt_trans_0: "None (Instant Cut)",
    opt_trans_1: "Slide Left",
    opt_trans_2: "Slide Right",
    opt_trans_3: "Slide Up",
    opt_trans_4: "Slide Down",
    opt_trans_5: "Crossfade / Dissolve",
    opt_trans_6: "Curtain (Center Split Wipe)",
    opt_trans_7: "Random (Shuffle Effects)",
    lbl_active_app: "Default / Active Screen",
    app_name_0: "Clock (Heart / Time)",
    app_name_1: "Indoor Environment (SHT30 Temp)",
    app_name_2: "Outdoor Weather (Open-Meteo)",
    app_name_3: "Crypto & Financial Markets",
    app_name_4: "YouTube Subscriber Count",
    app_name_5: "Live Airspace Radar (ADS-B)",
    app_name_6: "Battery & Energy Telemetry",
    app_name_7: "Stopwatch & Countdown Timer",
    app_name_8: "Retro Ambient Screensaver",
    grp_screensaver: "Retro Ambient Screensaver",
    lbl_ss_style: "Screensaver Visual Style",
    opt_ss_0: "Matrix Digital Rain (Cyberpunk Phosphor)",
    opt_ss_1: "NeoPixel Fire / Lava (Thermal Simulation)",
    opt_ss_2: "Pac-Man & Blinky (80s Arcade Chase)",
    opt_ss_3: "Conway's Game of Life (Cellular Automata)",
    opt_ss_4: "Dynamic Shuffle (Cycle All Styles)",
    lbl_ss_timeout: "Inactivity Auto-Activation Timeout",
    opt_ss_to_0: "Disabled (Carousel Only)",
    grp_timer: "Stopwatch & Countdown Timer Remote",
    btn_timer_toggle: "Toggle (Start/Pause)",
    btn_timer_reset: "Reset",
    lbl_timer_mode: "Mode:",
    lbl_carousel_screens: "Active Rotation Sequence (Check to Enable \u2022 Use \u25B2\u25BC to Reorder)",
    sub_battery: "Battery Telemetry Format",
    lbl_bat_mode: "Battery Display Format",
    opt_bat_alt: "Alternating (Percentage \u2022 Voltage every 3s)",
    opt_bat_pct: "Percentage Only (e.g. 94%)",
    opt_bat_volt: "Voltage Only (e.g. 4.12V)",
    grp_services: "Integrations & Feeds",
    sub_weather: "Meteorological Telemetry (Open-Meteo)",
    lbl_city_preset: "Predefined City Preset",
    opt_choose_city: "-- Choose City Preset --",
    lbl_city_name: "City Name",
    lbl_lat: "Geographic Coordinates",
    lbl_lon: "Geographic Coordinates",
    sub_finance: "Financial Markets & Cryptocurrency",
    lbl_asset_preset: "Asset / Index Preset",
    lbl_custom_ticker: "Custom Symbol",
    sub_youtube: "YouTube Channel Telemetry",
    lbl_yt_channel: "Channel ID or @Handle",
    lbl_yt_key: "YouTube Data API v3 Key",
    sub_ha_mqtt: "Home Assistant & MQTT Telemetry",
    chk_mqtt_en: "Enable MQTT Client & Home Assistant Auto-Discovery",
    lbl_mqtt_srv: "Broker IP or Hostname",
    lbl_mqtt_port: "Port",
    lbl_mqtt_user: "Authentication",
    lbl_mqtt_pass: "Authentication",
    lbl_mqtt_pre: "Topic Prefix",
    box_custom_push: "\U0001F4A1 <strong>Push Spotify &amp; Custom Screens:</strong><br>MQTT: Publish JSON to <code>ulanzi-tc001/custom/spotify</code> or <code>ulanzi-tc001/notify</code><br>HTTP REST: <code>POST /api/custom?name=spotify</code> or <code>POST /api/notify</code><br>Payload: <code>{\"text\":\"Song - Artist\",\"icon\":\"spotify\",\"progress\":45,\"duration\":8}</code>",
    grp_network: "Wi-Fi RF Link & Multi-Network Memory",
    desc_wifi_roam: "Stores up to 4 Wi-Fi networks. Automatically detects and connects to whichever saved network is within RF range with the highest signal strength (ideal for home / office roaming).",
    lbl_slot_1: "Profile 01 (Primary)",
    desc_slot_1: "Primary Wi-Fi access point (e.g. Home)",
    lbl_slot_2: "Profile 02 (Secondary)",
    desc_slot_2: "Second location (e.g. Office / Work / Cottage)",
    lbl_slot_3: "Profile 03 (Hotspot)",
    desc_slot_3: "Mobile or travel hotspot (e.g. Smartphone)",
    lbl_slot_4: "Profile 04 (Auxiliary)",
    desc_slot_4: "Backup or guest network",
    lbl_ssid: "Network Name (SSID)",
    btn_scan: "\u21BB Scan Nearby Networks",
    opt_scan_detect: "-- Detected Networks --",
    lbl_pass: "Wi-Fi Password",
    ph_pass: "Leave empty if unchanged",
    btn_save: "Save & Apply Settings",
    grp_ota: "Field Firmware Upgrade (OTA)",
    lbl_running_ver: "Running Version:",
    lbl_flash_local: "Flash Local Binary (.bin)",
    btn_flash: "\u21E7 Flash Firmware File",
    btn_check_ota: "\u2601 Check GitHub Releases",
    toast_saved: "\u2713 Parameters Committed to NVS",
    toast_saving: "Transmitting..."
  },
  sk: {
    app_title: "ULANZI TC001 EVOLVED",
    app_sub: "TAKTIK\u00DD TER\u00C9NNY PR\u00CDSTROJ // 8\u00D732 MATICOV\u00C1 TELEMETRIA",
    tab_clock: "DISPLEJ A HODINY",
    tab_carousel: "KOLOTO\u010C A OBRAZOVKY",
    tab_services: "INTEGR\u00C1CIE A D\u00C1TA",
    tab_network: "SIE\u0164 A SYST\u00C9M",
    tab_ota: "FIRMWARE OTA",
    tele_pwr: "NAP\u00C1JANIE // BAT\u00C9RIA",
    tele_cap: "ODHAD KAPACITY",
    tele_env: "PROSTREDIE",
    tele_hum: "RELAT\u00CDVNA VLHKOS\u0164",
    tele_opt: "OPTICK\u00DD SENZOR",
    tele_bri: "AUTOMATICK\u00DD JAS",
    tele_net: "RF SPOJENIE / DISPLEJ",
    tele_act: "AKT\u00CDVNA OBRAZOVKA",
    lbl_channel: "KAN\u00C1L:",
    save_desc: "Parametre s\u00FA trvalo ulo\u017Een\u00E9 v pam\u00E4ti NVS Flash s okam\u017Eitou platnos\u0165ou.",
    live_stream: "DISPLEJOV\u00C1 MATICA V RE\u00C1LNOM \u010CASE // 32\u00D78 NEOPIXEL",
    cap_bmp: "\u2399 SN\u00CDMKA BMP",
    btn_prev: "\u2190 PREDCH",
    btn_next: "\u010EAL\u0160IA \u2192",
    grp_clock: "Displej a Hodiny",
    sub_clock_layout: "Architekt\u00FAra hod\u00EDn a d\u00E1tumu",
    lbl_clk_mode: "Re\u017Eim zobrazenia hod\u00EDn",
    opt_clk_time: "Iba \u010Das",
    opt_clk_both: "Kombinovan\u00FD \u010Das a d\u00E1tum (striedanie)",
    opt_clk_date: "Iba d\u00E1tum",
    lbl_date_fmt: "Form\u00E1t d\u00E1tumu",
    lbl_clk_icon: "\u0160t\u00FDl ikony hod\u00EDn",
    opt_icon_cal: "Dynamick\u00FD kalend\u00E1r (de\u0148 1-31)",
    opt_icon_heart: "Srdie\u010Dko (Klasick\u00E9)",
    opt_icon_clock: "Cifern\u00EDk",
    opt_icon_none: "\u017Diadna ikona (pln\u00E1 \u0161\u00EDrka)",
    lbl_txt_align: "Zarovnanie textu",
    opt_align_left: "Zarovnan\u00E9 do\u013Eava s ikonou (Awtrix)",
    opt_align_center: "Vycentrovan\u00E9 na stred",
    chk_show_sec: "Zobrazi\u0165 sekundy na displeji \u010Dasu",
    chk_wd_bar: "Zobrazi\u0165 7 p\u00E1sikov dn\u00ED v t\u00FD\u017Edni pod \u010Dasom",
    lbl_wd_mon: "T\u00FD\u017Ede\u0148 za\u010D\u00EDna d\u0148om",
    opt_mon_iso: "Pondelok (ISO 8601)",
    opt_sun_us: "Nede\u013Ea (US \u0161tandard)",
    lbl_wd_acol: "Farba akt\u00EDvneho d\u0148a",
    lbl_wd_icol: "Farba neakt\u00EDvneho d\u0148a",
    opt_inact_auto: "Tlmen\u00E1 akt\u00EDvna farba (Auto - jemne stmaven\u00E1)",
    opt_inact_off: "Vypnut\u00E9 (\u010Dierna)",
    lbl_wd_wcol: "Farba v\u00EDkendu (Sobota a Nede\u013Ea)",
    opt_wknd_active: "Rovnak\u00E1 ako akt\u00EDvny de\u0148",
    sub_ambient: "Ambientn\u00FD jas a No\u010Dn\u00FD re\u017Eim",
    chk_auto_bri: "Automatick\u00FD jas cez LDR sveteln\u00FD senzor",
    lbl_day_bri: "Maxim\u00E1lny denn\u00FD jas",
    chk_night_mode: "No\u010Dn\u00FD re\u017Eim (Tich\u00FD tlmen\u00FD \u010Derven\u00FD \u010Das)",
    lbl_night_bri: "Minim\u00E1lny no\u010Dn\u00FD jas",
    lbl_night_start: "\u010Cas no\u010Dn\u00E9ho re\u017Eimu",
    lbl_night_end: "\u010Cas no\u010Dn\u00E9ho re\u017Eimu",
    grp_carousel: "Obrazovky a Prechody",
    chk_auto_rotate: "Automatick\u00E9 striedanie obrazoviek (Koloto\u010D)",
    lbl_rotate_interval: "Interval rot\u00E1cie",
    lbl_trans_fx: "Prechodov\u00FD efekt obrazoviek",
    opt_trans_0: "\u017Diadny (Okam\u017Eit\u00FD prepis)",
    opt_trans_1: "Posun do\u013Eava (Slide Left)",
    opt_trans_2: "Posun doprava (Slide Right)",
    opt_trans_3: "Posun nahor (Slide Up)",
    opt_trans_4: "Posun nadol (Slide Down)",
    opt_trans_5: "Prel\u00EDnanie (Crossfade / Dissolve)",
    opt_trans_6: "Opona (Center Split Wipe)",
    opt_trans_7: "N\u00E1hodn\u00FD efekt (Shuffle)",
    lbl_active_app: "Predvolen\u00E1 / Akt\u00EDvna obrazovka",
    app_name_0: "Hodiny (\u010Cas / srdce)",
    app_name_1: "Vn\u00FAtorn\u00E9 prostredie (SHT30 teplota)",
    app_name_2: "Predpove\u010F po\u010Dasia (Open-Meteo)",
    app_name_3: "Kryptomeny a Finan\u010Dn\u00E9 trhy",
    app_name_4: "Po\u010D\u00EDtadlom YouTube odberate\u013Eov",
    app_name_5: "\u017Div\u00FD radar lietadiel (ADS-B)",
    app_name_6: "Stav bat\u00E9rie a nap\u00E1jania",
    app_name_7: "Stopky a odpo\u010D\u00EDtavanie",
    app_name_8: "Retro \u0161etri\u010D obrazovky",
    grp_screensaver: "Retro \u0161etri\u010D obrazovky",
    lbl_ss_style: "\u0160t\u00FDl \u0161etri\u010Da obrazovky",
    opt_ss_0: "Matrix digit\u00E1lny d\u00E1\u017E\u010F (zelen\u00FD fosfor)",
    opt_ss_1: "Doom ohe\u0148 / term\u00E1lna l\u00E1va",
    opt_ss_2: "Pac-Man ark\u00E1dov\u00E1 nah\u00E1\u0148a\u010Dka (80. roky)",
    opt_ss_3: "Conwayova hra \u017Eivota (Game of Life)",
    opt_ss_4: "Dynamick\u00E9 striedanie \u0161t\u00FDlov",
    lbl_ss_timeout: "\u010Cas ne\u010Dinnosti pre automatick\u00E9 zapnutie",
    opt_ss_to_0: "Vypnut\u00E9 (iba v koloto\u010Di)",
    grp_timer: "Ovl\u00E1da\u010D stopiek a odpo\u010D\u00EDtavania",
    btn_timer_toggle: "Spusti\u0165 / Pozastavi\u0165",
    btn_timer_reset: "Vynulova\u0165",
    lbl_timer_mode: "Re\u017Eim:",
    lbl_carousel_screens: "Akt\u00EDvna rot\u00E1cia (za\u0161krtnite pre zapnutie \u2022 \u25B2\u25BC pre poradie)",
    sub_battery: "Telemetria bat\u00E9rie a nap\u00E1jania",
    lbl_bat_mode: "Form\u00E1t zobrazenia bat\u00E9rie",
    opt_bat_alt: "Striedav\u00E9 (Percent\u00E1 \u2022 Nap\u00E4tie ka\u017Ed\u00E9 3s)",
    opt_bat_pct: "Iba percent\u00E1 (napr. 94%)",
    opt_bat_volt: "Iba nap\u00E4tie (napr. 4.12V)",
    grp_services: "Integr\u00E1cie a D\u00E1ta",
    sub_weather: "Meteorologick\u00E1 telemetria (Open-Meteo)",
    lbl_city_preset: "Predvolen\u00E9 mesto",
    opt_choose_city: "-- Vyberte mesto zo zoznamu --",
    lbl_city_name: "N\u00E1zov mesta",
    lbl_lat: "Zemepisn\u00E9 s\u00FAradnice",
    lbl_lon: "Zemepisn\u00E9 s\u00FAradnice",
    sub_finance: "Finan\u010Dn\u00E9 trhy a Kryptomeny",
    lbl_asset_preset: "Predvolen\u00E9 akt\u00EDvum / Ticker",
    lbl_custom_ticker: "Vlastn\u00FD symbol",
    sub_youtube: "Telemetria YouTube kan\u00E1la",
    lbl_yt_channel: "ID kan\u00E1la alebo @Handle",
    lbl_yt_key: "K\u013E\u00FA\u010D YouTube Data API v3",
    sub_ha_mqtt: "Home Assistant & MQTT Telemetria",
    chk_mqtt_en: "Povoli\u0165 MQTT klienta & Home Assistant Auto-Discovery",
    lbl_mqtt_srv: "IP alebo n\u00E1zov MQTT brokera",
    lbl_mqtt_port: "Port",
    lbl_mqtt_user: "Autentifik\u00E1cia",
    lbl_mqtt_pass: "Autentifik\u00E1cia",
    lbl_mqtt_pre: "Prefix t\u00E9my (Topic Prefix)",
    box_custom_push: "\U0001F4A1 <strong>Odosielanie Spotify a vlastn\u00FDch obrazoviek:</strong><br>MQTT: Publikujte JSON na <code>ulanzi-tc001/custom/spotify</code> alebo <code>ulanzi-tc001/notify</code><br>HTTP REST: <code>POST /api/custom?name=spotify</code> alebo <code>POST /api/notify</code><br>Form\u00E1t: <code>{\"text\":\"Song - Artist\",\"icon\":\"spotify\",\"progress\":45,\"duration\":8}</code>",
    grp_network: "Wi-Fi RF Spojenie & Pam\u00E4\u0165 siet\u00ED",
    desc_wifi_roam: "Uklad\u00E1 a\u017E 4 Wi-Fi siete. Automaticky presk\u00FAma okolie a pripoj\u00ED sa k dostupnej ulo\u017Eenej sieti s najsilnej\u0161\u00EDm sign\u00E1lom (ide\u00E1lne na pren\u00E1\u0161anie domov / pr\u00E1ca).",
    lbl_slot_1: "Profil 01 (Prim\u00E1rna)",
    desc_slot_1: "Hlavn\u00E1 bezdr\u00F4tov\u00E1 sie\u0165 (napr. Domov)",
    lbl_slot_2: "Profil 02 (Sekund\u00E1rna)",
    desc_slot_2: "Druh\u00E9 miesto (napr. Pr\u00E1ca / Kancel\u00E1ria / Chata)",
    lbl_slot_3: "Profil 03 (Hotspot)",
    desc_slot_3: "Mobiln\u00FD pr\u00EDstupov\u00FD bod (napr. Smartf\u00F3n)",
    lbl_slot_4: "Profil 04 (Z\u00E1lo\u017En\u00E1)",
    desc_slot_4: "Z\u00E1lo\u017En\u00E1 alebo hos\u0165ovsk\u00E1 sie\u0165",
    lbl_ssid: "N\u00E1zov Wi-Fi siete (SSID)",
    btn_scan: "\u21BB Vyh\u013Eada\u0165 okolit\u00E9 siete",
    opt_scan_detect: "-- N\u00E1jden\u00E9 siete --",
    lbl_pass: "Heslo Wi-Fi",
    ph_pass: "Ponechajte pr\u00E1zdne, ak sa nemenia",
    btn_save: "Ulo\u017Ei\u0165 a pou\u017Ei\u0165 nastavenia",
    grp_ota: "Aktualiz\u00E1cia firmv\u00E9ru (OTA)",
    lbl_running_ver: "Aktu\u00E1lna verzia:",
    lbl_flash_local: "Nahra\u0165 lok\u00E1lny s\u00FAbor (.bin)",
    btn_flash: "\u21E7 Nahra\u0165 a flashn\u00FA\u0165 firmv\u00E9r",
    btn_check_ota: "\u2601 Skontrolova\u0165 vydania na GitHub",
    toast_saved: "\u2713 Parametre zap\u00EDsan\u00E9 do NVS",
    toast_saving: "Pren\u00E1\u0161am..."
  }
};

let currentLang = 'en';

function setLang(lang) {
  if (!I18N[lang]) lang = 'en';
  currentLang = lang;
  document.getElementById('ui_lang').value = lang;
  try { localStorage.setItem('tc001_lang', lang); } catch(e){}

  const bEn = document.getElementById('btn-en');
  const bSk = document.getElementById('btn-sk');
  if (bEn && bSk) {
    bEn.classList.toggle('selected', lang === 'en');
    bSk.classList.toggle('selected', lang === 'sk');
  }

  const dict = I18N[lang];
  document.querySelectorAll('[data-i18n]').forEach(el => {
    const key = el.getAttribute('data-i18n');
    if (dict[key]) {
      if (dict[key].indexOf('<') !== -1) el.innerHTML = dict[key];
      else el.textContent = dict[key];
    }
  });
  document.querySelectorAll('[data-i18n-ph]').forEach(el => {
    const key = el.getAttribute('data-i18n-ph');
    if (dict[key]) el.setAttribute('placeholder', dict[key]);
  });
}

function switchPlate(plateId) {
  document.querySelectorAll('.plate-btn').forEach(btn => {
    btn.classList.toggle('active', btn.getAttribute('data-plate') === plateId);
  });
  document.querySelectorAll('.control-panel-section').forEach(sec => {
    sec.classList.toggle('active', sec.id === 'panel-' + plateId);
  });
  try { localStorage.setItem('tc001_plate', plateId); } catch(e){}
}

function scanNetworks() {
  const sel = document.getElementById('scanSelect');
  sel.style.display = 'block';
  sel.innerHTML = '<option>' + (currentLang === 'sk' ? 'Vyh\u013Ead\u00E1vam siete...' : 'Scanning...') + '</option>';
  fetch('/scan')
    .then(r => r.json())
    .then(data => {
      const header = currentLang === 'sk' ? ('-- N\u00E1jden\u00E9 siete (' + data.length + ') --') : ('-- Detected Networks (' + data.length + ') --');
      sel.innerHTML = '<option value="">' + header + '</option>';
      data.forEach(net => {
        const opt = document.createElement('option');
        opt.value = net.ssid;
        opt.textContent = net.ssid + ' (' + net.rssi + ' dBm' + (net.secure ? ' \uD83D\uDD12' : '') + ')';
        sel.appendChild(opt);
      });
      const btns = document.getElementById('scanSlotButtons');
      if (btns) btns.style.display = 'inline-flex';
    })
    .catch(() => {
      sel.innerHTML = '<option value="">' + (currentLang === 'sk' ? 'Vyh\u013Ead\u00E1vanie zlyhalo' : 'Scan failed') + '</option>';
    });
}

function selectNetwork(val) {
  if (!val) return;
  const s1 = document.getElementById('ssid');
  const s2 = document.getElementById('ssid_2');
  const s3 = document.getElementById('ssid_3');
  const s4 = document.getElementById('ssid_4');
  if (s1 && !s1.value) s1.value = val;
  else if (s2 && !s2.value) s2.value = val;
  else if (s3 && !s3.value) s3.value = val;
  else if (s4 && !s4.value) s4.value = val;
  else if (s1) s1.value = val;
}

function fillSlotFromScan(slot) {
  const sel = document.getElementById('scanSelect');
  if (!sel || !sel.value) return;
  const inputId = (slot === 1) ? 'ssid' : ('ssid_' + slot);
  const el = document.getElementById(inputId);
  if (el) el.value = sel.value;
}

function clearWifiSlot(slot) {
  const ssidEl = document.getElementById('ssid_' + slot);
  const passEl = document.getElementById('password_' + slot);
  const delEl  = document.getElementById('del_wifi_' + slot);
  if (ssidEl) ssidEl.value = '';
  if (passEl) passEl.value = '';
  if (delEl) delEl.value = '1';
  const badge = document.getElementById('wifi_badge_' + slot);
  if (badge) {
    badge.innerText = '[CLEARED]';
    badge.style.color = 'var(--signal-orange)';
  }
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
    alert(currentLang === 'sk' ? 'Najprv vyberte .bin s\u00FAbor firmv\u00E9ru' : 'Please select a .bin firmware file first');
    return;
  }
  const file = fileInput.files[0];
  const formData = new FormData();
  formData.append('update', file);

  const prog = document.getElementById('ota_progress');
  const bar = document.getElementById('ota_bar');
  const msg = document.getElementById('ota_msg');
  prog.style.display = 'block';
  msg.textContent = (currentLang === 'sk' ? 'Odosielanie: ' : 'Uploading: ') + '0%';

  const xhr = new XMLHttpRequest();
  xhr.open('POST', '/update', true);
  xhr.upload.onprogress = function(e) {
    if (e.lengthComputable) {
      const pct = Math.round((e.loaded / e.total) * 100);
      bar.style.width = pct + '%';
      msg.textContent = (currentLang === 'sk' ? 'Nahr\u00E1vanie a flashovanie: ' : 'Uploading & Flashing: ') + pct + '%';
    }
  };
  xhr.onload = function() {
    if (xhr.status === 200) {
      bar.style.width = '100%';
      bar.style.background = 'var(--phosphor-green)';
      msg.innerHTML = '<strong style="color:var(--phosphor-green);">\u2713 ' + (currentLang === 'sk' ? 'Flash dokon\u010Den\u00FD! Re\u0161tartujem pr\u00EDstroj...' : 'Flash Complete! Rebooting instrument...') + '</strong>';
      setTimeout(() => location.reload(), 6000);
    } else {
      bar.style.background = 'var(--signal-red)';
      msg.innerHTML = '<strong style="color:var(--signal-red);">\u2717 ' + (currentLang === 'sk' ? 'Chyba aktualiz\u00E1cie: ' : 'Update Error: ') + xhr.responseText + '</strong>';
    }
  };
  xhr.onerror = function() {
    bar.style.background = 'var(--signal-red)';
    msg.innerHTML = '<strong style="color:var(--signal-red);">\u2717 ' + (currentLang === 'sk' ? 'Chyba spojenia' : 'Network error') + '</strong>';
  };
  xhr.send(formData);
}

function checkCloudOta() {
  const res = document.getElementById('cloud_ota_res');
  res.innerHTML = '<span style="color:var(--ink-muted);">' + (currentLang === 'sk' ? 'Kontrolujem GitHub vydania...' : 'Checking GitHub Releases...') + '</span>';
  fetch('/api/ota/check')
    .then(r => r.json())
    .then(data => {
      if (data.has_update) {
        const title = currentLang === 'sk' ? ('\u2605 Nov\u00E1 verzia k dispoz\u00EDcii: v' + data.latest_version) : ('\u2605 New Version Available: v' + data.latest_version);
        const btnText = currentLang === 'sk' ? ('In\u0161talova\u0165 v' + data.latest_version + ' teraz') : ('Install v' + data.latest_version + ' Now');
        res.innerHTML = '<div style="color:var(--phosphor-green); font-weight:700; margin-bottom:6px;">' + title + '</div>' +
          '<button type="button" class="tactile-btn primary btn-sm" onclick="installCloudOta(\'' + data.download_url + '\')">' + btnText + '</button>';
      } else {
        res.innerHTML = '<span style="color:var(--phosphor-green);">\u2713 ' + (currentLang === 'sk' ? ('Firmv\u00E9r je aktu\u00E1lny (v' + data.current_version + ')') : ('Firmware is up to date (v' + data.current_version + ')')) + '</span>';
      }
    })
    .catch(() => {
      res.innerHTML = '<span style="color:var(--signal-red);">' + (currentLang === 'sk' ? 'Nie je mo\u017En\u00E9 kontaktova\u0165 GitHub' : 'Cannot reach GitHub releases') + '</span>';
    });
}

function installCloudOta(url) {
  const res = document.getElementById('cloud_ota_res');
  res.innerHTML = '<span style="color:var(--signal-orange);">' + (currentLang === 'sk' ? 'S\u0165ahujem a flashujem z GitHubu... Sledujte displej pr\u00EDstroja!' : 'Downloading and flashing from GitHub... Watch instrument display!') + '</span>';
  const fd = new FormData();
  fd.append('url', url);
  fetch('/api/ota/install', { method: 'POST', body: fd })
    .then(r => r.json())
    .then(d => {
      setTimeout(() => location.reload(), 10000);
    });
}

let currentAppIndex = 0;
const appNames = ["Clock", "Indoor Temp", "Outdoor Weather", "Crypto", "YouTube", "Airspace", "Battery", "Timer", "Screensaver"];

function switchScreen(dir) {
  currentAppIndex = (currentAppIndex + dir + 9) % 9;
  fetch('/api/screen?set=' + currentAppIndex)
    .then(r => r.json())
    .then(d => {
      if (d.active_app != null) currentAppIndex = d.active_app;
      const name = d.app_name || appNames[currentAppIndex];
      const b = document.getElementById('screenNameBadge');
      if (b) b.innerText = name;
      const t = document.getElementById('teleScreenName');
      if (t) t.innerText = name;
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
      ctx.fillStyle = '#05070A';
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


let configLoaded = false;
function populateConfig(d) {
  if (!d) return;
  if (d.wifi_profiles && Array.isArray(d.wifi_profiles)) {
    d.wifi_profiles.forEach(p => {
      const slot = p.slot;
      const ssidEl = (slot === 1) ? document.getElementById('ssid') : document.getElementById('ssid_' + slot);
      const passEl = (slot === 1) ? document.getElementById('password') : document.getElementById('password_' + slot);
      if (ssidEl && !configLoaded) ssidEl.value = p.ssid || '';
      if (passEl && p.has_pass) {
        passEl.placeholder = '•••••••• (' + (currentLang === 'sk' ? 'Ulo\u017Een\u00E9' : 'Saved') + ')';
      }
      const badge = document.getElementById('wifi_badge_' + slot);
      if (badge) {
        if (p.active) {
          badge.innerText = '[CONNECTED]';
          badge.style.color = 'var(--phosphor-green)';
        } else if (p.ssid && p.ssid.length > 0) {
          badge.innerText = '[STANDBY]';
          badge.style.color = 'var(--lab-ochre)';
        } else {
          badge.innerText = '[EMPTY]';
          badge.style.color = 'var(--ink-muted)';
        }
      }
    });
  } else if (!configLoaded && d.ssid != null) {
    const el = document.getElementById('ssid'); if (el) el.value = d.ssid;
  }
  if (!configLoaded) {
    if (d.rotate_sec != null) { const el = document.getElementById('rotate'); if (el) el.value = d.rotate_sec; }
    if (d.auto_rotate != null) {
      const el = document.getElementById('auto_rotate');
      if (el) { el.checked = !!d.auto_rotate; const row = document.getElementById('rotate_delay_field'); if (row) row.style.display = d.auto_rotate ? 'table-row' : 'none'; }
    }
    if (d.active_app != null) { const el = document.getElementById('active_app'); if (el) el.value = d.active_app; }
    if (d.bat_mode != null) { const el = document.getElementById('bat_mode'); if (el) el.value = d.bat_mode; }
    if (d.transition_fx != null) { const el = document.getElementById('trans_fx'); if (el) el.value = d.transition_fx; }
    if (d.clk_mode != null) { const el = document.getElementById('clk_mode'); if (el) el.value = d.clk_mode; }
    if (d.show_sec != null) { const el = document.querySelector('input[name="show_sec"]'); if (el) el.checked = !!d.show_sec; }
    if (d.wd_bar != null) { const el = document.querySelector('input[name="wd_bar"]'); if (el) el.checked = !!d.wd_bar; }
    if (d.wd_mon != null) { const el = document.getElementById('wd_mon'); if (el) el.value = d.wd_mon ? '1' : '0'; }
    if (d.wd_acol != null) { const el = document.getElementById('wd_acol'); if (el) el.value = d.wd_acol; }
    if (d.wd_icol != null) { const el = document.getElementById('wd_icol'); if (el) el.value = d.wd_icol; }
    if (d.wd_wcol != null) { const el = document.getElementById('wd_wcol'); if (el) el.value = d.wd_wcol; }
    if (d.date_fmt != null) { const el = document.getElementById('date_fmt'); if (el) el.value = d.date_fmt; }
    if (d.clk_icon != null) { const el = document.getElementById('clk_icon'); if (el) el.value = d.clk_icon; }
    if (d.txt_align != null) { const el = document.getElementById('txt_align'); if (el) el.value = d.txt_align; }
    if (d.auto_bri != null) { const el = document.querySelector('input[name="auto_bri"]'); if (el) el.checked = !!d.auto_bri; }
    if (d.night_mode != null) { const el = document.querySelector('input[name="night_mode"]'); if (el) el.checked = !!d.night_mode; }
    if (d.night_start != null) { const el = document.getElementById('night_start'); if (el) el.value = d.night_start; }
    if (d.night_end != null) { const el = document.getElementById('night_end'); if (el) el.value = d.night_end; }
    if (d.day_bri != null) { const el = document.getElementById('day_bri'); if (el) { el.value = d.day_bri; const v = document.getElementById('day_bri_val'); if (v) v.innerText = d.day_bri + '%'; } }
    if (d.night_bri != null) { const el = document.getElementById('night_bri'); if (el) { el.value = d.night_bri; const v = document.getElementById('night_bri_val'); if (v) v.innerText = d.night_bri + '%'; } }
    if (d.weather_city != null) { const el = document.getElementById('w_city'); if (el) el.value = d.weather_city; }
    if (d.weather_lat != null) { const el = document.getElementById('w_lat'); if (el) el.value = d.weather_lat; }
    if (d.weather_lon != null) { const el = document.getElementById('w_lon'); if (el) el.value = d.weather_lon; }
    if (d.crypto_symbol != null) {
      const el = document.getElementById('c_sym');
      if (el) {
        el.value = d.crypto_symbol;
        if (el.value !== d.crypto_symbol) {
          const customEl = document.getElementById('c_sym_custom');
          if (customEl) customEl.value = d.crypto_symbol;
        }
      }
    }
    if (d.yt_channel != null) { const el = document.getElementById('yt_channel'); if (el) el.value = d.yt_channel; }
    if (d.yt_has_key) { const el = document.getElementById('yt_key'); if (el) el.value = '********'; }
    if (d.mqtt_en != null) {
      const el = document.getElementById('mqtt_en');
      if (el) {
        el.checked = !!d.mqtt_en;
        const setDiv = document.getElementById('mqtt_settings');
        if (setDiv) setDiv.style.display = d.mqtt_en ? 'block' : 'none';
      }
    }
    if (d.mqtt_srv != null) { const el = document.getElementById('mqtt_srv'); if (el) el.value = d.mqtt_srv; }
    if (d.mqtt_port != null) { const el = document.getElementById('mqtt_port'); if (el) el.value = d.mqtt_port; }
    if (d.mqtt_user != null) { const el = document.getElementById('mqtt_user'); if (el) el.value = d.mqtt_user; }
    if (d.mqtt_has_pass) { const el = document.getElementById('mqtt_pass'); if (el) el.value = '********'; }
    if (d.mqtt_pre != null) { const el = document.getElementById('mqtt_pre'); if (el) el.value = d.mqtt_pre; }
    if (d.ui_lang != null) {
      const el = document.getElementById('ui_lang');
      if (el) el.value = d.ui_lang;
      setLang(d.ui_lang);
    }
    if (d.screensaver_style != null) { const el = document.getElementById('ss_style'); if (el) el.value = d.screensaver_style; }
    if (d.screensaver_timeout != null) { const el = document.getElementById('ss_time'); if (el) el.value = d.screensaver_timeout; }
    if (d.car_ord != null && d.carousel_mask != null) {
      buildCarouselReorder(d.car_ord, d.carousel_mask);
    }
    configLoaded = true;
  }
}

function buildCarouselReorder(orderStr, mask) {
  const container = document.getElementById('carouselScreens');
  if (!container) return;
  const appNames = [
    '&#x23F0; CLOCK', '&#x1F321; INDOOR TEMP', '&#x26C5; OUTDOOR METEO',
    '&#x1F4B0; CRYPTO &amp; MARKETS', '&#x25B6; YOUTUBE', '&#x2708; AIRSPACE RADAR',
    '&#x26A1; BATTERY &amp; ENERGY', '&#x23F1; STOPWATCH &amp; TIMER',
    '&#x1F47E; SCREENSAVER'
  ];
  let order = (orderStr || '').split(',').map(s => parseInt(s.trim(), 10)).filter(n => !isNaN(n) && n >= 0 && n < 9);
  for (let i = 0; i < 9; i++) {
    if (!order.includes(i)) order.push(i);
  }
  container.innerHTML = '';
  order.forEach(appIdx => {
    const isChecked = (mask & (1 << appIdx)) !== 0;
    const div = document.createElement('div');
    div.className = 'reorder-item';
    div.setAttribute('data-app', appIdx);
    div.innerHTML = `
      <label class="actuator-label" style="margin:0;">
        <input type="checkbox" class="visually-hidden" name="car_app_${appIdx}" value="1" ${isChecked ? 'checked' : ''} onchange="updateOrderInput()">
        <span class="actuator-box"></span>
        <span class="reorder-name">${appNames[appIdx]}</span>
      </label>
      <div class="reorder-step-btns">
        <button type="button" class="tactile-btn btn-xs" onclick="moveScreen(this, -1)">&#x25B2;</button>
        <button type="button" class="tactile-btn btn-xs" onclick="moveScreen(this, 1)">&#x25BC;</button>
      </div>
    `;
    container.appendChild(div);
  });
  updateOrderInput();
}

function apiTimer(action, extraParams = '') {
  let url = '/api/timer?action=' + action + (extraParams ? '&' + extraParams : '');
  fetch(url, { method: 'POST' })
    .then(r => r.json())
    .then(d => updateTimerUI(d))
    .catch(() => {});
}

function switchTimerMode() {
  const currentMode = document.getElementById('timerModeBtn').innerText.toLowerCase();
  const nextMode = (currentMode === 'countdown') ? 'stopwatch' : 'countdown';
  apiTimer('reset', 'mode=' + nextMode);
}

function setTimerSec(sec) {
  apiTimer('reset', 'mode=countdown&duration=' + sec);
}

function updateTimerUI(d) {
  if (!d) return;
  const isStopwatch = (d.timer_mode === 'stopwatch' || d.mode === 'stopwatch');
  const modeLabel = document.getElementById('timerModeLabel');
  if (modeLabel) modeLabel.innerText = isStopwatch ? 'STOPWATCH' : 'COUNTDOWN TIMER';
  const modeBtn = document.getElementById('timerModeBtn');
  if (modeBtn) modeBtn.innerText = isStopwatch ? 'Stopwatch' : 'Countdown';
  let m = 0, s = 0, tenths = 0;
  if (isStopwatch) {
    const ms = (d.timer_elapsed_ms != null) ? d.timer_elapsed_ms : (d.elapsed_ms || 0);
    m = Math.floor(ms / 60000);
    s = Math.floor((ms % 60000) / 1000);
    tenths = Math.floor((ms % 1000) / 100);
    const disp = document.getElementById('timerDisplay');
    if (disp) disp.innerText = String(m).padStart(2, '0') + ':' + String(s).padStart(2, '0') + '.' + tenths;
  } else {
    const rem = (d.timer_remaining != null) ? d.timer_remaining :
                ((d.remaining != null) ? d.remaining : (d.timer_duration || d.duration || 300));
    m = Math.floor(rem / 60);
    s = rem % 60;
    const disp = document.getElementById('timerDisplay');
    if (disp) disp.innerText = String(m).padStart(2, '0') + ':' + String(s).padStart(2, '0');
  }
  const stateStr = d.timer_state || d.state || 'stopped';
  const stateLabel = document.getElementById('timerStateLabel');
  if (stateLabel) stateLabel.innerText = 'State: ' + stateStr.toUpperCase();
}

function updateTelemetry() {
  fetch('/status')
    .then(r => r.json())
    .then(d => {
      if (d.firmware_version != null) {
        const v = 'v' + d.firmware_version;
        const topEl = document.getElementById('topFwVer');
        if (topEl) topEl.innerText = v;
        const otaEl = document.getElementById('otaRunningVer');
        if (otaEl) otaEl.innerText = v;
        const footEl = document.getElementById('footerFwVer');
        if (footEl) footEl.innerText = 'FIRMWARE ' + v + ' \u2022 NVS ARMED';
      }
      if (d.active_app != null) {
        currentAppIndex = d.active_app;
        const name = d.app_name || appNames[currentAppIndex] || ("Screen " + currentAppIndex);
        const b = document.getElementById('screenNameBadge');
        if (b) b.innerText = name;
        const t = document.getElementById('teleScreenName');
        if (t) t.innerText = name;
      }
      if (d.battery_volts != null) {
        populateConfig(d);
        document.getElementById('teleBatteryVolts').innerText = d.battery_volts.toFixed(2);
        document.getElementById('teleBatteryPct').innerText = (d.battery_pct != null ? d.battery_pct : '--') + '%';
      }
      if (d.temperature_in != null && d.temperature_in > -40) {
        document.getElementById('teleTemp').innerText = d.temperature_in.toFixed(1);
        document.getElementById('teleHum').innerText = (d.humidity_in != null ? Math.round(d.humidity_in) : '--') + '% RH';
      }
      if (d.ldr_raw != null) {
        document.getElementById('teleLdr').innerText = d.ldr_raw;
      }
      if (d.rssi != null && d.rssi !== 0) {
        document.getElementById('teleRssi').innerText = d.rssi;
      }
      if (d.timer_mode != null) {
        updateTimerUI(d);
      }
      const dMqtt = document.getElementById('diodeMqtt');
      if (dMqtt) {
        dMqtt.className = d.mqtt_connected ? 'diode active-green' : 'diode';
      }
      const dWifi = document.getElementById('diodeWifi');
      if (dWifi) {
        dWifi.className = (d.state === 'connected') ? 'diode active-green' : 'diode active-orange';
      }
    })
    .catch(() => {});
}

setInterval(updateVirtualMatrix, 1500);
setInterval(updateTelemetry, 2500);
updateVirtualMatrix();
updateTelemetry();

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
    setTimeout(() => { t.style.display = 'none'; }, 250);
  }, 2200);
}

document.getElementById('settingsForm').addEventListener('submit', function(e) {
  e.preventDefault();
  updateOrderInput();
  const form = this;
  const btn = form.querySelector('button[type="submit"]');
  const origText = btn.innerText;
  btn.innerText = (I18N[currentLang] && I18N[currentLang].toast_saving) ? I18N[currentLang].toast_saving : 'Transmitting...';
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
    showToast((I18N[currentLang] && I18N[currentLang].toast_saved) ? I18N[currentLang].toast_saved : '\u2713 Parameters Committed to NVS');
    updateVirtualMatrix();
    updateTelemetry();
  })
  .catch(err => {
    btn.innerText = origText;
    btn.disabled = false;
    form.submit();
  });
});

// Initialize Portal Language and Plate
(function() {
  const nvsLang = document.getElementById('ui_lang').value;
  let savedLang = 'en';
  try { savedLang = localStorage.getItem('tc001_lang'); } catch(e){}
  const activeLang = (nvsLang && (nvsLang === 'sk' || nvsLang === 'en')) ? nvsLang : (savedLang || 'en');
  setLang(activeLang);

  let savedPlate = 'clock';
  try { savedPlate = localStorage.getItem('tc001_plate') || 'clock'; } catch(e){}
  switchPlate(savedPlate);
})();

function openBmc(e) {
  var btn = document.getElementById('bmc-wbtn');
  if (btn) {
    if (e && e.preventDefault) e.preventDefault();
    btn.click();
  }
}
</script>
<script data-name="BMC-Widget" data-cfasync="false" src="https://cdnjs.buymeacoffee.com/1.0.0/widget.prod.min.js" data-id="hackra" data-description="Support me on Buy me a coffee!" data-message="" data-color="#FFDD00" data-position="Right" data-x_margin="18" data-y_margin="18"></script>
</body>
</html>
)rawliteral";

static const char PAGE_SAVED[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta http-equiv="refresh" content="1;url=/">
<title>PARAMETERS COMMITTED // ULANZI TC001</title>
<style>
:root {
  --chassis-base: #121211;
  --chassis-plate: #171716;
  --hairline-main: #2A2A28;
  --hairline-bright: #3E3E3A;
  --ink-primary: #F4F3EF;
  --ink-muted: #6A6965;
  --signal-orange: #FF4F00;
  --phosphor-green: #00D062;
  --font-mono: 'JetBrains Mono', 'SF Mono', Consolas, ui-monospace, monospace;
}
*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; border-radius: 0 !important; }
body {
  min-height: 100vh;
  background: var(--chassis-base);
  color: var(--ink-primary);
  font-family: var(--font-mono);
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 16px;
}
.confirm-box {
  width: 100%;
  max-width: 440px;
  background: var(--chassis-plate);
  border: 1px solid var(--hairline-main);
  padding: 24px;
  position: relative;
}
.confirm-box::before { content: "+"; position: absolute; top: -6px; left: -5px; font-size: 10px; color: var(--ink-muted); }
.confirm-box::after { content: "+"; position: absolute; bottom: -6px; right: -5px; font-size: 10px; color: var(--ink-muted); }
.tech-tag { display: flex; justify-content: space-between; font-size: 10px; letter-spacing: 0.12em; color: var(--ink-muted); margin-bottom: 12px; }
.confirm-title { font-size: 16px; font-weight: 800; color: var(--phosphor-green); letter-spacing: 0.08em; margin-bottom: 8px; }
.confirm-desc { font-size: 12px; color: var(--ink-primary); line-height: 1.5; margin-bottom: 16px; }
.diode-bar { display: flex; align-items: center; gap: 8px; font-size: 10px; color: var(--signal-orange); border-top: 1px solid var(--hairline-main); padding-top: 12px; }
.diode { width: 6px; height: 6px; background: var(--signal-orange); display: inline-block; }
</style>
</head>
<body>
<div class="confirm-box">
  <div class="tech-tag"><span>NVS STORAGE SUBSYSTEM</span><span>FLASH OK</span></div>
  <div class="confirm-title">&#x2713; PARAMETERS COMMITTED</div>
  <div class="confirm-desc">Configuration successfully committed to non-volatile storage.<br>Reloading instrument terminal...</div>
  <div class="diode-bar"><span class="diode"></span><span>RETURNING TO FIELD TERMINAL IN 1s</span></div>
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
    bool wifiChanged = false;

    // 1. Process Wi-Fi credentials (Multi-Profile Memory)
    for (uint8_t slot = 0; slot < MAX_WIFI_PROFILES; slot++) {
        String ssidKey = (slot == 0) ? "ssid" : ("ssid_" + String(slot + 1));
        String passKey = (slot == 0) ? "password" : ("password_" + String(slot + 1));
        String delKey  = "del_wifi_" + String(slot + 1);

        if (s_server.hasArg(delKey) && s_server.arg(delKey) == "1") {
            Network_DeleteProfile(slot);
            wifiChanged = true;
        } else if (s_server.hasArg(ssidKey)) {
            String newSsid = s_server.arg(ssidKey);
            String newPass = s_server.hasArg(passKey) ? s_server.arg(passKey) : "";
            newSsid.trim();
            newPass.trim();

            if (newSsid.length() > 0) {
                if (newSsid != s_wifiProfiles[slot].ssid || newPass.length() > 0) {
                    Network_SaveProfile(slot, newSsid, (newPass.length() > 0) ? newPass : s_wifiProfiles[slot].pass);
                    wifiChanged = true;
                }
            } else if (s_server.hasArg(passKey) && s_wifiProfiles[slot].ssid.length() > 0) {
                Network_DeleteProfile(slot);
                wifiChanged = true;
            }
        }
    }

    // 2. Process Carousel & Screen Navigation
    Network_SetAutoRotate(s_server.hasArg("auto_rotate"));
    if (s_server.hasArg("car_ord")) {
        Network_SetCarouselOrderString(s_server.arg("car_ord"));
    }
    uint16_t carMask = 0;
    for (int i = 0; i < 9; i++) {
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
        if (app >= 0 && app < 9) {
            Network_SetActiveApp((uint8_t)app);
        }
    }

    if (s_server.hasArg("ss_style")) {
        int st = s_server.arg("ss_style").toInt();
        if (st >= 0 && st < SCREENSAVER_STYLE_COUNT) {
            Screensaver_SetStyle((ScreensaverStyle)st);
        }
    }
    if (s_server.hasArg("ss_time")) {
        int to = s_server.arg("ss_time").toInt();
        if (to >= 0 && to <= 60) {
            Screensaver_SetTimeoutMin((uint8_t)to);
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
    if (s_server.hasArg("wd_wcol")) {
        Network_SetWeekdayWeekendColorIndex((uint8_t)s_server.arg("wd_wcol").toInt());
    }
    if (s_server.hasArg("ui_lang")) {
        Network_SetPortalLanguage(s_server.arg("ui_lang"));
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

    JsonArray wifiArr = doc["wifi_profiles"].to<JsonArray>();
    for (int i = 0; i < MAX_WIFI_PROFILES; i++) {
        JsonObject p = wifiArr.add<JsonObject>();
        p["slot"] = i + 1;
        p["ssid"] = s_wifiProfiles[i].ssid;
        p["has_pass"] = (s_wifiProfiles[i].pass.length() > 0);
        p["active"] = (s_netState == NET_STATE_CONNECTED && s_wifiProfiles[i].ssid.length() > 0 && s_wifiProfiles[i].ssid == WiFi.SSID());
    }
    doc["rotate_sec"] = s_rotateSec;
    doc["auto_rotate"] = s_autoRotate;
    doc["carousel_mask"] = s_carouselMask;
    doc["transition_fx"] = s_transitionFx;
    doc["active_app"] = s_activeApp;
    static const char* kScreenNamesList[] = {
        "Clock", "Indoor Environment", "Outdoor Weather",
        "Crypto Ticker", "YouTube Analytics", "Live Airspace", "Battery & Power",
        "Stopwatch & Timer", "Ambient Screensaver"
    };
    doc["app_name"] = kScreenNamesList[s_activeApp % 9];
    doc["firmware_version"] = FW_VERSION;

    doc["car_ord"] = Network_GetCarouselOrderString();
    doc["bat_mode"] = s_batteryMode;
    doc["clk_mode"] = s_clockMode;
    doc["show_sec"] = s_showSeconds;
    doc["wd_bar"] = s_weekdayBar;
    doc["wd_mon"] = s_weekdayStartMon;
    doc["wd_acol"] = s_weekdayActiveColIdx;
    doc["wd_icol"] = s_weekdayInactColIdx;
    doc["wd_wcol"] = s_weekdayWeekendColIdx;
    doc["date_fmt"] = s_dateFormat;
    doc["clk_icon"] = s_clockIcon;
    doc["txt_align"] = s_textAlign;
    doc["night_start"] = s_nightStart;
    doc["night_end"] = s_nightEnd;
    doc["weather_lat"] = s_wLat;
    doc["weather_lon"] = s_wLon;
    doc["yt_channel"] = s_ytChannel;
    doc["yt_has_key"] = (s_ytApiKey.length() > 0);
    doc["mqtt_en"] = s_mqttEn;
    doc["mqtt_srv"] = s_mqttServer;
    doc["mqtt_port"] = s_mqttPort;
    doc["mqtt_user"] = s_mqttUser;
    doc["mqtt_has_pass"] = (s_mqttPass.length() > 0);
    doc["mqtt_pre"] = s_mqttPrefix;
    doc["ui_lang"] = s_portalLang;
    doc["github_repo"] = GITHUB_REPO;

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

    // Timer & Stopwatch telemetry
    doc["timer_mode"] = (Timer_GetMode() == TIMER_MODE_STOPWATCH) ? "stopwatch" : "countdown";
    doc["timer_state"] = (Timer_GetState() == TIMER_STATE_RUNNING) ? "running" :
                         (Timer_GetState() == TIMER_STATE_PAUSED) ? "paused" :
                         (Timer_GetState() == TIMER_STATE_EXPIRED) ? "expired" : "stopped";
    doc["timer_duration"] = Timer_GetDuration();
    doc["timer_remaining"] = Timer_GetRemainingSeconds();
    doc["timer_elapsed_ms"] = Timer_GetElapsedMillis();

    // Retro Ambient Screensaver telemetry
    doc["screensaver_style"] = (int)Screensaver_GetStyle();
    doc["screensaver_timeout"] = Screensaver_GetTimeoutMin();
    doc["screensaver_active"] = Screensaver_IsActive();

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
    for (int i = 0; i < MAX_WIFI_PROFILES; i++) {
        s_wifiProfiles[i].ssid = s_prefs.getString(s_wifiSsidKeys[i], "");
        s_wifiProfiles[i].pass = s_prefs.getString(s_wifiPassKeys[i], "");
    }
    s_ssid = s_wifiProfiles[0].ssid;
    s_pass = s_wifiProfiles[0].pass;
    s_autoRotate = s_prefs.getBool(PREF_KEY_AUTO_ROTATE, true);
    s_activeApp = (uint8_t)s_prefs.getUChar(PREF_KEY_ACTIVE_APP, 0);
    if (s_activeApp >= 9) s_activeApp = 0;
    s_rotateSec = (uint16_t)s_prefs.getUInt(PREF_KEY_ROTATE_SEC, DEFAULT_ROTATE_SEC);
    if (s_rotateSec < MIN_ROTATE_SEC || s_rotateSec > MAX_ROTATE_SEC) {
        s_rotateSec = DEFAULT_ROTATE_SEC;
    }
    s_carouselMask = s_prefs.getUShort(PREF_KEY_CAROUSEL_MASK, 0x1FF);
    if (s_carouselMask == 0) s_carouselMask = 0x1FF;

    String savedOrder = s_prefs.getString(PREF_KEY_CAROUSEL_ORDER, "0,1,2,3,4,5,6,7,8");
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
    if (s_weekdayActiveColIdx >= s_numWdActiveColors) s_weekdayActiveColIdx = 0;
    s_weekdayInactColIdx = (uint8_t)s_prefs.getUChar(PREF_KEY_WD_INACT_COL, 0);
    if (s_weekdayInactColIdx >= s_numWdInactiveColors) s_weekdayInactColIdx = 0;
    s_weekdayWeekendColIdx = (uint8_t)s_prefs.getUChar(PREF_KEY_WD_WKND_COL, 0);
    if (s_weekdayWeekendColIdx >= s_numWdWeekendColors) s_weekdayWeekendColIdx = 0;
    s_portalLang = s_prefs.getString(PREF_KEY_PORTAL_LANG, "en");
    if (s_portalLang != "sk" && s_portalLang != "en") s_portalLang = "en";
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
        s_server.send_P(200, "text/html", PAGE_PORTAL);
    });
    s_server.on("/save", HTTP_POST, handleSave);
    s_server.on("/scan", HTTP_GET, handleScan);
    s_server.on("/status", HTTP_GET, handleStatus);
    s_server.on("/api/wifi", HTTP_GET, []() {
        JsonDocument doc;
        doc["connected"] = (WiFi.status() == WL_CONNECTED);
        doc["active_ssid"] = (WiFi.status() == WL_CONNECTED) ? WiFi.SSID() : "";
        doc["ip"] = Network_GetLocalIP();
        doc["rssi"] = Network_GetRSSI();
        JsonArray arr = doc["profiles"].to<JsonArray>();
        for (int i = 0; i < MAX_WIFI_PROFILES; i++) {
            JsonObject p = arr.add<JsonObject>();
            p["slot"] = i + 1;
            p["ssid"] = s_wifiProfiles[i].ssid;
            p["has_pass"] = (s_wifiProfiles[i].pass.length() > 0);
            p["connected"] = (WiFi.status() == WL_CONNECTED && s_wifiProfiles[i].ssid.length() > 0 && s_wifiProfiles[i].ssid == WiFi.SSID());
        }
        String out;
        serializeJson(doc, out);
        s_server.send(200, "application/json", out);
    });

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
            if (app >= 0 && app < 9) {
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
            "Crypto Ticker", "YouTube Analytics", "Live Airspace", "Battery & Power",
            "Stopwatch & Timer", "Ambient Screensaver"
        };
        doc["app_name"] = kNames[Network_GetActiveApp() % 9];
        String out;
        serializeJson(doc, out);
        s_server.send(200, "application/json", out);
    });

    // Stopwatch & Countdown Timer REST API
    auto handleTimerApi = []() {
        if (s_server.hasArg("mode")) {
            String m = s_server.arg("mode");
            m.toLowerCase();
            if (m == "stopwatch" || m == "0") {
                Timer_SetMode(TIMER_MODE_STOPWATCH);
            } else if (m == "countdown" || m == "timer" || m == "1") {
                Timer_SetMode(TIMER_MODE_COUNTDOWN);
            }
        }
        if (s_server.hasArg("duration")) {
            uint32_t dur = (uint32_t)s_server.arg("duration").toInt();
            if (dur > 0) {
                Timer_SetDuration(dur);
            }
        }
        if (s_server.hasArg("action")) {
            String act = s_server.arg("action");
            act.toLowerCase();
            if (act == "start") {
                Timer_Start();
            } else if (act == "pause") {
                Timer_Pause();
            } else if (act == "toggle") {
                Timer_Toggle();
            } else if (act == "reset") {
                Timer_Reset();
            }
        }
        JsonDocument doc;
        doc["mode"] = (Timer_GetMode() == TIMER_MODE_STOPWATCH) ? "stopwatch" : "countdown";
        doc["state"] = (Timer_GetState() == TIMER_STATE_RUNNING) ? "running" :
                       (Timer_GetState() == TIMER_STATE_PAUSED) ? "paused" :
                       (Timer_GetState() == TIMER_STATE_EXPIRED) ? "expired" : "stopped";
        doc["duration"] = Timer_GetDuration();
        doc["remaining"] = Timer_GetRemainingSeconds();
        doc["elapsed_ms"] = Timer_GetElapsedMillis();
        String out;
        serializeJson(doc, out);
        s_server.send(200, "application/json", out);
    };

    s_server.on("/api/timer", HTTP_GET, handleTimerApi);
    s_server.on("/api/timer", HTTP_POST, handleTimerApi);

    // Retro Ambient Screensaver REST API
    auto handleScreensaverApi = []() {
        if (s_server.hasArg("style")) {
            int st = s_server.arg("style").toInt();
            if (st >= 0 && st < SCREENSAVER_STYLE_COUNT) {
                Screensaver_SetStyle((ScreensaverStyle)st);
            }
        }
        if (s_server.hasArg("timeout")) {
            int to = s_server.arg("timeout").toInt();
            if (to >= 0 && to <= 60) {
                Screensaver_SetTimeoutMin((uint8_t)to);
            }
        }
        if (s_server.hasArg("action")) {
            String act = s_server.arg("action");
            act.toLowerCase();
            if (act == "start" || act == "activate") {
                Screensaver_Activate();
            } else if (act == "stop" || act == "deactivate" || act == "wake") {
                Screensaver_Deactivate();
            }
        }
        JsonDocument doc;
        doc["style"] = (int)Screensaver_GetStyle();
        doc["active_style"] = (int)Screensaver_GetActiveStyle();
        doc["timeout"] = Screensaver_GetTimeoutMin();
        doc["active"] = Screensaver_IsActive();
        doc["idle_ms"] = Screensaver_GetIdleMs();
        String out;
        serializeJson(doc, out);
        s_server.send(200, "application/json", out);
    };

    s_server.on("/api/screensaver", HTTP_GET, handleScreensaverApi);
    s_server.on("/api/screensaver", HTTP_POST, handleScreensaverApi);

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

    if (Network_HasSavedCredentials()) {
        Network_ConnectStation();
    } else {
        Network_StartAP();
    }
}

void Network_ConnectStation() {
    int savedCount = 0;
    int firstValid = -1;
    for (int i = 0; i < MAX_WIFI_PROFILES; i++) {
        if (s_wifiProfiles[i].ssid.length() > 0) {
            savedCount++;
            if (firstValid < 0) firstValid = i;
        }
    }

    if (savedCount == 0) {
        Serial.println("[WiFi] No saved credentials found. Starting AP...");
        Network_StartAP();
        return;
    }

    int targetSlot = firstValid;

    // If multiple profiles exist, perform an intelligent fast RF scan to find which network is in range
    if (savedCount > 1) {
        Serial.printf("[WiFi] %d saved profiles found. Scanning for available networks...\n", savedCount);
        WiFi.mode(WIFI_STA);
        WiFi.disconnect(true);
        delay(50);
        int16_t n = WiFi.scanNetworks(false, false, false, 250);
        int bestProfile = -1;
        int bestRssi = -1000;
        if (n > 0) {
            for (int i = 0; i < n; i++) {
                String foundSsid = WiFi.SSID(i);
                for (int p = 0; p < MAX_WIFI_PROFILES; p++) {
                    if (s_wifiProfiles[p].ssid.length() > 0 && s_wifiProfiles[p].ssid == foundSsid) {
                        int rssi = WiFi.RSSI(i);
                        Serial.printf("[WiFi] Scan matched saved profile %d '%s' (RSSI: %d dBm)\n",
                                      p + 1, foundSsid.c_str(), rssi);
                        if (rssi > bestRssi) {
                            bestRssi = rssi;
                            bestProfile = p;
                        }
                    }
                }
            }
            WiFi.scanDelete();
        }
        if (bestProfile >= 0) {
            targetSlot = bestProfile;
            Serial.printf("[WiFi] Selected best in-range profile %d: '%s' (%d dBm)\n",
                          targetSlot + 1, s_wifiProfiles[targetSlot].ssid.c_str(), bestRssi);
        } else {
            Serial.printf("[WiFi] None of the %d saved networks detected in scan. Trying profile %d ('%s')...\n",
                          savedCount, targetSlot + 1, s_wifiProfiles[targetSlot].ssid.c_str());
        }
    } else {
        Serial.printf("[WiFi] Single saved profile %d: '%s'\n", targetSlot + 1, s_wifiProfiles[targetSlot].ssid.c_str());
    }

    s_activeProfileIdx = targetSlot;
    s_ssid = s_wifiProfiles[targetSlot].ssid;
    s_pass = s_wifiProfiles[targetSlot].pass;
    s_netState = NET_STATE_CONNECTING;
    s_connectStartMs = millis();

    if (WiFi.getMode() != WIFI_STA) {
        WiFi.mode(WIFI_STA);
    }
    WiFi.disconnect(true);
    WiFi.begin(s_ssid.c_str(), s_pass.c_str());
    s_server.begin();
    Serial.printf("[WiFi] Connecting to profile %d ('%s')...\n", s_activeProfileIdx + 1, s_ssid.c_str());
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
            Serial.printf("[WiFi] Connected to '%s'! IP: %s, RSSI: %d dBm\n",
                          WiFi.SSID().c_str(), WiFi.localIP().toString().c_str(), WiFi.RSSI());
        } else if (now - s_connectStartMs >= WIFI_CONNECT_TIMEOUT_MS) {
            // Find next saved profile that hasn't been tried yet
            int nextSlot = -1;
            for (int p = s_activeProfileIdx + 1; p < MAX_WIFI_PROFILES; p++) {
                if (s_wifiProfiles[p].ssid.length() > 0) {
                    nextSlot = p;
                    break;
                }
            }
            if (nextSlot >= 0) {
                s_activeProfileIdx = nextSlot;
                s_ssid = s_wifiProfiles[nextSlot].ssid;
                s_pass = s_wifiProfiles[nextSlot].pass;
                s_connectStartMs = now;
                Serial.printf("[WiFi] Timeout on previous profile. Trying profile %d ('%s')...\n",
                              nextSlot + 1, s_ssid.c_str());
                WiFi.disconnect(true);
                WiFi.begin(s_ssid.c_str(), s_pass.c_str());
            } else {
                Serial.printf("[WiFi] Connection timeout on all saved profiles. Starting AP mode.\n");
                Network_StartAP();
            }
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
    for (int i = 0; i < MAX_WIFI_PROFILES; i++) {
        if (s_wifiProfiles[i].ssid.length() > 0) return true;
    }
    return false;
}

void Network_SaveProfile(uint8_t slot, const String& ssid, const String& pass) {
    if (slot >= MAX_WIFI_PROFILES) return;
    s_wifiProfiles[slot].ssid = ssid;
    s_wifiProfiles[slot].pass = pass;
    s_prefs.putString(s_wifiSsidKeys[slot], ssid);
    s_prefs.putString(s_wifiPassKeys[slot], pass);
    if (slot == 0) {
        s_ssid = ssid;
        s_pass = pass;
    }
    Serial.printf("[WiFi] Saved profile %u: '%s'\n", (unsigned)(slot + 1), ssid.c_str());
}

void Network_DeleteProfile(uint8_t slot) {
    if (slot >= MAX_WIFI_PROFILES) return;
    s_wifiProfiles[slot].ssid = "";
    s_wifiProfiles[slot].pass = "";
    s_prefs.remove(s_wifiSsidKeys[slot]);
    s_prefs.remove(s_wifiPassKeys[slot]);
    if (slot == 0) {
        s_ssid = "";
        s_pass = "";
    }
    Serial.printf("[WiFi] Deleted profile %u from NVS\n", (unsigned)(slot + 1));
}

String Network_GetProfileSSID(uint8_t slot) {
    if (slot >= MAX_WIFI_PROFILES) return "";
    return s_wifiProfiles[slot].ssid;
}

bool Network_HasProfilePass(uint8_t slot) {
    if (slot >= MAX_WIFI_PROFILES) return false;
    return (s_wifiProfiles[slot].pass.length() > 0);
}

void Network_SaveCredentials(const String& ssid, const String& pass) {
    Network_SaveProfile(0, ssid, pass);
}

void Network_ForgetCredentials() {
    for (uint8_t i = 0; i < MAX_WIFI_PROFILES; i++) {
        Network_DeleteProfile(i);
    }
    Serial.printf("[WiFi] Cleared all Wi-Fi credentials from NVS\n");
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
    if (app < 9) {
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
    s_weekdayActiveColIdx = (idx < s_numWdActiveColors) ? idx : 0;
    s_prefs.putUChar(PREF_KEY_WD_ACTIVE_COL, s_weekdayActiveColIdx);
}
uint16_t Network_GetWeekdayActiveColor() {
    return s_wdActiveColors[(s_weekdayActiveColIdx < s_numWdActiveColors) ? s_weekdayActiveColIdx : 0];
}

uint8_t Network_GetWeekdayInactiveColorIndex() { return s_weekdayInactColIdx; }
void Network_SetWeekdayInactiveColorIndex(uint8_t idx) {
    s_weekdayInactColIdx = (idx < s_numWdInactiveColors) ? idx : 0;
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
    return s_wdInactiveColors[(s_weekdayInactColIdx < s_numWdInactiveColors) ? s_weekdayInactColIdx : 0];
}

uint8_t Network_GetWeekdayWeekendColorIndex() { return s_weekdayWeekendColIdx; }
void Network_SetWeekdayWeekendColorIndex(uint8_t idx) {
    s_weekdayWeekendColIdx = (idx < s_numWdWeekendColors) ? idx : 0;
    s_prefs.putUChar(PREF_KEY_WD_WKND_COL, s_weekdayWeekendColIdx);
}
uint16_t Network_GetWeekdayWeekendColor() {
    if (s_weekdayWeekendColIdx == 0) {
        return 0; // 0 indicates "Same as Active Weekday" (no dedicated weekend accent)
    }
    return s_wdWeekendColors[(s_weekdayWeekendColIdx < s_numWdWeekendColors) ? s_weekdayWeekendColIdx : 0];
}
uint16_t Network_GetWeekdayInactiveWeekendColor() {
    return Network_GetWeekdayWeekendColor();
}

String Network_GetPortalLanguage() { return s_portalLang; }
void Network_SetPortalLanguage(const String& lang) {
    s_portalLang = (lang == "sk") ? "sk" : "en";
    s_prefs.putString(PREF_KEY_PORTAL_LANG, s_portalLang);
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

uint16_t Network_GetCarouselMask() { return s_carouselMask; }
void Network_SetCarouselMask(uint16_t mask) {
    s_carouselMask = (mask == 0) ? 0x01 : (mask & 0x1FF);
    s_prefs.putUShort(PREF_KEY_CAROUSEL_MASK, s_carouselMask);
}

bool Network_IsAppInCarousel(uint8_t appIndex) {
    if (appIndex >= 9) return false;
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
    for (int i = 0; i < 9; i++) {
        if (i > 0) res += ",";
        res += String(s_carouselOrder[i]);
    }
    return res;
}

void Network_SetCarouselOrderString(const String& str) {
    int idx = 0;
    int start = 0;
    bool seen[9] = {false};
    uint8_t newOrder[9];
    
    while (idx < 9 && start < (int)str.length()) {
        int comma = str.indexOf(',', start);
        String token = (comma == -1) ? str.substring(start) : str.substring(start, comma);
        token.trim();
        int val = token.toInt();
        if (val >= 0 && val < 9 && !seen[val]) {
            newOrder[idx++] = (uint8_t)val;
            seen[val] = true;
        }
        if (comma == -1) break;
        start = comma + 1;
    }
    for (int i = 0; i < 9; i++) {
        if (!seen[i] && idx < 9) {
            newOrder[idx++] = (uint8_t)i;
            seen[i] = true;
        }
    }
    for (int i = 0; i < 9; i++) {
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
