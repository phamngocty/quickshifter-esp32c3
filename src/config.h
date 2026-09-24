#pragma once
#include <Arduino.h>

static const size_t MAX_AUTO_MAP = 12;

struct AutoMapEntry {
  int rpmMin;
  int rpmMax;
  int cutMs;
};

// --- Giao diện Web UI & Auto RPM Limiter Global Variables ---
extern bool allow_double_click; 
extern bool limiter_enable;     

// --- Thông số thuật toán Hard Cut ---
extern int target_rpm;          // V.d: 7000
extern int rpm_hysteresis;      // V.d: 200 (Thoát khi rớt dưới 6800)
extern unsigned long hard_cut_ms;       // Thời gian ngắt lửa (V.d: 30ms)
extern unsigned long recovery_spark_ms; // Thời gian trả lửa (V.d: 20ms)

struct QSConfig {
  bool shiftActiveLow;
  int shiftDebounceMs;

  bool relayPolarityHighToCut;

  float ppr;
  float rpmScaleFactor;
  float rpmOffset;

  bool modeAuto;
  int cutTimeFixed;
  uint8_t autoMapCount;
  AutoMapEntry autoMap[MAX_AUTO_MAP];

  // Auto RPM Limiter Config
  bool allow_double_click;
  bool limiter_enable;
  int target_rpm;
  int rpm_hysteresis;
  unsigned long hard_cut_ms;
  unsigned long recovery_spark_ms;

  int apTimeoutMs;
  char apPassword[65];
  int minRpm;

  bool cutIgnEnabled;
  bool cutInjEnabled;

  // Manual Backfire Config
  bool manualBackfireEnabled;
  uint32_t manualBackfireHoldMs;
  uint8_t manualBackfireMode;       // 0=Off, 1=Continuous, 2=Pulse
  uint32_t manualPulseCutMs;        // A: thời gian ngắt (tích xăng)
  uint32_t manualPulseFireMs;       // B: thời gian bật lại (đốt)
  uint32_t manualPulseOffMs;        // C: thời gian ngắt tiếp
  uint8_t manualPulseRepeat;        // D: số lần lặp chu kỳ
  int manualMinRpm;                 // RPM tối thiểu để kích
  bool manualIgnOnly;               // Chỉ cắt IGN (không cắt INJ)

  // Soft Restore (Hồi lửa êm dịu chống giật xích tải)
  bool softRestoreEnabled;
  uint8_t softRestoreDampMs;        // Thời gian ngắt giảm chấn (5-25ms)
};

const QSConfig &getConfig();
QSConfig &getMutableConfig();
bool loadConfig();
bool saveConfig();
bool ensureDefaultConfig();
int config_resolve_cut_ms(int rpm);
void config_normalize_auto_map();
const AutoMapEntry *config_find_auto_map_entry(int rpm, uint8_t *outIndex = nullptr);

