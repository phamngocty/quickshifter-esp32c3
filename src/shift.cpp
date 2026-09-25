#include "shift.h"

static uint8_t shiftPin;
static bool initialized = false;
static bool activeLow = true;
static int dbMs = 30;
static shift_callback_t press_cb = nullptr;
static shift_state_callback_t state_cb = nullptr;

static bool stableState = false;
static unsigned long pressStartMs = 0;  // Thời điểm nhấn xuống
static unsigned long lastSampleMs = 0;
static int8_t integrateCounter = 0;
static const unsigned long SAMPLE_INTERVAL_MS = 2; // Lấy mẫu mỗi 2ms

static void configure_pin_mode() {
  pinMode(shiftPin, activeLow ? INPUT_PULLUP : INPUT_PULLDOWN);
}

int shift_get_raw_level() {
  if (!initialized) return -1;
  return digitalRead(shiftPin);
}

static void refresh_state_from_pin(bool notify) {
  int rawLevel = digitalRead(shiftPin);
  bool rawActive = (rawLevel == (activeLow ? LOW : HIGH));
  stableState = rawActive;
  int maxCount = dbMs / (int)SAMPLE_INTERVAL_MS;
  if (maxCount < 3) maxCount = 3;
  if (maxCount > 30) maxCount = 30;
  integrateCounter = rawActive ? maxCount : 0;
  pressStartMs = stableState ? millis() : 0;
  if (notify && state_cb) {
    state_cb(stableState);
  }
}

void shift_configure(bool activeLowCfg, int debounceMs) {
  if (!initialized) return;
  activeLow = activeLowCfg;
  dbMs = debounceMs < 5 ? 5 : (debounceMs > 100 ? 100 : debounceMs);
  configure_pin_mode();
  refresh_state_from_pin(true);
}

void shift_init(uint8_t pin, bool activeLowCfg, int debounceMs, shift_callback_t callback) {
  shiftPin = pin;
  press_cb = callback;
  initialized = true;
  shift_configure(activeLowCfg, debounceMs);
}

void shift_update() {
  if (!initialized) return;
  unsigned long now = millis();
  if (now - lastSampleMs < SAMPLE_INTERVAL_MS) {
    return;
  }
  lastSampleMs = now;

  bool rawActive = (digitalRead(shiftPin) == (activeLow ? LOW : HIGH));
  int maxCount = dbMs / (int)SAMPLE_INTERVAL_MS;
  if (maxCount < 3) maxCount = 3;
  if (maxCount > 30) maxCount = 30;

  if (rawActive) {
    if (integrateCounter < maxCount) {
      integrateCounter++;
    }
  } else {
    if (integrateCounter > 0) {
      integrateCounter--;
    }
  }

  // Thuật toán tích phân mẫu lọc nhiễu dội phím & xung bugi (Glitch Filter / Hysteresis)
  if (integrateCounter >= maxCount && !stableState) {
    stableState = true;
    pressStartMs = now;
    if (state_cb) state_cb(true);
    if (press_cb) press_cb();
  } else if (integrateCounter == 0 && stableState) {
    stableState = false;
    pressStartMs = 0;
    if (state_cb) state_cb(false);
  }
}

unsigned long shift_get_press_duration() {
  if (!stableState) return 0;
  return millis() - pressStartMs;
}

void shift_set_state_callback(shift_state_callback_t cb) {
  state_cb = cb;
  if (state_cb) {
    state_cb(stableState);
  }
}

bool shift_is_pressed() {
  return stableState;
}
