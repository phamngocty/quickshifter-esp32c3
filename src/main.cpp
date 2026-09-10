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

// Double-Click tracking variables
static unsigned long lastShiftPressMs = 0;
static uint8_t shiftPressCount = 0;
static const unsigned long DOUBLE_CLICK_TIMEOUT_MS = 500;

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

// Auto RPM Limiter State Machine
enum LimiterState : uint8_t
{
  LIMITER_STANDBY = 0,
  LIMITER_CUT_STATE = 1,
  LIMITER_RECOVERY_STATE = 2
};

static LimiterState limiterState = LIMITER_STANDBY;
static unsigned long limiterStateTimerMs = 0;

static void handle_auto_rev_limiter(int current_rpm)
{
  // Phân quyền ưu tiên chân cắt lửa:
  // Nếu Lock xe, Quickshift đang đá số, hoặc Manual Backfire đang giữ cần số,
  // Limiter phải nhường quyền hoàn toàn và trở về STANDBY.
  if (lock_is_engaged() || shiftCutLatched || manualBF.state != SHIFT_IDLE)
  {
    if (limiterState != LIMITER_STANDBY)
    {
      limiterState = LIMITER_STANDBY;
    }
    return;
  }

  // Nếu tính năng bị tắt
  if (!limiter_enable)
  {
    if (limiterState != LIMITER_STANDBY)
    {
      cut_trigger_dual_fast(0, 0, current_rpm);
      limiterState = LIMITER_STANDBY;
    }
    return;
  }

  unsigned long now = millis();

  switch (limiterState)
  {
  case LIMITER_STANDBY:
    if (current_rpm >= target_rpm)
    {
      // Vòng tua vượt ngưỡng -> Xuất HIGH (cắt lửa)
      cut_trigger_dual_fast(hard_cut_ms, hard_cut_ms, current_rpm);
      limiterStateTimerMs = now;
      limiterState = LIMITER_CUT_STATE;
    }
    break;

  case LIMITER_CUT_STATE:
    if (now - limiterStateTimerMs >= hard_cut_ms)
    {
      // Hết thời gian ngắt -> Xuất LOW (trả lửa)
      cut_trigger_dual_fast(0, 0, current_rpm);
      limiterStateTimerMs = now;
      limiterState = LIMITER_RECOVERY_STATE;
    }
    break;

  case LIMITER_RECOVERY_STATE:
    if (now - limiterStateTimerMs >= recovery_spark_ms)
    {
      // Hết thời gian hồi lửa -> Kiểm tra lại RPM
      if (current_rpm > (target_rpm - rpm_hysteresis))
      {
        // Vòng tua vẫn cao -> Tiếp tục CUT
        cut_trigger_dual_fast(hard_cut_ms, hard_cut_ms, current_rpm);
        limiterStateTimerMs = now;
        limiterState = LIMITER_CUT_STATE;
      }
      else
      {
        // Vòng tua rớt an toàn -> Về STANDBY (Giữ LOW)
        cut_trigger_dual_fast(0, 0, current_rpm);
        limiterState = LIMITER_STANDBY;
      }
    }
    break;
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
  unsigned long now = millis();

  // 1. Thuật toán Double-Click Toggle Limiter
  if (allow_double_click)
  {
    if (shiftPressCount == 1 && (now - lastShiftPressMs <= DOUBLE_CLICK_TIMEOUT_MS))
    {
      limiter_enable = !limiter_enable;
      
      // Đồng bộ Web UI qua SSE
      web_send_limiter_sync(limiter_enable);

      DynamicJsonDocument meta(128);
      meta["source"] = "double_click";
      meta["limiter_enable"] = limiter_enable;
      log_push(rpm_get(), false, "DOUBLE_CLICK", limiter_enable ? "LIMITER_ENABLED" : "LIMITER_DISABLED", &meta, nullptr);

      // Reset bộ đếm click, tuyệt đối không can thiệp ngoại vi
      shiftPressCount = 0;
      lastShiftPressMs = 0;
      return;
    }
    else
    {
      shiftPressCount = 1;
      lastShiftPressMs = now;
    }
  }

  // 2. Logic Quickshifter gốc
  if (lock_is_engaged())
    return;
  if (shiftCutLatched)
    return;
  if (cut_is_active())
    return;

  int currentRpm = rpm_get();
  const QSConfig &cfg = getConfig();

  if (currentRpm < cfg.minRpm)
    return;
  unsigned long baseCut = cfg.modeAuto ? config_resolve_cut_ms(currentRpm) : cfg.cutTimeFixed;
  unsigned long ignCut = baseCut;
  unsigned long injCut = baseCut;

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
  log_push(currentRpm, cut_is_active(), shiftPressed ? "PRESSED" : "RELEASED", eventLabel, nullptr, nullptr);

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
  handle_auto_rev_limiter(loopRpm);
  handle_manual_backfire(loopRpm, getConfig());
  shift_update();
  cut_poll();
  lock_loop();
  web_handle();
  ota_manager_loop();
  delay(1);
}

