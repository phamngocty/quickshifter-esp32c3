#include "lock.h"
#include "cut.h"
#include "log.h"
#include <Preferences.h>
#include <ArduinoJson.h>

static Preferences prefs;
static bool prefsReady = false;

static bool lockEnabled = false;
static bool unlocked = true;
static char lockPass[5] = "1001";

static bool pressActive = false;
static unsigned long pressStart = 0;
static unsigned long retryReadyMs = 0;
static unsigned long spamReleaseMs = 0;
static uint8_t failCount = 0;
static unsigned long failWindowStart = 0;

static const unsigned long SHORT_MIN = 50;
static unsigned long shortMaxMs = 300;
static unsigned long longMinMs = 500;
static unsigned long resetHoldMs = 2000;
static const unsigned long SHORT_MAX_MIN = 120;
static const unsigned long SHORT_MAX_MAX = 600;
static const unsigned long LONG_MIN_MIN = 350;
static const unsigned long LONG_MIN_MAX = 2000;
static const unsigned long RESET_MIN = 1000;
static const unsigned long RESET_MAX = 6000;
static const unsigned long RETRY_DELAY = 2000;
static const unsigned long SPAM_WINDOW = 60000;
static const uint8_t SPAM_LIMIT = 10;
static const unsigned long SPAM_LOCKOUT = 30000;

static char inputBuffer[5] = "";
static uint8_t bufLen = 0;

static void sanitize_timing() {
  if (shortMaxMs < SHORT_MAX_MIN) shortMaxMs = SHORT_MAX_MIN;
  if (shortMaxMs > SHORT_MAX_MAX) shortMaxMs = SHORT_MAX_MAX;
  if (longMinMs < LONG_MIN_MIN) longMinMs = LONG_MIN_MIN;
  if (longMinMs > LONG_MIN_MAX) longMinMs = LONG_MIN_MAX;
  if (longMinMs <= shortMaxMs + 50) {
    longMinMs = shortMaxMs + 120;
    if (longMinMs > LONG_MIN_MAX) longMinMs = LONG_MIN_MAX;
  }
  if (resetHoldMs < RESET_MIN) resetHoldMs = RESET_MIN;
  if (resetHoldMs > RESET_MAX) resetHoldMs = RESET_MAX;
  if (resetHoldMs <= longMinMs + 100) {
    resetHoldMs = longMinMs + 300;
    if (resetHoldMs > RESET_MAX) resetHoldMs = RESET_MAX;
  }
}

static void persist() {
  if (!prefsReady) return;
  prefs.putBool("enabled", lockEnabled);
  prefs.putString("pass", lockPass);
  prefs.putUInt("smax", shortMaxMs);
  prefs.putUInt("lmin", longMinMs);
  prefs.putUInt("rhold", resetHoldMs);
}

static void reset_input(bool fullReset) {
  bufLen = 0;
  inputBuffer[0] = '\0';
  pressActive = false;
  pressStart = 0;
  if (fullReset) {
    retryReadyMs = millis() + RETRY_DELAY;
  }
}

static void reset_fail_counter() {
  failCount = 0;
  failWindowStart = millis();
  spamReleaseMs = 0;
}

static void log_event(const char *label, const char *note = nullptr, const JsonDocument *extra = nullptr) {
  if (!label) label = "LOCK";
  DynamicJsonDocument meta(384);
  meta["lockEnabled"] = lockEnabled;
  meta["note"] = note ? note : "";
  if (extra) {
    JsonObjectConst obj = extra->as<JsonObjectConst>();
    for (JsonPairConst kv : obj) {
      meta[kv.key()] = kv.value();
    }
  }
  log_push(0, false, "LOCK", label, &meta);
}

static void set_lock_state(bool enabled, bool persistChange = true, const char *noteOverride = nullptr) {
  lockEnabled = enabled;
  unlocked = !lockEnabled;
  cut_force_lock(lockEnabled);
  const bool shouldPersist = persistChange && prefsReady;
  const char *label = lockEnabled ? "LOCK_ON" : (persistChange ? "LOCK_OFF" : "LOCK_OFF_TMP");
  const char *note = noteOverride;
  if (!note) {
    if (lockEnabled) {
      note = "relay forced";
    } else if (persistChange) {
      note = "relay released";
    } else {
      note = "session release";
    }
  }
  log_event(label, note);
  if (shouldPersist) {
    persist();
  }
}

static bool validate_pass(const char *pass) {
  if (!pass) return false;
  size_t len = strlen(pass);
  if (len != 4) return false;
  for (size_t i = 0; i < len; ++i) {
    if (pass[i] != '0' && pass[i] != '1') return false;
  }
  return true;
}

bool lock_update(bool enabled, const char *pass, int shortMaxOverride, int longMinOverride, int resetHoldOverride) {
  if (pass && *pass) {
    if (!validate_pass(pass)) return false;
    strncpy(lockPass, pass, sizeof(lockPass));
    lockPass[4] = '\0';
  }
  bool timingChanged = false;
  if (shortMaxOverride >= 0) {
    shortMaxMs = (unsigned long)shortMaxOverride;
    timingChanged = true;
  }
  if (longMinOverride >= 0) {
    longMinMs = (unsigned long)longMinOverride;
    timingChanged = true;
  }
  if (resetHoldOverride >= 0) {
    resetHoldMs = (unsigned long)resetHoldOverride;
    timingChanged = true;
  }
  sanitize_timing();
  bool stateChanged = (enabled != lockEnabled);
  if (stateChanged) {
    set_lock_state(enabled, true);
  } else if (timingChanged || (pass && *pass)) {
    persist();
  }
  if (timingChanged) {
    log_event("TIMING_SET", nullptr);
  }
  reset_input(false);
  return true;
}

void lock_init() {
  if (!prefsReady) {
    prefsReady = prefs.begin("lock", false);
  }
  if (prefsReady) {
    lockEnabled = prefs.getBool("enabled", false);
    String saved = prefs.getString("pass", "1001");
    saved.toCharArray(lockPass, sizeof(lockPass));
    shortMaxMs = prefs.getUInt("smax", shortMaxMs);
    longMinMs = prefs.getUInt("lmin", longMinMs);
    resetHoldMs = prefs.getUInt("rhold", resetHoldMs);
  }
  if (!validate_pass(lockPass)) {
    strncpy(lockPass, "1001", sizeof(lockPass));
    lockPass[4] = '\0';
  }
  sanitize_timing();
  unlocked = !lockEnabled;
  reset_fail_counter();
  reset_input(false);
  if (lockEnabled) {
    cut_force_lock(true);
  }
}

void lock_loop() {
  if (failWindowStart == 0) {
    failWindowStart = millis();
  }
  if (failCount && (millis() - failWindowStart) > SPAM_WINDOW) {
    reset_fail_counter();
  }
  if (spamReleaseMs && millis() >= spamReleaseMs) {
    spamReleaseMs = 0;
  }
}

bool lock_is_enabled() { return lockEnabled; }
bool lock_is_unlocked() { return unlocked; }
bool lock_is_engaged() { return lockEnabled && !unlocked; }

static bool spam_block_active() {
  return spamReleaseMs && millis() < spamReleaseMs;
}

static void log_pass_buffer(const char *label, char bit, unsigned long holdMs) {
  if (!label) label = "PASS_BUF";
  DynamicJsonDocument extra(256);
  extra["buffer"] = inputBuffer;
  extra["bufLen"] = bufLen;
  if (bit == '0' || bit == '1') extra["bit"] = String(bit);
  if (holdMs) extra["holdMs"] = holdMs;
  log_event(label, inputBuffer, &extra);
}

static void log_pass_reset(const char *buffer, uint8_t len, unsigned long holdMs) {
  DynamicJsonDocument extra(256);
  const char *bufferView = buffer ? buffer : "";
  extra["buffer"] = bufferView;
  extra["bufLen"] = len;
  if (holdMs) extra["holdMs"] = holdMs;
  extra["thresholdMs"] = resetHoldMs;
  log_event("PASS_RESET", bufferView, &extra);
}

void lock_handle_shift(bool pressed) {
  if (!lock_is_engaged()) return;
  if (spam_block_active()) return;
  unsigned long now = millis();
  if (pressed) {
    if (now < retryReadyMs) {
      return;
    }
    if (!pressActive) {
      pressActive = true;
      pressStart = now;
    }
    return;
  }

  if (!pressActive) return;
  pressActive = false;
  unsigned long held = now - pressStart;
  pressStart = 0;
  if (held >= resetHoldMs) {
    char snapshot[5];
    strncpy(snapshot, inputBuffer, sizeof(snapshot));
    snapshot[4] = '\0';
    uint8_t snapshotLen = bufLen;
    log_pass_reset(snapshot, snapshotLen, held);
    reset_input(false);
    return;
  }
  char bit = '\0';
  if (held >= SHORT_MIN && held <= shortMaxMs) {
    bit = '0';
  } else if (held >= longMinMs) {
    bit = '1';
  } else {
    reset_input(true);
    return;
  }
  if (bufLen < 4) {
    inputBuffer[bufLen++] = bit;
    inputBuffer[bufLen] = '\0';
    log_pass_buffer("PASS_BUF", bit, held);
  }
  if (bufLen < 4) return;
  bool success = (strncmp(inputBuffer, lockPass, 4) == 0);
  if (success) {
    log_event("PASS_OK", inputBuffer);
    set_lock_state(false, false, "PASS unlock (volatile)");
    reset_fail_counter();
    reset_input(false);
    return;
  }
  log_event("PASS_FAIL", inputBuffer);
  bufLen = 0;
  inputBuffer[0] = '\0';
  retryReadyMs = now + RETRY_DELAY;
  if (failWindowStart == 0) failWindowStart = now;
  if ((now - failWindowStart) > SPAM_WINDOW) {
    failCount = 0;
    failWindowStart = now;
  }
  failCount++;
  if (failCount >= SPAM_LIMIT) {
    spamReleaseMs = now + SPAM_LOCKOUT;
    log_event("LOCKOUT", "30s timeout");
    failCount = 0;
    failWindowStart = now;
  }
}

void lock_get_status(LockStatus &status) {
  status.enabled = lockEnabled;
  status.unlocked = unlocked;
  status.engaged = lock_is_engaged();
  status.retryHold = millis() < retryReadyMs;
  status.retryMs = status.retryHold ? (retryReadyMs - millis()) : 0;
  status.spamHold = spam_block_active();
  status.spamMs = status.spamHold ? (spamReleaseMs - millis()) : 0;
  strncpy(status.pass, lockPass, sizeof(status.pass));
  status.pass[4] = '\0';
  status.shortMinMs = SHORT_MIN;
  status.shortMaxMs = shortMaxMs;
  status.longMinMs = longMinMs;
  status.resetHoldMs = resetHoldMs;
}

void lock_reset_fail_window() {
  reset_fail_counter();
  reset_input(false);
}
