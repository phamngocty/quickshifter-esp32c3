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
#include "fs_updater.h"

// Pin map (per PROJECT OVERVIEW)
static const uint8_t RPM_PIN = 2;
static const uint8_t RELAY_IGN = 6;
static const uint8_t RELAY_INJ = 7;
static const uint8_t SHIFT_SW = 4;
static const uint8_t TEST_PWM = 8;

static bool shiftPressed = false;
static bool shiftCutLatched = false;

// Shift Lever Multi-Click tracking (3 clicks to Turn ON, 2 clicks to Turn OFF)
static unsigned long lastShiftPressMs = 0;
static uint8_t shiftPressCount = 0;
static const unsigned long MULTI_CLICK_TIMEOUT_MS = 800;

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
      DynamicJsonDocument meta(96);
      meta["targetRpm"] = target_rpm;
      meta["cutMs"] = hard_cut_ms;
      log_push(current_rpm, true, "LIMITER", "HARD_CUT_TRIGGER", &meta);
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
        log_push(current_rpm, false, "LIMITER", "HARD_CUT_RELEASE");
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
  {
    if (!shift_is_pressed())
    {
      manualBF.state = SHIFT_IDLE;
      break;
    }

    // Kiểm tra đã giữ đủ lâu chưa (tối thiểu 250ms)
    unsigned long safeHoldMs = cfg.manualBackfireHoldMs < 250 ? 250 : cfg.manualBackfireHoldMs;
    if (pressDuration >= safeHoldMs && canActivate())
    {
      // Kích hoạt manual backfire
      if (cfg.manualBackfireMode == 1)
      {
        manualBF.state = SHIFT_MANUAL_MODE1;
        if (cfg.manualIgnOnly)
        {
          cut_trigger_dual_fast(999999, 0, currentRpm); // Cắt liên tục đến khi nhả
        }
        else
        {
          cut_trigger_dual_fast(999999, 999999, currentRpm); // Cắt liên tục đến khi nhả
        }
      }
      else if (cfg.manualBackfireMode == 2)
      {
        manualBF.state = SHIFT_MANUAL_MODE2;
        manualBF.pulseCount = 0;
        manualBF.phase = 0; // bắt đầu ở A (CUT)
        if (cfg.manualIgnOnly)
        {
          cut_trigger_dual_fast(cfg.manualPulseCutMs, 0, currentRpm);
        }
        else
        {
          cut_trigger_dual_fast(cfg.manualPulseCutMs, cfg.manualPulseCutMs, currentRpm);
        }
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
  }

  case SHIFT_MANUAL_MODE1:
  {
    // Mode 1: Cắt liên tục cho đến khi nhả cần số (không giới hạn timeout)
    if (!shift_is_pressed() || !cfg.manualBackfireEnabled || lock_is_engaged())
    {
      // Nhả cần số: trả lửa ngay lập tức
      cut_trigger_dual_fast(0, 0, currentRpm);
      manualBF.state = SHIFT_IDLE;
      manualBF.phase = 0;
      manualBF.pulseCount = 0;
    }
    break;
  }

  case SHIFT_MANUAL_MODE2:
  {
    // Mode 2: A (CUT dài) -> (B FIRE, C CUT ngắn) lặp D lần
    // Failsafe: Giới hạn tối đa 3 giây để chống tắt máy và chống kẹt cảm biến
    bool timeoutReached = (now - manualBF.cutStartMs >= 3000UL);
    if (!shift_is_pressed() || !cfg.manualBackfireEnabled || lock_is_engaged() || (currentRpm > 0 && currentRpm < cfg.minRpm) || timeoutReached)
    {
      // Nhả cần số hoặc chạm ngưỡng garanti an toàn hoặc hết 3s an toàn
      cut_trigger_dual_fast(0, 0, currentRpm);
      manualBF.state = SHIFT_IDLE;
      manualBF.phase = 0;
      manualBF.pulseCount = 0;
      DynamicJsonDocument meta(96);
      meta["source"] = "manual_backfire";
      meta["interrupted"] = true;
      if (timeoutReached) meta["reason"] = "timeout_failsafe";
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
      break;

    case 1: // B: FIRE (không cắt)
      if (elapsed >= (unsigned long)cfg.manualPulseFireMs)
      {
        // Sang C: CUT ngắn
        manualBF.phase = 2;
        manualBF.lastToggleMs = now;
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

    case 2: // C: CUT ngắn sau FIRE
      if (elapsed >= (unsigned long)cfg.manualPulseOffMs)
      {
        // Hoàn tất một cặp B-C
        manualBF.pulseCount++;
        manualBF.lastToggleMs = now;
        if (manualBF.pulseCount >= cfg.manualPulseRepeat)
        {
          // Hết D lần → quay lại A mới
          manualBF.phase = 0;
          if (cfg.manualIgnOnly)
          {
            cut_trigger_dual_fast(cfg.manualPulseCutMs, 0, currentRpm);
          }
          else
          {
            cut_trigger_dual_fast(cfg.manualPulseCutMs, cfg.manualPulseCutMs, currentRpm);
          }
        }
        else
        {
          // Còn lượt → quay lại B tiếp theo (FIRE)
          manualBF.phase = 1;
          cut_trigger_dual(0, 0, currentRpm);
        }
      }
      break;
    }
    break;
  }

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
      manualBF.phase = 0;
      manualBF.pulseCount = 0;
      cut_trigger_dual_fast(0, 0, rpm_get()); // Tắt relay
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
  int currentRpm = rpm_get();
  const QSConfig &cfg = getConfig();

  // Đánh thức hoặc gia hạn thời gian mở Wi-Fi AP nếu người dùng thao tác cần số khi dừng xe
  if (currentRpm <= cfg.minRpm)
  {
    web_wake_ap();
  }

  // 1. Thuật toán điều khiển Auto RPM Limiter qua cần số:
  // - BẬT: Nhấp 3 cái liên tiếp (Triple-Click) khi Limiter đang TẮT
  // - TẮT: Nhấp 2 cái liên tiếp (Double-Click) khi Limiter đang BẬT
  // Nhận lệnh ở mọi dải tua máy (không ràng buộc điều kiện Min RPM).
  if (allow_double_click)
  {
    if (now - lastShiftPressMs <= MULTI_CLICK_TIMEOUT_MS)
    {
      shiftPressCount++;
    }
    else
    {
      shiftPressCount = 1;
    }
    lastShiftPressMs = now;

    if (limiter_enable)
    {
      // Đang BẬT -> Cần nhấp 2 cái để TẮT
      if (shiftPressCount >= 2)
      {
        limiter_enable = false;
        getMutableConfig().limiter_enable = false;

        // Đồng bộ Web UI qua SSE
        web_send_limiter_sync(false);

        DynamicJsonDocument meta(128);
        meta["source"] = "double_click";
        meta["limiter_enable"] = false;
        log_push(currentRpm, false, "DOUBLE_CLICK", "LIMITER_DISABLED", &meta, nullptr);

        shiftPressCount = 0;
        lastShiftPressMs = 0;
        return; // Đã nhận lệnh TẮT thành công -> Thoát
      }
    }
    else
    {
      // Đang TẮT -> Cần nhấp 3 cái để BẬT
      if (shiftPressCount >= 3)
      {
        limiter_enable = true;
        getMutableConfig().limiter_enable = true;

        // Đồng bộ Web UI qua SSE
        web_send_limiter_sync(true);

        DynamicJsonDocument meta(128);
        meta["source"] = "triple_click";
        meta["limiter_enable"] = true;
        log_push(currentRpm, false, "TRIPLE_CLICK", "LIMITER_ENABLED", &meta, nullptr);

        shiftPressCount = 0;
        lastShiftPressMs = 0;
        return; // Đã nhận lệnh BẬT thành công -> Thoát
      }
    }

    // Nếu tua máy dưới minRpm, chặn không cut nhầm vì tua thấp
    if (currentRpm < cfg.minRpm)
    {
      return;
    }
    // Nếu tua máy >= minRpm và chưa kích hoạt đủ số nhấp (1 nhấp hoặc 2 nhấp khi đang tắt),
    // cho phép rơi xuống logic Quickshifter để sang số bình thường!
  }

  // 2. Logic Quickshifter gốc
  if (ota_manager_is_active() || fs_updater_is_active())
  {
    log_push(currentRpm, false, "QS_BLOCKED", "OTA_ACTIVE");
    return;
  }
  if (lock_is_engaged())
  {
    log_push(currentRpm, false, "QS_BLOCKED", "MORSE_LOCK_ENGAGED");
    return;
  }
  if (shiftCutLatched)
  {
    log_push(currentRpm, false, "QS_BLOCKED", "SHIFT_LATCHED");
    return;
  }

  // Nếu Auto Rev Limiter đang cắt lửa (limiterState != LIMITER_STANDBY),
  // Quickshifter có quyền ưu tiên tuyệt đối (Preemption) ngắt chu kỳ Limiter để thực hiện cắt số.
  if (limiterState != LIMITER_STANDBY)
  {
    limiterState = LIMITER_STANDBY;
    log_push(currentRpm, false, "LIMITER", "PREEMPTED_BY_QS");
  }
  else if (cut_is_active())
  {
    // Nếu cut đang chạy từ lần đá số trước thì bỏ qua để chống double trigger
    log_push(currentRpm, false, "QS_BLOCKED", "CUT_IN_PROGRESS");
    return;
  }

  if (currentRpm < cfg.minRpm)
  {
    DynamicJsonDocument meta(96);
    meta["minRpm"] = cfg.minRpm;
    log_push(currentRpm, false, "QS_BLOCKED", "RPM_BELOW_MIN", &meta);
    return;
  }
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
  limiter_enable = false; // Đảm bảo luôn tắt Auto RPM Limiter khi khởi động

  log_init();
  ota_manager_init();
  ota_manager_mark_app_valid();

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

  DynamicJsonDocument bootMeta(128);
  bootMeta["freeHeap"] = ESP.getFreeHeap();
  bootMeta["resetReason"] = (int)esp_reset_reason();
  log_push(0, false, "SYS", "SYS_BOOT_OK", &bootMeta, nullptr);

  Serial.println("Initialization complete");
}

void loop()
{
  if (ota_manager_is_active() || fs_updater_is_active())
  {
    cut_poll();
    web_handle();
    ota_manager_loop();
    delay(1);
    return;
  }

  rpm_update();
  int loopRpm = rpm_get();
  handle_auto_rev_limiter(loopRpm);
  handle_manual_backfire(loopRpm, getConfig());
  shift_update();
  if (shiftCutLatched && shift_get_press_duration() > 3000UL)
  {
    shiftCutLatched = false; // Failsafe: Giải phóng cờ chốt nếu cần số bị giữ quá 3s
  }
  if (shiftPressCount > 0 && (millis() - lastShiftPressMs > MULTI_CLICK_TIMEOUT_MS))
  {
    shiftPressCount = 0;
  }
  cut_poll();
  lock_loop();
  web_handle();
  ota_manager_loop();
  delay(2);
}

