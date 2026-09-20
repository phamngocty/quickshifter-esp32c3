#include "config.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

static QSConfig cfg;
const char *CONFIG_PATH = "/config.json";
const char *CONFIG_TMP_PATH = "/config.tmp";
const char *CONFIG_BAK_PATH = "/config.bak";
static const int DEFAULT_BUCKET_WIDTH = 1000;
static const uint8_t DEFAULT_AUTO_MAP_COUNT = 8;
static const size_t CONFIG_JSON_CAPACITY = 8192;

// --- Global Auto RPM Limiter Configuration Variables ---
bool allow_double_click = true; 
bool limiter_enable = false;     
int target_rpm = 7000;          
int rpm_hysteresis = 200;      
unsigned long hard_cut_ms = 30;       
unsigned long recovery_spark_ms = 20; 

static bool ensureFsMounted() {
  static bool mounted = false;
  if (!mounted) {
    mounted = LittleFS.begin();
  }
  return mounted;
}

static void setDefaultAutoMap() {
  cfg.autoMapCount = DEFAULT_AUTO_MAP_COUNT;
  // 150cc baseline default (same as UI 8-band preset): 4000..12500 rpm.
  const int defaults[DEFAULT_AUTO_MAP_COUNT] = {106, 94, 89, 80, 72, 69, 66, 57};
  const int mins[DEFAULT_AUTO_MAP_COUNT] = {4000, 5000, 6000, 7000, 8000, 9000, 10000, 11000};
  const int maxs[DEFAULT_AUTO_MAP_COUNT] = {4999, 5999, 6999, 7999, 8999, 9999, 10999, 12500};
  for (uint8_t i = 0; i < cfg.autoMapCount; ++i) {
    cfg.autoMap[i].rpmMin = mins[i];
    cfg.autoMap[i].rpmMax = maxs[i];
    cfg.autoMap[i].cutMs = defaults[i];
  }
}

void config_normalize_auto_map() {
  if (cfg.autoMapCount == 0) {
    setDefaultAutoMap();
    return;
  }

  // clamp counts
  if (cfg.autoMapCount > MAX_AUTO_MAP) cfg.autoMapCount = MAX_AUTO_MAP;

  for (uint8_t i = 0; i < cfg.autoMapCount; ++i) {
    AutoMapEntry &entry = cfg.autoMap[i];
    if (entry.rpmMin < 0) entry.rpmMin = 0;
    if (entry.rpmMax < entry.rpmMin + 50) entry.rpmMax = entry.rpmMin + 50;
    if (entry.cutMs < 5) entry.cutMs = 5;
    if (entry.cutMs > 200) entry.cutMs = 200;
  }

  // sort ascending by rpmMin
  for (uint8_t i = 0; i + 1 < cfg.autoMapCount; ++i) {
    for (uint8_t j = i + 1; j < cfg.autoMapCount; ++j) {
      if (cfg.autoMap[j].rpmMin < cfg.autoMap[i].rpmMin) {
        AutoMapEntry tmp = cfg.autoMap[i];
        cfg.autoMap[i] = cfg.autoMap[j];
        cfg.autoMap[j] = tmp;
      }
    }
  }

  // ensure ordering and no gaps reversal
  for (uint8_t i = 1; i < cfg.autoMapCount; ++i) {
    if (cfg.autoMap[i].rpmMin <= cfg.autoMap[i - 1].rpmMin) {
      cfg.autoMap[i].rpmMin = cfg.autoMap[i - 1].rpmMin + 1;
    }
    if (cfg.autoMap[i].rpmMin <= cfg.autoMap[i - 1].rpmMax) {
      cfg.autoMap[i].rpmMin = cfg.autoMap[i - 1].rpmMax + 1;
    }
    if (cfg.autoMap[i].rpmMax <= cfg.autoMap[i].rpmMin) {
      cfg.autoMap[i].rpmMax = cfg.autoMap[i].rpmMin + 50;
    }
  }
}

const QSConfig &getConfig() {
  return cfg;
}

QSConfig &getMutableConfig() {
  return cfg;
}

void setDefaults() {
  cfg.shiftActiveLow = true;
  cfg.shiftDebounceMs = 30;

  cfg.relayPolarityHighToCut = true;

  cfg.ppr = 1.0f;
  cfg.rpmScaleFactor = 2.2f;
  cfg.rpmOffset = 0.0f;

  cfg.modeAuto = true;
  cfg.cutTimeFixed = 90;
  setDefaultAutoMap();

  // Auto RPM Limiter Defaults
  cfg.allow_double_click = true;
  cfg.limiter_enable = false;
  cfg.target_rpm = 7000;
  cfg.rpm_hysteresis = 200;
  cfg.hard_cut_ms = 30;
  cfg.recovery_spark_ms = 20;

  // Sync globals
  allow_double_click = cfg.allow_double_click;
  limiter_enable = cfg.limiter_enable;
  target_rpm = cfg.target_rpm;
  rpm_hysteresis = cfg.rpm_hysteresis;
  hard_cut_ms = cfg.hard_cut_ms;
  recovery_spark_ms = cfg.recovery_spark_ms;

  cfg.apTimeoutMs = 120000;
  cfg.apPassword[0] = '\0';
  cfg.minRpm = 1500;

  cfg.cutIgnEnabled = true;
  cfg.cutInjEnabled = false;

  // Manual Backfire defaults
  cfg.manualBackfireEnabled = true;
  cfg.manualBackfireHoldMs = 1500;
  cfg.manualBackfireMode = 1;
  cfg.manualPulseCutMs = 200;
  cfg.manualPulseFireMs = 50;
  cfg.manualPulseOffMs = 200;
  cfg.manualPulseRepeat = 3;
  cfg.manualMinRpm = 0;
  cfg.manualIgnOnly = true;
}

bool ensureDefaultConfig() {
  if (!ensureFsMounted()) return false;
  if (!LittleFS.exists(CONFIG_PATH)) {
    setDefaults();
    return saveConfig();
  }
  return true;
}

static void loadAutoMapFromJson(JsonVariant variant) {
  cfg.autoMapCount = 0;
  if (!variant.is<JsonArray>()) {
    // legacy format: array of ints
    JsonArray arr = variant;
    if (arr.isNull()) return;
    for (JsonVariant v : arr) {
      if (cfg.autoMapCount >= MAX_AUTO_MAP) break;
      AutoMapEntry &entry = cfg.autoMap[cfg.autoMapCount++];
      entry.rpmMin = (cfg.autoMapCount - 1) * DEFAULT_BUCKET_WIDTH;
      entry.rpmMax = entry.rpmMin + DEFAULT_BUCKET_WIDTH - 1;
      entry.cutMs = v.as<int>();
    }
    return;
  }

  JsonArray objArr = variant.as<JsonArray>();
  for (JsonVariant item : objArr) {
    if (cfg.autoMapCount >= MAX_AUTO_MAP) break;
    AutoMapEntry &entry = cfg.autoMap[cfg.autoMapCount++];
    if (item.is<JsonObject>()) {
      JsonObject o = item.as<JsonObject>();
      entry.rpmMin = o["min"] | ((cfg.autoMapCount - 1) * DEFAULT_BUCKET_WIDTH);
      entry.rpmMax = o["max"] | (entry.rpmMin + DEFAULT_BUCKET_WIDTH - 1);
      entry.cutMs = o["cut"] | 50;
    } else {
      entry.rpmMin = (cfg.autoMapCount - 1) * DEFAULT_BUCKET_WIDTH;
      entry.rpmMax = entry.rpmMin + DEFAULT_BUCKET_WIDTH - 1;
      entry.cutMs = item.as<int>();
    }
  }
}

bool loadConfig() {
  if (!ensureFsMounted()) return false;
  if (!LittleFS.exists(CONFIG_PATH) && LittleFS.exists(CONFIG_BAK_PATH)) {
    LittleFS.rename(CONFIG_BAK_PATH, CONFIG_PATH);
  }
  if (!LittleFS.exists(CONFIG_PATH)) {
    setDefaults();
    return saveConfig();
  }

  File f = LittleFS.open(CONFIG_PATH, "r");
  if (!f) return false;

  DynamicJsonDocument doc(CONFIG_JSON_CAPACITY);
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (err) {
    setDefaults();
    return saveConfig();
  }

  // Start from known-good defaults, then override with persisted values.
  setDefaults();

  cfg.shiftActiveLow = doc["shiftActiveLow"] | true;
  cfg.shiftDebounceMs = doc["shiftDebounceMs"] | 30;
  cfg.relayPolarityHighToCut = doc["relayPolarityHighToCut"] | true;
  cfg.ppr = doc["ppr"] | 1.0f;
  cfg.rpmScaleFactor = doc["rpmScaleFactor"] | cfg.rpmScaleFactor;
  cfg.rpmOffset = doc["rpmOffset"] | cfg.rpmOffset;
  cfg.modeAuto = doc["modeAuto"] | true;
  cfg.cutTimeFixed = doc["cutTimeFixed"] | cfg.cutTimeFixed;
  
  // Auto RPM Limiter load
  cfg.allow_double_click = doc.containsKey("allow_double_click") ? doc["allow_double_click"].as<bool>() : true;
  cfg.limiter_enable = false; // Luôn tắt khi khởi động để đảm bảo an toàn
  cfg.target_rpm = doc["target_rpm"] | 7000;
  cfg.rpm_hysteresis = doc["rpm_hysteresis"] | 200;
  cfg.hard_cut_ms = doc["hard_cut_ms"] | 30UL;
  cfg.recovery_spark_ms = doc["recovery_spark_ms"] | 20UL;

  // Validation
  if (cfg.target_rpm < 1000) cfg.target_rpm = 1000;
  if (cfg.target_rpm > 20000) cfg.target_rpm = 20000;
  if (cfg.rpm_hysteresis < 10) cfg.rpm_hysteresis = 10;
  if (cfg.rpm_hysteresis > 2000) cfg.rpm_hysteresis = 2000;
  if (cfg.hard_cut_ms < 5) cfg.hard_cut_ms = 5;
  if (cfg.hard_cut_ms > 500) cfg.hard_cut_ms = 500;
  if (cfg.recovery_spark_ms < 5) cfg.recovery_spark_ms = 5;
  if (cfg.recovery_spark_ms > 500) cfg.recovery_spark_ms = 500;

  // Sync to globals
  allow_double_click = cfg.allow_double_click;
  limiter_enable = false; // Luôn tắt khi khởi động
  target_rpm = cfg.target_rpm;
  rpm_hysteresis = cfg.rpm_hysteresis;
  hard_cut_ms = cfg.hard_cut_ms;
  recovery_spark_ms = cfg.recovery_spark_ms;

  cfg.apTimeoutMs = doc["apTimeoutMs"] | 120000;
  const char *apPassRaw = doc["apPassword"] | "";
  String apPassStr = apPassRaw;
  apPassStr.trim();
  apPassStr.toCharArray(cfg.apPassword, sizeof(cfg.apPassword));
  cfg.apPassword[sizeof(cfg.apPassword)-1] = '\0';
  cfg.minRpm = doc["minRpm"] | 1500;
  if (cfg.minRpm < 0) cfg.minRpm = 0;
  if (cfg.minRpm > 18000) cfg.minRpm = 18000;

  cfg.cutIgnEnabled = doc["cutIgnEnabled"] | cfg.cutIgnEnabled;
  cfg.cutInjEnabled = doc["cutInjEnabled"] | cfg.cutInjEnabled;

  cfg.manualBackfireEnabled = doc["manualBackfireEnabled"] | cfg.manualBackfireEnabled;
  cfg.manualBackfireHoldMs = doc["manualBackfireHoldMs"] | cfg.manualBackfireHoldMs;
  cfg.manualBackfireMode = doc["manualBackfireMode"] | 1;
  cfg.manualPulseCutMs = doc["manualPulseCutMs"] | 200;
  cfg.manualPulseFireMs = doc["manualPulseFireMs"] | 50;
  cfg.manualPulseOffMs = doc["manualPulseOffMs"] | 200;
  cfg.manualPulseRepeat = doc["manualPulseRepeat"] | 3;
  cfg.manualMinRpm = doc["manualMinRpm"] | cfg.manualMinRpm;
  cfg.manualIgnOnly = doc["manualIgnOnly"] | true;

  if (cfg.manualMinRpm < 0) cfg.manualMinRpm = 0;
  if (cfg.manualMinRpm > 10000) cfg.manualMinRpm = 10000;

  if (cfg.manualBackfireMode > 2) cfg.manualBackfireMode = 2;
  if (cfg.manualBackfireHoldMs < 100) cfg.manualBackfireHoldMs = 100;
  if (cfg.manualBackfireHoldMs > 10000) cfg.manualBackfireHoldMs = 10000;
  if (cfg.manualPulseCutMs < 5) cfg.manualPulseCutMs = 5;
  if (cfg.manualPulseCutMs > 2000) cfg.manualPulseCutMs = 2000;
  if (cfg.manualPulseFireMs < 5) cfg.manualPulseFireMs = 5;
  if (cfg.manualPulseFireMs > 2000) cfg.manualPulseFireMs = 2000;
  if (cfg.manualPulseOffMs < 5) cfg.manualPulseOffMs = 5;
  if (cfg.manualPulseOffMs > 2000) cfg.manualPulseOffMs = 2000;
  if (cfg.manualPulseRepeat == 0) cfg.manualPulseRepeat = 1;
  if (cfg.manualPulseRepeat > 20) cfg.manualPulseRepeat = 20;

  JsonVariant mapVariant = doc["autoMap"];
  if (!mapVariant.isNull()) {
    loadAutoMapFromJson(mapVariant);
  } else {
    setDefaultAutoMap();
  }

  if (cfg.autoMapCount == 0) {
    setDefaultAutoMap();
  }
  config_normalize_auto_map();

  return true;
}

bool saveConfig() {
  if (!ensureFsMounted()) return false;
  File f = LittleFS.open(CONFIG_TMP_PATH, "w");
  if (!f) return false;

  // Sync globals to cfg
  cfg.allow_double_click = allow_double_click;
  cfg.limiter_enable = limiter_enable;
  cfg.target_rpm = target_rpm;
  cfg.rpm_hysteresis = rpm_hysteresis;
  cfg.hard_cut_ms = hard_cut_ms;
  cfg.recovery_spark_ms = recovery_spark_ms;

  DynamicJsonDocument doc(CONFIG_JSON_CAPACITY);
  doc["shiftActiveLow"] = cfg.shiftActiveLow;
  doc["shiftDebounceMs"] = cfg.shiftDebounceMs;
  doc["relayPolarityHighToCut"] = cfg.relayPolarityHighToCut;
  doc["ppr"] = cfg.ppr;
  doc["rpmScaleFactor"] = cfg.rpmScaleFactor;
  doc["rpmOffset"] = cfg.rpmOffset;
  doc["modeAuto"] = cfg.modeAuto;
  doc["cutTimeFixed"] = cfg.cutTimeFixed;
  JsonArray arr = doc.createNestedArray("autoMap");
  for (uint8_t i = 0; i < cfg.autoMapCount; ++i) {
    JsonObject obj = arr.createNestedObject();
    obj["min"] = cfg.autoMap[i].rpmMin;
    obj["max"] = cfg.autoMap[i].rpmMax;
    obj["cut"] = cfg.autoMap[i].cutMs;
  }
  
  // Auto RPM Limiter save
  doc["allow_double_click"] = cfg.allow_double_click;
  doc["limiter_enable"] = false; // Luôn lưu false để khởi động lại luôn ở trạng thái tắt
  doc["target_rpm"] = cfg.target_rpm;
  doc["rpm_hysteresis"] = cfg.rpm_hysteresis;
  doc["hard_cut_ms"] = cfg.hard_cut_ms;
  doc["recovery_spark_ms"] = cfg.recovery_spark_ms;

  doc["apTimeoutMs"] = cfg.apTimeoutMs;
  doc["apPassword"] = cfg.apPassword;
  doc["minRpm"] = cfg.minRpm;
  doc["cutIgnEnabled"] = cfg.cutIgnEnabled;
  doc["cutInjEnabled"] = cfg.cutInjEnabled;

  // Save manual backfire config
  doc["manualBackfireEnabled"] = cfg.manualBackfireEnabled;
  doc["manualBackfireHoldMs"] = cfg.manualBackfireHoldMs;
  doc["manualBackfireMode"] = cfg.manualBackfireMode;
  doc["manualPulseCutMs"] = cfg.manualPulseCutMs;
  doc["manualPulseFireMs"] = cfg.manualPulseFireMs;
  doc["manualPulseOffMs"] = cfg.manualPulseOffMs;
  doc["manualPulseRepeat"] = cfg.manualPulseRepeat;
  doc["manualMinRpm"] = cfg.manualMinRpm;
  doc["manualIgnOnly"] = cfg.manualIgnOnly;

  bool res = (serializeJson(doc, f) > 0);
  f.flush();
  f.close();
  if (!res) {
    LittleFS.remove(CONFIG_TMP_PATH);
    return false;
  }

  if (LittleFS.exists(CONFIG_BAK_PATH)) {
    LittleFS.remove(CONFIG_BAK_PATH);
  }

  if (LittleFS.exists(CONFIG_PATH)) {
    if (!LittleFS.rename(CONFIG_PATH, CONFIG_BAK_PATH)) {
      LittleFS.remove(CONFIG_TMP_PATH);
      return false;
    }
  }

  if (!LittleFS.rename(CONFIG_TMP_PATH, CONFIG_PATH)) {
    if (LittleFS.exists(CONFIG_BAK_PATH)) {
      LittleFS.rename(CONFIG_BAK_PATH, CONFIG_PATH);
    }
    return false;
  }

  if (LittleFS.exists(CONFIG_BAK_PATH)) {
    LittleFS.remove(CONFIG_BAK_PATH);
  }
  return true;
}

int config_resolve_cut_ms(int rpm) {
  if (cfg.autoMapCount == 0) return cfg.cutTimeFixed;
  for (uint8_t i = 0; i < cfg.autoMapCount; ++i) {
    const AutoMapEntry &entry = cfg.autoMap[i];
    if (rpm >= entry.rpmMin && rpm <= entry.rpmMax) {
      return entry.cutMs;
    }
  }
  return cfg.cutTimeFixed;
}

const AutoMapEntry *config_find_auto_map_entry(int rpm, uint8_t *outIndex) {
  if (cfg.autoMapCount == 0) return nullptr;
  for (uint8_t i = 0; i < cfg.autoMapCount; ++i) {
    const AutoMapEntry &entry = cfg.autoMap[i];
    if (rpm >= entry.rpmMin && rpm <= entry.rpmMax) {
      if (outIndex) *outIndex = i;
      return &entry;
    }
  }
  return nullptr;
}

