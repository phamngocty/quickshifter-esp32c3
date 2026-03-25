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

// Manual Backfire State Machine
enum ShiftState
{
  SHIFT_IDLE,
  SHIFT_QUICKCUT_ACTIVE,
  SHIFT_WAIT_RELEASE,
  SHIFT_MANUAL_MODE1,
  SHIFT_MANUAL_MODE2
};

struct ManualBackfireRuntime
{
  ShiftState state;
  unsigned long cutStartMs;
  unsigned long lastToggleMs;
  uint8_t pulseCount;
  uint8_t phase; // 0=CUT(A), 1=FIRE(B), 2=OFF(C)
};

static ManualBackfireRuntime manualBF = {SHIFT_IDLE, 0, 0, 0, 0};

struct BackfireRuntime
{
  unsigned long lastShiftMs = 0;
  unsigned long lastBackfireMs = 0;
  uint8_t burstCount = 0;
};

struct BackfireResult
{
  unsigned long ignExtra = 0;
  unsigned long injExtra = 0;
  uint8_t profileUsed = BACKFIRE_PROFILE_BASIC;
  uint8_t burstIndex = 0;
  int bandIndex = -1;
  bool applied = false;
};

static BackfireRuntime backfireState;
struct DecelBackfireState
{
  int prevRpm = 0;
  uint8_t pulsesRemaining = 0;
  unsigned long nextPulseMs = 0;
};

static DecelBackfireState decelBackfire;

static inline unsigned long clampExtra(int value)
{
  return value > 0 ? static_cast<unsigned long>(value) : 0UL;
}
static BackfireResult resolve_backfire(int rpm, const QSConfig &cfg)
{
  BackfireResult result;
  if (!cfg.backfireMode)
    return result;
  unsigned long now = millis();
  bool rpmOk = rpm >= cfg.backfireRpmThreshold;
  if (cfg.backfireProfile != BACKFIRE_PROFILE_BURST)
  {
    backfireState.burstCount = 0;
  }

  auto applyExtras = [&](unsigned long ignExtra, unsigned long injExtra, BackfireProfile profile)
  {
    if (ignExtra == 0 && injExtra == 0)
      return;
    result.ignExtra = ignExtra;
    result.injExtra = cfg.cutInjEnabled ? injExtra : 0;
    result.profileUsed = profile;
    result.applied = true;
    backfireState.lastBackfireMs = now;
  };

  auto cooldownPassed = [&](int cooldownMs)
  {
    if (cooldownMs <= 0)
      return true;
    return (now - backfireState.lastBackfireMs) >= static_cast<unsigned long>(cooldownMs);
  };

  switch (cfg.backfireProfile)
  {
  case BACKFIRE_PROFILE_STAGED:
  {
    if (!rpmOk)
      break;
    uint8_t idx = 0;
    const AutoMapEntry *entry = config_find_auto_map_entry(rpm, &idx);
    if (!entry)
      break;
    if (!entry->backfireEnabled || entry->backfireAddMs <= 0)
      break;
    if (!cooldownPassed(cfg.backfireCooldownMs))
      break;
    result.bandIndex = idx;
    unsigned long add = clampExtra(entry->backfireAddMs);
    applyExtras(add, add, BACKFIRE_PROFILE_STAGED);
  }
  break;
  case BACKFIRE_PROFILE_BURST:
  {
    if (!rpmOk)
    {
      backfireState.burstCount = 0;
      break;
    }
    if ((now - backfireState.lastShiftMs) <= static_cast<unsigned long>(cfg.backfireBurstWindowMs))
    {
      if (backfireState.burstCount < cfg.backfireBurstMax)
      {
        backfireState.burstCount++;
      }
    }
    else
    {
      backfireState.burstCount = 1;
    }
    if (backfireState.burstCount == 0)
      backfireState.burstCount = 1;
    backfireState.lastShiftMs = now;
    unsigned long add = clampExtra(cfg.backfireAddMs);
    if (backfireState.burstCount > 1)
    {
      add += clampExtra(cfg.backfireBurstExtraMs) * static_cast<unsigned long>(backfireState.burstCount - 1);
    }
    result.burstIndex = backfireState.burstCount;
    applyExtras(add, add, BACKFIRE_PROFILE_BURST);
  }
  break;
  case BACKFIRE_PROFILE_BLEND:
  {
    if (!rpmOk)
      break;
    if (!(cfg.cutIgnEnabled && cfg.cutInjEnabled))
      break;
    if (!cooldownPassed(cfg.backfireCooldownMs))
      break;
    unsigned long shared = clampExtra(cfg.backfireAddMs);
    unsigned long ignExtra = shared + clampExtra(cfg.backfireIgnExtraMs);
    applyExtras(ignExtra, shared, BACKFIRE_PROFILE_BLEND);
  }
  break;
  case BACKFIRE_PROFILE_DECEL:
  {
    // handled asynchronously via handle_decel_backfire()
  }
  break;
  case BACKFIRE_PROFILE_BASIC:
  default:
  {
    if (!rpmOk)
      break;
    if (!cooldownPassed(cfg.backfireCooldownMs))
      break;
    unsigned long add = clampExtra(cfg.backfireAddMs);
    applyExtras(add, add, BACKFIRE_PROFILE_BASIC);
  }
  break;
  }
  return result;
}

static void handle_decel_backfire(int currentRpm, const QSConfig &cfg)
{
  if (decelBackfire.prevRpm == 0)
    decelBackfire.prevRpm = currentRpm;
  if (!cfg.backfireMode || cfg.backfireProfile != BACKFIRE_PROFILE_DECEL)
  {
    decelBackfire.pulsesRemaining = 0;
    decelBackfire.prevRpm = currentRpm;
    return;
  }
  unsigned long now = millis();
  int prev = decelBackfire.prevRpm;
  int drop = prev - currentRpm;
  bool rpmHigh = prev >= cfg.backfireDecelHighRpm;
  bool dropDetected = rpmHigh && drop >= cfg.backfireDecelDropRpm;
  if (dropDetected)
  {
    bool cooldownOk = (now - backfireState.lastBackfireMs) >= static_cast<unsigned long>(cfg.backfireCooldownMs);
    if (cooldownOk)
    {
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
  if (decelBackfire.pulsesRemaining == 0)
    return;
  if (cut_is_active())
    return;
  if (now < decelBackfire.nextPulseMs)
    return;

  unsigned long pulseMs = clampExtra(cfg.backfireDecelPulseMs);
  if (pulseMs == 0)
  {
    decelBackfire.pulsesRemaining = 0;
    return;
  }
  cut_trigger_dual_fast(pulseMs, 0, currentRpm);
  unsigned long spacing = cfg.backfireDecelOffMs > 0 ? static_cast<unsigned long>(cfg.backfireDecelOffMs) : 0UL;
  decelBackfire.nextPulseMs = now + pulseMs + spacing;
  if (decelBackfire.pulsesRemaining > 0)
  {
    decelBackfire.pulsesRemaining--;
  }
  DynamicJsonDocument meta(160);
  meta["source"] = "backfire";
  meta["profile"] = BACKFIRE_PROFILE_DECEL;
  meta["pulseMs"] = pulseMs;
  meta["remaining"] = decelBackfire.pulsesRemaining;
  log_push(currentRpm, true, "SYS", "BF_DECEL_PULSE", &meta, nullptr);
  if (decelBackfire.pulsesRemaining == 0)
  {
    backfireState.lastBackfireMs = now;
  }
}

// Xử lý Manual Backfire (gọi trong loop)
static void handle_manual_backfire(int currentRpm, const QSConfig &cfg)
{
  if (!cfg.manualBackfireEnabled)
  {
    manualBF.state = SHIFT_IDLE;
    return;
  }

  unsigned long now = millis();
  unsigned long pressDuration = shift_get_press_duration();

  // Kiểm tra điều kiện kích hoạt manual backfire
  auto canActivate = [&]() -> bool
  {
    return cfg.manualBackfireEnabled && lock_is_unlocked() && currentRpm >= cfg.manualMinRpm && cfg.manualBackfireMode > 0;
  };

  // State machine
  switch (manualBF.state)
  {
  case SHIFT_IDLE:
    // Không làm gì
    break;

  case SHIFT_QUICKCUT_ACTIVE:
    // Sau khi quickshift kết thúc, chờ xem có giữ lâu không
    if (!cut_is_active() && shift_is_pressed())
    {
      manualBF.state = SHIFT_WAIT_RELEASE;
    }
    else if (!shift_is_pressed())
    {
      manualBF.state = SHIFT_IDLE;
    }
    break;

  case SHIFT_WAIT_RELEASE:
    if (!shift_is_pressed())
    {
      manualBF.state = SHIFT_IDLE;
      break;
    }

    // Kiểm tra đã giữ đủ lâu chưa
    if (pressDuration >= cfg.manualBackfireHoldMs && canActivate())
    {
      // Kích hoạt manual backfire
      if (cfg.manualBackfireMode == 1)
      {
        manualBF.state = SHIFT_MANUAL_MODE1;
      }
      else if (cfg.manualBackfireMode == 2)
      {
        manualBF.state = SHIFT_MANUAL_MODE2;
        manualBF.pulseCount = 0;
        manualBF.phase = 0; // bắt đầu ở A (CUT)
      }
      manualBF.cutStartMs = now;
      manualBF.lastToggleMs = now;

      DynamicJsonDocument meta(192);
      meta["source"] = "manual_backfire";
      meta["mode"] = cfg.manualBackfireMode;
      meta["holdMs"] = pressDuration;
      meta["rpm"] = currentRpm;
      log_push(currentRpm, false, "ACTIVATED", "MANUAL_BF_START", &meta, nullptr);
    }
    break;

  case SHIFT_MANUAL_MODE1:
    // Mode 1: Cắt liên tục cho đến khi nhả
    if (shift_is_pressed() && canActivate())
    {
      // Trigger cut liên tục
      if (cfg.manualIgnOnly)
      {
        cut_trigger_dual(999999, 0, currentRpm); // Chỉ IGN
      }
      else
      {
        cut_trigger_dual(999999, 999999, currentRpm); // Cả 2
      }
    }
    else
    {
      // Nhả hoặc không còn điều kiện
      cut_trigger_dual(0, 0, currentRpm);
      manualBF.state = SHIFT_IDLE;
    }
    break;

  case SHIFT_MANUAL_MODE2:
    // Mode 2: A (CUT dài) -> (B FIRE, C CUT ngắn) lặp D lần
    if (!shift_is_pressed() || !canActivate())
    {
      // Nhả giữa chừng hoặc mất điều kiện
      cut_trigger_dual(0, 0, currentRpm);
      manualBF.state = SHIFT_IDLE;
      DynamicJsonDocument meta(96);
      meta["source"] = "manual_backfire";
      meta["interrupted"] = true;
      log_push(currentRpm, false, "INTERRUPTED", "MANUAL_BF_MODE2_STOP", &meta, nullptr);
      break;
    }

    // Thời gian đã trôi qua trong phase hiện tại
    unsigned long elapsed = now - manualBF.lastToggleMs;

    switch (manualBF.phase)
    {
    case 0: // A: CUT dài
      if (elapsed >= (unsigned long)cfg.manualPulseCutMs)
      {
        // Kết thúc A → sang B1 (FIRE)
        cut_trigger_dual(0, 0, currentRpm);
        manualBF.phase = 1;
        manualBF.pulseCount = 0; // reset bộ đếm vòng B-C
        manualBF.lastToggleMs = now;
      }
      else
      {
        // Duy trì cắt trong A
        if (cfg.manualIgnOnly)
        {
          cut_trigger_dual_fast(cfg.manualPulseCutMs, 0, currentRpm);
        }
        else
        {
          cut_trigger_dual_fast(cfg.manualPulseCutMs, cfg.manualPulseCutMs, currentRpm);
        }
      }
      break;

    case 1: // B: FIRE (không cắt)
      if (elapsed >= (unsigned long)cfg.manualPulseFireMs)
      {
        // Sang C: CUT ngắn
        manualBF.phase = 2;
        manualBF.lastToggleMs = now;
        cut_trigger_dual(0, 0, currentRpm); // đảm bảo đang bật lửa trước khi cắt ngắn
      }
      else
      {
        cut_trigger_dual(0, 0, currentRpm);
      }
      break;

    case 2: // C: CUT ngắn sau FIRE
      if (elapsed >= (unsigned long)cfg.manualPulseOffMs)
      {
        // Hoàn tất một cặp B-C
        manualBF.pulseCount++;
        if (manualBF.pulseCount >= cfg.manualPulseRepeat)
        {
          // Hết D lần → quay lại A mới
          manualBF.phase = 0;
        }
        else
        {
          // Còn lượt → quay lại B tiếp theo
          manualBF.phase = 1;
        }
        manualBF.lastToggleMs = now;
        cut_trigger_dual(0, 0, currentRpm); // kết thúc C, tắt cắt để chuẩn bị B
      }
      else
      {
        // Duy trì cắt ngắn trong C
        if (cfg.manualIgnOnly)
        {
          cut_trigger_dual_fast(cfg.manualPulseOffMs, 0, currentRpm);
        }
        else
        {
          cut_trigger_dual_fast(cfg.manualPulseOffMs, cfg.manualPulseOffMs, currentRpm);
        }
      }
      break;
    }
    break;
  }
}

void onShiftStateChanged(bool pressed)
{
  lock_handle_shift(pressed);
  shiftPressed = pressed;
  if (!pressed)
  {
    shiftCutLatched = false;
    // Reset manual backfire state khi nhả
    if (manualBF.state != SHIFT_IDLE)
    {
      manualBF.state = SHIFT_IDLE;
      cut_trigger_dual(0, 0, rpm_get()); // Tắt relay
      DynamicJsonDocument meta(96);
      meta["source"] = "manual_backfire";
      meta["event"] = "released";
      log_push(rpm_get(), false, "RELEASED", "MANUAL_BF_END", &meta, nullptr);
    }
  }
  log_push(rpm_get(), cut_is_active(), pressed ? "PRESSED" : "RELEASED", "SHIFT");
}

void onShiftPressed()
{
  if (lock_is_engaged())
    return;
  if (shiftCutLatched)
    return;
  if (cut_is_active())
    return;

  // Trigger quickshift ngay lập tức
  int currentRpm = rpm_get();
  const QSConfig &cfg = getConfig();

  if (currentRpm < cfg.minRpm)
    return;
  unsigned long baseCut = cfg.modeAuto ? config_resolve_cut_ms(currentRpm) : cfg.cutTimeFixed;
  unsigned long ignCut = baseCut;
  unsigned long injCut = baseCut;

  // Không áp dụng backfire tự động nếu manual backfire enabled
  // (sẽ xử lý sau khi giữ lâu)
  if (!cfg.manualBackfireEnabled)
  {
    BackfireResult bf = resolve_backfire(currentRpm, cfg);
    if (bf.applied)
    {
      ignCut += bf.ignExtra;
      injCut += bf.injExtra;
    }
    DynamicJsonDocument metaDoc(192);
    JsonDocument *metaPtr = nullptr;
    if (bf.applied)
    {
      JsonObject meta = metaDoc.to<JsonObject>();
      meta["source"] = "backfire";
      meta["profile"] = cfg.backfireProfile;
      meta["ignExtra"] = bf.ignExtra;
      if (bf.injExtra)
        meta["injExtra"] = bf.injExtra;
      if (bf.burstIndex)
        meta["burst"] = bf.burstIndex;
      if (bf.bandIndex >= 0)
        meta["band"] = bf.bandIndex;
      metaPtr = &metaDoc;
    }
    char eventLabel[64];
    const char *modeLabel = cfg.modeAuto ? "CUT_AUTO" : "CUT_MAN";
    if (cfg.cutIgnEnabled && cfg.cutInjEnabled && ignCut != injCut)
    {
      snprintf(eventLabel, sizeof(eventLabel), "%s IGN%lums INJ%lums", modeLabel, ignCut, injCut);
    }
    else
    {
      snprintf(eventLabel, sizeof(eventLabel), "%s %lums", modeLabel, ignCut);
    }
    log_push(currentRpm, cut_is_active(), shiftPressed ? "PRESSED" : "RELEASED", eventLabel, metaPtr, nullptr);
  }
  else
  {
    // Manual backfire enabled - chỉ quickshift đơn giản
    char eventLabel[64];
    const char *modeLabel = cfg.modeAuto ? "CUT_AUTO" : "CUT_MAN";
    snprintf(eventLabel, sizeof(eventLabel), "%s %lums (MBF_ARMED)", modeLabel, ignCut);
    log_push(currentRpm, cut_is_active(), shiftPressed ? "PRESSED" : "RELEASED", eventLabel, nullptr, nullptr);
  }

  cut_trigger_dual(ignCut, injCut, currentRpm);
  shiftCutLatched = cut_is_active();
  manualBF.state = SHIFT_QUICKCUT_ACTIVE;
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("Quickshifter v2 starting");

  if (!LittleFS.begin(true))
    Serial.println("LittleFS init failed");
  if (!ensureDefaultConfig())
  {
    Serial.println("Failed to ensure default config");
  }
  if (!loadConfig())
  {
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
  rpm_set_offset(cfg.rpmOffset);
  rpm_init(RPM_PIN);

  cut_init(RELAY_IGN, RELAY_INJ, cfg.relayPolarityHighToCut);

  shift_init(SHIFT_SW, cfg.shiftActiveLow, cfg.shiftDebounceMs, onShiftPressed);
  shift_set_state_callback(onShiftStateChanged);
  lock_init();

  web_init();

  Serial.println("Initialization complete");
}

void loop()
{
  rpm_update();
  int loopRpm = rpm_get();
  handle_decel_backfire(loopRpm, getConfig());
  handle_manual_backfire(loopRpm, getConfig()); // Thêm xử lý manual backfire
  shift_update();
  cut_poll();
  lock_loop();
  web_handle();
  ota_manager_loop();
  delay(10);
}
