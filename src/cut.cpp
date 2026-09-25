#include "cut.h"
#include "config.h"
#include <Arduino.h>
#include <esp_timer.h>

static uint8_t pinIgn = 0;
static uint8_t pinInj = 0;
static bool polHighToCut = true;

static unsigned long ignEndMs = 0;
static unsigned long injEndMs = 0;
static unsigned long lastCutMs = 0;
static volatile bool ignActive = false;
static volatile bool injActive = false;
static bool lockForced = false;
static const unsigned long CUT_MIN_INTERVAL_MS = 200;

static bool lastIgnCut = false;
static bool lastInjCut = false;
static bool outputStateInitialized = false;

// --- Soft Restore & Hardware Timer Engine ---
enum SoftRestorePhase : uint8_t {
  RESTORE_IDLE = 0,
  RESTORE_MAIN_CUT = 1,
  RESTORE_INITIAL_SPARK = 2,
  RESTORE_DAMPED_CUT = 3
};

static volatile SoftRestorePhase restorePhase = RESTORE_IDLE;
static esp_timer_handle_t cut_timer = nullptr;
static uint32_t currentDampMs = 10;

static void refresh_outputs();

static void IRAM_ATTR on_cut_timer(void *arg)
{
  (void)arg;
  if (lockForced) return;

  const QSConfig &cfg = getConfig();

  switch (restorePhase)
  {
  case RESTORE_MAIN_CUT:
    // Kết thúc cắt chính:
    if (cfg.softRestoreEnabled && cfg.cutIgnEnabled && currentDampMs > 0)
    {
      // Pha 2 (Initial Spark Restore): Trả lửa nhịp 1 trong 12ms để đốt hòa khí mồi
      ignActive = false;
      injActive = false;
      refresh_outputs();
      restorePhase = RESTORE_INITIAL_SPARK;
      esp_timer_start_once(cut_timer, 12000ULL); // 12ms = 12,000us
    }
    else
    {
      // Không dùng Soft Restore -> Trả lửa/xăng hoàn toàn ngay lập tức
      ignActive = false;
      injActive = false;
      refresh_outputs();
      restorePhase = RESTORE_IDLE;
    }
    break;

  case RESTORE_INITIAL_SPARK:
    // Pha 3 (Soft Damping Cut): Ngắt nhẹ một nhịp phụ để triệt tiêu đỉnh mô-men giật xích
    ignActive = true;  // Chỉ ngắt lửa nhẹ, không ngắt xăng
    injActive = false;
    refresh_outputs();
    restorePhase = RESTORE_DAMPED_CUT;
    esp_timer_start_once(cut_timer, (uint64_t)currentDampMs * 1000ULL);
    break;

  case RESTORE_DAMPED_CUT:
  default:
    // Pha 4: Hoàn tất toàn bộ chu trình hồi lửa -> Trả lửa 100%
    ignActive = false;
    injActive = false;
    refresh_outputs();
    restorePhase = RESTORE_IDLE;
    break;
  }
}

void cut_init(uint8_t ignPin, uint8_t injPin, bool polarityHighToCut)
{
  pinIgn = ignPin;
  pinInj = injPin;
  polHighToCut = polarityHighToCut;
  // Set safe non-cut level BEFORE pinMode to prevent boot glitches
  digitalWrite(pinIgn, polHighToCut ? LOW : HIGH);
  digitalWrite(pinInj, polHighToCut ? LOW : HIGH);
  pinMode(pinIgn, OUTPUT);
  pinMode(pinInj, OUTPUT);
  outputStateInitialized = false;
  refresh_outputs();

  // Khởi tạo Hardware Timer độ chính xác micro-giây
  if (!cut_timer)
  {
    const esp_timer_create_args_t timer_args = {
        .callback = &on_cut_timer,
        .arg = nullptr,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "qs_cut_tmr",
        .skip_unhandled_events = true
    };
    esp_timer_create(&timer_args, &cut_timer);
  }
}

bool cut_is_active()
{
  if (lockForced)
    return true;
  if (restorePhase != RESTORE_IDLE)
    return true;
  unsigned long now = millis();
  bool ign = ignActive && ((long)(now - ignEndMs) < 0);
  bool inj = injActive && ((long)(now - injEndMs) < 0);
  return ign || inj;
}

static void drivePin(uint8_t pin, bool allowControl, bool cutOn)
{
  if (!allowControl)
  {
    digitalWrite(pin, polHighToCut ? LOW : HIGH);
    return;
  }
  digitalWrite(pin, cutOn ? (polHighToCut ? HIGH : LOW) : (polHighToCut ? LOW : HIGH));
}

static void refresh_outputs()
{
  const QSConfig &cfg = getConfig();
  bool ignCut = lockForced || (cfg.cutIgnEnabled && ignActive);
  bool injCut = lockForced || (cfg.cutInjEnabled && injActive);
  if (!outputStateInitialized || ignCut != lastIgnCut)
  {
    drivePin(pinIgn, cfg.cutIgnEnabled || lockForced, ignCut);
    lastIgnCut = ignCut;
  }
  if (!outputStateInitialized || injCut != lastInjCut)
  {
    drivePin(pinInj, cfg.cutInjEnabled || lockForced, injCut);
    lastInjCut = injCut;
  }
  outputStateInitialized = true;
}

static bool cut_trigger_internal(unsigned long ignMs, unsigned long injMs, int currentRpm, bool bypassGuard, bool enableSoftRestore)
{
  (void)currentRpm;
  if (lockForced)
    return false;
  unsigned long now = millis();

  // Tắt cưỡng bức (Immediate OFF) khi truyền 0ms
  if (ignMs == 0 && injMs == 0)
  {
    if (cut_timer)
    {
      esp_timer_stop(cut_timer);
    }
    restorePhase = RESTORE_IDLE;
    ignActive = false;
    injActive = false;
    ignEndMs = now;
    injEndMs = now;
    refresh_outputs();
    return true;
  }

  // Allow immediate OFF commands (0ms) to pass even within guard window
  if (!bypassGuard && now - lastCutMs < CUT_MIN_INTERVAL_MS && (ignMs > 0 || injMs > 0))
    return false;

  const QSConfig &cfg = getConfig();
  if (!cfg.cutIgnEnabled && !cfg.cutInjEnabled)
    return false;

  // Dừng timer đang chạy trước đó (nếu có)
  if (cut_timer)
  {
    esp_timer_stop(cut_timer);
  }

  if (cfg.cutIgnEnabled && ignMs > 0)
  {
    ignActive = true;
    ignEndMs = now + ignMs;
  }
  else
  {
    ignActive = false;
    ignEndMs = now;
  }

  if (cfg.cutInjEnabled && injMs > 0)
  {
    injActive = true;
    injEndMs = now + injMs;
  }
  else
  {
    injActive = false;
    injEndMs = now;
  }

  if (!bypassGuard && (ignMs > 0 || injMs > 0))
  {
    lastCutMs = now;
  }
  refresh_outputs();

  // Thiết lập Hardware Timer & Soft Restore
  if (cut_timer && ignMs > 0)
  {
    if (enableSoftRestore && cfg.softRestoreEnabled)
    {
      currentDampMs = cfg.softRestoreDampMs;
      restorePhase = RESTORE_MAIN_CUT;
    }
    else
    {
      restorePhase = RESTORE_IDLE;
    }
    esp_timer_start_once(cut_timer, (uint64_t)ignMs * 1000ULL);
  }
  else
  {
    restorePhase = RESTORE_IDLE;
  }

  return true;
}

// Dùng cho Quickshifter thông thường: Có áp dụng Soft Restore chống giật xích
void cut_trigger_dual(unsigned long ignMs, unsigned long injMs, int currentRpm)
{
  cut_trigger_internal(ignMs, injMs, currentRpm, false, true);
}

// Dùng cho Auto RPM Limiter / Popcorn: Không áp dụng Soft Restore để băm nhịp đanh gọn
void cut_trigger_dual_fast(unsigned long ignMs, unsigned long injMs, int currentRpm)
{
  cut_trigger_internal(ignMs, injMs, currentRpm, true, false);
}

void cut_trigger_ms(unsigned long ms, int currentRpm)
{
  cut_trigger_dual(ms, ms, currentRpm);
}

// Giám sát Fail-safe trong loop() phòng ngừa sự cố
void cut_poll()
{
  if (lockForced)
  {
    refresh_outputs();
    return;
  }
  unsigned long now = millis();

  // Quản lý hoàn tất ngắt lửa/kim phun khi ở trạng thái RESTORE_IDLE
  if (restorePhase == RESTORE_IDLE)
  {
    bool changed = false;
    if (ignActive && (long)(now - ignEndMs) >= 0)
    {
      ignActive = false;
      changed = true;
    }
    if (injActive && (long)(now - injEndMs) >= 0)
    {
      injActive = false;
      changed = true;
    }
    if (changed)
    {
      refresh_outputs();
    }
  }
}

void cut_force_lock(bool locked)
{
  lockForced = locked;
  if (lockForced && cut_timer)
  {
    esp_timer_stop(cut_timer);
    restorePhase = RESTORE_IDLE;
  }
  refresh_outputs();
}
