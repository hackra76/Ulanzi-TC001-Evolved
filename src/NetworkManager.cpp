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
    0x04DF, // 7: Deep Sky Blue (#0099FF)
    0xF9B3, // 8: Hot Coral (#FF3399)
    0x37F3, // 9: Lime Mint (#33FF99)
    0xFDB0  // 10: Gold Amber (#FFB700)
};
static const size_t s_numWdActiveColors = sizeof(s_wdActiveColors) / sizeof(s_wdActiveColors[0]);

static const uint16_t s_wdInactiveColors[] = {
    0x0000, // 0: Auto (computed dynamically from active color at ~65% intensity)
    0x7432, // 1: Slate Gray (#708090)
    0x9342, // 2: Dim Amber (#906810)
    0x3186, // 3: Dark Charcoal (#333333)
    0x19F1, // 4: Dim Navy Blue (#1E3A8A)
    0x02B2, // 5: Dim Teal (#005577)
    0x7082, // 6: Dim Wine Red (#771111)
    0x12A2, // 7: Dim Forest Green (#115511)
    0x40AA, // 8: Dim Purple (#441155)
    0x0000  // 9: Off (Black)
};
static const size_t s_numWdInactiveColors = sizeof(s_wdInactiveColors) / sizeof(s_wdInactiveColors[0]);

static const uint16_t s_wdWeekendColors[] = {
    0x0000, // 0: Same as Active Day Color
    0xF800, // 1: Red (#FF2200 - Classic Weekend)
    0xFD20, // 2: Orange (#FF8800)
    0xFFE0, // 3: Yellow (#FFFF00)
    0xF9B3, // 4: Hot Coral (#FF3399)
    0xF81F, // 5: Magenta (#FF00FF)
    0x04DF, // 6: Deep Sky Blue (#0099FF)
    0x07FF, // 7: Cyan (#00F0FF)
    0x37F3, // 8: Lime Mint (#33FF99)
    0xFDB0, // 9: Gold Amber (#FFB700)
    0xFFFF  // 10: White (#FFFFFF)
};
static const size_t s_numWdWeekendColors = sizeof(s_wdWeekendColors) / sizeof(s_wdWeekendColors[0]);

static uint32_t s_connectStartMs = 0;
static uint32_t s_lastWifiCheckMs = 0;
static bool s_pendingReconnect = false;
static uint32_t s_reconnectTriggerMs = 0;

static String buildPortalHtml() {
    String html = F(R"rawliteral(
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
      <a href="https://github.com/{{GITHUB_REPO}}" target="_blank" rel="noopener" class="fw-badge" title="GitHub Repository">
        <span class="badge-dot"></span> v{{FW_VERSION}}
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
        <span class="sub-val" id="teleBriLevel">{{DAY_BRI}}% CEIL</span>
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
        <span class="sub-val" id="teleScreenName">{{APP_NAME}}</span>
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
        <span id="screenNameBadge" class="ind-name">{{APP_NAME}}</span>
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
    <input type="hidden" id="ui_lang" name="ui_lang" value="{{PORTAL_LANG}}">

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
              <option value="0" {{CLK_MODE_0_SEL}} data-i18n="opt_clk_time">Time Only</option>
              <option value="1" {{CLK_MODE_1_SEL}} data-i18n="opt_clk_both">Combined Time &amp; Date (Alternating)</option>
              <option value="2" {{CLK_MODE_2_SEL}} data-i18n="opt_clk_date">Date Only</option>
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
              <option value="0" {{DATE_FMT_0_SEL}}>DD.MM. (e.g. 14.10.)</option>
              <option value="1" {{DATE_FMT_1_SEL}}>DD MMM (e.g. 14 OCT)</option>
              <option value="2" {{DATE_FMT_2_SEL}}>MM/DD (e.g. 10/14)</option>
              <option value="3" {{DATE_FMT_3_SEL}}>DAY DD (e.g. WED 14)</option>
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
              <option value="0" {{CLK_ICON_0_SEL}} data-i18n="opt_icon_cal">Dynamic Calendar (Date 1-31)</option>
              <option value="1" {{CLK_ICON_1_SEL}} data-i18n="opt_icon_heart">Heart (Classic)</option>
              <option value="2" {{CLK_ICON_2_SEL}} data-i18n="opt_icon_clock">Clock Face</option>
              <option value="3" {{CLK_ICON_3_SEL}} data-i18n="opt_icon_none">None (Full Width)</option>
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
              <option value="0" {{TXT_ALIGN_0_SEL}} data-i18n="opt_align_left">Left-Aligned with Icon (Awtrix)</option>
              <option value="1" {{TXT_ALIGN_1_SEL}} data-i18n="opt_align_center">Centered in Window</option>
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
              <input type="checkbox" class="visually-hidden" name="show_sec" value="1" {{SHOW_SEC_CHK}}>
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
              <input type="checkbox" class="visually-hidden" name="wd_bar" value="1" {{WD_BAR_CHK}}>
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
              <option value="1" {{WD_MON_1_SEL}} data-i18n="opt_mon_iso">Monday (ISO 8601)</option>
              <option value="0" {{WD_MON_0_SEL}} data-i18n="opt_sun_us">Sunday (US Standard)</option>
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
              <option value="0" {{WD_ACOL_0_SEL}}>Cyan (#00F0FF)</option>
              <option value="1" {{WD_ACOL_1_SEL}}>Orange (#FF8800)</option>
              <option value="2" {{WD_ACOL_2_SEL}}>Green (#00FF00)</option>
              <option value="3" {{WD_ACOL_3_SEL}}>Yellow (#FFFF00)</option>
              <option value="4" {{WD_ACOL_4_SEL}}>Red (#FF2200)</option>
              <option value="5" {{WD_ACOL_5_SEL}}>White (#FFFFFF)</option>
              <option value="6" {{WD_ACOL_6_SEL}}>Magenta (#FF00FF)</option>
              <option value="7" {{WD_ACOL_7_SEL}}>Electric Blue (#0066FF)</option>
              <option value="8" {{WD_ACOL_8_SEL}}>Lime Green (#76FF03)</option>
              <option value="9" {{WD_ACOL_9_SEL}}>Amber Gold (#FFC107)</option>
              <option value="10" {{WD_ACOL_10_SEL}}>Soft Pink (#FF4081)</option>
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
              <option value="0" {{WD_ICOL_0_SEL}} data-i18n="opt_inact_auto">Dim Active Color (Auto - Slightly Dimmer)</option>
              <option value="1" {{WD_ICOL_1_SEL}}>Slate Gray (#708090)</option>
              <option value="2" {{WD_ICOL_2_SEL}}>Dim Amber (#906810)</option>
              <option value="3" {{WD_ICOL_3_SEL}}>Dark Blue (#001A40)</option>
              <option value="4" {{WD_ICOL_4_SEL}}>Deep Wine (#3A000A)</option>
              <option value="5" {{WD_ICOL_5_SEL}}>Forest Green (#002A0D)</option>
              <option value="6" {{WD_ICOL_6_SEL}}>Charcoal (#252525)</option>
              <option value="7" {{WD_ICOL_7_SEL}}>Muted Purple (#2A1035)</option>
              <option value="8" {{WD_ICOL_8_SEL}}>Dark Cyan (#003038)</option>
              <option value="9" {{WD_ICOL_9_SEL}} data-i18n="opt_inact_off">Off (Black)</option>
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
              <option value="0" {{WD_WCOL_0_SEL}} data-i18n="opt_wknd_active">Same as Active Weekday</option>
              <option value="1" {{WD_WCOL_1_SEL}}>Bright Red (#FF2200)</option>
              <option value="2" {{WD_WCOL_2_SEL}}>Coral Orange (#FF6B4A)</option>
              <option value="3" {{WD_WCOL_3_SEL}}>Gold (#FFD700)</option>
              <option value="4" {{WD_WCOL_4_SEL}}>Hot Pink (#FF1493)</option>
              <option value="5" {{WD_WCOL_5_SEL}}>Purple (#9C27B0)</option>
              <option value="6" {{WD_WCOL_6_SEL}}>Cyan (#00F0FF)</option>
              <option value="7" {{WD_WCOL_7_SEL}}>Lime (#76FF03)</option>
              <option value="8" {{WD_WCOL_8_SEL}}>Pure White (#FFFFFF)</option>
              <option value="9" {{WD_WCOL_9_SEL}}>Sky Blue (#00BFFF)</option>
              <option value="10" {{WD_WCOL_10_SEL}}>Emerald (#00E676)</option>
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
              <input type="checkbox" class="visually-hidden" name="auto_bri" value="1" {{AUTO_BRI_CHK}}>
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
              <input type="range" id="day_bri" name="day_bri" min="5" max="100" value="{{DAY_BRI}}" oninput="document.getElementById('day_bri_val').innerText = this.value + '%'">
              <span id="day_bri_val" style="font-weight:700; color:var(--signal-orange); min-width:44px; text-align:right;">{{DAY_BRI}}%</span>
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
              <input type="checkbox" class="visually-hidden" name="night_mode" value="1" {{NIGHT_MODE_CHK}}>
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
              <input type="range" id="night_bri" name="night_bri" min="1" max="25" value="{{NIGHT_BRI}}" oninput="document.getElementById('night_bri_val').innerText = this.value + '%'">
              <span id="night_bri_val" style="font-weight:700; color:var(--signal-orange); min-width:44px; text-align:right;">{{NIGHT_BRI}}%</span>
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
              <input type="number" id="night_start" name="night_start" min="0" max="23" value="{{NIGHT_START}}" style="max-width:80px;">
              <span style="color:var(--ink-muted);">&rarr;</span>
              <input type="number" id="night_end" name="night_end" min="0" max="23" value="{{NIGHT_END}}" style="max-width:80px;">
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
              <input type="checkbox" class="visually-hidden" id="auto_rotate" name="auto_rotate" value="1" {{AUTO_ROTATE_CHK}} onchange="document.getElementById('rotate_delay_field').style.display = this.checked ? 'table-row' : 'none'">
              <span class="actuator-box"></span>
              <span data-i18n="chk_auto_rotate">Auto-Rotate Screens (Carousel)</span>
            </label>
          </td>
        </tr>
        <tr id="rotate_delay_field" style="{{ROTATE_DELAY_DISPLAY}}">
          <td class="label-cell">
            <div class="param-name" data-i18n="lbl_rotate_interval">Cycle Interval</div>
            <div class="param-desc">Dwell time per screen (3 - 120 seconds)</div>
          </td>
          <td>
            <input type="number" id="rotate" name="rotate" min="3" max="120" value="{{ROTATE}}" required style="max-width:120px;">
          </td>
        </tr>
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="lbl_trans_fx">Screen Transition Effect</div>
            <div class="param-desc">Direct matrix raw frame animation</div>
          </td>
          <td>
            <select id="trans_fx" name="trans_fx">
              <option value="0" {{TRANS_0_SEL}} data-i18n="opt_trans_0">None (Instant Cut)</option>
              <option value="1" {{TRANS_1_SEL}} data-i18n="opt_trans_1">Slide Left</option>
              <option value="2" {{TRANS_2_SEL}} data-i18n="opt_trans_2">Slide Right</option>
              <option value="3" {{TRANS_3_SEL}} data-i18n="opt_trans_3">Slide Up</option>
              <option value="4" {{TRANS_4_SEL}} data-i18n="opt_trans_4">Slide Down</option>
              <option value="5" {{TRANS_5_SEL}} data-i18n="opt_trans_5">Crossfade / Dissolve</option>
              <option value="6" {{TRANS_6_SEL}} data-i18n="opt_trans_6">Curtain (Center Split Wipe)</option>
              <option value="7" {{TRANS_7_SEL}} data-i18n="opt_trans_7">Random (Shuffle Effects)</option>
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
              <option value="0" {{APP_0_SEL}} data-i18n="app_name_0">Clock (Heart / Time)</option>
              <option value="1" {{APP_1_SEL}} data-i18n="app_name_1">Indoor Environment (SHT30 Temp)</option>
              <option value="2" {{APP_2_SEL}} data-i18n="app_name_2">Outdoor Weather (Open-Meteo)</option>
              <option value="3" {{APP_3_SEL}} data-i18n="app_name_3">Crypto &amp; Financial Markets</option>
              <option value="4" {{APP_4_SEL}} data-i18n="app_name_4">YouTube Subscriber Count</option>
              <option value="5" {{APP_5_SEL}} data-i18n="app_name_5">Live Airspace Radar (ADS-B)</option>
              <option value="6" {{APP_6_SEL}} data-i18n="app_name_6">Battery &amp; Energy Telemetry</option>
            </select>
          </td>
        </tr>
      </table>

      <div class="section-legend" data-i18n="lbl_carousel_screens">Active Rotation Sequence (Check to Enable &bull; Use &#x25B2;&#x25BC; to Reorder)</div>
      <input type="hidden" id="car_ord" name="car_ord" value="{{CAR_ORD}}">
      <div class="reorder-list" id="carouselScreens">
        {{REORDER_ITEMS}}
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
              <option value="2" {{BAT_MODE_2_SEL}} data-i18n="opt_bat_alt">Alternating (Percentage &bull; Voltage every 3s)</option>
              <option value="0" {{BAT_MODE_0_SEL}} data-i18n="opt_bat_pct">Percentage Only (e.g. 94%)</option>
              <option value="1" {{BAT_MODE_1_SEL}} data-i18n="opt_bat_volt">Voltage Only (e.g. 4.12V)</option>
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
            <input type="text" id="w_city" name="w_city" value="{{W_CITY}}" placeholder="e.g. Bratislava">
          </td>
        </tr>
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="lbl_lat">Geographic Coordinates</div>
            <div class="param-desc">Decimal latitude and longitude</div>
          </td>
          <td>
            <div style="display:flex; gap:10px;">
              <input type="text" id="w_lat" name="w_lat" value="{{W_LAT}}" placeholder="Lat: 48.1486">
              <input type="text" id="w_lon" name="w_lon" value="{{W_LON}}" placeholder="Lon: 17.1077">
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
                <option value="BTC" {{C_BTC_SEL}}>Bitcoin (BTC / USDT)</option>
                <option value="ETH" {{C_ETH_SEL}}>Ethereum (ETH / USDT)</option>
                <option value="SOL" {{C_SOL_SEL}}>Solana (SOL / USDT)</option>
                <option value="BNB" {{C_BNB_SEL}}>Binance Coin (BNB / USDT)</option>
              </optgroup>
              <optgroup label="Stocks &amp; Indices">
                <option value="AAPL" {{C_AAPL_SEL}}>Apple Inc. (AAPL - Nasdaq)</option>
                <option value="TSLA" {{C_TSLA_SEL}}>Tesla Inc. (TSLA - Nasdaq)</option>
                <option value="NVDA" {{C_NVDA_SEL}}>Nvidia Corp. (NVDA - Nasdaq)</option>
                <option value="SPX" {{C_SPX_SEL}}>S&amp;P 500 Index (^GSPC)</option>
              </optgroup>
              <optgroup label="Commodities &amp; Forex">
                <option value="GOLD" {{C_GOLD_SEL}}>Gold Bullion (GC=F / USD)</option>
                <option value="EUR" {{C_EUR_SEL}}>EUR / USD (Euro Forex)</option>
                <option value="USD" {{C_USD_SEL}}>USD / EUR (US Dollar Forex)</option>
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
            <input type="text" id="c_sym_custom" name="c_sym_custom" value="{{C_SYM_CUSTOM}}" placeholder="e.g. AAPL, GOLD, EUR, MSFT, BTC">
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
            <input type="text" id="yt_channel" name="yt_channel" value="{{YT_CHANNEL}}" placeholder="e.g. @MKBHD or UCxxxxxxxx">
          </td>
        </tr>
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="lbl_yt_key">YouTube Data API v3 Key</div>
            <div class="param-desc">Google Cloud API authentication key</div>
          </td>
          <td>
            <input type="password" id="yt_key" name="yt_key" value="{{YT_KEY}}" placeholder="Google Cloud API key">
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
              <input type="checkbox" class="visually-hidden" name="mqtt_en" id="mqtt_en" {{MQTT_EN_CHK}} onchange="document.getElementById('mqtt_settings').style.display = this.checked ? 'block' : 'none';">
              <span class="actuator-box"></span>
              <span data-i18n="chk_mqtt_en">Enable MQTT Client &amp; Home Assistant Auto-Discovery</span>
            </label>
          </td>
        </tr>
      </table>

      <div id="mqtt_settings" style="{{MQTT_DISPLAY}}">
        <table class="param-table">
          <tr>
            <td class="label-cell">
              <div class="param-name" data-i18n="lbl_mqtt_srv">Broker IP or Hostname</div>
              <div class="param-desc">MQTT broker address</div>
            </td>
            <td>
              <div style="display:flex; gap:10px;">
                <input type="text" id="mqtt_srv" name="mqtt_srv" value="{{MQTT_SRV}}" placeholder="192.168.0.100 or homeassistant.local" style="flex:2;">
                <input type="number" id="mqtt_port" name="mqtt_port" value="{{MQTT_PORT}}" placeholder="1883" style="flex:1; max-width:90px;">
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
                <input type="text" id="mqtt_user" name="mqtt_user" value="{{MQTT_USER}}" placeholder="Username">
                <input type="password" id="mqtt_pass" name="mqtt_pass" value="{{MQTT_PASS}}" placeholder="Password">
              </div>
            </td>
          </tr>
          <tr>
            <td class="label-cell">
              <div class="param-name" data-i18n="lbl_mqtt_pre">Topic Prefix</div>
              <div class="param-desc">Base topic for sensors and notifications</div>
            </td>
            <td>
              <input type="text" id="mqtt_pre" name="mqtt_pre" value="{{MQTT_PRE}}" placeholder="ulanzi-tc001">
            </td>
          </tr>
        </table>
        <div style="font-size:10px; color:var(--ink-secondary); line-height:1.5; background:var(--chassis-inset); padding:10px 12px; border:1px solid var(--hairline-main); margin-bottom:14px;" data-i18n="box_custom_push">
          &#x1F4A1; <strong>Push Spotify &amp; Custom Screens:</strong><br>
          MQTT: Publish JSON to <code>{{MQTT_PRE}}/custom/spotify</code> or <code>{{MQTT_PRE}}/notify</code><br>
          HTTP REST: <code>POST /api/custom?name=spotify</code> or <code>POST /api/notify</code><br>
          Payload: <code>{"text":"Song - Artist","icon":"spotify","progress":45,"duration":8}</code>
        </div>
      </div>
    </div>

    <!-- PANEL 04: NETWORK & SYSTEM -->
    <div class="control-panel-section" id="panel-network">
      <div class="section-legend" data-i18n="grp_network">Wi-Fi RF Link Configuration</div>
      <table class="param-table">
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="lbl_ssid">Network Name (SSID)</div>
            <div class="param-desc">2.4 GHz 802.11b/g/n wireless access point</div>
          </td>
          <td>
            <input type="text" id="ssid" name="ssid" value="{{SSID}}" placeholder="Enter Wi-Fi SSID" required autocomplete="off">
            <div style="display:flex; gap:8px; margin-top:6px;">
              <button type="button" class="tactile-btn btn-sm" onclick="scanNetworks()" data-i18n="btn_scan">&#x21BB; Scan Nearby Networks</button>
            </div>
            <select id="scanSelect" style="display:none; margin-top:6px;" onchange="selectNetwork(this.value)">
              <option value="" data-i18n="opt_scan_detect">-- Detected Networks --</option>
            </select>
          </td>
        </tr>
        <tr>
          <td class="label-cell">
            <div class="param-name" data-i18n="lbl_pass">Wi-Fi Password</div>
            <div class="param-desc">WPA2/WPA3 Pre-Shared Key</div>
          </td>
          <td>
            <input type="password" id="password" name="password" placeholder="Leave empty if unchanged" data-i18n-ph="ph_pass">
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
      <span data-i18n="lbl_running_ver">Running Version:</span> <strong style="color:var(--signal-orange);">v{{FW_VERSION}}</strong>
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
    <div>ULANZI TC001 EVOLVED // TACTILE FIELD INSTRUMENT</div>
    <div>FIRMWARE v{{FW_VERSION}} &bull; NVS ARMED</div>
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
    box_custom_push: "\U0001F4A1 <strong>Push Spotify &amp; Custom Screens:</strong><br>MQTT: Publish JSON to <code>{{MQTT_PRE}}/custom/spotify</code> or <code>{{MQTT_PRE}}/notify</code><br>HTTP REST: <code>POST /api/custom?name=spotify</code> or <code>POST /api/notify</code><br>Payload: <code>{\"text\":\"Song - Artist\",\"icon\":\"spotify\",\"progress\":45,\"duration\":8}</code>",
    grp_network: "Wi-Fi RF Link Configuration",
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
    box_custom_push: "\U0001F4A1 <strong>Odosielanie Spotify a vlastn\u00FDch obrazoviek:</strong><br>MQTT: Publikujte JSON na <code>{{MQTT_PRE}}/custom/spotify</code> alebo <code>{{MQTT_PRE}}/notify</code><br>HTTP REST: <code>POST /api/custom?name=spotify</code> alebo <code>POST /api/notify</code><br>Form\u00E1t: <code>{\"text\":\"Song - Artist\",\"icon\":\"spotify\",\"progress\":45,\"duration\":8}</code>",
    grp_network: "Wi-Fi RF Spojenie",
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
    })
    .catch(() => {
      sel.innerHTML = '<option value="">' + (currentLang === 'sk' ? 'Vyh\u013Ead\u00E1vanie zlyhalo' : 'Scan failed') + '</option>';
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

let currentAppIndex = {{ACTIVE_APP}};
const appNames = ["Clock", "Indoor Temp", "Outdoor Weather", "Crypto", "YouTube", "Airspace", "Battery"];

function switchScreen(dir) {
  currentAppIndex = (currentAppIndex + dir + 7) % 7;
  fetch('/api/screen?set=' + currentAppIndex)
    .then(r => r.json())
    .then(d => {
      const name = d.app_name || appNames[currentAppIndex];
      document.getElementById('screenNameBadge').innerText = name;
      document.getElementById('teleScreenName').innerText = name;
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

function updateTelemetry() {
  fetch('/status')
    .then(r => r.json())
    .then(d => {
      if (d.battery_volts != null) {
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
        "&#x23F0; CLOCK", "&#x1F321; INDOOR TEMP", "&#x26C5; OUTDOOR METEO", "&#x1F4B0; CRYPTO &amp; MARKETS", "&#x25B6; YOUTUBE", "&#x2708; AIRSPACE RADAR", "&#x26A1; BATTERY &amp; ENERGY"
    };
    for (int i = 0; i < 7; i++) {
        uint8_t appIdx = s_carouselOrder[i];
        if (appIdx >= 7) appIdx = i;
        bool isChecked = (s_carouselMask & (1 << appIdx)) != 0;

        reorderHtml += "<div class=\"reorder-item\" data-app=\"" + String(appIdx) + "\">";
        reorderHtml += "<label class=\"actuator-label\" style=\"margin:0;\">";
        reorderHtml += "<input type=\"checkbox\" class=\"visually-hidden\" name=\"car_app_" + String(appIdx) + "\" value=\"1\" " + (isChecked ? "checked " : "") + "onchange=\"updateOrderInput()\">";
        reorderHtml += "<span class=\"actuator-box\"></span>";
        reorderHtml += "<span class=\"reorder-name\">" + String(kAppIconsNames[appIdx]) + "</span>";
        reorderHtml += "</label>";
        reorderHtml += "<div class=\"reorder-step-btns\">";
        reorderHtml += "<button type=\"button\" class=\"tactile-btn btn-xs\" onclick=\"moveScreen(this, -1)\">&#x25B2;</button>";
        reorderHtml += "<button type=\"button\" class=\"tactile-btn btn-xs\" onclick=\"moveScreen(this, 1)\">&#x25BC;</button>";
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
    for (size_t i = 0; i < s_numWdActiveColors; i++) {
        char tag[24];
        snprintf(tag, sizeof(tag), "{{WD_ACOL_%d_SEL}}", (int)i);
        html.replace(tag, (s_weekdayActiveColIdx == i) ? "selected" : "");
    }
    for (size_t i = 0; i < s_numWdInactiveColors; i++) {
        char tag[24];
        snprintf(tag, sizeof(tag), "{{WD_ICOL_%d_SEL}}", (int)i);
        html.replace(tag, (s_weekdayInactColIdx == i) ? "selected" : "");
    }
    for (size_t i = 0; i < s_numWdWeekendColors; i++) {
        char tag[24];
        snprintf(tag, sizeof(tag), "{{WD_WCOL_%d_SEL}}", (int)i);
        html.replace(tag, (s_weekdayWeekendColIdx == i) ? "selected" : "");
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
    html.replace("{{GITHUB_REPO}}", GITHUB_REPO);
    html.replace("{{PORTAL_LANG}}", s_portalLang);
    return html;
}

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
        return Network_GetWeekdayActiveColor();
    }
    return s_wdWeekendColors[(s_weekdayWeekendColIdx < s_numWdWeekendColors) ? s_weekdayWeekendColIdx : 0];
}
uint16_t Network_GetWeekdayInactiveWeekendColor() {
    if (s_weekdayInactColIdx == 0) {
        uint16_t wc = Network_GetWeekdayWeekendColor();
        uint8_t r = (wc >> 11) & 0x1F;
        uint8_t g = (wc >> 5) & 0x3F;
        uint8_t b = wc & 0x1F;
        r = (uint8_t)(((uint16_t)r * 170) >> 8);
        g = (uint8_t)(((uint16_t)g * 170) >> 8);
        b = (uint8_t)(((uint16_t)b * 170) >> 8);
        return (r << 11) | (g << 5) | b;
    }
    return Network_GetWeekdayInactiveColor();
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
