#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "config.h"
#include "rpm.h"
#include "shift.h"
#include "cut.h"
#include "log.h"
#include "web.h"
#include "lock.h"
#include "ota_manager.h"

// Pin map (per PROJECT OVERVIEW)
static const uint8_t RPM_PIN = 2;
static const uint8_t RELAY_IGN = 6;
static const uint8_t RELAY_INJ = 7;
static const uint8_t SHIFT_SW = 4;
static const uint8_t TEST_PWM = 8;

static bool shiftPressed = false;
static bool shiftCutLatched = false;

struct BackfireRuntime {
  unsigned long lastShiftMs = 0;
  unsigned long lastBackfireMs = 0;
  uint8_t burstCount = 0;
};

struct BackfireResult {
  unsigned long ignExtra = 0;
  unsigned long injExtra = 0;
  uint8_t profileUsed = BACKFIRE_PROFILE_BASIC;
  uint8_t burstIndex = 0;
  int bandIndex = -1;
  bool applied = false;
};

static BackfireRuntime backfireState;
struct DecelBackfireState {
  int prevRpm = 0;
  uint8_t pulsesRemaining = 0;
  unsigned long nextPulseMs = 0;
};

static DecelBackfireState decelBackfire;

static inline unsigned long clampExtra(int value) {
  return value > 0 ? static_cast<unsigned long>(value) : 0UL;
}

static BackfireResult resolve_backfire(int rpm, const QSConfig &cfg) {
  BackfireResult result;
  if (!cfg.backfireMode) return result;
  unsigned long now = millis();
  bool rpmOk = rpm >= cfg.backfireRpmThreshold;
  if (cfg.backfireProfile != BACKFIRE_PROFILE_BURST) {
    backfireState.burstCount = 0;
  }

  auto applyExtras = [&](unsigned long ignExtra, unsigned long injExtra, BackfireProfile profile) {
    if (ignExtra == 0 && injExtra == 0) return;
    result.ignExtra = ignExtra;
    result.injExtra = cfg.cutInjEnabled ? injExtra : 0;
    result.profileUsed = profile;
    result.applied = true;
    backfireState.lastBackfireMs = now;
  };

  auto cooldownPassed = [&](int cooldownMs) {
    if (cooldownMs <= 0) return true;
    return (now - backfireState.lastBackfireMs) >= static_cast<unsigned long>(cooldownMs);
  };

  switch (cfg.backfireProfile) {
    case BACKFIRE_PROFILE_STAGED: {
      if (!rpmOk) break;
      uint8_t idx = 0;
      const AutoMapEntry *entry = config_find_auto_map_entry(rpm, &idx);
      if (!entry) break;
      if (!entry->backfireEnabled || entry->backfireAddMs <= 0) break;
      if (!cooldownPassed(cfg.backfireCooldownMs)) break;
      result.bandIndex = idx;
      unsigned long add = clampExtra(entry->backfireAddMs);
      applyExtras(add, add, BACKFIRE_PROFILE_STAGED);
    } break;
    case BACKFIRE_PROFILE_BURST: {
      if (!rpmOk) {
        backfireState.burstCount = 0;
        break;
      }
      if ((now - backfireState.lastShiftMs) <= static_cast<unsigned long>(cfg.backfireBurstWindowMs)) {
        if (backfireState.burstCount < cfg.backfireBurstMax) {
          backfireState.burstCount++;
        }
      } else {
        backfireState.burstCount = 1;
      }
      if (backfireState.burstCount == 0) backfireState.burstCount = 1;
      backfireState.lastShiftMs = now;
      unsigned long add = clampExtra(cfg.backfireAddMs);
      if (backfireState.burstCount > 1) {
        add += clampExtra(cfg.backfireBurstExtraMs) * static_cast<unsigned long>(backfireState.burstCount - 1);
      }
      result.burstIndex = backfireState.burstCount;
      applyExtras(add, add, BACKFIRE_PROFILE_BURST);
    } break;
    case BACKFIRE_PROFILE_BLEND: {
      if (!rpmOk) break;
      if (!(cfg.cutIgnEnabled && cfg.cutInjEnabled)) break;
      if (!cooldownPassed(cfg.backfireCooldownMs)) break;
      unsigned long shared = clampExtra(cfg.backfireAddMs);
      unsigned long ignExtra = shared + clampExtra(cfg.backfireIgnExtraMs);
      applyExtras(ignExtra, shared, BACKFIRE_PROFILE_BLEND);
    } break;
    case BACKFIRE_PROFILE_DECEL: {
      // handled asynchronously via handle_decel_backfire()
    } break;
    case BACKFIRE_PROFILE_BASIC:
    default: {
      if (!rpmOk) break;
      if (!cooldownPassed(cfg.backfireCooldownMs)) break;
      unsigned long add = clampExtra(cfg.backfireAddMs);
      applyExtras(add, add, BACKFIRE_PROFILE_BASIC);
    } break;
  }
  return result;
}

static void handle_decel_backfire(int currentRpm, const QSConfig &cfg) {
  if (decelBackfire.prevRpm == 0) decelBackfire.prevRpm = currentRpm;
  if (!cfg.backfireMode || cfg.backfireProfile != BACKFIRE_PROFILE_DECEL) {
    decelBackfire.pulsesRemaining = 0;
    decelBackfire.prevRpm = currentRpm;
    return;
  }
  unsigned long now = millis();
  int prev = decelBackfire.prevRpm;
  int drop = prev - currentRpm;
  bool rpmHigh = prev >= cfg.backfireDecelHighRpm;
  bool dropDetected = rpmHigh && drop >= cfg.backfireDecelDropRpm;
  if (dropDetected) {
    bool cooldownOk = (now - backfireState.lastBackfireMs) >= static_cast<unsigned long>(cfg.backfireCooldownMs);
    if (cooldownOk) {
      decelBackfire.pulsesRemaining = cfg.backfireDecelPulseCount;
      decelBackfire.nextPulseMs = now;
      backfireState.lastBackfireMs = now;
      DynamicJsonDocument meta(192);
      meta["source"] = "backfire";
      meta["profile"] = BACKFIRE_PROFILE_DECEL;
      meta["rpmPrev"] = prev;
      meta["dropRpm"] = drop;
      meta["pulses"] = decelBackfire.pulsesRemaining;
      log_push(currentRpm, false, "SYS", "BF_DECEL_ARM", &meta, nullptr);
    }
  }
  decelBackfire.prevRpm = currentRpm;
  if (decelBackfire.pulsesRemaining == 0) return;
  if (cut_is_active()) return;
  if (now < decelBackfire.nextPulseMs) return;

  unsigned long pulseMs = clampExtra(cfg.backfireDecelPulseMs);
  if (pulseMs == 0) {
    decelBackfire.pulsesRemaining = 0;
    return;
  }
  cut_trigger_dual_fast(pulseMs, 0, currentRpm);
  unsigned long spacing = cfg.backfireDecelOffMs > 0 ? static_cast<unsigned long>(cfg.backfireDecelOffMs) : 0UL;
  decelBackfire.nextPulseMs = now + pulseMs + spacing;
  if (decelBackfire.pulsesRemaining > 0) {
    decelBackfire.pulsesRemaining--;
  }
  DynamicJsonDocument meta(160);
  meta["source"] = "backfire";
  meta["profile"] = BACKFIRE_PROFILE_DECEL;
  meta["pulseMs"] = pulseMs;
  meta["remaining"] = decelBackfire.pulsesRemaining;
  log_push(currentRpm, true, "SYS", "BF_DECEL_PULSE", &meta, nullptr);
  if (decelBackfire.pulsesRemaining == 0) {
    backfireState.lastBackfireMs = now;
  }
}

void onShiftStateChanged(bool pressed) {
  lock_handle_shift(pressed);
  shiftPressed = pressed;
  if (!pressed) {
    shiftCutLatched = false;
  }
  log_push(rpm_get(), cut_is_active(), pressed ? "PRESSED" : "RELEASED", "SHIFT");
}

void onShiftPressed(){
  if (lock_is_engaged()) return;
  if (shiftCutLatched) return;
  if (cut_is_active()) return;
  int currentRpm = rpm_get();
  const QSConfig &cfg = getConfig();
  unsigned long baseCut = cfg.modeAuto ? config_resolve_cut_ms(currentRpm) : cfg.cutTimeFixed;
  unsigned long ignCut = baseCut;
  unsigned long injCut = baseCut;
  BackfireResult bf = resolve_backfire(currentRpm, cfg);
  if (bf.applied) {
    ignCut += bf.ignExtra;
    injCut += bf.injExtra;
  }
  cut_trigger_dual(ignCut, injCut, currentRpm);
  char eventLabel[64];
  const char *modeLabel = cfg.modeAuto ? "CUT_AUTO" : "CUT_MAN";
  if (cfg.cutIgnEnabled && cfg.cutInjEnabled && ignCut != injCut) {
    snprintf(eventLabel, sizeof(eventLabel), "%s IGN%lums INJ%lums", modeLabel, ignCut, injCut);
  } else {
    snprintf(eventLabel, sizeof(eventLabel), "%s %lums", modeLabel, ignCut);
  }
  DynamicJsonDocument metaDoc(192);
  JsonDocument *metaPtr = nullptr;
  if (bf.applied) {
    JsonObject meta = metaDoc.to<JsonObject>();
    meta["source"] = "backfire";
    meta["profile"] = cfg.backfireProfile;
    meta["ignExtra"] = bf.ignExtra;
    if (bf.injExtra) meta["injExtra"] = bf.injExtra;
    if (bf.burstIndex) meta["burst"] = bf.burstIndex;
    if (bf.bandIndex >= 0) meta["band"] = bf.bandIndex;
    metaPtr = &metaDoc;
  }
  log_push(currentRpm, cut_is_active(), shiftPressed ? "PRESSED" : "RELEASED", eventLabel, metaPtr, nullptr);
  shiftCutLatched = cut_is_active();
}

void setup() {
  Serial.begin(115200);
  while (!Serial) ;
  Serial.println("Quickshifter v2 starting");

  if (!LittleFS.begin(true)) Serial.println("LittleFS init failed");
  if (!ensureDefaultConfig()) {
    Serial.println("Failed to ensure default config");
  }
  if (!loadConfig()) {
    Serial.println("Failed to load config, using defaults");
  }

  log_init();
  ota_manager_init();
  ota_manager_mark_app_valid();

  pinMode(RELAY_IGN, OUTPUT);
  pinMode(RELAY_INJ, OUTPUT);
  pinMode(TEST_PWM, OUTPUT);

  const QSConfig &cfg = getConfig();

  rpm_set_ppr(cfg.ppr);
  rpm_set_scale(cfg.rpmScaleFactor);
  rpm_init(RPM_PIN);

  cut_init(RELAY_IGN, RELAY_INJ, cfg.relayPolarityHighToCut);

  shift_init(SHIFT_SW, cfg.shiftActiveLow, cfg.shiftDebounceMs, onShiftPressed);
  shift_set_state_callback(onShiftStateChanged);
  lock_init();

  web_init();

  Serial.println("Initialization complete");
}

void loop() {
  rpm_update();
  int loopRpm = rpm_get();
  handle_decel_backfire(loopRpm, getConfig());
  shift_update();
  cut_poll();
  lock_loop();
  web_handle();
  ota_manager_loop();
  delay(10);
}
