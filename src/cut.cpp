#include "cut.h"
#include "config.h"
#include <Arduino.h>

static uint8_t pinIgn, pinInj;
static bool polHighToCut = true;

static unsigned long ignEndMs = 0;
static unsigned long injEndMs = 0;
static unsigned long lastCutMs = 0;
static bool ignActive = false;
static bool injActive = false;
static bool lockForced = false;
static const unsigned long CUT_MIN_INTERVAL_MS = 200;

static bool lastIgnCut = false;
static bool lastInjCut = false;
static bool outputStateInitialized = false;

static void refresh_outputs();

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
}

bool cut_is_active()
{
  if (lockForced)
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

static bool cut_trigger_internal(unsigned long ignMs, unsigned long injMs, int currentRpm, bool bypassGuard)
{
  (void)currentRpm;
  if (lockForced)
    return false;
  unsigned long now = millis();
  // Allow immediate OFF commands (0ms) to pass even within guard window
  if (!bypassGuard && now - lastCutMs < CUT_MIN_INTERVAL_MS && (ignMs > 0 || injMs > 0))
    return false;

  const QSConfig &cfg = getConfig();
  if (!cfg.cutIgnEnabled && !cfg.cutInjEnabled)
    return false;

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
  return true;
}

void cut_trigger_dual(unsigned long ignMs, unsigned long injMs, int currentRpm)
{
  cut_trigger_internal(ignMs, injMs, currentRpm, false);
}

void cut_trigger_dual_fast(unsigned long ignMs, unsigned long injMs, int currentRpm)
{
  cut_trigger_internal(ignMs, injMs, currentRpm, true);
}

void cut_trigger_ms(unsigned long ms, int currentRpm)
{
  cut_trigger_dual(ms, ms, currentRpm);
}

// to be called in loop to clear outputs
void cut_poll()
{
  if (lockForced)
  {
    refresh_outputs();
    return;
  }
  unsigned long now = millis();
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

void cut_force_lock(bool locked)
{
  lockForced = locked;
  refresh_outputs();
}
