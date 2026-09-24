#include "shift.h"

static uint8_t shiftPin;
static bool initialized = false;
static bool activeLow = true;
static int dbMs = 30;
static shift_callback_t press_cb = nullptr;
static shift_state_callback_t state_cb = nullptr;

static bool stableState = false;
static bool lastRawState = false;
static unsigned long lastChangeMs = 0;
static unsigned long pressStartMs = 0;  // Thời điểm nhấn xuống

static void configure_pin_mode() {
  pinMode(shiftPin, activeLow ? INPUT_PULLUP : INPUT_PULLDOWN);
}

static void refresh_state_from_pin(bool notify) {
  bool raw = digitalRead(shiftPin) == (activeLow ? LOW : HIGH);
  stableState = raw;
  lastRawState = raw;
  lastChangeMs = millis();
  if (stableState) {
    pressStartMs = millis();
  } else {
    pressStartMs = 0;
  }
  if (notify && state_cb) {
    state_cb(stableState);
  }
}

void shift_configure(bool activeLowCfg, int debounceMs) {
  if (!initialized) return;
  activeLow = activeLowCfg;
  dbMs = debounceMs;
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
  bool raw = digitalRead(shiftPin) == (activeLow ? LOW : HIGH);
  unsigned long now = millis();

  if (raw != lastRawState) {
    lastRawState = raw;
    lastChangeMs = now;
  }

  if (now - lastChangeMs >= (unsigned long)dbMs) {
    if (raw != stableState) {
      stableState = raw;
      if (stableState) {
        pressStartMs = now;  // Lưu thời điểm nhấn
      } else {
        pressStartMs = 0;
      }
      if (state_cb) state_cb(stableState);
      if (stableState && press_cb) {
        press_cb();
      }
    }
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
