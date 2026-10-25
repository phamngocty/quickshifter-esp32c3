#pragma once
#include <Arduino.h>

struct LockStatus {
  bool enabled;
  bool unlocked;
  bool engaged;
  bool retryHold;
  uint32_t retryMs;
  bool spamHold;
  uint32_t spamMs;
  char pass[5];
  uint32_t shortMinMs;
  uint32_t shortMaxMs;
  uint32_t longMinMs;
  uint32_t resetHoldMs;
};

void lock_init();
void lock_loop();
void lock_handle_shift(bool pressed);
bool lock_is_enabled();
bool lock_is_engaged();
bool lock_is_unlocked();
void lock_get_status(LockStatus &status);
bool lock_update(bool enabled, const char *pass, int shortMaxOverride = -1, int longMinOverride = -1, int resetHoldOverride = -1);
void lock_reset_fail_window();
const char* lock_get_input_buffer();
uint8_t lock_get_buf_len();
bool lock_try_unlock(const char *pass);
