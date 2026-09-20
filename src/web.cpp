#include <Arduino.h>
#include "web.h"
#include "config.h"
#include "cut.h"
#include "log.h"
#include "rpm.h"
#include "shift.h"
#include "lock.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <math.h>
#include <cstring>
#include "ota_manager.h"
#include "fs_updater.h"

static AsyncWebServer server(80);
static AsyncEventSource events("/log/stream");
static unsigned long lastTelemetryMs = 0;
static const size_t WEB_CONFIG_JSON_CAPACITY = 8192;
static const size_t MAX_CAL_SAMPLES = 64;

struct UploadStatus
{
  bool ok = true;
  String message;
};

static void send_json_status(AsyncWebServerRequest *r, bool ok, const String &msg)
{
  DynamicJsonDocument doc(128);
  doc["ok"] = ok;
  if (msg.length())
  {
    doc["message"] = msg;
  }
  String out;
  serializeJson(doc, out);
  r->send(ok ? 200 : 500, "application/json", out);
}

static void writeAutoMapJson(const QSConfig &c, JsonArray arr);
static void writeConfigJson(const QSConfig &c, JsonObject obj);

static bool validateApPassword(const String &pass)
{
  size_t len = pass.length();
  if (len == 0)
    return true;
  if (len < 8 || len > 63)
    return false;
  for (size_t i = 0; i < len; ++i)
  {
    char c = pass[i];
    if (c < 32 || c > 126)
      return false;
  }
  return true;
}

static int clampManualMinRpm(int value)
{
  if (value < 0)
    return 0;
  if (value > 10000)
    return 10000;
  return value;
}

static int clampMinRpm(int value)
{
  if (value < 0)
    return 0;
  if (value > 18000)
    return 18000;
  return value;
}

static bool applyApPassword(QSConfig &cfg, const String &pass)
{
  char buffer[sizeof(cfg.apPassword)];
  pass.toCharArray(buffer, sizeof(buffer));
  buffer[sizeof(buffer) - 1] = '\0';
  if (strncmp(cfg.apPassword, buffer, sizeof(cfg.apPassword)) == 0)
  {
    return false;
  }
  strncpy(cfg.apPassword, buffer, sizeof(cfg.apPassword));
  cfg.apPassword[sizeof(cfg.apPassword) - 1] = '\0';
  return true;
}

static void restartAccessPoint();

// Pin constants per spec
static const uint8_t RPM_PIN = 2;
static const uint8_t RELAY_IGN = 6;
static const uint8_t RELAY_INJ = 7;
static const uint8_t SHIFT_SW = 4;
static const uint8_t TEST_PWM = 8;
static const uint8_t TEST_PWM_CH = 5;
static const uint8_t TEST_PWM_RES = 8;
static const uint8_t TEST_PWM_DUTY = 128;

static void stop_test_pwm_output()
{
  ledcDetachPin(TEST_PWM);
  pinMode(TEST_PWM, OUTPUT);
  digitalWrite(TEST_PWM, LOW);
}

void on_new_log(const LogEntry &e, const JsonDocument *meta, const QSConfig *cfgSnapshot)
{
  DynamicJsonDocument doc(4096);
  doc["t"] = e.t;
  doc["rpm"] = e.rpm;
  doc["cut"] = e.cut;
  doc["shift"] = e.shift;
  doc["event"] = e.event;
  if (meta)
  {
    doc["changes"] = meta->as<JsonVariantConst>();
  }
  if (cfgSnapshot)
  {
    JsonObject cfgObj = doc.createNestedObject("config");
    writeConfigJson(*cfgSnapshot, cfgObj);
  }
  String out;
  serializeJson(doc, out);
  events.send(out.c_str(), "log", millis());
}

void web_send_limiter_sync(bool enabled)
{
  DynamicJsonDocument doc(128);
  doc["type"] = "limiter_sync";
  doc["limiter"] = enabled;
  doc["limiter_enable"] = enabled;
  String out;
  serializeJson(doc, out);
  events.send(out.c_str(), "telemetry", millis());
}

static void send_live_telemetry()
{
  DynamicJsonDocument doc(256);
  doc["t"] = millis();
  doc["rpm"] = rpm_get();
  doc["rpmRaw"] = rpm_get_raw();
  doc["cut"] = cut_is_active();
  doc["shift"] = shift_is_pressed() ? "PRESSED" : "RELEASED";
  doc["lock"] = lock_is_engaged() ? "LOCKED" : (lock_is_enabled() ? "ARMED" : "OFF");
  doc["limiter"] = limiter_enable;
  doc["allowDoubleClick"] = allow_double_click;
  String out;
  serializeJson(doc, out);
  events.send(out.c_str(), "telemetry", millis());
}

static void send_lock_status(AsyncWebServerRequest *r)
{
  LockStatus st;
  lock_get_status(st);
  DynamicJsonDocument doc(512);
  doc["enabled"] = st.enabled;
  doc["unlocked"] = st.unlocked;
  doc["engaged"] = st.engaged;
  doc["hasPass"] = (strlen(st.pass) == 4);
  doc["retryHold"] = st.retryHold;
  doc["retryMs"] = st.retryMs;
  doc["spamHold"] = st.spamHold;
  doc["spamMs"] = st.spamMs;
  doc["shortMinMs"] = st.shortMinMs;
  doc["shortMaxMs"] = st.shortMaxMs;
  doc["longMinMs"] = st.longMinMs;
  doc["resetHoldMs"] = st.resetHoldMs;
  String out;
  serializeJson(doc, out);
  r->send(200, "application/json", out);
}

static bool apActive = false;
static unsigned long apStartTimeMs = 0;
static unsigned long apLastActiveMs = 0;
static bool pendingApRestart = false;
static unsigned long apRestartMs = 0;

bool startAP();

void web_wake_ap()
{
  if (!apActive)
  {
    Serial.println("[Wi-Fi] Waking up SoftAP from standby...");
    startAP();
  }
  else
  {
    apStartTimeMs = millis();
    apLastActiveMs = millis();
  }
}

bool web_is_ap_active()
{
  return apActive;
}

bool startAP()
{
  const QSConfig &cfg = getConfig();
  String ssid = "QS_tu_lam_dien_tu_";
  uint32_t mac = (uint32_t)((ESP.getEfuseMac() >> 24) & 0xFFFFFF);
  ssid += String(mac, HEX);
  WiFi.mode(WIFI_AP);
  WiFi.setSleep(false);

  IPAddress local(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  if (!WiFi.softAPConfig(local, gateway, subnet))
  {
    Serial.println("SoftAP config failed, using default IP");
  }
  WiFi.softAPsetHostname("qs-esp32c3");

  const char *pass = cfg.apPassword;
  bool hasPass = pass && strlen(pass) >= 8;
  bool ok = hasPass ? WiFi.softAP(ssid.c_str(), pass, 1, 0, 4) : WiFi.softAP(ssid.c_str());
  if (ok)
  {
    Serial.print("SoftAP started: ");
    Serial.println(ssid);
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
    Serial.println(hasPass ? "SoftAP secured with password" : "SoftAP open (no password)");
    apActive = true;
    apStartTimeMs = millis();
    apLastActiveMs = millis();
  }
  else
  {
    Serial.println("SoftAP start failed");
    apActive = false;
  }
  return ok;
}

static void restartAccessPoint()
{
  WiFi.softAPdisconnect(true);
  delay(100);
  startAP();
}

static void writeAutoMapJson(const QSConfig &c, JsonArray arr)
{
  for (uint8_t i = 0; i < c.autoMapCount; ++i)
  {
    JsonObject o = arr.createNestedObject();
    o["min"] = c.autoMap[i].rpmMin;
    o["max"] = c.autoMap[i].rpmMax;
    o["cut"] = c.autoMap[i].cutMs;
  }
}

static void writeConfigJson(const QSConfig &c, JsonObject obj)
{
  obj["shiftActiveLow"] = c.shiftActiveLow;
  obj["shiftDebounceMs"] = c.shiftDebounceMs;
  obj["relayPolarityHighToCut"] = c.relayPolarityHighToCut;
  obj["ppr"] = c.ppr;
  obj["rpmScaleFactor"] = c.rpmScaleFactor;
  obj["rpmOffset"] = c.rpmOffset;
  obj["modeAuto"] = c.modeAuto;
  obj["cutTimeFixed"] = c.cutTimeFixed;
  
  // Auto RPM Limiter JSON
  obj["allow_double_click"] = c.allow_double_click;
  obj["limiter_enable"] = c.limiter_enable;
  obj["target_rpm"] = c.target_rpm;
  obj["rpm_hysteresis"] = c.rpm_hysteresis;
  obj["hard_cut_ms"] = c.hard_cut_ms;
  obj["recovery_spark_ms"] = c.recovery_spark_ms;

  obj["apTimeoutMs"] = c.apTimeoutMs;
  obj["apPassword"] = c.apPassword;
  obj["minRpm"] = c.minRpm;
  obj["cutIgnEnabled"] = c.cutIgnEnabled;
  obj["cutInjEnabled"] = c.cutInjEnabled;
  obj["manualBackfireEnabled"] = c.manualBackfireEnabled;
  obj["manualBackfireHoldMs"] = c.manualBackfireHoldMs;
  obj["manualBackfireMode"] = c.manualBackfireMode;
  obj["manualPulseCutMs"] = c.manualPulseCutMs;
  obj["manualPulseFireMs"] = c.manualPulseFireMs;
  obj["manualPulseOffMs"] = c.manualPulseOffMs;
  obj["manualPulseRepeat"] = c.manualPulseRepeat;
  obj["manualMinRpm"] = c.manualMinRpm;
  obj["manualIgnOnly"] = c.manualIgnOnly;
  obj["autoMapCount"] = c.autoMapCount;
  JsonArray arr = obj.createNestedArray("autoMap");
  writeAutoMapJson(c, arr);
}

static bool autoMapEquals(const QSConfig &a, const QSConfig &b)
{
  if (a.autoMapCount != b.autoMapCount)
    return false;
  for (uint8_t i = 0; i < a.autoMapCount; ++i)
  {
    const AutoMapEntry &ea = a.autoMap[i];
    const AutoMapEntry &eb = b.autoMap[i];
    if (ea.rpmMin != eb.rpmMin || ea.rpmMax != eb.rpmMax || ea.cutMs != eb.cutMs)
    {
      return false;
    }
  }
  return true;
}

static void collectConfigChanges(const QSConfig &before, const QSConfig &after, JsonObject out)
{
  bool changed = false;
#define EMIT_CHANGE(field)     \
  do                           \
  {                            \
    out[#field] = after.field; \
    changed = true;            \
  } while (0)

  if (before.shiftActiveLow != after.shiftActiveLow)
    EMIT_CHANGE(shiftActiveLow);
  if (before.shiftDebounceMs != after.shiftDebounceMs)
    EMIT_CHANGE(shiftDebounceMs);
  if (before.relayPolarityHighToCut != after.relayPolarityHighToCut)
    EMIT_CHANGE(relayPolarityHighToCut);
  if (before.ppr != after.ppr)
    EMIT_CHANGE(ppr);
  if (before.rpmScaleFactor != after.rpmScaleFactor)
    EMIT_CHANGE(rpmScaleFactor);
  if (before.rpmOffset != after.rpmOffset)
    EMIT_CHANGE(rpmOffset);
  if (before.modeAuto != after.modeAuto)
    EMIT_CHANGE(modeAuto);
  if (before.cutTimeFixed != after.cutTimeFixed)
    EMIT_CHANGE(cutTimeFixed);
  if (before.allow_double_click != after.allow_double_click)
    EMIT_CHANGE(allow_double_click);
  if (before.limiter_enable != after.limiter_enable)
    EMIT_CHANGE(limiter_enable);
  if (before.target_rpm != after.target_rpm)
    EMIT_CHANGE(target_rpm);
  if (before.rpm_hysteresis != after.rpm_hysteresis)
    EMIT_CHANGE(rpm_hysteresis);
  if (before.hard_cut_ms != after.hard_cut_ms)
    EMIT_CHANGE(hard_cut_ms);
  if (before.recovery_spark_ms != after.recovery_spark_ms)
    EMIT_CHANGE(recovery_spark_ms);
  if (before.apTimeoutMs != after.apTimeoutMs)
    EMIT_CHANGE(apTimeoutMs);
  if (before.minRpm != after.minRpm)
    EMIT_CHANGE(minRpm);
  if (before.cutIgnEnabled != after.cutIgnEnabled)
    EMIT_CHANGE(cutIgnEnabled);
  if (before.cutInjEnabled != after.cutInjEnabled)
    EMIT_CHANGE(cutInjEnabled);
  if (before.manualBackfireEnabled != after.manualBackfireEnabled)
    EMIT_CHANGE(manualBackfireEnabled);
  if (before.manualBackfireHoldMs != after.manualBackfireHoldMs)
    EMIT_CHANGE(manualBackfireHoldMs);
  if (before.manualBackfireMode != after.manualBackfireMode)
    EMIT_CHANGE(manualBackfireMode);
  if (before.manualPulseCutMs != after.manualPulseCutMs)
    EMIT_CHANGE(manualPulseCutMs);
  if (before.manualPulseFireMs != after.manualPulseFireMs)
    EMIT_CHANGE(manualPulseFireMs);
  if (before.manualPulseOffMs != after.manualPulseOffMs)
    EMIT_CHANGE(manualPulseOffMs);
  if (before.manualPulseRepeat != after.manualPulseRepeat)
    EMIT_CHANGE(manualPulseRepeat);
  if (before.manualMinRpm != after.manualMinRpm)
    EMIT_CHANGE(manualMinRpm);
  if (before.manualIgnOnly != after.manualIgnOnly)
    EMIT_CHANGE(manualIgnOnly);
  if (strncmp(before.apPassword, after.apPassword, sizeof(before.apPassword)) != 0)
  {
    out["apPasswordChanged"] = true;
    changed = true;
  }
#undef EMIT_CHANGE
  if (!autoMapEquals(before, after))
  {
    JsonArray arr = out.createNestedArray("autoMap");
    writeAutoMapJson(after, arr);
    changed = true;
  }

  out["source"] = "webui";
  if (!changed)
  {
    out["note"] = "no-change";
  }
}


static void updateAutoMapFromPayload(JsonVariant variant)
{
  if (!variant.is<JsonArray>())
    return;
  JsonArray arr = variant.as<JsonArray>();
  QSConfig &cfg = const_cast<QSConfig &>(getConfig());
  cfg.autoMapCount = 0;
  for (JsonVariant item : arr)
  {
    if (cfg.autoMapCount >= MAX_AUTO_MAP)
      break;
    AutoMapEntry &entry = cfg.autoMap[cfg.autoMapCount++];
    if (item.is<JsonObject>())
    {
      JsonObject o = item.as<JsonObject>();
      entry.rpmMin = o["min"] | (cfg.autoMapCount - 1) * 1000;
      entry.rpmMax = o["max"] | (entry.rpmMin + 999);
      entry.cutMs = o["cut"] | 50;
    }
    else
    {
      entry.rpmMin = (cfg.autoMapCount - 1) * 1000;
      entry.rpmMax = entry.rpmMin + 999;
      entry.cutMs = item.as<int>();
    }
  }
  config_normalize_auto_map();
}

struct CalSampleStats
{
  bool ok = false;
  float avg = 0.0f;
  float stdDev = 0.0f;
  float minVal = 0.0f;
  float maxVal = 0.0f;
  uint16_t rawCount = 0;
  uint16_t usedCount = 0;
};

static bool collectCalSamples(JsonArray arr, float *buffer, size_t capacity, size_t &countOut)
{
  countOut = 0;
  if (arr.isNull() || !buffer || capacity == 0)
  {
    return false;
  }

  for (JsonVariant v : arr)
  {
    if (countOut >= capacity)
      break;
    float sample = v.as<float>();
    if (!isfinite(sample))
      continue;
    if (sample <= 0.0f || sample > 50000.0f)
      continue;
    buffer[countOut++] = sample;
  }
  return countOut > 0;
}

static void sortSamplesAscending(float *values, size_t count)
{
  if (!values || count < 2)
    return;
  for (size_t i = 1; i < count; ++i)
  {
    float key = values[i];
    size_t j = i;
    while (j > 0 && values[j - 1] > key)
    {
      values[j] = values[j - 1];
      --j;
    }
    values[j] = key;
  }
}

static CalSampleStats computeCalSampleStats(JsonArray arr)
{
  CalSampleStats stats;
  float samples[MAX_CAL_SAMPLES] = {0.0f};
  size_t count = 0;
  if (!collectCalSamples(arr, samples, MAX_CAL_SAMPLES, count))
  {
    return stats;
  }

  stats.rawCount = static_cast<uint16_t>(count);
  sortSamplesAscending(samples, count);

  // Trim 10% at each tail to reduce spike/noise effect, but keep enough points.
  size_t trim = count / 10;
  if (trim > 2)
    trim = 2;
  size_t start = trim;
  size_t end = count - trim;
  if (end <= start || (end - start) < 3)
  {
    start = 0;
    end = count;
  }

  const size_t used = end - start;
  if (used == 0)
  {
    return stats;
  }

  stats.minVal = samples[start];
  stats.maxVal = samples[end - 1];
  stats.usedCount = static_cast<uint16_t>(used);

  float sum = 0.0f;
  for (size_t i = start; i < end; ++i)
  {
    sum += samples[i];
  }
  stats.avg = sum / used;

  float variance = 0.0f;
  for (size_t i = start; i < end; ++i)
  {
    float diff = samples[i] - stats.avg;
    variance += diff * diff;
  }
  variance /= used;
  stats.stdDev = sqrtf(variance);
  stats.ok = true;
  return stats;
}

void web_init()
{
  LittleFS.begin();
  if (!startAP())
  {
    Serial.println("Retrying SoftAP...");
    delay(500);
    startAP();
  }
  server.addHandler(&events);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *r)
            { r->send(LittleFS, "/index.html", "text/html"); });

  server.on("/config/get", HTTP_GET, [](AsyncWebServerRequest *r)
            {
    DynamicJsonDocument doc(WEB_CONFIG_JSON_CAPACITY);
    const QSConfig &c = getConfig();
    JsonObject root = doc.to<JsonObject>();
    writeConfigJson(c, root);
    String out;
    serializeJson(doc, out);
    r->send(200, "application/json", out); });

  server.on("/lock", HTTP_GET, [](AsyncWebServerRequest *r)
            { send_lock_status(r); });

  server.on("/lock", HTTP_POST, [](AsyncWebServerRequest *r) {}, NULL, [](AsyncWebServerRequest *r, uint8_t *data, size_t len, size_t index, size_t total)
            {
    DynamicJsonDocument doc(512);
    DeserializationError err = deserializeJson(doc, (char*)data, len);
    if (err) { r->send(400, "text/plain", "invalid json"); return; }
    bool enabled = doc.containsKey("enabled") ? doc["enabled"].as<bool>() : lock_is_enabled();
    String passStr = doc["pass"] | "";
    const char *passPtr = passStr.length() ? passStr.c_str() : nullptr;
    int shortOverride = doc.containsKey("shortMaxMs") ? doc["shortMaxMs"].as<int>() : -1;
    int longOverride = doc.containsKey("longMinMs") ? doc["longMinMs"].as<int>() : -1;
    int resetOverride = doc.containsKey("resetHoldMs") ? doc["resetHoldMs"].as<int>() : -1;
    if (!lock_update(enabled, passPtr, shortOverride, longOverride, resetOverride)) {
      r->send(400, "text/plain", "invalid pass");
      return;
    }
    send_lock_status(r); });

  server.on("/config/set", HTTP_POST, [](AsyncWebServerRequest *r)
            {
              // handled in body
            },
            NULL, [](AsyncWebServerRequest *r, uint8_t *data, size_t len, size_t index, size_t total)
            {
    String *payload;
    if (index == 0) {
      payload = new String();
      payload->reserve(total);
      r->_tempObject = payload;
    } else {
      payload = reinterpret_cast<String*>(r->_tempObject);
    }
    if (!payload) {
      r->send(500, "text/plain", "alloc fail");
      return;
    }
    payload->concat(String(reinterpret_cast<char*>(data), len));
    if (index + len < total) {
      return; // wait for remaining chunks
    }

    QSConfig before = getConfig();
    DynamicJsonDocument doc(WEB_CONFIG_JSON_CAPACITY);
    DeserializationError err = deserializeJson(doc, *payload);
    delete payload;
    r->_tempObject = nullptr;
    if (err) { r->send(400, "text/plain", "invalid json"); return; }
    QSConfig &c = const_cast<QSConfig&>(getConfig());
    bool apPassChanged = false;
    c.shiftActiveLow = doc["shiftActiveLow"] | c.shiftActiveLow;
    c.shiftDebounceMs = doc["shiftDebounceMs"] | c.shiftDebounceMs;
    c.relayPolarityHighToCut = doc["relayPolarityHighToCut"] | c.relayPolarityHighToCut;
    c.ppr = doc["ppr"] | c.ppr;
    c.rpmScaleFactor = doc["rpmScaleFactor"] | c.rpmScaleFactor;
    c.rpmOffset = doc["rpmOffset"] | c.rpmOffset;
    c.modeAuto = doc["modeAuto"] | c.modeAuto;
    c.cutTimeFixed = doc["cutTimeFixed"] | c.cutTimeFixed;
    
    // Auto RPM Limiter fields
    if (doc.containsKey("allow_double_click")) {
      c.allow_double_click = doc["allow_double_click"].as<bool>();
      allow_double_click = c.allow_double_click;
    }
    if (doc.containsKey("limiter_enable")) {
      c.limiter_enable = doc["limiter_enable"].as<bool>();
      limiter_enable = c.limiter_enable;
    }
    if (doc.containsKey("target_rpm")) {
      c.target_rpm = doc["target_rpm"].as<int>();
      if (c.target_rpm < 1000) c.target_rpm = 1000;
      if (c.target_rpm > 20000) c.target_rpm = 20000;
      target_rpm = c.target_rpm;
    }
    if (doc.containsKey("rpm_hysteresis")) {
      c.rpm_hysteresis = doc["rpm_hysteresis"].as<int>();
      if (c.rpm_hysteresis < 10) c.rpm_hysteresis = 10;
      if (c.rpm_hysteresis > 2000) c.rpm_hysteresis = 2000;
      rpm_hysteresis = c.rpm_hysteresis;
    }
    if (doc.containsKey("hard_cut_ms")) {
      c.hard_cut_ms = doc["hard_cut_ms"].as<unsigned long>();
      if (c.hard_cut_ms < 5) c.hard_cut_ms = 5;
      if (c.hard_cut_ms > 500) c.hard_cut_ms = 500;
      hard_cut_ms = c.hard_cut_ms;
    }
    if (doc.containsKey("recovery_spark_ms")) {
      c.recovery_spark_ms = doc["recovery_spark_ms"].as<unsigned long>();
      if (c.recovery_spark_ms < 5) c.recovery_spark_ms = 5;
      if (c.recovery_spark_ms > 500) c.recovery_spark_ms = 500;
      recovery_spark_ms = c.recovery_spark_ms;
    }

    if (doc.containsKey("apTimeoutMs")) {
      c.apTimeoutMs = doc["apTimeoutMs"].as<int>();
      if (c.apTimeoutMs < 0) c.apTimeoutMs = 0;
      if (c.apTimeoutMs > 3600000) c.apTimeoutMs = 3600000;
      apStartTimeMs = millis();
      apLastActiveMs = millis();
    }
    if (doc.containsKey("minRpm")) {
      c.minRpm = clampMinRpm(doc["minRpm"].as<int>());
    }
    c.cutIgnEnabled = doc["cutIgnEnabled"] | c.cutIgnEnabled;
    c.cutInjEnabled = doc["cutInjEnabled"] | c.cutInjEnabled;
    c.manualBackfireEnabled = doc["manualBackfireEnabled"] | c.manualBackfireEnabled;
    c.manualBackfireHoldMs = doc["manualBackfireHoldMs"] | c.manualBackfireHoldMs;
    c.manualBackfireMode = doc["manualBackfireMode"] | c.manualBackfireMode;
    c.manualPulseCutMs = doc["manualPulseCutMs"] | c.manualPulseCutMs;
    c.manualPulseFireMs = doc["manualPulseFireMs"] | c.manualPulseFireMs;
    c.manualPulseOffMs = doc["manualPulseOffMs"] | c.manualPulseOffMs;
    c.manualPulseRepeat = doc["manualPulseRepeat"] | c.manualPulseRepeat;
    if (doc.containsKey("manualMinRpm")) {
      c.manualMinRpm = clampManualMinRpm(doc["manualMinRpm"].as<int>());
    }
    c.manualIgnOnly = doc["manualIgnOnly"] | c.manualIgnOnly;
    if (doc.containsKey("apPassword")) {
      String pass = doc["apPassword"].as<String>();
      pass.trim();
      if (!validateApPassword(pass)) {
        r->send(400, "text/plain", "invalid ap password");
        return;
      }
      apPassChanged = applyApPassword(c, pass);
    }
    if (doc.containsKey("autoMap")) {
      updateAutoMapFromPayload(doc["autoMap"]);
    }
    shift_configure(c.shiftActiveLow, c.shiftDebounceMs);
    rpm_set_ppr(c.ppr);
    rpm_set_scale(c.rpmScaleFactor);
    rpm_set_offset(c.rpmOffset);
    if (!saveConfig()) {
      r->send(500, "text/plain", "save fail");
      return;
    }
    DynamicJsonDocument changesDoc(1536);
    JsonObject changeObj = changesDoc.to<JsonObject>();
    collectConfigChanges(before, c, changeObj);
    log_push(0, false, "SYS", "CFG_SAVE", &changesDoc, &c);
    r->send(200, "application/json", "{\"ok\":true}");
    if (apPassChanged) {
      pendingApRestart = true;
      apRestartMs = millis() + 800; // Cho 800ms để gói phản hồi HTTP gửi trọn vẹn đến điện thoại trước khi khởi động lại SoftAP
    } });

  server.on("/log/ui", HTTP_POST, [](AsyncWebServerRequest *r)
            {
              // body handled below
            },
            NULL, [](AsyncWebServerRequest *r, uint8_t *data, size_t len, size_t index, size_t total)
            {
    DynamicJsonDocument doc(512);
    DeserializationError err = deserializeJson(doc, (char*)data, len);
    if (err) { r->send(400, "text/plain", "invalid json"); return; }
    const char *label = doc["event"] | "UI_EVENT";
    if (!label || !*label) label = "UI_EVENT";
    DynamicJsonDocument metaDoc(512);
    JsonObject metaObj = metaDoc.to<JsonObject>();
    metaObj["source"] = doc["source"] | "ui";
    if (doc.containsKey("meta")) {
      metaObj["meta"] = doc["meta"].as<JsonVariantConst>();
    }
    if (doc.containsKey("note")) {
      metaObj["note"] = doc["note"].as<const char*>();
    }
    bool hasMeta = metaObj.size() > 0;
    log_push(rpm_get(), false, "UI", label, hasMeta ? &metaDoc : nullptr, nullptr);
    r->send(200, "application/json", "{\"ok\":true}"); });

  server.on("/export", HTTP_GET, [](AsyncWebServerRequest *r)
            {
    DynamicJsonDocument doc(WEB_CONFIG_JSON_CAPACITY);
    const QSConfig &c = getConfig();
    JsonObject root = doc.to<JsonObject>();
    writeConfigJson(c, root);
    String out;
    serializeJson(doc, out);
    r->send(200, "application/json", out); });

  server.on("/import", HTTP_POST, [](AsyncWebServerRequest *r)
            {
              // response handled after body received
            },
            NULL, [](AsyncWebServerRequest *r, uint8_t *data, size_t len, size_t index, size_t total)
            {
    String *payload;
    if (index == 0) {
      payload = new String();
      payload->reserve(total);
      r->_tempObject = payload;
    } else {
      payload = reinterpret_cast<String*>(r->_tempObject);
    }
    if (!payload) {
      r->send(500, "text/plain", "alloc fail");
      return;
    }
    payload->concat(String(reinterpret_cast<char*>(data), len));
    if (index + len < total) {
      return; // wait for remaining chunks
    }

    DynamicJsonDocument doc(WEB_CONFIG_JSON_CAPACITY);
    DeserializationError err = deserializeJson(doc, *payload);
    delete payload;
    r->_tempObject = nullptr;
    if (err) {
      r->send(400, "text/plain", "invalid json");
      return;
    }

    QSConfig before = getConfig();
    QSConfig &c = const_cast<QSConfig&>(getConfig());
    bool apPassChanged = false;
    c.shiftActiveLow = doc["shiftActiveLow"] | c.shiftActiveLow;
    c.shiftDebounceMs = doc["shiftDebounceMs"] | c.shiftDebounceMs;
    c.relayPolarityHighToCut = doc["relayPolarityHighToCut"] | c.relayPolarityHighToCut;
    c.ppr = doc["ppr"] | c.ppr;
    c.rpmScaleFactor = doc["rpmScaleFactor"] | c.rpmScaleFactor;
    c.rpmOffset = doc["rpmOffset"] | c.rpmOffset;
    c.modeAuto = doc["modeAuto"] | c.modeAuto;
    c.cutTimeFixed = doc["cutTimeFixed"] | c.cutTimeFixed;
    
    // Auto RPM Limiter fields
    if (doc.containsKey("allow_double_click")) {
      c.allow_double_click = doc["allow_double_click"].as<bool>();
      allow_double_click = c.allow_double_click;
    }
    if (doc.containsKey("limiter_enable")) {
      c.limiter_enable = doc["limiter_enable"].as<bool>();
      limiter_enable = c.limiter_enable;
    }
    if (doc.containsKey("target_rpm")) {
      c.target_rpm = doc["target_rpm"].as<int>();
      if (c.target_rpm < 1000) c.target_rpm = 1000;
      if (c.target_rpm > 20000) c.target_rpm = 20000;
      target_rpm = c.target_rpm;
    }
    if (doc.containsKey("rpm_hysteresis")) {
      c.rpm_hysteresis = doc["rpm_hysteresis"].as<int>();
      if (c.rpm_hysteresis < 10) c.rpm_hysteresis = 10;
      if (c.rpm_hysteresis > 2000) c.rpm_hysteresis = 2000;
      rpm_hysteresis = c.rpm_hysteresis;
    }
    if (doc.containsKey("hard_cut_ms")) {
      c.hard_cut_ms = doc["hard_cut_ms"].as<unsigned long>();
      if (c.hard_cut_ms < 5) c.hard_cut_ms = 5;
      if (c.hard_cut_ms > 500) c.hard_cut_ms = 500;
      hard_cut_ms = c.hard_cut_ms;
    }
    if (doc.containsKey("recovery_spark_ms")) {
      c.recovery_spark_ms = doc["recovery_spark_ms"].as<unsigned long>();
      if (c.recovery_spark_ms < 5) c.recovery_spark_ms = 5;
      if (c.recovery_spark_ms > 500) c.recovery_spark_ms = 500;
      recovery_spark_ms = c.recovery_spark_ms;
    }

    if (doc.containsKey("apTimeoutMs")) {
      c.apTimeoutMs = doc["apTimeoutMs"].as<int>();
      if (c.apTimeoutMs < 0) c.apTimeoutMs = 0;
      if (c.apTimeoutMs > 3600000) c.apTimeoutMs = 3600000;
      apStartTimeMs = millis();
      apLastActiveMs = millis();
    }
    if (doc.containsKey("minRpm")) {
      c.minRpm = clampMinRpm(doc["minRpm"].as<int>());
    }
    c.cutIgnEnabled = doc["cutIgnEnabled"] | c.cutIgnEnabled;
    c.cutInjEnabled = doc["cutInjEnabled"] | c.cutInjEnabled;
    c.manualBackfireEnabled = doc["manualBackfireEnabled"] | c.manualBackfireEnabled;
    c.manualBackfireHoldMs = doc["manualBackfireHoldMs"] | c.manualBackfireHoldMs;
    c.manualBackfireMode = doc["manualBackfireMode"] | c.manualBackfireMode;
    c.manualPulseCutMs = doc["manualPulseCutMs"] | c.manualPulseCutMs;
    c.manualPulseFireMs = doc["manualPulseFireMs"] | c.manualPulseFireMs;
    c.manualPulseOffMs = doc["manualPulseOffMs"] | c.manualPulseOffMs;
    c.manualPulseRepeat = doc["manualPulseRepeat"] | c.manualPulseRepeat;
    if (doc.containsKey("manualMinRpm")) {
      c.manualMinRpm = clampManualMinRpm(doc["manualMinRpm"].as<int>());
    }
    c.manualIgnOnly = doc["manualIgnOnly"] | c.manualIgnOnly;
    if (doc.containsKey("apPassword")) {
      String pass = doc["apPassword"].as<String>();
      pass.trim();
      if (!validateApPassword(pass)) {
        r->send(400, "text/plain", "invalid ap password");
        return;
      }
      apPassChanged = applyApPassword(c, pass);
    }
    if (doc.containsKey("autoMap")) {
      updateAutoMapFromPayload(doc["autoMap"]);
    }

    shift_configure(c.shiftActiveLow, c.shiftDebounceMs);
    rpm_set_ppr(c.ppr);
    rpm_set_scale(c.rpmScaleFactor);
    rpm_set_offset(c.rpmOffset);
    if (!saveConfig()) {
      r->send(500, "text/plain", "save fail");
      return;
    }

    DynamicJsonDocument changesDoc(1536);
    JsonObject changeObj = changesDoc.to<JsonObject>();
    collectConfigChanges(before, c, changeObj);
    changeObj["source"] = "import";
    log_push(0, false, "SYS", "CFG_IMPORT", &changesDoc, &c);
    r->send(200, "application/json", "{\"ok\":true}");
    if (apPassChanged) {
      pendingApRestart = true;
      apRestartMs = millis() + 800;
    } });


  server.on("/test/cut", HTTP_GET, [](AsyncWebServerRequest *r)
            {
    const QSConfig &c = getConfig();
    int duration = r->hasParam("ms") ? r->getParam("ms")->value().toInt() : c.cutTimeFixed;
    if (duration < 5) duration = 5;
    if (duration > 500) duration = 500; // Kaizen: Poka-Yoke limit
    cut_trigger_ms(duration, rpm_get());
    r->send(200, "application/json", "{\"ok\":true}"); });

  server.on("/test/rpm", HTTP_GET, [](AsyncWebServerRequest *r)
            {
    if (r->hasParam("rpm")) {
      String s = r->getParam("rpm")->value();
      int rpm = s.toInt();
      float ppr = getConfig().ppr;
      if (rpm < 0) rpm = 0;
      float freq = rpm * ppr / 60.0f;
      if (freq < 1.0f) freq = 1.0f;
      stop_test_pwm_output();
      ledcSetup(TEST_PWM_CH, (uint32_t)freq, TEST_PWM_RES);
      ledcAttachPin(TEST_PWM, TEST_PWM_CH);
      ledcWrite(TEST_PWM_CH, TEST_PWM_DUTY);
      char label[48];
      snprintf(label, sizeof(label), "TEST_PWM target=%drpm", rpm);
      DynamicJsonDocument meta(256);
      meta["freqHz"] = freq;
      meta["ppr"] = ppr;
      meta["duty"] = 50;
      meta["source"] = "test-rpm";
      log_push(rpm_get(), false, "SYS", label, &meta);
      r->send(200, "application/json", "{\"ok\":true}");
    } else {
      r->send(400, "text/plain", "missing rpm");
    } });

  server.on("/test/rpm/stop", HTTP_POST, [](AsyncWebServerRequest *r)
            {
    stop_test_pwm_output();
    DynamicJsonDocument meta(128);
    meta["source"] = "test-rpm";
    log_push(rpm_get(), false, "SYS", "TEST_PWM_STOP", &meta);
    r->send(200, "application/json", "{\"ok\":true}"); });

  server.on("/ota/firmware", HTTP_POST, [](AsyncWebServerRequest *r)
            {
      UploadStatus *state = reinterpret_cast<UploadStatus*>(r->_tempObject);
      String msg = state ? state->message : "ota idle";
      bool ok = state ? state->ok : false;
      delete state;
      r->_tempObject = nullptr;
      send_json_status(r, ok, msg); }, [](AsyncWebServerRequest *r, const String &, size_t index, uint8_t *data, size_t len, bool final)
            {
      UploadStatus *state = reinterpret_cast<UploadStatus*>(r->_tempObject);
      if (index == 0 && !state) {
        state = new UploadStatus();
        r->_tempObject = state;
        size_t total = r->contentLength();
        if (total == 0) {
          state->ok = false;
          state->message = "missing content-length";
          return;
        }
        String err;
        if (!ota_manager_begin(total, err)) {
          state->ok = false;
          state->message = err;
          return;
        }
        DynamicJsonDocument meta(256);
        meta["stage"] = "start";
        meta["bytes"] = total;
        log_push(0, false, "SYS", "OTA_FW_BEGIN", &meta);
      }
      if (!state->ok) {
        return;
      }
      String err;
      if (!ota_manager_write(data, len, err)) {
        state->ok = false;
        state->message = err;
        ota_manager_abort(err.c_str());
        return;
      }
      if (final) {
        if (!ota_manager_finalize(err)) {
          state->ok = false;
          state->message = err;
          ota_manager_abort(err.c_str());
        } else {
          state->message = "firmware staged";
          DynamicJsonDocument meta(256);
          meta["stage"] = "done";
          meta["bytes"] = ota_manager_bytes_written();
          log_push(0, false, "SYS", "OTA_FW_DONE", &meta);
        }
      } });

  server.on("/ota/fs", HTTP_POST, [](AsyncWebServerRequest *r)
            {
      UploadStatus *state = reinterpret_cast<UploadStatus*>(r->_tempObject);
      String msg = state ? state->message : "fs idle";
      bool ok = state ? state->ok : false;
      delete state;
      r->_tempObject = nullptr;
      send_json_status(r, ok, msg); }, [](AsyncWebServerRequest *r, const String &, size_t index, uint8_t *data, size_t len, bool final)
            {
      UploadStatus *state = reinterpret_cast<UploadStatus*>(r->_tempObject);
      if (index == 0 && !state) {
        state = new UploadStatus();
        r->_tempObject = state;
        size_t total = r->contentLength();
        if (total == 0) {
          state->ok = false;
          state->message = "missing content-length";
          return;
        }
        String err;
        if (!fs_updater_begin(total, err)) {
          state->ok = false;
          state->message = err;
          return;
        }
        DynamicJsonDocument meta(256);
        meta["stage"] = "start";
        meta["bytes"] = total;
        log_push(0, false, "SYS", "OTA_FS_BEGIN", &meta);
      }
      if (!state->ok) {
        return;
      }
      String err;
      if (!fs_updater_write(data, len, err)) {
        state->ok = false;
        state->message = err;
        return;
      }
      if (final) {
        if (!fs_updater_finalize(err)) {
          state->ok = false;
          state->message = err;
        } else {
          state->message = "fs staged";
          ota_manager_schedule_reboot(1500);
          DynamicJsonDocument meta(256);
          meta["stage"] = "done";
          meta["bytes"] = fs_updater_bytes_written();
          log_push(0, false, "SYS", "OTA_FS_DONE", &meta);
        }
      } });

  server.on("/calibrate", HTTP_POST, [](AsyncWebServerRequest *r)
            {
              // body handled below
            },
            NULL, [](AsyncWebServerRequest *r, uint8_t *data, size_t len, size_t index, size_t total)
            {
    String *payload;
    if (index == 0) {
      payload = new String();
      payload->reserve(total);
      r->_tempObject = payload;
    } else {
      payload = reinterpret_cast<String*>(r->_tempObject);
    }
    if (!payload) {
      r->send(500, "text/plain", "alloc fail");
      return;
    }
    payload->concat(String(reinterpret_cast<char*>(data), len));
    if (index + len < total) {
      return;
    }

    DynamicJsonDocument doc(2048);
    DeserializationError err = deserializeJson(doc, *payload);
    delete payload;
    r->_tempObject = nullptr;
    if (err) { r->send(400, "text/plain", "invalid json"); return; }

    float target = doc["targetRpm"] | 0;
    JsonArray samples = doc["measuredSamples"].as<JsonArray>();
    if (target <= 0 || samples.isNull() || samples.size() == 0) {
      r->send(400, "text/plain", "invalid data");
      return;
    }

    CalSampleStats stats = computeCalSampleStats(samples);
    if (!stats.ok || stats.avg <= 0.0f) {
      r->send(400, "text/plain", "invalid samples");
      return;
    }

    QSConfig &cfg = const_cast<QSConfig&>(getConfig());
    const float oldFactor = cfg.rpmScaleFactor;
    const float oldOffset = cfg.rpmOffset;
    const float numerator = target - oldOffset;
    const float denominator = stats.avg - oldOffset;
    if (fabsf(denominator) < 1.0f) {
      r->send(400, "text/plain", "sample too close to offset");
      return;
    }

    float ratio = numerator / denominator;
    float newFactor = oldFactor * ratio;
    if (!isfinite(newFactor) || newFactor < 0.01f || newFactor > 20.0f) {
      r->send(400, "text/plain", "new factor out of range");
      return;
    }

    cfg.rpmScaleFactor = newFactor;
    rpm_set_scale(newFactor);
    rpm_set_offset(oldOffset);
    saveConfig();

    DynamicJsonDocument resp(512);
    resp["newFactor"] = newFactor;
    resp["oldFactor"] = oldFactor;
    resp["avgMeasured"] = stats.avg;
    resp["stdDev"] = stats.stdDev;
    resp["count"] = stats.rawCount;
    resp["usedCount"] = stats.usedCount;
    resp["minUsed"] = stats.minVal;
    resp["maxUsed"] = stats.maxVal;
    String out;
    serializeJson(resp, out);
    r->send(200, "application/json", out); });

  server.on("/calibrate/two-point", HTTP_POST, [](AsyncWebServerRequest *r)
            {
              // body handled below
            },
            NULL, [](AsyncWebServerRequest *r, uint8_t *data, size_t len, size_t index, size_t total)
            {
    String *payload;
    if (index == 0) {
      payload = new String();
      payload->reserve(total);
      r->_tempObject = payload;
    } else {
      payload = reinterpret_cast<String*>(r->_tempObject);
    }
    if (!payload) {
      r->send(500, "text/plain", "alloc fail");
      return;
    }
    payload->concat(String(reinterpret_cast<char*>(data), len));
    if (index + len < total) {
      return;
    }

    DynamicJsonDocument doc(3072);
    DeserializationError err = deserializeJson(doc, *payload);
    delete payload;
    r->_tempObject = nullptr;
    if (err) { r->send(400, "text/plain", "invalid json"); return; }

    float lowTarget = doc["lowTargetRpm"] | 0;
    float highTarget = doc["highTargetRpm"] | 0;
    JsonArray lowSamples = doc["lowMeasuredSamples"].as<JsonArray>();
    JsonArray highSamples = doc["highMeasuredSamples"].as<JsonArray>();

    if (lowTarget <= 0 || highTarget <= 0 || highTarget <= lowTarget ||
        lowSamples.isNull() || highSamples.isNull() ||
        lowSamples.size() == 0 || highSamples.size() == 0) {
      r->send(400, "text/plain", "invalid data");
      return;
    }

    CalSampleStats lowStats = computeCalSampleStats(lowSamples);
    CalSampleStats highStats = computeCalSampleStats(highSamples);
    if (!lowStats.ok || !highStats.ok || lowStats.avg <= 0.0f || highStats.avg <= 0.0f) {
      r->send(400, "text/plain", "invalid samples");
      return;
    }

    if (fabsf(highStats.avg - lowStats.avg) < 1.0f) {
      r->send(400, "text/plain", "invalid sample spread");
      return;
    }

    float alpha = (highTarget - lowTarget) / (highStats.avg - lowStats.avg);
    float beta = lowTarget - (alpha * lowStats.avg);
    if (!isfinite(alpha) || !isfinite(beta)) {
      r->send(400, "text/plain", "invalid calibration result");
      return;
    }

    QSConfig &cfg = const_cast<QSConfig&>(getConfig());
    float oldScale = cfg.rpmScaleFactor;
    float oldOffset = cfg.rpmOffset;
    float newScale = alpha * oldScale;
    float newOffset = (alpha * oldOffset) + beta;
    if (!isfinite(newScale) || !isfinite(newOffset) || newScale < 0.01f || newScale > 20.0f) {
      r->send(400, "text/plain", "new scale/offset out of range");
      return;
    }

    cfg.rpmScaleFactor = newScale;
    cfg.rpmOffset = newOffset;
    rpm_set_scale(newScale);
    rpm_set_offset(newOffset);
    saveConfig();

    DynamicJsonDocument resp(1024);
    resp["newFactor"] = newScale;
    resp["newOffset"] = newOffset;
    resp["oldFactor"] = oldScale;
    resp["oldOffset"] = oldOffset;
    resp["alpha"] = alpha;
    resp["beta"] = beta;
    resp["lowAvgMeasured"] = lowStats.avg;
    resp["highAvgMeasured"] = highStats.avg;
    resp["lowStdDev"] = lowStats.stdDev;
    resp["highStdDev"] = highStats.stdDev;
    resp["lowCount"] = lowStats.rawCount;
    resp["highCount"] = highStats.rawCount;
    resp["lowUsedCount"] = lowStats.usedCount;
    resp["highUsedCount"] = highStats.usedCount;
    String out;
    serializeJson(resp, out);
    r->send(200, "application/json", out); });

  server.begin();
  log_set_callback(on_new_log);
}

void web_handle()
{
  unsigned long now = millis();

  // 1. Thực hiện khởi động lại AP có delay sau khi gửi response thành công
  if (pendingApRestart && now >= apRestartMs)
  {
    pendingApRestart = false;
    Serial.println("[Wi-Fi] Restarting SoftAP with new credentials...");
    restartAccessPoint();
  }

  // 2. Kiểm tra và thực thi AP Timeout (Tự động tắt Wi-Fi khi không có kết nối)
  const QSConfig &cfg = getConfig();
  if (apActive && cfg.apTimeoutMs > 0)
  {
    uint8_t stationCount = WiFi.softAPgetStationNum();
    if (stationCount > 0)
    {
      // Có điện thoại/máy tính kết nối -> giữ kết nối và làm mới thời gian hoạt động
      apLastActiveMs = now;
    }
    else
    {
      // Không có thiết bị kết nối:
      // Tự tắt AP nếu đã quá apTimeoutMs từ lúc khởi động HOẶC từ lúc client cuối ngắt kết nối
      if ((now - apStartTimeMs >= (unsigned long)cfg.apTimeoutMs) &&
          (now - apLastActiveMs >= (unsigned long)cfg.apTimeoutMs))
      {
        Serial.println("[Wi-Fi] AP timeout reached with 0 clients -> Turning OFF SoftAP");
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_OFF);
        apActive = false;
      }
    }
  }

  // 3. Gửi Live Telemetry
  if (now - lastTelemetryMs >= 250)
  {
    lastTelemetryMs = now;
    send_live_telemetry();
  }
}
