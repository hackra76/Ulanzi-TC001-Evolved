#include "MqttService.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "Config.h"
#include "NetworkManager.h"
#include "Sensors.h"
#include "DisplayMatrix.h"
#include "Buzzer.h"
#include "CustomAppService.h"

static WiFiClient s_wifiClient;
static PubSubClient s_mqttClient(s_wifiClient);
static Preferences s_prefs;

static bool s_enabled = false;
static String s_server = "";
static uint16_t s_port = DEFAULT_MQTT_PORT;
static String s_user = "";
static String s_pass = "";
static String s_prefix = DEFAULT_MQTT_PREFIX;

static String s_nodeId = "";
static uint32_t s_lastConnectAttempt = 0;
static uint32_t s_lastTelemetryMs = 0;
static bool s_discoveryPublished = false;

static const char* kScreenNames[] = {
    "Clock", "Indoor Environment", "Outdoor Weather",
    "Crypto Ticker", "YouTube Analytics", "Live Airspace", "Battery & Power",
    "Stopwatch & Timer", "Ambient Screensaver"
};

static void publishDiscoveryEntity(const char* component, const char* objectId, const char* name,
                                   const char* configJson) {
    String topic = "homeassistant/" + String(component) + "/" + s_nodeId + "/" + String(objectId) + "/config";
    s_mqttClient.publish(topic.c_str(), configJson, true);
}

static void publishAllDiscovery() {
    Serial.printf("[MQTT] Publishing Home Assistant Auto-Discovery for device %s...\n", s_nodeId.c_str());

    String devCommon = ",\"dev\":{\"ids\":[\"" + s_nodeId + "\"],\"name\":\"Ulanzi TC001\",\"mdl\":\"TC001 Evolved\",\"mf\":\"Ulanzi\",\"sw\":\"" + String(FW_VERSION) + "\",\"cu\":\"http://" + Network_GetLocalIP() + "\"}";

    // 1. Temperature Sensor
    {
        String payload = "{\"name\":\"Temperature\",\"stat_t\":\"" + s_prefix + "/sensor/temperature\",\"unit_of_meas\":\"°C\",\"dev_cla\":\"temperature\",\"stat_cla\":\"measurement\",\"uniq_id\":\"" + s_nodeId + "_temp\"" + devCommon + "}";
        publishDiscoveryEntity("sensor", "temperature", "Temperature", payload.c_str());
    }

    // 2. Humidity Sensor
    {
        String payload = "{\"name\":\"Humidity\",\"stat_t\":\"" + s_prefix + "/sensor/humidity\",\"unit_of_meas\":\"%\",\"dev_cla\":\"humidity\",\"stat_cla\":\"measurement\",\"uniq_id\":\"" + s_nodeId + "_hum\"" + devCommon + "}";
        publishDiscoveryEntity("sensor", "humidity", "Humidity", payload.c_str());
    }

    // 3. Battery Percent Sensor
    {
        String payload = "{\"name\":\"Battery\",\"stat_t\":\"" + s_prefix + "/sensor/battery_percent\",\"unit_of_meas\":\"%\",\"dev_cla\":\"battery\",\"stat_cla\":\"measurement\",\"uniq_id\":\"" + s_nodeId + "_bat_pct\"" + devCommon + "}";
        publishDiscoveryEntity("sensor", "battery_percent", "Battery", payload.c_str());
    }

    // 4. Battery Voltage Sensor
    {
        String payload = "{\"name\":\"Battery Voltage\",\"stat_t\":\"" + s_prefix + "/sensor/battery_voltage\",\"unit_of_meas\":\"V\",\"dev_cla\":\"voltage\",\"stat_cla\":\"measurement\",\"uniq_id\":\"" + s_nodeId + "_bat_v\"" + devCommon + "}";
        publishDiscoveryEntity("sensor", "battery_voltage", "Battery Voltage", payload.c_str());
    }

    // 5. Illuminance Sensor
    {
        String payload = "{\"name\":\"Ambient Light\",\"stat_t\":\"" + s_prefix + "/sensor/illuminance\",\"dev_cla\":\"illuminance\",\"uniq_id\":\"" + s_nodeId + "_ldr\"" + devCommon + "}";
        publishDiscoveryEntity("sensor", "illuminance", "Ambient Light", payload.c_str());
    }

    // 6. Active Screen Sensor
    {
        String payload = "{\"name\":\"Active Screen\",\"stat_t\":\"" + s_prefix + "/sensor/active_screen\",\"uniq_id\":\"" + s_nodeId + "_act_scr\"" + devCommon + "}";
        publishDiscoveryEntity("sensor", "active_screen", "Active Screen", payload.c_str());
    }

    // 7. Display Matrix Light / Brightness
    {
        String payload = "{\"name\":\"Display Matrix\",\"cmd_t\":\"" + s_prefix + "/light/set\",\"stat_t\":\"" + s_prefix + "/light/state\",\"bri_cmd_t\":\"" + s_prefix + "/light/brightness/set\",\"bri_stat_t\":\"" + s_prefix + "/light/brightness/state\",\"bri_scl\":255,\"uniq_id\":\"" + s_nodeId + "_light\"" + devCommon + "}";
        publishDiscoveryEntity("light", "matrix", "Display Matrix", payload.c_str());
    }

    // 8. Auto Brightness Switch
    {
        String payload = "{\"name\":\"Auto Brightness\",\"cmd_t\":\"" + s_prefix + "/switch/auto_brightness/set\",\"stat_t\":\"" + s_prefix + "/switch/auto_brightness/state\",\"uniq_id\":\"" + s_nodeId + "_auto_bri\"" + devCommon + "}";
        publishDiscoveryEntity("switch", "auto_brightness", "Auto Brightness", payload.c_str());
    }

    // 9. Carousel Auto-Rotate Switch
    {
        String payload = "{\"name\":\"Carousel Auto Rotate\",\"cmd_t\":\"" + s_prefix + "/switch/auto_rotate/set\",\"stat_t\":\"" + s_prefix + "/switch/auto_rotate/state\",\"uniq_id\":\"" + s_nodeId + "_auto_rot\"" + devCommon + "}";
        publishDiscoveryEntity("switch", "auto_rotate", "Carousel Auto Rotate", payload.c_str());
    }

    // 10. Buzzer Trigger Button
    {
        String payload = "{\"name\":\"Buzzer Chime\",\"cmd_t\":\"" + s_prefix + "/button/buzzer/set\",\"uniq_id\":\"" + s_nodeId + "_buzzer\"" + devCommon + "}";
        publishDiscoveryEntity("button", "buzzer", "Buzzer Chime", payload.c_str());
    }

    // 11. Reboot Button
    {
        String payload = "{\"name\":\"Restart Device\",\"cmd_t\":\"" + s_prefix + "/button/reboot/set\",\"uniq_id\":\"" + s_nodeId + "_reboot\"" + devCommon + "}";
        publishDiscoveryEntity("button", "reboot", "Restart Device", payload.c_str());
    }

    // 12. Screen Selector
    {
        String payload = "{\"name\":\"Select Screen\",\"cmd_t\":\"" + s_prefix + "/select/screen/set\",\"stat_t\":\"" + s_prefix + "/sensor/active_screen\",\"ops\":[\"Clock\",\"Indoor Environment\",\"Outdoor Weather\",\"Crypto Ticker\",\"YouTube Analytics\",\"Live Airspace\",\"Battery & Power\",\"Stopwatch & Timer\",\"Ambient Screensaver\"],\"uniq_id\":\"" + s_nodeId + "_sel_scr\"" + devCommon + "}";
        publishDiscoveryEntity("select", "screen", "Select Screen", payload.c_str());
    }

    // 13. Night Mode Switch
    {
        String payload = "{\"name\":\"Night Mode\",\"cmd_t\":\"" + s_prefix + "/switch/night_mode/set\",\"stat_t\":\"" + s_prefix + "/switch/night_mode/state\",\"uniq_id\":\"" + s_nodeId + "_night_mode\"" + devCommon + "}";
        publishDiscoveryEntity("switch", "night_mode", "Night Mode", payload.c_str());
    }

    s_discoveryPublished = true;
    Serial.println("[MQTT] Auto-Discovery payloads published successfully.");
}

static void onMqttMessage(char* topic, byte* payload, unsigned int length) {
    char msg[1024];
    if (length >= sizeof(msg)) length = sizeof(msg) - 1;
    memcpy(msg, payload, length);
    msg[length] = '\0';

    String topicStr = String(topic);
    String msgStr = String(msg);
    msgStr.trim();

    Serial.printf("[MQTT] Recv topic '%s': %s\n", topicStr.c_str(), msgStr.c_str());

    // 1. Custom App Push Topic: <prefix>/custom/<appName>
    String customPrefix = s_prefix + "/custom/";
    if (topicStr.startsWith(customPrefix)) {
        String appName = topicStr.substring(customPrefix.length());
        appName.trim();
        if (appName.length() > 0) {
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, msgStr);
            if (!err) {
                CustomApp_Push(appName, doc.as<JsonVariantConst>());
            } else {
                // Plain string fallback
                doc["text"] = msgStr;
                CustomApp_Push(appName, doc.as<JsonVariantConst>());
            }
            Serial.printf("[MQTT] Custom app '%s' updated\n", appName.c_str());
        }
        return;
    }

    // 2. Notification Push: <prefix>/notify
    if (topicStr == s_prefix + "/notify") {
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, msgStr);
        if (!err) {
            CustomApp_PushNotification(doc.as<JsonVariantConst>());
        } else {
            doc["text"] = msgStr;
            CustomApp_PushNotification(doc.as<JsonVariantConst>());
        }
        Serial.println("[MQTT] Notification triggered via MQTT");
        return;
    }

    // 3. Display Matrix Power
    if (topicStr == s_prefix + "/light/set") {
        if (msgStr.equalsIgnoreCase("OFF")) {
            DisplayMatrix_SetBrightness(0);
        } else {
            uint8_t dayBri = Network_GetDayBrightness();
            DisplayMatrix_SetBrightness(dayBri > 0 ? dayBri : 35);
        }
        MqttService_PublishState();
        return;
    }

    // 4. Matrix Brightness
    if (topicStr == s_prefix + "/light/brightness/set") {
        int bri = msgStr.toInt();
        bri = constrain(bri, 0, 255);
        DisplayMatrix_SetBrightness((uint8_t)bri);
        if (bri > 0) {
            Network_SetDayBrightness((uint8_t)bri);
        }
        MqttService_PublishState();
        return;
    }

    // 5. Auto Brightness
    if (topicStr == s_prefix + "/switch/auto_brightness/set") {
        bool en = msgStr.equalsIgnoreCase("ON") || msgStr == "1";
        Network_SetAutoBrightness(en);
        MqttService_PublishState();
        return;
    }

    // 6. Carousel Auto-Rotate
    if (topicStr == s_prefix + "/switch/auto_rotate/set") {
        bool en = msgStr.equalsIgnoreCase("ON") || msgStr == "1";
        Network_SetAutoRotate(en);
        MqttService_PublishState();
        return;
    }

    // 6b. Night Mode
    if (topicStr == s_prefix + "/switch/night_mode/set") {
        bool en = msgStr.equalsIgnoreCase("ON") || msgStr == "1";
        Network_SetNightMode(en);
        MqttService_PublishState();
        return;
    }

    // 7. Select Screen
    if (topicStr == s_prefix + "/select/screen/set") {
        for (int i = 0; i < 9; i++) {
            if (msgStr.equalsIgnoreCase(kScreenNames[i])) {
                Network_SetActiveApp(i);
                Network_SetAutoRotate(false);
                MqttService_PublishState();
                return;
            }
        }
        int appNum = msgStr.toInt();
        if (appNum >= 0 && appNum < 9) {
            Network_SetActiveApp(appNum);
            Network_SetAutoRotate(false);
            MqttService_PublishState();
        }
        return;
    }

    // 8. Buzzer Button
    if (topicStr == s_prefix + "/button/buzzer/set") {
        Buzzer_Alert();
        return;
    }

    // 9. Reboot Button
    if (topicStr == s_prefix + "/button/reboot/set") {
        Serial.println("[MQTT] Reboot requested via MQTT");
        Buzzer_Click();
        delay(200);
        ESP.restart();
        return;
    }
}

void MqttService_Init() {
    s_prefs.begin(PREF_NAMESPACE, false);
    s_enabled = s_prefs.getBool(PREF_KEY_MQTT_ENABLED, false);
    s_server = s_prefs.getString(PREF_KEY_MQTT_SERVER, "");
    s_port = s_prefs.getUShort(PREF_KEY_MQTT_PORT, DEFAULT_MQTT_PORT);
    s_user = s_prefs.getString(PREF_KEY_MQTT_USER, "");
    s_pass = s_prefs.getString(PREF_KEY_MQTT_PASS, "");
    s_prefix = s_prefs.getString(PREF_KEY_MQTT_PREFIX, DEFAULT_MQTT_PREFIX);

    String mac = WiFi.macAddress();
    mac.replace(":", "");
    s_nodeId = "tc001_" + mac.substring(6);
    s_nodeId.toLowerCase();

    s_mqttClient.setBufferSize(1536);
    s_mqttClient.setCallback(onMqttMessage);

    CustomApp_Init();
}

void MqttService_Update() {
    if (!s_enabled || !Network_IsConnected() || s_server.length() == 0) {
        if (s_mqttClient.connected()) {
            s_mqttClient.disconnect();
        }
        return;
    }

    uint32_t now = millis();

    if (!s_mqttClient.connected()) {
        if (now - s_lastConnectAttempt >= 5000) {
            s_lastConnectAttempt = now;
            s_mqttClient.setServer(s_server.c_str(), s_port);

            String clientId = "TC001-" + s_nodeId;
            String lwtTopic = s_prefix + "/status";

            Serial.printf("[MQTT] Connecting to %s:%u as '%s'...\n", s_server.c_str(), s_port, clientId.c_str());

            bool ok = false;
            if (s_user.length() > 0) {
                ok = s_mqttClient.connect(clientId.c_str(), s_user.c_str(), s_pass.c_str(),
                                         lwtTopic.c_str(), 0, true, "offline");
            } else {
                ok = s_mqttClient.connect(clientId.c_str(), lwtTopic.c_str(), 0, true, "offline");
            }

            if (ok) {
                Serial.println("[MQTT] Connected to broker!");
                s_mqttClient.publish(lwtTopic.c_str(), "online", true);

                // Publish Home Assistant discovery
                publishAllDiscovery();

                // Subscriptions
                s_mqttClient.subscribe((s_prefix + "/light/set").c_str());
                s_mqttClient.subscribe((s_prefix + "/light/brightness/set").c_str());
                s_mqttClient.subscribe((s_prefix + "/switch/auto_brightness/set").c_str());
                s_mqttClient.subscribe((s_prefix + "/switch/auto_rotate/set").c_str());
                s_mqttClient.subscribe((s_prefix + "/switch/night_mode/set").c_str());
                s_mqttClient.subscribe((s_prefix + "/select/screen/set").c_str());
                s_mqttClient.subscribe((s_prefix + "/button/buzzer/set").c_str());
                s_mqttClient.subscribe((s_prefix + "/button/reboot/set").c_str());
                s_mqttClient.subscribe((s_prefix + "/custom/+").c_str());
                s_mqttClient.subscribe((s_prefix + "/notify").c_str());

                // Initial state update
                MqttService_PublishState();
                MqttService_PublishTelemetry();
            } else {
                Serial.printf("[MQTT] Connection failed, rc=%d\n", s_mqttClient.state());
            }
        }
        return;
    }

    s_mqttClient.loop();

    // Periodic telemetry publish (every 10 seconds)
    if (now - s_lastTelemetryMs >= 10000) {
        s_lastTelemetryMs = now;
        MqttService_PublishTelemetry();
    }
}

bool MqttService_IsConnected() {
    return s_mqttClient.connected();
}

void MqttService_PublishState() {
    if (!s_mqttClient.connected()) return;

    // Light State & Brightness
    uint8_t curBri = DisplayMatrix_GetBrightness();
    s_mqttClient.publish((s_prefix + "/light/state").c_str(), curBri > 0 ? "ON" : "OFF", true);
    s_mqttClient.publish((s_prefix + "/light/brightness/state").c_str(), String(curBri).c_str(), true);

    // Switches
    s_mqttClient.publish((s_prefix + "/switch/auto_brightness/state").c_str(),
                         Network_GetAutoBrightness() ? "ON" : "OFF", true);
    s_mqttClient.publish((s_prefix + "/switch/auto_rotate/state").c_str(),
                         Network_GetAutoRotate() ? "ON" : "OFF", true);
    s_mqttClient.publish((s_prefix + "/switch/night_mode/state").c_str(),
                         Network_GetNightMode() ? "ON" : "OFF", true);

    // Active Screen
    uint8_t app = Network_GetActiveApp();
    s_mqttClient.publish((s_prefix + "/sensor/active_screen").c_str(), kScreenNames[app % 9], true);
}

void MqttService_PublishTelemetry() {
    if (!s_mqttClient.connected()) return;

    const SensorData& s = Sensors_GetData();

    // Temperature & Humidity
    if (s.shtAvailable) {
        s_mqttClient.publish((s_prefix + "/sensor/temperature").c_str(), String(s.temperature, 1).c_str(), true);
        s_mqttClient.publish((s_prefix + "/sensor/humidity").c_str(), String(s.humidity, 1).c_str(), true);
    }

    // Battery
    s_mqttClient.publish((s_prefix + "/sensor/battery_percent").c_str(), String(s.batteryPercent).c_str(), true);
    s_mqttClient.publish((s_prefix + "/sensor/battery_voltage").c_str(), String(s.batteryVolts, 2).c_str(), true);

    // Light
    s_mqttClient.publish((s_prefix + "/sensor/illuminance").c_str(), String(s.ldrRaw).c_str(), true);

    MqttService_PublishState();
}

// Getters & Setters
bool MqttService_IsEnabled() { return s_enabled; }
void MqttService_SetEnabled(bool en) {
    s_enabled = en;
    s_prefs.putBool(PREF_KEY_MQTT_ENABLED, s_enabled);
}

String MqttService_GetServer() { return s_server; }
void MqttService_SetServer(const String& srv) {
    s_server = srv;
    s_prefs.putString(PREF_KEY_MQTT_SERVER, s_server);
}

uint16_t MqttService_GetPort() { return s_port; }
void MqttService_SetPort(uint16_t port) {
    s_port = port;
    s_prefs.putUShort(PREF_KEY_MQTT_PORT, s_port);
}

String MqttService_GetUser() { return s_user; }
void MqttService_SetUser(const String& user) {
    s_user = user;
    s_prefs.putString(PREF_KEY_MQTT_USER, s_user);
}

String MqttService_GetPass() { return s_pass; }
void MqttService_SetPass(const String& pass) {
    s_pass = pass;
    s_prefs.putString(PREF_KEY_MQTT_PASS, s_pass);
}

String MqttService_GetPrefix() { return s_prefix; }
void MqttService_SetPrefix(const String& prefix) {
    s_prefix = prefix;
    s_prefs.putString(PREF_KEY_MQTT_PREFIX, s_prefix);
}
