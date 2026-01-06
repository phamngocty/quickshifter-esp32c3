#include "rpm.h"
#include <Arduino.h>

// PIN will be set in init
static volatile unsigned long pulseCount = 0;
static unsigned long lastSampleMs = 0;
static int lastRpm = 0;
static float scale = 1.0f;
static float pprVal = 1.0f;

static void IRAM_ATTR rpm_isr() {
  pulseCount++;
}

void rpm_set_ppr(float ppr) {
  pprVal = ppr;
}

void rpm_set_scale(float factor) {
  if (factor <= 0.01f) factor = 0.01f;
  scale = factor;
}

float rpm_get_scale() {
  return scale;
}

void rpm_init(uint8_t pin) {
  pulseCount = 0;
  lastSampleMs = millis();
  pinMode(pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pin), rpm_isr, RISING);
}

void rpm_update() {
  unsigned long now = millis();
  if (now - lastSampleMs >= 200) { // sample every 200ms
    noInterrupts();
    unsigned long count = pulseCount;
    pulseCount = 0;
    interrupts();

    float secs = (now - lastSampleMs) / 1000.0f;
    float pulses_per_sec = count / secs;
    float rpm = pulses_per_sec * 60.0f / pprVal;
    rpm = rpm * scale;
    lastRpm = (int)rpm;
    lastSampleMs = now;
  }
}

int rpm_get() {
  return lastRpm;
}
