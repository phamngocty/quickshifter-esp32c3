#include "rpm.h"
#include <Arduino.h>

static volatile uint32_t pulseCount = 0;
static volatile uint32_t acceptedPulseCount = 0;
static volatile uint32_t lastEdgeUs = 0;
static volatile uint32_t lastValidPeriodUs = 0;

static unsigned long lastUpdateMs = 0;
static int lastRpm = 0;
static float scale = 1.0f;
static float pprVal = 1.0f;

static const uint32_t ISR_DEBOUNCE_US = 4000;
static const uint32_t SIGNAL_TIMEOUT_US = 300000;
static const uint32_t UPDATE_INTERVAL_MS = 20;
static const uint8_t RPM_MA_SIZE = 6;

static float rpmBuffer[RPM_MA_SIZE] = {0.0f};
static uint8_t rpmBufferIndex = 0;
static uint8_t rpmBufferCount = 0;
static uint32_t lastAcceptedSnapshot = 0;

static inline void rpm_filter_reset() {
  for (uint8_t i = 0; i < RPM_MA_SIZE; ++i) {
    rpmBuffer[i] = 0.0f;
  }
  rpmBufferIndex = 0;
  rpmBufferCount = 0;
}

static inline void rpm_filter_push(float rpm) {
  rpmBuffer[rpmBufferIndex] = rpm;
  rpmBufferIndex = (rpmBufferIndex + 1) % RPM_MA_SIZE;
  if (rpmBufferCount < RPM_MA_SIZE) {
    rpmBufferCount++;
  }
}

static inline float rpm_filter_average() {
  if (rpmBufferCount == 0) {
    return 0.0f;
  }
  float sum = 0.0f;
  for (uint8_t i = 0; i < rpmBufferCount; ++i) {
    sum += rpmBuffer[i];
  }
  return sum / rpmBufferCount;
}

static void IRAM_ATTR rpm_isr() {
  const uint32_t nowUs = static_cast<uint32_t>(micros());
  const uint32_t prevUs = lastEdgeUs;

  if (prevUs != 0) {
    const uint32_t deltaUs = nowUs - prevUs;
    if (deltaUs < ISR_DEBOUNCE_US) {
      return;
    }
    lastValidPeriodUs = deltaUs;
  }

  lastEdgeUs = nowUs;
  pulseCount++;
  acceptedPulseCount++;
}

void rpm_set_ppr(float ppr) {
  if (ppr <= 0.01f) {
    ppr = 0.01f;
  }
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
  noInterrupts();
  pulseCount = 0;
  acceptedPulseCount = 0;
  lastEdgeUs = 0;
  lastValidPeriodUs = 0;
  interrupts();

  lastUpdateMs = millis();
  lastAcceptedSnapshot = 0;
  lastRpm = 0;
  rpm_filter_reset();

  pinMode(pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pin), rpm_isr, RISING);
}

void rpm_update() {
  const unsigned long nowMs = millis();
  if ((nowMs - lastUpdateMs) < UPDATE_INTERVAL_MS) {
    return;
  }
  lastUpdateMs = nowMs;

  uint32_t periodUsSnapshot = 0;
  uint32_t acceptedSnapshot = 0;
  uint32_t lastEdgeSnapshot = 0;

  noInterrupts();
  periodUsSnapshot = lastValidPeriodUs;
  acceptedSnapshot = acceptedPulseCount;
  lastEdgeSnapshot = lastEdgeUs;
  interrupts();

  if (acceptedSnapshot != lastAcceptedSnapshot && periodUsSnapshot >= ISR_DEBOUNCE_US) {
    float rawRpm = (60000000.0f / (periodUsSnapshot * pprVal)) * scale;
    if (rawRpm < 0.0f) {
      rawRpm = 0.0f;
    }
    rpm_filter_push(rawRpm);
    lastAcceptedSnapshot = acceptedSnapshot;
  }

  const uint32_t nowUs = static_cast<uint32_t>(micros());
  if (lastEdgeSnapshot == 0 || (nowUs - lastEdgeSnapshot) > SIGNAL_TIMEOUT_US) {
    rpm_filter_reset();
    lastRpm = 0;
    return;
  }

  lastRpm = static_cast<int>(rpm_filter_average() + 0.5f);
}

int rpm_get() {
  return lastRpm;
}
